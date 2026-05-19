#include <algorithm>
#include <cstring>

#include <imgui.h>
#include <imgui_internal.h>
#include <openvr.h>
#include <d3dcompiler.h>

#include "../VR.hpp"

#include "D3D11Component.hpp"

#pragma comment(lib, "d3dcompiler")

#ifdef VERBOSE_D3D11
#define LOG_VERBOSE(...) spdlog::info(__VA_ARGS__)
#else
#define LOG_VERBOSE
#endif

namespace vrmod {
vr::EVRCompositorError D3D11Component::on_frame(VR* vr) {
    if (m_left_eye_tex == nullptr) {
        setup();
    }

    auto& hook = g_framework->get_d3d11_hook();

    // get device
    auto device = hook->get_device();

    // Get the context.
    ComPtr<ID3D11DeviceContext> context{};

    device->GetImmediateContext(&context);

    // get swapchain
    auto swapchain = hook->get_swap_chain();

    // get back buffer
    ComPtr<ID3D11Texture2D> backbuffer{};

    swapchain->GetBuffer(0, IID_PPV_ARGS(&backbuffer));

    if (backbuffer == nullptr) {
        spdlog::error("[VR] Failed to get back buffer.");
        return vr::VRCompositorError_None;
    }

    draw_comfort_vignette(vr, backbuffer.Get());

    auto runtime = vr->get_runtime();

    // If m_frame_count is even, we're rendering the left eye.
    if (vr->m_render_frame_count % 2 == vr->m_left_eye_interval) {
        if (runtime->is_openxr() && runtime->ready()) {
            LOG_VERBOSE("Copying left eye");
            m_openxr.copy(0, backbuffer.Get());
        }

        if (runtime->is_openvr()) {
            // Copy the back buffer to the left eye texture (m_left_eye_tex0 holds the intermediate frame).
            context->CopyResource(m_left_eye_tex.Get(), backbuffer.Get());
            vr::Texture_t left_eye{(void*)m_left_eye_tex.Get(), vr::TextureType_DirectX, vr::ColorSpace_Auto};

            auto e = vr::VRCompositor()->Submit(vr::Eye_Left, &left_eye, &vr->m_left_bounds);

            bool submitted = true;

            if (e != vr::VRCompositorError_None) {
                spdlog::error("[VR] VRCompositor failed to submit left eye: {}", (int)e);
                vr->m_submitted = false;
                return e;
            }
        }
    } else {
        if (runtime->ready()) {
            if (runtime->is_openxr()) {
                LOG_VERBOSE("Copying right eye");
                m_openxr.copy(1, backbuffer.Get());
            }

            if (runtime->get_synchronize_stage() == VRRuntime::SynchronizeStage::VERY_LATE || !runtime->got_first_sync) {
                runtime->synchronize_frame();

                if (!runtime->got_first_poses) {
                    runtime->update_poses();
                }
            }
        }

        if (runtime->is_openxr() && vr->m_openxr->ready()) {
            if (runtime->get_synchronize_stage() == VRRuntime::SynchronizeStage::VERY_LATE || !vr->m_openxr->frame_began) {
                LOG_VERBOSE("Beginning frame.");
                vr->m_openxr->begin_frame();
            }

            LOG_VERBOSE("Ending frame");
            auto result = vr->m_openxr->end_frame();

            vr->m_openxr->needs_pose_update = true;
            vr->m_submitted = result == XR_SUCCESS;
        }

        if (runtime->is_openvr()) {
            // Copy the back buffer to the right eye texture.
            context->CopyResource(m_right_eye_tex.Get(), backbuffer.Get());
            vr::Texture_t right_eye{(void*)m_right_eye_tex.Get(), vr::TextureType_DirectX, vr::ColorSpace_Auto};

            auto e = vr::VRCompositor()->Submit(vr::Eye_Right, &right_eye, &vr->m_right_bounds);

            bool submitted = true;

            if (e != vr::VRCompositorError_None) {
                spdlog::error("[VR] VRCompositor failed to submit right eye: {}", (int)e);
                vr->m_submitted = false;
                return e;
            }

            vr->m_submitted = true;
        }

        if (runtime->ready() && vr->m_desktop_fix->value()) {
            hook->ignore_next_present();
        }
    }

    return vr::VRCompositorError_None;
}

void D3D11Component::on_reset(VR* vr) {
    m_left_eye_tex.Reset();
    m_right_eye_tex.Reset();
    m_left_eye_depthstencil.Reset();
    m_right_eye_depthstencil.Reset();
    m_vignette_vertex_shader.Reset();
    m_vignette_pixel_shader.Reset();
    m_vignette_constants.Reset();
    m_vignette_blend_state.Reset();

    if (vr->get_runtime()->is_openxr() && vr->get_runtime()->loaded) {
        if (m_openxr.last_resolution[0] != vr->get_hmd_width() || m_openxr.last_resolution[1] != vr->get_hmd_height()) {
            m_openxr.create_swapchains();
        }
    }
}

void D3D11Component::setup() {
    // Get device and swapchain.
    auto& hook = g_framework->get_d3d11_hook();
    auto device = hook->get_device();
    auto swapchain = hook->get_swap_chain();

    // Get back buffer.
    ComPtr<ID3D11Texture2D> backbuffer{};

    swapchain->GetBuffer(0, IID_PPV_ARGS(&backbuffer));

    // Get backbuffer description.
    D3D11_TEXTURE2D_DESC backbuffer_desc{};

    backbuffer->GetDesc(&backbuffer_desc);

    backbuffer_desc.BindFlags |= D3D11_BIND_RENDER_TARGET;

    // Create eye textures.
    device->CreateTexture2D(&backbuffer_desc, nullptr, &m_left_eye_tex);
    device->CreateTexture2D(&backbuffer_desc, nullptr, &m_right_eye_tex);

    // copy backbuffer into right eye
    // Get the context.
    ComPtr<ID3D11DeviceContext> context{};

    device->GetImmediateContext(&context);
    context->CopyResource(m_right_eye_tex.Get(), backbuffer.Get());

    // Make depth stencils for both eyes.
    auto depthstencil = hook->get_last_depthstencil_used();

    if (depthstencil != nullptr) {
        D3D11_TEXTURE2D_DESC depthstencil_desc{};

        depthstencil->GetDesc(&depthstencil_desc);

        // Create eye depthstencils.
        device->CreateTexture2D(&depthstencil_desc, nullptr, &m_left_eye_depthstencil);
        device->CreateTexture2D(&depthstencil_desc, nullptr, &m_right_eye_depthstencil);

        // Copy the current depthstencil into the right eye.
        context->CopyResource(m_right_eye_depthstencil.Get(), depthstencil.Get());
    }

    spdlog::info("[VR] d3d11 textures have been setup");
}

bool D3D11Component::setup_comfort_vignette() {
    if (m_vignette_vertex_shader != nullptr && m_vignette_pixel_shader != nullptr && m_vignette_constants != nullptr && m_vignette_blend_state != nullptr) {
        return true;
    }

    auto& hook = g_framework->get_d3d11_hook();
    auto device = hook->get_device();

    static const char* vertex_shader = R"(
        struct VSOut {
            float4 pos : SV_Position;
            float2 uv : TEXCOORD0;
        };

        VSOut main(uint id : SV_VertexID) {
            float2 positions[3] = {
                float2(-1.0, -1.0),
                float2(-1.0,  3.0),
                float2( 3.0, -1.0)
            };

            float2 uvs[3] = {
                float2(0.0, 1.0),
                float2(0.0, -1.0),
                float2(2.0, 1.0)
            };

            VSOut output;
            output.pos = float4(positions[id], 0.0, 1.0);
            output.uv = uvs[id];
            return output;
        }
    )";

