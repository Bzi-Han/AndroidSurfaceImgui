#include "AImGui.h"

#include <ImGui-SharedDrawData/modules/ImGuiSharedDrawData.h>

namespace android
{
    AImGui::AImGui(RenderType renderType, bool autoUpdateOrientation)
        : m_renderType(renderType),
          m_autoUpdateOrientation(autoUpdateOrientation)
    {
        InitEnvironment();
    }

    AImGui::~AImGui()
    {
        UnInitEnvironment();
    }

    void AImGui::BeginFrame()
    {
        if (!m_state)
            return;

        if (m_autoUpdateOrientation)
        {
            auto displayInfo = ANativeWindowCreator::GetDisplayInfo();

            // Check if display orientation is changed
            if (m_rotateTheta != displayInfo.theta)
            {
                UnInitEnvironment();
                InitEnvironment();
            }
        }

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplAndroid_NewFrame();
        if (RenderType::RenderClient == m_renderType || RenderType::RenderNative == m_renderType)
            ImGui::NewFrame();
    }
    void AImGui::EndFrame()
    {
        if (!m_state)
            return;

        if (RenderType::RenderClient == m_renderType)
        {
            ImGui::Render();
            const auto &sharedData = ImGui::GetSharedDrawData();
            if (!sharedData.empty())
            {
                uint32_t packetSize = static_cast<uint32_t>(sharedData.size());
                WriteData(&packetSize, sizeof(packetSize));
                WriteData(const_cast<uint8_t *>(sharedData.data()), sharedData.size());
            }
        }
        else if (RenderType::RenderServer == m_renderType)
        {
            if (!m_renderDataReadLock)
            {
                auto drawData = ImGui::RenderSharedDrawData(m_serverRenderData);
                if (nullptr != drawData)
                {
                    glClear(GL_COLOR_BUFFER_BIT);
                    ImGui_ImplOpenGL3_RenderDrawData(drawData);
                    eglSwapBuffers(m_defaultDisplay, m_eglSurface);
                }
                m_renderCompleteSignal.notify_one();
            }
        }
        else if (RenderType::RenderNative == m_renderType)
        {
            ImGui::Render();
            glClear(GL_COLOR_BUFFER_BIT);
            ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
            eglSwapBuffers(m_defaultDisplay, m_eglSurface);
        }
    }

    void AImGui::ProcessInputEvent()
    {
        static ATouchEvent::TouchEvent event{};

        if (RenderType::RenderServer == m_renderType || RenderType::RenderNative == m_renderType)
        {
            static ATouchEvent touchEvent;

            if (!touchEvent.GetTouchEvent(&event))
                return;
            event.TransformToScreen(m_screenWidth, m_screenHeight, m_rotateTheta);

            if (RenderType::RenderServer == m_renderType)
                WriteData(&event, sizeof(event));
        }
        else
        {
            auto readResult = ReadData(&event, sizeof(event));
            if (0 >= readResult)
            {
                LogDebug("[-] Client can not read input event, readResult:%d  %d:%s", readResult, errno, strerror(errno));
                return;
            }
        }

        if (RenderType::RenderClient == m_renderType || RenderType::RenderNative == m_renderType)
        {
            auto &imguiIO = ImGui::GetIO();
            switch (event.type)
            {
            case ATouchEvent::EventType::move:
            {
                imguiIO.MousePos = ImVec2(event.x, event.y);
                break;
            }
            case ATouchEvent::EventType::touchDown:
            {
                imguiIO.MousePos = ImVec2(event.x, event.y);
                imguiIO.MouseDown[0] = true; // Mouse left down
                break;
            }
            case ATouchEvent::EventType::touchUp:
            {
                imguiIO.MousePos = ImVec2(event.x, event.y);
                imguiIO.MouseDown[0] = false; // Mouse left up
                break;
            }
            default:
                break;
            }
        }
    }

