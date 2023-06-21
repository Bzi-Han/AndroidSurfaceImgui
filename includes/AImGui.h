#ifndef A_IMGUI_H // !A_IMGUI_H
#define A_IMGUI_H

#include "Global.h"
#include "ANativeWindowCreator.h"
#include "ATouchEvent.h"

#include <imgui/imgui.h>
#include <imgui/backends/imgui_impl_android.h>
#include <imgui/backends/imgui_impl_opengl3.h>

#include <EGL/egl.h>
#include <GLES3/gl3.h>

namespace android
{
    class AImGui
    {
    public:
        AImGui(bool autoUpdateOrientation = false);
        ~AImGui();

        void BeginFrame();
        void EndFrame();

        void ProcessInputEvent();

        constexpr operator bool() const
        {
            return m_state;
        }

    private:
        bool InitEnvironment();
        void UnInitEnvironment();

    private:
        bool m_state = false;
        bool m_autoUpdateOrientation;

        int m_rotateTheta = 0;
        int m_screenWidth = -1, m_screenHeight = -1;

        ANativeWindow *m_nativeWindow = nullptr;
        EGLDisplay m_defaultDisplay = EGL_NO_DISPLAY;
        EGLSurface m_eglSurface = EGL_NO_SURFACE;
        EGLContext m_eglContext = EGL_NO_CONTEXT;
        ImGuiContext *m_imguiContext = nullptr;
    };
}

#endif // !A_IMGUI_H