#ifndef A_NATIVE_WINDOW_CREATOR_H // !A_NATIVE_WINDOW_CREATOR_H
#define A_NATIVE_WINDOW_CREATOR_H

#include <android/native_window.h>
#include <dlfcn.h>

#include <cstddef>
#include <unordered_map>

namespace android
{
    namespace detail
    {
        namespace ui
        {
            // A LayerStack identifies a Z-ordered group of layers. A layer can only be associated to a single
            // LayerStack, but a LayerStack can be associated to multiple displays, mirroring the same content.
            struct LayerStack
            {
                uint32_t id = UINT32_MAX;
            };

            enum class Rotation
            {
                Rotation0 = 0,
                Rotation90 = 1,
                Rotation180 = 2,
                Rotation270 = 3
            };

            // A simple value type representing a two-dimensional size.
            struct Size
            {
                int32_t width = -1;
                int32_t height = -1;
            };

            // Transactional state of physical or virtual display. Note that libgui defines
            // android::DisplayState as a superset of android::ui::DisplayState.
            struct DisplayState
            {
                LayerStack layerStack;
                Rotation orientation = Rotation::Rotation0;
                Size layerStackSpaceRect;
            };
        }

        struct String8;

        struct LayerMetadata;

        struct Surface;

        struct SurfaceControl;

        struct SurfaceComposerClient;

        template <typename any_t>
        struct StrongPointer
        {
            union
            {
                any_t *pointer;
                char padding[sizeof(std::max_align_t)];
            };

            inline any_t *operator->() const { return pointer; }
            inline any_t *get() const { return pointer; }
            inline explicit operator bool() const { return nullptr != pointer; }
        };

        struct Functionals
        {
            void (*RefBase__IncStrong)(void *thiz, void *id) = nullptr;
            void (*RefBase__DecStrong)(void *thiz, void *id) = nullptr;

            void (*String8__Constructor)(void *thiz, const char *const data) = nullptr;
            void (*String8__Destructor)(void *thiz) = nullptr;

            void (*LayerMetadata__Constructor)(void *thiz) = nullptr;

            void (*SurfaceComposerClient__Constructor)(void *thiz) = nullptr;
            void (*SurfaceComposerClient__Destructor)(void *thiz) = nullptr;
            StrongPointer<void> (*SurfaceComposerClient__CreateSurface)(void *thiz, void *name, uint32_t w, uint32_t h, int32_t format, uint32_t flags, void *parentHandle, void *layerMetadata, uint32_t *outTransformHint) = nullptr;
            StrongPointer<void> (*SurfaceComposerClient__GetInternalDisplayToken)() = nullptr;
            int32_t (*SurfaceComposerClient__GetDisplayState)(StrongPointer<void> &display, ui::DisplayState *displayState) = nullptr;

            int32_t (*SurfaceControl__Validate)(void *thiz) = nullptr;
            StrongPointer<Surface> (*SurfaceControl__GetSurface)(void *thiz) = nullptr;
            void (*SurfaceControl__DisConnect)(void *thiz) = nullptr;

            Functionals()
            {
#ifdef __LP64__
                static auto libgui = dlopen("/system/lib64/libgui.so", RTLD_LAZY);
                static auto libutils = dlopen("/system/lib64/libutils.so", RTLD_LAZY);
#else
                static auto libgui = dlopen("/system/lib/libgui.so", RTLD_LAZY);
                static auto libutils = dlopen("/system/lib/libutils.so", RTLD_LAZY);
#endif

                RefBase__IncStrong = reinterpret_cast<decltype(RefBase__IncStrong)>(dlsym(libutils, "_ZNK7android7RefBase9incStrongEPKv"));
                RefBase__DecStrong = reinterpret_cast<decltype(RefBase__DecStrong)>(dlsym(libutils, "_ZNK7android7RefBase9decStrongEPKv"));

                String8__Constructor = reinterpret_cast<decltype(String8__Constructor)>(dlsym(libutils, "_ZN7android7String8C2EPKc"));
                String8__Destructor = reinterpret_cast<decltype(String8__Destructor)>(dlsym(libutils, "_ZN7android7String8D2Ev"));

                LayerMetadata__Constructor = reinterpret_cast<decltype(LayerMetadata__Constructor)>(dlsym(libgui, "_ZN7android13LayerMetadataC2Ev"));

                SurfaceComposerClient__Constructor = reinterpret_cast<decltype(SurfaceComposerClient__Constructor)>(dlsym(libgui, "_ZN7android21SurfaceComposerClientC2Ev"));
                SurfaceComposerClient__CreateSurface = reinterpret_cast<decltype(SurfaceComposerClient__CreateSurface)>(dlsym(libgui, "_ZN7android21SurfaceComposerClient13createSurfaceERKNS_7String8EjjijRKNS_2spINS_7IBinderEEENS_13LayerMetadataEPj"));
                SurfaceComposerClient__GetInternalDisplayToken = reinterpret_cast<decltype(SurfaceComposerClient__GetInternalDisplayToken)>(dlsym(libgui, "_ZN7android21SurfaceComposerClient23getInternalDisplayTokenEv"));
                SurfaceComposerClient__GetDisplayState = reinterpret_cast<decltype(SurfaceComposerClient__GetDisplayState)>(dlsym(libgui, "_ZN7android21SurfaceComposerClient15getDisplayStateERKNS_2spINS_7IBinderEEEPNS_2ui12DisplayStateE"));

                SurfaceControl__Validate = reinterpret_cast<decltype(SurfaceControl__Validate)>(dlsym(libgui, "_ZNK7android14SurfaceControl8validateEv"));
                SurfaceControl__GetSurface = reinterpret_cast<decltype(SurfaceControl__GetSurface)>(dlsym(libgui, "_ZN7android14SurfaceControl10getSurfaceEv"));
                SurfaceControl__DisConnect = reinterpret_cast<decltype(SurfaceControl__DisConnect)>(dlsym(libgui, "_ZN7android14SurfaceControl10disconnectEv"));
            }