    static const char* pixel_shader = R"(
        cbuffer VignetteConstants : register(b0) {
            float amount;
            float inner_width;
            float outer_width;
            float aspect;
            float outer_side;
            float padding0;
            float padding1;
            float padding2;
        };

        float4 main(float4 pos : SV_Position, float2 uv : TEXCOORD0) : SV_Target {
            float side_distance = outer_side < 0.0 ? uv.x : (1.0 - uv.x);
            float vertical_distance = min(uv.y, 1.0 - uv.y);

            float side_edge = 1.0 - smoothstep(inner_width, outer_width, side_distance);
            float vertical_edge = 1.0 - smoothstep(inner_width, outer_width, vertical_distance);
            float edge = max(side_edge, vertical_edge);

            return float4(0.0, 0.0, 0.0, saturate(edge * amount));
        }
    )";

    ComPtr<ID3DBlob> vs_blob{};
    ComPtr<ID3DBlob> ps_blob{};
    ComPtr<ID3DBlob> errors{};

    if (FAILED(D3DCompile(vertex_shader, strlen(vertex_shader), nullptr, nullptr, nullptr, "main", "vs_4_0", 0, 0, &vs_blob, &errors))) {
        spdlog::error("[VR] Failed to compile comfort vignette vertex shader");
        return false;
    }

    errors.Reset();

    if (FAILED(D3DCompile(pixel_shader, strlen(pixel_shader), nullptr, nullptr, nullptr, "main", "ps_4_0", 0, 0, &ps_blob, &errors))) {
        spdlog::error("[VR] Failed to compile comfort vignette pixel shader");
        return false;
    }

    if (FAILED(device->CreateVertexShader(vs_blob->GetBufferPointer(), vs_blob->GetBufferSize(), nullptr, &m_vignette_vertex_shader))) {
        spdlog::error("[VR] Failed to create comfort vignette vertex shader");
        return false;
    }

    if (FAILED(device->CreatePixelShader(ps_blob->GetBufferPointer(), ps_blob->GetBufferSize(), nullptr, &m_vignette_pixel_shader))) {
        spdlog::error("[VR] Failed to create comfort vignette pixel shader");
        return false;
    }

    D3D11_BUFFER_DESC buffer_desc{};
    buffer_desc.ByteWidth = sizeof(float) * 8;
    buffer_desc.Usage = D3D11_USAGE_DYNAMIC;
    buffer_desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    buffer_desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

    if (FAILED(device->CreateBuffer(&buffer_desc, nullptr, &m_vignette_constants))) {
        spdlog::error("[VR] Failed to create comfort vignette constant buffer");
        return false;
    }

    D3D11_BLEND_DESC blend_desc{};
    blend_desc.RenderTarget[0].BlendEnable = TRUE;
    blend_desc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
    blend_desc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
    blend_desc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
    blend_desc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
    blend_desc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_INV_SRC_ALPHA;
    blend_desc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
    blend_desc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

    if (FAILED(device->CreateBlendState(&blend_desc, &m_vignette_blend_state))) {
        spdlog::error("[VR] Failed to create comfort vignette blend state");
        return false;
    }

    return true;
}

