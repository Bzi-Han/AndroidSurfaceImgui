#include "AImGui.h"

namespace android
{
    AImGui::AImGui(bool autoUpdateOrientation)
        : m_autoUpdateOrientation(autoUpdateOrientation)
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
        ImGui::NewFrame();
    }
    void AImGui::EndFrame()
    {
        if (!m_state)
            return;

        glClear(GL_COLOR_BUFFER_BIT);
        glFlush();

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        eglSwapBuffers(m_defaultDisplay, m_eglSurface);
    }

    void AImGui::ProcessInputEvent()
    {
        static ATouchEvent touchEvent;
        static ATouchEvent::TouchEvent event{};

        if (!touchEvent.GetTouchEvent(&event))
            return;
        event.TransformToScreen(m_screenWidth, m_screenHeight, m_rotateTheta);

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

    bool AImGui::InitEnvironment()
    {
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

        m_imguiContext = nullptr;
        m_eglContext = EGL_NO_CONTEXT;
        m_eglSurface = EGL_NO_SURFACE;
        m_defaultDisplay = EGL_NO_DISPLAY;
        m_nativeWindow = nullptr;
        m_state = false;
    }
}