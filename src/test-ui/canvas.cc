#include "Global.h"
#include "AImGui.h"

#include <thread>
#include <iostream>

int main()
{
    android::AImGui imgui(android::AImGui::Options{.renderType = android::AImGui::RenderType::RenderServer, .exchangeFontData = true});

    if (!imgui)
    {
        LogInfo("[-] ImGui initialization failed");
        return 0;
    }

    std::thread processInputEventThread(
        [&]
        {
            while (imgui)
            {
                imgui.ProcessInputEvent();
                std::this_thread::sleep_for(std::chrono::microseconds(1));
            }
        });

    while (imgui)
    {
        imgui.BeginFrame();
        imgui.EndFrame();
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }

    if (processInputEventThread.joinable())
        processInputEventThread.join();

    return 0;
}