void D3D11Component::draw_comfort_vignette(VR* vr, ID3D11Texture2D* backbuffer) {
    if (backbuffer == nullptr || !vr->m_comfort_vignette->value() || vr->m_comfort_vignette_amount <= 0.01f) {
        return;
    }

    if (!setup_comfort_vignette()) {
        return;
    }

    auto& hook = g_framework->get_d3d11_hook();
    auto device = hook->get_device();

    ComPtr<ID3D11DeviceContext> context{};
    device->GetImmediateContext(&context);

    D3D11_TEXTURE2D_DESC backbuffer_desc{};
    backbuffer->GetDesc(&backbuffer_desc);

    ComPtr<ID3D11RenderTargetView> rtv{};

    if (FAILED(device->CreateRenderTargetView(backbuffer, nullptr, &rtv))) {
        return;
    }

    const auto amount = std::clamp(vr->m_comfort_vignette_amount * vr->m_comfort_vignette_strength->value(), 0.0f, 1.0f);
    const auto begin_angle = std::clamp(vr->m_comfort_vignette_begin_angle->value(), 1.0f, 89.0f);
    const auto end_angle = std::clamp(std::max(vr->m_comfort_vignette_end_angle->value(), begin_angle + 1.0f), 2.0f, 89.0f);
    const auto max_inner_width = std::clamp((begin_angle / 89.0f) * 0.5f, 0.01f, 0.45f);
    const auto max_outer_width = std::clamp((end_angle / 89.0f) * 0.5f, max_inner_width + 0.01f, 0.5f);
    const auto inner_width = std::max(max_inner_width * amount, 0.001f);
    const auto outer_width = std::max(max_outer_width * amount, inner_width + 0.001f);
    const auto is_left_eye = vr->m_render_frame_count % 2 == vr->m_left_eye_interval;

    const float constants[8]{
        amount,
        inner_width,
        outer_width,
        (float)backbuffer_desc.Width / (float)std::max(backbuffer_desc.Height, 1u),
        is_left_eye ? -1.0f : 1.0f,
        0.0f,
        0.0f,
        0.0f,
    };

    D3D11_MAPPED_SUBRESOURCE mapped{};

    if (FAILED(context->Map(m_vignette_constants.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped))) {
        return;
    }

    memcpy(mapped.pData, constants, sizeof(constants));
    context->Unmap(m_vignette_constants.Get(), 0);

    D3D11_VIEWPORT viewport{};
    viewport.Width = (float)backbuffer_desc.Width;
    viewport.Height = (float)backbuffer_desc.Height;
    viewport.MinDepth = 0.0f;
    viewport.MaxDepth = 1.0f;

    const float blend_factor[4]{0.0f, 0.0f, 0.0f, 0.0f};

    context->OMSetRenderTargets(1, rtv.GetAddressOf(), nullptr);
    context->OMSetBlendState(m_vignette_blend_state.Get(), blend_factor, 0xffffffff);
    context->RSSetViewports(1, &viewport);
    context->IASetInputLayout(nullptr);
    context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    context->VSSetShader(m_vignette_vertex_shader.Get(), nullptr, 0);
    context->PSSetShader(m_vignette_pixel_shader.Get(), nullptr, 0);
    context->PSSetConstantBuffers(0, 1, m_vignette_constants.GetAddressOf());
    context->Draw(3, 0);
}

