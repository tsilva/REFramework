#pragma once

#include "VRRuntime.hpp"

#include <chrono>

namespace runtimes {
struct OpenVR final : public VRRuntime {
    OpenVR() {
        this->custom_stage = SynchronizeStage::EARLY;
    }

    virtual ~OpenVR() {
        this->destroy();
    }

    std::string_view name() const override {
        return "OpenVR";
    }

    VRRuntime::Type type() const override { 
        return VRRuntime::Type::OPENVR;
    }

    bool ready() const override {
        return VRRuntime::ready() && this->is_hmd_active && this->got_first_poses;
    }

    bool is_startup_pause_grace_period() const {
        using namespace std::chrono_literals;
        return !this->got_first_poses || std::chrono::steady_clock::now() - this->initialized_at < 5s;
    }

    VRRuntime::Error synchronize_frame() override;
    VRRuntime::Error update_poses() override;
    VRRuntime::Error update_render_target_size() override;

    uint32_t get_width() const override;
    uint32_t get_height() const override;

    VRRuntime::Error consume_events(std::function<void(void*)> callback) override;
    VRRuntime::Error update_matrices(float nearz, float farz) override;

    void destroy() override;

    bool is_hmd_active{false};
    bool was_hmd_active{true};
    std::chrono::steady_clock::time_point initialized_at{std::chrono::steady_clock::now()};

    uint32_t w{0};
    uint32_t h{0};

    vr::IVRSystem* hmd{nullptr};

    std::array<vr::TrackedDevicePose_t, vr::k_unMaxTrackedDeviceCount> real_render_poses;
    std::array<vr::TrackedDevicePose_t, vr::k_unMaxTrackedDeviceCount> real_game_poses;

    std::array<vr::TrackedDevicePose_t, vr::k_unMaxTrackedDeviceCount> render_poses;
    std::array<vr::TrackedDevicePose_t, vr::k_unMaxTrackedDeviceCount> game_poses;
};
}
