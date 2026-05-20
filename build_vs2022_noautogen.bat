@echo off
setlocal
cd /d "%~dp0"
powershell -NoProfile -ExecutionPolicy Bypass -File "%~dp0dev\build-dev.ps1" -SkipCmkrGeneration %*