void D3D11Component::OpenXR::initialize(XrSessionCreateInfo& session_info) {
    std::scoped_lock _{this->mtx};

    auto& hook = g_framework->get_d3d11_hook();

    auto device = hook->get_device();

    this->binding.device = device;

    PFN_xrGetD3D11GraphicsRequirementsKHR fn = nullptr;
    xrGetInstanceProcAddr(VR::get()->m_openxr->instance, "xrGetD3D11GraphicsRequirementsKHR", (PFN_xrVoidFunction*)(&fn));

    if (fn == nullptr) {
        spdlog::error("[VR] xrGetD3D11GraphicsRequirementsKHR not found");
        return;
    }

    // get existing adapter from device
    ComPtr<IDXGIDevice> dxgi_device{};
    
    if (FAILED(device->QueryInterface(IID_PPV_ARGS(&dxgi_device)))) {
        spdlog::error("[VR] failed to get DXGI device from D3D11 device");
        return;
    }
    
    ComPtr<IDXGIAdapter> adapter{};

    if (FAILED(dxgi_device->GetAdapter(&adapter))) {
        spdlog::error("[VR] failed to get DXGI adapter from DXGI device");
        return;
    }

    DXGI_ADAPTER_DESC desc{};

    if (FAILED(adapter->GetDesc(&desc))) {
        spdlog::error("[VR] failed to get DXGI adapter description");
        return;
    }
    
    XrGraphicsRequirementsD3D11KHR gr{XR_TYPE_GRAPHICS_REQUIREMENTS_D3D11_KHR};
    gr.adapterLuid = desc.AdapterLuid;
    gr.minFeatureLevel = D3D_FEATURE_LEVEL_11_0;

    fn(VR::get()->m_openxr->instance, VR::get()->m_openxr->system, &gr);

    session_info.next = &this->binding;
}