            static const Functionals &GetInstance()
            {
                static Functionals functionals;

                return functionals;
            }
        };

        struct String8
        {
            char data[1024];

            String8(const char *const string)
            {
                Functionals::GetInstance().String8__Constructor(data, string);
            }

            ~String8()
            {
                Functionals::GetInstance().String8__Destructor(data);
            }

            operator void *()
            {
                return reinterpret_cast<void *>(data);
            }
        };

        struct LayerMetadata
        {
            char data[1024];

            LayerMetadata()
            {
                Functionals::GetInstance().LayerMetadata__Constructor(data);
            }

            operator void *()
            {
                return reinterpret_cast<void *>(data);
            }
        };

        struct Surface
        {
        };

        struct SurfaceControl
        {
            void *data;

            SurfaceControl() : data(nullptr) {}
            SurfaceControl(void *data) : data(data) {}

            int32_t Validate()
            {
                if (nullptr == data)
                    return 0;

                return Functionals::GetInstance().SurfaceControl__Validate(data);
            }

            Surface *GetSurface()
            {
                if (nullptr == data)
                    return nullptr;

                auto result = Functionals::GetInstance().SurfaceControl__GetSurface(data);

                return reinterpret_cast<Surface *>(reinterpret_cast<size_t>(result.pointer) + 16);
            }

            void DisConnect()
            {
                if (nullptr == data)
                    return;

                Functionals::GetInstance().SurfaceControl__DisConnect(data);
            }

            void DestroySurface(Surface *surface)
            {
                if (nullptr == data || nullptr == surface)
                    return;

                Functionals::GetInstance().RefBase__DecStrong(reinterpret_cast<Surface *>(reinterpret_cast<size_t>(surface) - 16), this);
                DisConnect();
                Functionals::GetInstance().RefBase__DecStrong(data, this);
            }
        };

        struct SurfaceComposerClient
        {
            char data[1024];

            SurfaceComposerClient()
            {
                Functionals::GetInstance().SurfaceComposerClient__Constructor(data);
                Functionals::GetInstance().RefBase__IncStrong(data, this);
            }

            SurfaceControl CreateSurface(const char *name, int32_t width, int32_t height)
            {
                void *parentHandle = nullptr;
                String8 windowName(name);
                LayerMetadata layerMetadata;

                auto result = Functionals::GetInstance().SurfaceComposerClient__CreateSurface(data, windowName, width, height, 1, 0, &parentHandle, layerMetadata, nullptr);

                return {result.get()};
            }

            bool GetDisplayInfo(ui::DisplayState *displayInfo)
            {
                auto defaultDisplay = Functionals::GetInstance().SurfaceComposerClient__GetInternalDisplayToken();

                if (nullptr == defaultDisplay.get())
                    return false;

                return 0 == Functionals::GetInstance().SurfaceComposerClient__GetDisplayState(defaultDisplay, displayInfo);
            }
        };

    }

    class ANativeWindowCreator
    {
    public:
        struct DisplayInfo
        {
            int32_t theta;
            int32_t width;
            int32_t height;
        };

    public:
        static detail::SurfaceComposerClient &GetComposerInstance()
        {
            static detail::SurfaceComposerClient surfaceComposerClient;

            return surfaceComposerClient;
        }

        static DisplayInfo GetDisplayInfo()
        {
            auto &surfaceComposerClient = GetComposerInstance();
            detail::ui::DisplayState displayInfo{};

            if (!surfaceComposerClient.GetDisplayInfo(&displayInfo))
                return {};

            return DisplayInfo{
                .theta = 90 * static_cast<int32_t>(displayInfo.orientation),
                .width = displayInfo.layerStackSpaceRect.width,
                .height = displayInfo.layerStackSpaceRect.height,
            };
        }

        static ANativeWindow *Create(const char *name, int32_t width = -1, int32_t height = -1)
        {
            auto &surfaceComposerClient = GetComposerInstance();

            while (-1 == width || -1 == height)
            {
                detail::ui::DisplayState displayInfo{};

                if (!surfaceComposerClient.GetDisplayInfo(&displayInfo))
                    break;

                width = displayInfo.layerStackSpaceRect.width;
                height = displayInfo.layerStackSpaceRect.height;

                break;
            }

            auto surfaceControl = surfaceComposerClient.CreateSurface(name, width, height);
            auto nativeWindow = reinterpret_cast<ANativeWindow *>(surfaceControl.GetSurface());

            m_cachedSurfaceControl.emplace(nativeWindow, std::move(surfaceControl));
            return nativeWindow;
        }

        static void Destroy(ANativeWindow *nativeWindow)
        {
            if (!m_cachedSurfaceControl.contains(nativeWindow))
                return;

            m_cachedSurfaceControl[nativeWindow].DestroySurface(reinterpret_cast<detail::Surface *>(nativeWindow));
            m_cachedSurfaceControl.erase(nativeWindow);
        }

    private:
        inline static std::unordered_map<ANativeWindow *, detail::SurfaceControl> m_cachedSurfaceControl;
    };
}

#endif // !A_NATIVE_WINDOW_CREATOR_H