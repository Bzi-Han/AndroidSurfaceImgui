#include "Global.h"
#include "AImGui.h"

#include <thread>
#include <iostream>

int main()
{
    android::AImGui imgui(android::AImGui::Options{.renderType = android::AImGui::RenderType::RenderNative, .autoUpdateOrientation = true});
    bool state = true, showDemoWindow = false, showAnotherWindow = false;
    ImVec4 clearColor(0.45f, 0.55f, 0.60f, 1.00f);

    if (!imgui)
    {
        LogInfo("[-] ImGui initialization failed");
        return 0;
    }

    std::thread processInputEventThread(
        [&]
        {
            while (state)
            {
                imgui.ProcessInputEvent();
                std::this_thread::sleep_for(std::chrono::microseconds(1));
            }
        });

    while (state)
    {
        imgui.BeginFrame();

        // 1. Show the big demo window (Most of the sample code is in ImGui::ShowDemoWindow()! You can browse its code to learn more about Dear ImGui!).
        if (showDemoWindow)
            ImGui::ShowDemoWindow(&showDemoWindow);

        // 2. Show a simple window that we create ourselves. We use a Begin/End pair to create a named window.
        {
            static float f = 0.0f;
            static int counter = 0;

            ImGui::Begin("Hello, world!", &state); // Create a window called "Hello, world!" and append into it.

            ImGui::Text("This is some useful text.");        // Display some text (you can use a format strings too)
            ImGui::Checkbox("Demo Window", &showDemoWindow); // Edit bools storing our window open/close state
            ImGui::Checkbox("Another Window", &showAnotherWindow);

            ImGui::SliderFloat("float", &f, 0.0f, 1.0f);            // Edit 1 float using a slider from 0.0f to 1.0f
            ImGui::ColorEdit3("clear color", (float *)&clearColor); // Edit 3 floats representing a color

            if (ImGui::Button("Button")) // Buttons return true when clicked (most widgets return true when edited/activated)
                counter++;
            ImGui::SameLine();
            ImGui::Text("counter = %d", counter);

            ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
            ImGui::End();
        }

        // 3. Show another simple window.
        if (showAnotherWindow)
        {
            ImGui::Begin("Another Window", &showAnotherWindow); // Pass a pointer to our bool variable (the window will have a closing button that will clear the bool when clicked)
            ImGui::Text("Hello from another window!");
            if (ImGui::Button("Close Me"))
                showAnotherWindow = false;
            ImGui::End();
        }

        imgui.EndFrame();
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }

    if (processInputEventThread.joinable())
        processInputEventThread.join();

    return 0;
}