std::optional<std::string> D3D11Component::OpenXR::create_swapchains() {
    std::scoped_lock _{this->mtx};

    spdlog::info("[VR] Creating OpenXR swapchains for D3D11");

    this->destroy_swapchains();

    auto& hook = g_framework->get_d3d11_hook();
    auto device = hook->get_device();
    auto swapchain = hook->get_swap_chain();

    // Get back buffer.
    ComPtr<ID3D11Texture2D> backbuffer{};

    swapchain->GetBuffer(0, IID_PPV_ARGS(&backbuffer));

    if (backbuffer == nullptr) {
        spdlog::error("[VR] Failed to get back buffer.");
        return "Failed to get back buffer.";
    }

    // Get backbuffer description.
    D3D11_TEXTURE2D_DESC backbuffer_desc{};
    backbuffer->GetDesc(&backbuffer_desc);

    backbuffer_desc.BindFlags |= D3D11_BIND_RENDER_TARGET;

    auto vr = VR::get();
    auto& openxr = *vr->m_openxr;

    this->contexts.clear();
    this->contexts.resize(openxr.views.size());

    // Create eye textures.
    for (auto i = 0; i < openxr.views.size(); ++i) {
        const auto& vp = openxr.view_configs[i];

        spdlog::info("[VR] Creating swapchain for eye {}", i);
        spdlog::info("[VR] Width: {}", vr->get_hmd_width());
        spdlog::info("[VR] Height: {}", vr->get_hmd_height());

        backbuffer_desc.Width = vr->get_hmd_width();
        backbuffer_desc.Height = vr->get_hmd_height();

        XrSwapchainCreateInfo swapchain_create_info{XR_TYPE_SWAPCHAIN_CREATE_INFO};
        swapchain_create_info.arraySize = 1;
        swapchain_create_info.format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
        swapchain_create_info.width = backbuffer_desc.Width;
        swapchain_create_info.height = backbuffer_desc.Height;
        swapchain_create_info.mipCount = 1;
        swapchain_create_info.faceCount = 1;
        swapchain_create_info.sampleCount = backbuffer_desc.SampleDesc.Count;
        swapchain_create_info.usageFlags = XR_SWAPCHAIN_USAGE_SAMPLED_BIT | XR_SWAPCHAIN_USAGE_COLOR_ATTACHMENT_BIT;

        runtimes::OpenXR::Swapchain swapchain{};
        swapchain.width = swapchain_create_info.width;
        swapchain.height = swapchain_create_info.height;

        if (xrCreateSwapchain(openxr.session, &swapchain_create_info, &swapchain.handle) != XR_SUCCESS) {
            spdlog::error("[VR] D3D11: Failed to create swapchain.");
            return "Failed to create swapchain.";
        }

        vr->m_openxr->swapchains.push_back(swapchain);

        uint32_t image_count{};
        auto result = xrEnumerateSwapchainImages(swapchain.handle, 0, &image_count, nullptr);

        if (result != XR_SUCCESS) {
            spdlog::error("[VR] Failed to enumerate swapchain images.");
            return "Failed to enumerate swapchain images.";
        }

        auto& ctx = this->contexts[i];

        ctx.textures.clear();
        ctx.textures.resize(image_count);

        for (uint32_t j = 0; j < image_count; ++j) {
            spdlog::info("[VR] Creating swapchain image {} for swapchain {}", j, i);

            ctx.textures[j] = {XR_TYPE_SWAPCHAIN_IMAGE_D3D11_KHR};

            if (FAILED(device->CreateTexture2D(&backbuffer_desc, nullptr, &ctx.textures[j].texture))) {
                spdlog::error("[VR] Failed to create swapchain texture {} {}", i, j);
                return "Failed to create swapchain texture.";
            }

            // get immediate context
            ComPtr<ID3D11DeviceContext> context{};
            device->GetImmediateContext(&context);
            context->CopyResource(ctx.textures[j].texture, backbuffer.Get());
        }

        result = xrEnumerateSwapchainImages(swapchain.handle, image_count, &image_count, (XrSwapchainImageBaseHeader*)&ctx.textures[0]);

        if (result != XR_SUCCESS) {
            spdlog::error("[VR] Failed to enumerate swapchain images after texture creation.");
            return "Failed to enumerate swapchain images after texture creation.";
        }
    }

    this->last_resolution = {vr->get_hmd_width(), vr->get_hmd_height()};

    spdlog::info("[VR] Successfully created OpenXR swapchains for D3D11");

    return std::nullopt;
}