    bool AImGui::InitEnvironment()
    {
        // Initialize rpc
        m_transportAddress.sun_family = AF_UNIX;
        strcpy(m_transportAddress.sun_path, "/data/local/tmp/socket.RenderService.AImGui");
        if (RenderType::RenderClient == m_renderType)
        {
            m_clientFd = socket(AF_UNIX, SOCK_STREAM, 0);
            if (0 > m_clientFd)
            {
                LogDebug("[-] Client fd create failed, m_clientFd:%d", m_clientFd);
                return false;
            }

            if (0 > connect(m_clientFd, reinterpret_cast<sockaddr *>(&m_transportAddress), sizeof(m_transportAddress)))
            {
                LogDebug("[-] Client connect to server failed, %d:%s", errno, strerror(errno));
                return false;
            }
        }
        else if (RenderType::RenderServer == m_renderType)
        {
            m_serverFd = socket(AF_UNIX, SOCK_STREAM, 0);
            if (0 > m_serverFd)
            {
                LogDebug("[-] Server fd create failed, m_serverFd:%d", m_serverFd);
                return false;
            }

            if (0 > bind(m_serverFd, reinterpret_cast<sockaddr *>(&m_transportAddress), sizeof(m_transportAddress)))
            {
                LogDebug("[-] Server bind fd failed, %d:%s", errno, strerror(errno));
                return false;
            }

            if (0 > listen(m_serverFd, 1))
            {
                LogDebug("[-] Server listen fd failed, %d:%s", errno, strerror(errno));
                return false;
            }

            m_serverWorkerThread = std::make_unique<std::thread>(&AImGui::ServerWorker, this);
        }

        // Initialize display orientation
        auto displayInfo = ANativeWindowCreator::GetDisplayInfo();
        LogInfo("[=] Display angle:%d width:%d height:%d", displayInfo.theta, displayInfo.width, displayInfo.height);

        // Create native window
        m_nativeWindow = ANativeWindowCreator::Create("AImGui");
        if (nullptr == m_nativeWindow)
        {
            LogDebug("[-] ANativeWindow create failed");
            return false;
        }

        // Acquire native window
        LogInfo("[=] Acquiring native window");
        ANativeWindow_acquire(m_nativeWindow);
        LogInfo("[+] Native window acquired");

        // EGL initialization
        m_defaultDisplay = eglGetDisplay(EGL_DEFAULT_DISPLAY);
        if (EGL_NO_DISPLAY == m_defaultDisplay)
        {
            LogDebug("[-] EGL get default display failed: %d", eglGetError());
            return false;
        }

        if (EGL_TRUE != eglInitialize(m_defaultDisplay, 0, 0))
        {
            LogDebug("[-] EGL initialize failed: %d", eglGetError());
            return false;
        }

        EGLint numEglConfig = 0;
        EGLConfig eglConfig{};
        std::pair<EGLint, EGLint> eglConfigAttribList[] = {
            {EGL_SURFACE_TYPE, EGL_WINDOW_BIT},
            {EGL_RENDERABLE_TYPE, EGL_OPENGL_ES3_BIT},
            {EGL_BLUE_SIZE, 5},
            {EGL_GREEN_SIZE, 6},
            {EGL_RED_SIZE, 5},
            {EGL_BUFFER_SIZE, 32},
            {EGL_DEPTH_SIZE, 16},
            {EGL_STENCIL_SIZE, 8},
            {EGL_NONE, EGL_NONE},
        };
        if (EGL_TRUE != eglChooseConfig(m_defaultDisplay, reinterpret_cast<const EGLint *>(eglConfigAttribList), &eglConfig, 1, &numEglConfig))
        {
            LogDebug("[-] EGL choose config failed: %d", eglGetError());
            return false;
        }

        EGLint eglBufferFormat;
        if (EGL_TRUE != eglGetConfigAttrib(m_defaultDisplay, eglConfig, EGL_NATIVE_VISUAL_ID, &eglBufferFormat))
        {
            LogDebug("[-] EGL get config attribute failed: %d", eglGetError());
            return false;
        }
        ANativeWindow_setBuffersGeometry(m_nativeWindow, 0, 0, eglBufferFormat);
        m_eglSurface = eglCreateWindowSurface(m_defaultDisplay, eglConfig, m_nativeWindow, nullptr);
        if (EGL_NO_SURFACE == m_eglSurface)
        {
            LogDebug("[-] EGL create window surface failed: %d", eglGetError());
            return false;
        }

        EGLint eglContextAttribList[] = {EGL_CONTEXT_CLIENT_VERSION, 2, EGL_NONE};
        m_eglContext = eglCreateContext(m_defaultDisplay, eglConfig, EGL_NO_CONTEXT, eglContextAttribList);
        if (EGL_NO_CONTEXT == m_eglContext)
        {
            LogDebug("[-] EGL create context failed: %d", eglGetError());
            return false;
        }

        if (EGL_TRUE != eglMakeCurrent(m_defaultDisplay, m_eglSurface, m_eglSurface, m_eglContext))
        {
            LogDebug("[-] EGL make current failed: %d", eglGetError());
            return false;
        }

        // ImGui initialization
        IMGUI_CHECKVERSION();

        m_imguiContext = ImGui::CreateContext();
        if (nullptr == m_imguiContext)
        {
            LogDebug("[-] ImGui create context failed");
            return false;
        }

        auto &imguiIO = ImGui::GetIO();
        imguiIO.IniFilename = nullptr;

        ImFontConfig fontConfig;
        fontConfig.SizePixels = 22.f;
        imguiIO.Fonts->AddFontDefault(&fontConfig);

        ImGui::StyleColorsDark();
        ImGui::GetStyle().ScaleAllSizes(3.f);

        if (!ImGui_ImplAndroid_Init(m_nativeWindow))
        {
            LogDebug("[-] ImGui init android implement failed");
            return false;
        }
        if (!ImGui_ImplOpenGL3_Init("#version 300 es"))
        {
            LogDebug("[-] ImGui init OpenGL3 failed");
            return false;
        }

        glViewport(0, 0, displayInfo.width, displayInfo.height);
        glClearColor(0.f, 0.f, 0.f, 0.f);

        m_rotateTheta = displayInfo.theta;
        m_screenWidth = displayInfo.width;
        m_screenHeight = displayInfo.height;

        return (m_state = true);
    }
    void AImGui::UnInitEnvironment()
    {
        if (nullptr != m_imguiContext)
        {
            ImGui_ImplOpenGL3_Shutdown();
            ImGui_ImplAndroid_Shutdown();
            ImGui::DestroyContext(m_imguiContext);
        }

        if (EGL_NO_DISPLAY != m_defaultDisplay)
        {
            eglMakeCurrent(m_defaultDisplay, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);

            if (EGL_NO_CONTEXT != m_eglContext)
                eglDestroyContext(m_defaultDisplay, m_eglContext);
            if (EGL_NO_SURFACE != m_eglSurface)
                eglDestroySurface(m_defaultDisplay, m_eglSurface);

            eglTerminate(m_defaultDisplay);
        }

        if (nullptr != m_nativeWindow)
            ANativeWindow_release(m_nativeWindow);

        close(m_clientFd);
        close(m_serverFd);

        m_imguiContext = nullptr;
        m_eglContext = EGL_NO_CONTEXT;
        m_eglSurface = EGL_NO_SURFACE;
        m_defaultDisplay = EGL_NO_DISPLAY;
        m_nativeWindow = nullptr;
        m_state = false;
    }

