#ifndef A_IMGUI_H // !A_IMGUI_H
#define A_IMGUI_H

#include <imgui/imgui.h>
#include <imgui/backends/imgui_impl_android.h>
#include <imgui/backends/imgui_impl_opengl3.h>

#include <EGL/egl.h>
#include <GLES3/gl3.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <android/keycodes.h>

#include <thread>
#include <memory>
#include <vector>
#include <atomic>
#include <mutex>
#include <condition_variable>

namespace android
{
    class AImGui
    {
    public:
        enum class RenderType
        {
            RenderNative,
            RenderServer,
            RenderClient,
        };

        enum class RenderState
        {
            SetFont,
            Rendering,
            ReadData,
        };

    public:
        AImGui(RenderType renderType = RenderType::RenderNative, bool autoUpdateOrientation = false);
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

        void ServerWorker();

        int ReadData(void *buffer, size_t readSize);
        void WriteData(void *data, size_t size);

    private:
        bool m_state = false;
        bool m_autoUpdateOrientation;

        int m_rotateTheta = 0;
        int m_screenWidth = -1, m_screenHeight = -1;

        RenderType m_renderType;
        size_t m_maxPacketSize = 1 * 1024 * 1024; // 1MB
        sockaddr_in m_transportAddress{};
        int m_serverFd, m_clientFd;
        std::unique_ptr<std::thread> m_serverWorkerThread;
        std::vector<uint8_t> m_serverFontData;
        std::vector<uint8_t> m_serverRenderData, m_serverRenderDataBack;
        std::atomic<RenderState> m_renderState = RenderState::ReadData;

        ANativeWindow *m_nativeWindow = nullptr;
        EGLDisplay m_defaultDisplay = EGL_NO_DISPLAY;
        EGLSurface m_eglSurface = EGL_NO_SURFACE;
        EGLContext m_eglContext = EGL_NO_CONTEXT;
        ImGuiContext *m_imguiContext = nullptr;
    };
}

#endif // !A_IMGUI_H