void D3D11Component::OpenXR::destroy_swapchains() {
    std::scoped_lock _{this->mtx};

	if (this->contexts.empty()) {
        return;
    }

    spdlog::info("[VR] Destroying swapchains.");

    for (auto i = 0; i < this->contexts.size(); ++i) {
        auto& ctx = this->contexts[i];

        auto result = xrDestroySwapchain(VR::get()->m_openxr->swapchains[i].handle);

        if (result != XR_SUCCESS) {
            spdlog::error("[VR] Failed to destroy swapchain {}.", i);
        } else {
            spdlog::info("[VR] Destroyed swapchain {}.", i);
        }

        for (auto& tex : ctx.textures) {
            tex.texture->Release();
        }
        
        ctx.textures.clear();
    }

    this->contexts.clear();
    VR::get()->m_openxr->swapchains.clear();
}

void D3D11Component::OpenXR::copy(uint32_t swapchain_idx, ID3D11Texture2D* resource) {
    std::scoped_lock _{this->mtx};

    auto vr = VR::get();

    if (vr->m_openxr->frame_state.shouldRender != XR_TRUE) {
        return;
    }

    if (!vr->m_openxr->frame_began) {
        spdlog::error("[VR] OpenXR: Frame not begun when trying to copy.");
        //return;
    }

    if (this->contexts[swapchain_idx].num_textures_acquired > 0) {
        spdlog::info("[VR] Already acquired textures for swapchain {}?", swapchain_idx);
    }

    auto device = g_framework->get_d3d11_hook()->get_device();
    
    // get immediate context
    ComPtr<ID3D11DeviceContext> context;
    device->GetImmediateContext(&context);

    const auto& swapchain = vr->m_openxr->swapchains[swapchain_idx];
    auto& ctx = this->contexts[swapchain_idx];

    XrSwapchainImageAcquireInfo acquire_info{XR_TYPE_SWAPCHAIN_IMAGE_ACQUIRE_INFO};

    uint32_t texture_index{};
    LOG_VERBOSE("Acquiring swapchain image for {}", swapchain_idx);
    auto result = xrAcquireSwapchainImage(swapchain.handle, &acquire_info, &texture_index);

    if (result != XR_SUCCESS) {
        spdlog::error("[VR] xrAcquireSwapchainImage failed: {}", vr->m_openxr->get_result_string(result));
    } else {
        ctx.num_textures_acquired++;

        XrSwapchainImageWaitInfo wait_info{XR_TYPE_SWAPCHAIN_IMAGE_WAIT_INFO};
        //wait_info.timeout = std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::seconds(1)).count();
        wait_info.timeout = XR_INFINITE_DURATION;

        LOG_VERBOSE("Waiting on swapchain image for {}", swapchain_idx);
        result = xrWaitSwapchainImage(swapchain.handle, &wait_info);

        if (result != XR_SUCCESS) {
            spdlog::error("[VR] xrWaitSwapchainImage failed: {}", vr->m_openxr->get_result_string(result));
        } else {
            LOG_VERBOSE("Copying swapchain image {} for {}", texture_index, swapchain_idx);
            context->CopyResource(ctx.textures[texture_index].texture, resource);

            XrSwapchainImageReleaseInfo release_info{XR_TYPE_SWAPCHAIN_IMAGE_RELEASE_INFO};

            LOG_VERBOSE("Releasing swapchain image for {}", swapchain_idx);
            auto result = xrReleaseSwapchainImage(swapchain.handle, &release_info);

            if (result != XR_SUCCESS) {
                spdlog::error("[VR] xrReleaseSwapchainImage failed: {}", vr->m_openxr->get_result_string(result));
                return;
            }

            ctx.num_textures_acquired--;
        }
    }
}
} // namespace vrmod