    void AImGui::ServerWorker()
    {
        m_clientFd = accept(m_serverFd, nullptr, nullptr);
        if (0 > m_clientFd)
        {
            LogDebug("[-] Server accept client connect failed, %d:%s", errno, strerror(errno));
            m_state = false;
            return;
        }

        uint32_t packetSize = 0;
        while (m_state)
        {
            if (static_cast<int>(sizeof(packetSize)) > ReadData(&packetSize, sizeof(packetSize)))
            {
                LogDebug("[-] Server can not read packet size, %d:%s", errno, strerror(errno));
                break;
            }
            if (packetSize > m_maxPacketSize)
            {
                LogDebug("[-] Packet is too large: %2.f", packetSize / 1024.f / 1024.f);
                break;
            }

            m_renderDataReadLock = true;
            if (m_serverRenderData.size() < packetSize)
                m_serverRenderData.resize(packetSize);
            auto readResult = ReadData(m_serverRenderData.data(), packetSize);
            if (0 >= readResult)
            {
                LogDebug("[-] Client disconnect or read failed, readResult:%d  %d:%s", readResult, errno, strerror(errno));
                break;
            }
            m_renderDataReadLock = false;

            // Wait for rendering complete
            {
                std::unique_lock<std::mutex> locker(m_renderingMutex);

                m_renderCompleteSignal.wait(locker);
            }
        }

        m_state = false;
    }

    int AImGui::ReadData(void *buffer, size_t readSize)
    {
        size_t packetReaded = 0;

        while (packetReaded < readSize)
        {
            auto readResult = read(m_clientFd, reinterpret_cast<char *>(buffer) + packetReaded, readSize - packetReaded);
            if (0 >= readResult)
                return readResult;
            packetReaded += readResult;
        }

        return static_cast<int>(packetReaded);
    }
    void AImGui::WriteData(void *data, size_t size)
    {
        write(m_clientFd, data, size);
    }
}