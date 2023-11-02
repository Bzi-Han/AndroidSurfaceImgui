#ifndef A_NATIVE_WINDOW_CREATOR_H // !A_NATIVE_WINDOW_CREATOR_H
#define A_NATIVE_WINDOW_CREATOR_H

#include <android/native_window.h>
#include <android/log.h>
#include <dlfcn.h>
#include <sys/system_properties.h>

#include <cstddef>
#include <unordered_map>
#include <string>
#include <vector>

#define ResolveMethod(ClassName, MethodName, Handle, MethodSignature)                                                                    \
    ClassName##__##MethodName = reinterpret_cast<decltype(ClassName##__##MethodName)>(symbolMethod.Find(Handle, MethodSignature));       \
    if (nullptr == ClassName##__##MethodName)                                                                                            \
    {                                                                                                                                    \
        __android_log_print(ANDROID_LOG_ERROR, "ImGui", "[-] Method not found: %s -> %s::%s", MethodSignature, #ClassName, #MethodName); \
    }

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

            typedef int64_t nsecs_t; // nano-seconds
            struct DisplayInfo
            {
                uint32_t w{0};
                uint32_t h{0};
                float xdpi{0};
                float ydpi{0};
                float fps{0};
                float density{0};
                uint8_t orientation{0};
                bool secure{false};
                nsecs_t appVsyncOffset{0};
                nsecs_t presentationDeadline{0};
                uint32_t viewportW{0};
                uint32_t viewportH{0};
            };

            enum class DisplayType
            {
                DisplayIdMain = 0,
                DisplayIdHdmi = 1
            };

            struct PhysicalDisplayId
            {
                uint64_t value;
            };
        }

        struct String8;

        struct LayerMetadata;

        struct Surface;

        struct SurfaceControl;

        struct SurfaceComposerClientTransaction;

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
            struct SymbolMethod
            {
                void *(*Open)(const char *filename, int flag) = nullptr;
                void *(*Find)(void *handle, const char *symbol) = nullptr;
                int (*Close)(void *handle) = nullptr;
            };

            size_t systemVersion = 13;

            void (*RefBase__IncStrong)(void *thiz, void *id) = nullptr;
            void (*RefBase__DecStrong)(void *thiz, void *id) = nullptr;

            void (*String8__Constructor)(void *thiz, const char *const data) = nullptr;
            void (*String8__Destructor)(void *thiz) = nullptr;

            void (*LayerMetadata__Constructor)(void *thiz) = nullptr;

            void (*SurfaceComposerClient__Constructor)(void *thiz) = nullptr;
            void (*SurfaceComposerClient__Destructor)(void *thiz) = nullptr;
            StrongPointer<void> (*SurfaceComposerClient__CreateSurface)(void *thiz, void *name, uint32_t w, uint32_t h, int32_t format, uint32_t flags, void *parentHandle, void *layerMetadata, uint32_t *outTransformHint) = nullptr;
            StrongPointer<void> (*SurfaceComposerClient__GetInternalDisplayToken)() = nullptr;
            StrongPointer<void> (*SurfaceComposerClient__GetBuiltInDisplay)(ui::DisplayType type) = nullptr;
            int32_t (*SurfaceComposerClient__GetDisplayState)(StrongPointer<void> &display, ui::DisplayState *displayState) = nullptr;
            int32_t (*SurfaceComposerClient__GetDisplayInfo)(StrongPointer<void> &display, ui::DisplayInfo *displayInfo) = nullptr;
            std::vector<ui::PhysicalDisplayId> (*SurfaceComposerClient__GetPhysicalDisplayIds)() = nullptr;
            StrongPointer<void> (*SurfaceComposerClient__GetPhysicalDisplayToken)(ui::PhysicalDisplayId displayId) = nullptr;

            void (*SurfaceComposerClient__Transaction__Constructor)(void *thiz) = nullptr;
            void *(*SurfaceComposerClient__Transaction__SetLayer)(void *thiz, StrongPointer<void> &surfaceControl, int32_t z) = nullptr;
            void *(*SurfaceComposerClient__Transaction__SetTrustedOverlay)(void *thiz, StrongPointer<void> &surfaceControl, bool isTrustedOverlay) = nullptr;
            int32_t (*SurfaceComposerClient__Transaction__Apply)(void *thiz, bool synchronous, bool oneWay) = nullptr;

            int32_t (*SurfaceControl__Validate)(void *thiz) = nullptr;
            StrongPointer<Surface> (*SurfaceControl__GetSurface)(void *thiz) = nullptr;
            void (*SurfaceControl__DisConnect)(void *thiz) = nullptr;

            Functionals(const SymbolMethod &symbolMethod)
            {
                std::string systemVersionString(128, 0);

                systemVersionString.resize(__system_property_get("ro.build.version.release", systemVersionString.data()));
                if (!systemVersionString.empty())
                    systemVersion = std::stoi(systemVersionString);

                if (9 > systemVersion)
                {
                    __android_log_print(ANDROID_LOG_ERROR, "ImGui", "[-] Unsupported system version: %zu", systemVersion);
                    return;
                }

                static std::unordered_map<size_t, std::unordered_map<void **, const char *>> patchesTable = {
                    {
                        14,
                        {
                            {reinterpret_cast<void **>(&LayerMetadata__Constructor), "_ZN7android3gui13LayerMetadataC2Ev"},
                            {reinterpret_cast<void **>(&SurfaceComposerClient__CreateSurface), "_ZN7android21SurfaceComposerClient13createSurfaceERKNS_7String8EjjiiRKNS_2spINS_7IBinderEEENS_3gui13LayerMetadataEPj"},
                        },
                    },
                    {
                        12,
                        {
                            {reinterpret_cast<void **>(&SurfaceComposerClient__Transaction__Apply), "_ZN7android21SurfaceComposerClient11Transaction5applyEb"},
                        },
                    },
                    {
                        11,
                        {
                            {reinterpret_cast<void **>(&SurfaceComposerClient__CreateSurface), "_ZN7android21SurfaceComposerClient13createSurfaceERKNS_7String8EjjijPNS_14SurfaceControlENS_13LayerMetadataEPj"},
                            {reinterpret_cast<void **>(&SurfaceControl__GetSurface), "_ZNK7android14SurfaceControl10getSurfaceEv"},
                        },
                    },
                    {
                        10,
                        {
                            {reinterpret_cast<void **>(&SurfaceComposerClient__CreateSurface), "_ZN7android21SurfaceComposerClient13createSurfaceERKNS_7String8EjjijPNS_14SurfaceControlENS_13LayerMetadataE"},
                            {reinterpret_cast<void **>(&SurfaceControl__GetSurface), "_ZNK7android14SurfaceControl10getSurfaceEv"},
                        },
                    },
                    {
                        9,
                        {
                            {reinterpret_cast<void **>(&SurfaceComposerClient__CreateSurface), "_ZN7android21SurfaceComposerClient13createSurfaceERKNS_7String8EjjijPNS_14SurfaceControlEii"},
                            {reinterpret_cast<void **>(&SurfaceComposerClient__GetBuiltInDisplay), "_ZN7android21SurfaceComposerClient17getBuiltInDisplayEi"},
                            {reinterpret_cast<void **>(&SurfaceControl__GetSurface), "_ZNK7android14SurfaceControl10getSurfaceEv"},
                        },
                    },
                };

#ifdef __LP64__
                auto libgui = symbolMethod.Open("/system/lib64/libgui.so", RTLD_LAZY);
                auto libutils = symbolMethod.Open("/system/lib64/libutils.so", RTLD_LAZY);
#else
                auto libgui = symbolMethod.Open("/system/lib/libgui.so", RTLD_LAZY);
                auto libutils = symbolMethod.Open("/system/lib/libutils.so", RTLD_LAZY);
#endif

                ResolveMethod(RefBase, IncStrong, libutils, "_ZNK7android7RefBase9incStrongEPKv");
                ResolveMethod(RefBase, DecStrong, libutils, "_ZNK7android7RefBase9decStrongEPKv");

                ResolveMethod(String8, Constructor, libutils, "_ZN7android7String8C2EPKc");
                ResolveMethod(String8, Destructor, libutils, "_ZN7android7String8D2Ev");

                ResolveMethod(LayerMetadata, Constructor, libgui, "_ZN7android13LayerMetadataC2Ev");

                ResolveMethod(SurfaceComposerClient, Constructor, libgui, "_ZN7android21SurfaceComposerClientC2Ev");
                ResolveMethod(SurfaceComposerClient, CreateSurface, libgui, "_ZN7android21SurfaceComposerClient13createSurfaceERKNS_7String8EjjijRKNS_2spINS_7IBinderEEENS_13LayerMetadataEPj");
                ResolveMethod(SurfaceComposerClient, GetInternalDisplayToken, libgui, "_ZN7android21SurfaceComposerClient23getInternalDisplayTokenEv");
                ResolveMethod(SurfaceComposerClient, GetDisplayState, libgui, "_ZN7android21SurfaceComposerClient15getDisplayStateERKNS_2spINS_7IBinderEEEPNS_2ui12DisplayStateE");
                ResolveMethod(SurfaceComposerClient, GetDisplayInfo, libgui, "_ZN7android21SurfaceComposerClient14getDisplayInfoERKNS_2spINS_7IBinderEEEPNS_11DisplayInfoE");
                ResolveMethod(SurfaceComposerClient, GetPhysicalDisplayIds, libgui, "_ZN7android21SurfaceComposerClient21getPhysicalDisplayIdsEv");
                ResolveMethod(SurfaceComposerClient, GetPhysicalDisplayToken, libgui, "_ZN7android21SurfaceComposerClient23getPhysicalDisplayTokenENS_17PhysicalDisplayIdE");

                ResolveMethod(SurfaceComposerClient__Transaction, Constructor, libgui, "_ZN7android21SurfaceComposerClient11TransactionC2Ev");
                ResolveMethod(SurfaceComposerClient__Transaction, SetLayer, libgui, "_ZN7android21SurfaceComposerClient11Transaction8setLayerERKNS_2spINS_14SurfaceControlEEEi");
                ResolveMethod(SurfaceComposerClient__Transaction, SetTrustedOverlay, libgui, "_ZN7android21SurfaceComposerClient11Transaction17setTrustedOverlayERKNS_2spINS_14SurfaceControlEEEb");
                ResolveMethod(SurfaceComposerClient__Transaction, Apply, libgui, "_ZN7android21SurfaceComposerClient11Transaction5applyEbb");

                ResolveMethod(SurfaceControl, Validate, libgui, "_ZNK7android14SurfaceControl8validateEv");
                ResolveMethod(SurfaceControl, GetSurface, libgui, "_ZN7android14SurfaceControl10getSurfaceEv");
                ResolveMethod(SurfaceControl, DisConnect, libgui, "_ZN7android14SurfaceControl10disconnectEv");

                if (patchesTable.contains(systemVersion))
                {
                    for (const auto &[patchTo, signature] : patchesTable.at(systemVersion))
                    {
                        *patchTo = symbolMethod.Find(libgui, signature);
                        if (nullptr != *patchTo)
                            continue;

                        __android_log_print(ANDROID_LOG_ERROR, "ImGui", "[-] Patch method not found: %s", signature);
                    }
                }

                symbolMethod.Close(libutils);
                symbolMethod.Close(libgui);
            }

            static const Functionals &GetInstance(const SymbolMethod &symbolMethod = {.Open = dlopen, .Find = dlsym, .Close = dlclose})
            {
                static Functionals functionals(symbolMethod);

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
                if (9 < Functionals::GetInstance().systemVersion)
                    Functionals::GetInstance().LayerMetadata__Constructor(data);
            }

            operator void *()
            {
                if (9 < Functionals::GetInstance().systemVersion)
                    return reinterpret_cast<void *>(data);
                else
                    return nullptr;
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

                return reinterpret_cast<Surface *>(reinterpret_cast<size_t>(result.pointer) + sizeof(std::max_align_t) / 2);
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

                Functionals::GetInstance().RefBase__DecStrong(reinterpret_cast<Surface *>(reinterpret_cast<size_t>(surface) - sizeof(std::max_align_t) / 2), this);
                DisConnect();
                Functionals::GetInstance().RefBase__DecStrong(data, this);
            }
        };

        struct SurfaceComposerClientTransaction
        {
            char data[1024];

            SurfaceComposerClientTransaction()
            {
                Functionals::GetInstance().SurfaceComposerClient__Transaction__Constructor(data);
            }

            void *SetLayer(StrongPointer<void> &surfaceControl, int32_t z)
            {
                return Functionals::GetInstance().SurfaceComposerClient__Transaction__SetLayer(data, surfaceControl, z);
            }

            void *SetTrustedOverlay(StrongPointer<void> &surfaceControl, bool isTrustedOverlay)
            {
                return Functionals::GetInstance().SurfaceComposerClient__Transaction__SetTrustedOverlay(data, surfaceControl, isTrustedOverlay);
            }

            int32_t Apply(bool synchronous, bool oneWay)
            {
                if (12 >= Functionals::GetInstance().systemVersion)
                    return reinterpret_cast<int32_t (*)(void *, bool)>(Functionals::GetInstance().SurfaceComposerClient__Transaction__Apply)(data, synchronous);
                else
                    return Functionals::GetInstance().SurfaceComposerClient__Transaction__Apply(data, synchronous, oneWay);
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
                uint32_t flags = 0;

                if (12 <= Functionals::GetInstance().systemVersion)
                {
                    static void *fakeParentHandleForBinder = nullptr;
                    parentHandle = &fakeParentHandleForBinder;
                }
                auto result = Functionals::GetInstance().SurfaceComposerClient__CreateSurface(data, windowName, width, height, 1, flags, parentHandle, layerMetadata, nullptr);

                if (12 <= Functionals::GetInstance().systemVersion)
                {
                    static SurfaceComposerClientTransaction transaction;

                    transaction.SetTrustedOverlay(result, true);
                    transaction.Apply(false, true);
                }

                return {result.get()};
            }

            bool GetDisplayInfo(ui::DisplayState *displayInfo)
            {
                StrongPointer<void> defaultDisplay;

                if (9 >= Functionals::GetInstance().systemVersion)
                    defaultDisplay = Functionals::GetInstance().SurfaceComposerClient__GetBuiltInDisplay(ui::DisplayType::DisplayIdMain);
                else
                {
                    if (14 > Functionals::GetInstance().systemVersion)
                        defaultDisplay = Functionals::GetInstance().SurfaceComposerClient__GetInternalDisplayToken();
                    else
                    {
                        auto displayIds = Functionals::GetInstance().SurfaceComposerClient__GetPhysicalDisplayIds();
                        if (displayIds.empty())
                            return false;

                        defaultDisplay = Functionals::GetInstance().SurfaceComposerClient__GetPhysicalDisplayToken(displayIds[0]);
                    }
                }

                if (nullptr == defaultDisplay.get())
                    return false;

                if (11 <= Functionals::GetInstance().systemVersion)
                    return 0 == Functionals::GetInstance().SurfaceComposerClient__GetDisplayState(defaultDisplay, displayInfo);
                else
                {
                    ui::DisplayInfo realDisplayInfo{};
                    if (0 != Functionals::GetInstance().SurfaceComposerClient__GetDisplayInfo(defaultDisplay, &realDisplayInfo))
                        return false;

                    displayInfo->layerStackSpaceRect.width = realDisplayInfo.w;
                    displayInfo->layerStackSpaceRect.height = realDisplayInfo.h;
                    displayInfo->orientation = static_cast<ui::Rotation>(realDisplayInfo.orientation);

                    return true;
                }
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

#undef ResolveMethod

#endif // !A_NATIVE_WINDOW_CREATOR_H