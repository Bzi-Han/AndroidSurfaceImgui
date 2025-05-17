/*
 * MIT License
 *
 * Copyright (c) 2023 Bzi-Han
 * Project: https://github.com/Bzi-Han/AndroidSurfaceImgui
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#ifndef A_NATIVE_WINDOW_CREATOR_H // !A_NATIVE_WINDOW_CREATOR_H
#define A_NATIVE_WINDOW_CREATOR_H

#include <dlfcn.h>
#include <sys/system_properties.h>

#include <android/log.h>
#include <android/native_window.h>

#include <cstdint>
#include <memory>
#include <string>
#include <string_view>
#include <array>
#include <vector>
#include <unordered_map>

#ifndef LOGTAG
#define LOGTAG "AImGui"

#define LOGTAG_DEFINED 1
#endif // !TAG

#ifndef LogInfo
#define LogInfo(formatter, ...) __android_log_print(ANDROID_LOG_INFO, LOGTAG, formatter __VA_OPT__(, ) __VA_ARGS__)
#define LogDebug(formatter, ...) __android_log_print(ANDROID_LOG_DEBUG, LOGTAG, formatter __VA_OPT__(, ) __VA_ARGS__)
#define LogError(formatter, ...) __android_log_print(ANDROID_LOG_ERROR, LOGTAG, formatter __VA_OPT__(, ) __VA_ARGS__)

#define LogInfo_DEFINED 1
#endif // !LogInfo

namespace android::detail::types
{
    /**
     * The following types and structures are adapted from AOSP.
     */

    enum class PixelFormat
    {
        UNKNOWN = 0,
        CUSTOM = -4,
        TRANSLUCENT = -3,
        TRANSPARENT = -2,
        OPAQUE = -1,
        RGBA_8888 = 1,
        RGBX_8888 = 2,
        RGB_888 = 3,
        RGB_565 = 4,
        BGRA_8888 = 5,
        RGBA_5551 = 6,
        RGBA_4444 = 7,
        RGBA_FP16 = 22,
        RGBA_1010102 = 43,
        R_8 = 0x38,
    };

    enum class WindowFlags
    { // (keep in sync with SurfaceControl.java)
        eHidden = 0x00000004,
        eDestroyBackbuffer = 0x00000020,
        eSkipScreenshot = 0x00000040,
        eSecure = 0x00000080,
        eNonPremultiplied = 0x00000100,
        eOpaque = 0x00000400,
        eProtectedByApp = 0x00000800,
        eProtectedByDRM = 0x00001000,
        eCursorWindow = 0x00002000,
        eNoColorFill = 0x00004000,

        eFXSurfaceBufferQueue = 0x00000000,
        eFXSurfaceEffect = 0x00020000,
        eFXSurfaceBufferState = 0x00040000,
        eFXSurfaceContainer = 0x00080000,
        eFXSurfaceMask = 0x000F0000,
    };

    enum class MetadataType
    {
        OWNER_UID = 1,
        WINDOW_TYPE = 2,
        TASK_ID = 3,
        MOUSE_CURSOR = 4,
        ACCESSIBILITY_ID = 5,
        OWNER_PID = 6,
        DEQUEUE_TIME = 7,
        GAME_MODE = 8
    };

    template <typename any_t>
    struct StrongPointer
    {
        union {
            any_t *pointer;
            char padding[sizeof(std::max_align_t)];
        };

        inline any_t *operator->() const
        {
            return pointer;
        }
        inline any_t *get() const
        {
            return pointer;
        }
        inline explicit operator bool() const
        {
            return nullptr != pointer;
        }
    };
} // namespace android::detail::types

namespace android::detail::types::ui
{
    /**
     * The following types and structures are adapted from AOSP android::ui.
     */

    typedef int64_t nsecs_t; // nano-seconds

    enum class Rotation
    {
        Rotation0 = 0,
        Rotation90 = 1,
        Rotation180 = 2,
        Rotation270 = 3
    };

    enum class DisplayType
    {
        DisplayIdMain = 0,
        DisplayIdHdmi = 1
    };

    // A LayerStack identifies a Z-ordered group of layers. A layer can only be associated to a single
    // LayerStack, but a LayerStack can be associated to multiple displays, mirroring the same content.
    struct LayerStack
    {
        uint32_t id = UINT32_MAX;
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

    struct PhysicalDisplayId
    {
        uint64_t value;
    };
} // namespace android::detail::types::ui

namespace android::detail::types::apis::libutils
{
    namespace generic
    {
        using RefBase__IncStrong = void (*)(void *thiz, void *id);
        using RefBase__DecStrong = void (*)(void *thiz, void *id);

        using String8__Constructor = void *(*)(void *thiz, const char *const string);
        using String8__Destructor = void (*)(void *thiz);
    } // namespace generic
} // namespace android::detail::types::apis::libutils

namespace android::detail::types::apis::libgui
{
    namespace v5_v7
    {
        // SurfaceComposerClient::createSurface(
        //         const String8& name,
        //         uint32_t w,
        //         uint32_t h,
        //         PixelFormat format,
        //         uint32_t flags)
        using SurfaceComposerClient__CreateSurface = StrongPointer<void> (*)(void *thiz, void *name, uint32_t w, uint32_t h, PixelFormat format, WindowFlags flags);

        using SurfaceComposerClient__OpenGlobalTransaction__Static = void (*)();
        using SurfaceComposerClient__CloseGlobalTransaction__Static = void (*)(bool synchronous);

        using SurfaceControl__SetLayer = void *(*)(void *thiz, int32_t z);

        // enum {
        //     // The API number used to indicate the currently connected producer
        //     CURRENTLY_CONNECTED_API = -1,
        //
        //     // The API number used to indicate that no producer is connected
        //     NO_CONNECTED_API        = 0,
        // };
        using Surface__DisConnect = void *(*)(void *thiz, int api);
    } // namespace v5_v7

    namespace v8_v9
    {
        // SurfaceComposerClient::createSurface(
        //         const String8& name,
        //         uint32_t w,
        //         uint32_t h,
        //         PixelFormat format,
        //         uint32_t flags,
        //         SurfaceControl* parent,
        //         uint32_t windowType,
        //         uint32_t ownerUid)
        using SurfaceComposerClient__CreateSurface = StrongPointer<void> (*)(void *thiz, void *name, uint32_t w, uint32_t h, PixelFormat format, WindowFlags flags, void *parent, uint32_t windowType, uint32_t ownerUid);
    } // namespace v8_v9

    namespace v10
    {
        // SurfaceComposerClient::createSurface(const String8& name, uint32_t w, uint32_t h,
        //         PixelFormat format, uint32_t flags,
        //         SurfaceControl* parent,
        //         LayerMetadata metadata)
        using SurfaceComposerClient__CreateSurface = StrongPointer<void> (*)(void *thiz, void *name, uint32_t w, uint32_t h, PixelFormat format, WindowFlags flags, void *parent, void *metadata);
    } // namespace v10

    namespace v11
    {
        // SurfaceComposerClient::createSurface(const String8& name, uint32_t w, uint32_t h,
        //         PixelFormat format, uint32_t flags,
        //         SurfaceControl* parent,
        //         LayerMetadata metadata,
        //         uint32_t* outTransformHint)
        using SurfaceComposerClient__CreateSurface = StrongPointer<void> (*)(void *thiz, void *name, uint32_t w, uint32_t h, PixelFormat format, WindowFlags flags, void *parent, void *metadata, uint32_t *outTransformHint);
    } // namespace v11

    namespace v12_v13
    {
        // SurfaceComposerClient::createSurface(const String8& name, uint32_t w, uint32_t h,
        //         PixelFormat format, uint32_t flags,
        //         const sp<IBinder>& parentHandle,
        //         LayerMetadata metadata,
        //         uint32_t* outTransformHint)
        using SurfaceComposerClient__CreateSurface = StrongPointer<void> (*)(void *thiz, void *name, uint32_t w, uint32_t h, PixelFormat format, WindowFlags flags, void **parentHandle, void *metadata, uint32_t *outTransformHint);
    } // namespace v12_v13

    namespace v14_v16
    {
        // SurfaceComposerClient::createSurface(const String8& name, uint32_t w, uint32_t h,
        //         PixelFormat format, int32_t flags,
        //         const sp<IBinder>& parentHandle,
        //         LayerMetadata metadata,
        //         uint32_t* outTransformHint)
        using SurfaceComposerClient__CreateSurface = StrongPointer<void> (*)(void *thiz, void *name, uint32_t w, uint32_t h, PixelFormat format, WindowFlags flags, void **parentHandle, void *metadata, uint32_t *outTransformHint);
    } // namespace v14_v16

    namespace v8_v12
    {
        using SurfaceComposerClient__Transaction__Apply = int32_t (*)(void *thiz, bool synchronous);
    } // namespace v8_v12

    namespace v13_v16
    {
        using SurfaceComposerClient__Transaction__Apply = int32_t (*)(void *thiz, bool synchronous, bool oneWay);
    } // namespace v13_v16

    namespace generic
    {
        using LayerMetadata__Constructor = void (*)(void *thiz);

        using SurfaceComposerClient__Constructor = void *(*)(void *thiz);
        using SurfaceComposerClient__MirrorSurface = StrongPointer<void> (*)(void *thiz, void *mirrorFromSurface);
        // Static
        using SurfaceComposerClient__GetInternalDisplayToken__Static = StrongPointer<void> (*)();
        using SurfaceComposerClient__GetBuiltInDisplay__Static = StrongPointer<void> (*)(ui::DisplayType type);
        using SurfaceComposerClient__GetDisplayState__Static = int32_t (*)(StrongPointer<void> &display, ui::DisplayState *displayState);
        using SurfaceComposerClient__GetDisplayInfo__Static = int32_t (*)(StrongPointer<void> &display, ui::DisplayInfo *displayInfo);
        using SurfaceComposerClient__GetPhysicalDisplayIds__Static = std::vector<ui::PhysicalDisplayId> (*)();
        using SurfaceComposerClient__GetPhysicalDisplayToken__Static = StrongPointer<void> (*)(ui::PhysicalDisplayId displayId);

        using SurfaceComposerClient__Transaction__Constructor = void *(*)(void *thiz);
        using SurfaceComposerClient__Transaction__SetLayer = void *(*)(void *thiz, StrongPointer<void> &surfaceControl, int32_t z);
        using SurfaceComposerClient__Transaction__SetTrustedOverlay = void *(*)(void *thiz, StrongPointer<void> &surfaceControl, bool isTrustedOverlay);
        using SurfaceComposerClient__Transaction__SetLayerStack = void *(*)(void *thiz, StrongPointer<void> &surfaceControl, uint32_t layerStack);
        using SurfaceComposerClient__Transaction__Show = void *(*)(void *thiz, StrongPointer<void> &surfaceControl);
        using SurfaceComposerClient__Transaction__Hide = void *(*)(void *thiz, StrongPointer<void> &surfaceControl);
        using SurfaceComposerClient__Transaction__Reparent = void *(*)(void *thiz, StrongPointer<void> &surfaceControl, StrongPointer<void> &newParentHandle);

        using SurfaceControl__GetSurface = StrongPointer<void> (*)(void *thiz);
        using SurfaceControl__DisConnect = void (*)(void *thiz);
    } // namespace generic
} // namespace android::detail::types::apis::libgui

namespace android::detail::types::apis
{
    struct ApiDescriptor
    {
        size_t minVersion;
        size_t maxVersion;
        void **storeToTarget;
        const char *apiSignature;

        bool IsSupported(size_t currentVersion) const
        {
            return currentVersion >= minVersion && currentVersion <= maxVersion;
        }
    };
} // namespace android::detail::types::apis

namespace android::detail::apis
{
    namespace libutils
    {
        struct RefBase
        {
            struct ApiTable
            {
                void *IncStrong;
                void *DecStrong;
            };

            inline static ApiTable Api;
        };

        struct String8
        {
            struct ApiTable
            {
                void *Constructor;
                void *Destructor;
            };

            inline static ApiTable Api;
        };
    } // namespace libutils

    namespace libgui
    {
        struct LayerMetadata
        {
            struct ApiTable
            {
                void *Constructor;
            };

            inline static ApiTable Api;
        };

        struct SurfaceComposerClient
        {
            struct Transaction;

            struct ApiTable
            {
                void *Constructor;
                void *CreateSurface;
                void *MirrorSurface;
                void *GetInternalDisplayToken;
                void *GetBuiltInDisplay;
                void *GetDisplayState;
                void *GetDisplayInfo;
                void *GetPhysicalDisplayIds;
                void *GetPhysicalDisplayToken;

                void *OpenGlobalTransaction;
                void *CloseGlobalTransaction;
            };

            inline static ApiTable Api;
        };

        struct SurfaceComposerClient::Transaction
        {
            struct ApiTable
            {
                void *Constructor;
                void *SetLayer;
                void *SetTrustedOverlay;
                void *SetLayerStack;
                void *Show;
                void *Hide;
                void *Reparent;
                void *Apply;
            };

            inline static ApiTable Api;
        };

        struct SurfaceControl
        {
            struct ApiTable
            {
                void *GetSurface;
                void *DisConnect;

                void *SetLayer;
            };

            inline static ApiTable Api;
        };

        struct Surface
        {
            struct ApiTable
            {
                void *DisConnect;
            };

            inline static ApiTable Api;
        };
    } // namespace libgui
} // namespace android::detail::apis

namespace android::detail::compat
{
    constexpr size_t SupportedMinVersion = 5;

    static size_t SystemVersion = 13;

    struct String8
    {
        char data[1024];

        String8(const char *const string)
        {
            reinterpret_cast<types::apis::libutils::generic::String8__Constructor>(apis::libutils::String8::Api.Constructor)(data, string);
        }

        ~String8()
        {
            reinterpret_cast<types::apis::libutils::generic::String8__Destructor>(apis::libutils::String8::Api.Destructor)(data);
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
            if (9 < SystemVersion)
                reinterpret_cast<types::apis::libgui::generic::LayerMetadata__Constructor>(apis::libgui::LayerMetadata::Api.Constructor)(data);
        }

        operator void *()
        {
            if (9 < SystemVersion)
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

        SurfaceControl()
            : data(nullptr)
        {
        }
        SurfaceControl(void *data)
            : data(data)
        {
        }

        operator bool() const
        {
            return nullptr != data;
        }

        operator types::StrongPointer<void> &()
        {
            static types::StrongPointer<void> result;

            result.pointer = data;

            return result;
        }

        Surface *GetSurface()
        {
            if (nullptr == data)
                return nullptr;

            auto result = reinterpret_cast<types::apis::libgui::generic::SurfaceControl__GetSurface>(apis::libgui::SurfaceControl::Api.GetSurface)(data);

            return reinterpret_cast<Surface *>(reinterpret_cast<size_t>(result.pointer) + sizeof(std::max_align_t) / 2);
        }

        void DisConnect()
        {
            if (nullptr == data)
                return;

            reinterpret_cast<types::apis::libgui::generic::SurfaceControl__DisConnect>(apis::libgui::SurfaceControl::Api.DisConnect)(data);
        }

        void SetLayer(int32_t z)
        {
            if (nullptr == data || 8 > SystemVersion)
                return;

            reinterpret_cast<types::apis::libgui::v5_v7::SurfaceControl__SetLayer>(apis::libgui::SurfaceControl::Api.SetLayer)(data, z);
        }

        void DestroySurface(Surface *surface)
        {
            if (nullptr == data || nullptr == surface)
                return;

            reinterpret_cast<types::apis::libutils::generic::RefBase__DecStrong>(apis::libutils::RefBase::Api.DecStrong)(reinterpret_cast<Surface *>(reinterpret_cast<size_t>(surface) - sizeof(std::max_align_t) / 2), this);
            if (7 > SystemVersion)
                reinterpret_cast<types::apis::libgui::v5_v7::Surface__DisConnect>(apis::libgui::Surface::Api.DisConnect)(reinterpret_cast<Surface *>(reinterpret_cast<size_t>(surface) - sizeof(std::max_align_t) / 2), -1);
            else
                DisConnect();
            reinterpret_cast<types::apis::libutils::generic::RefBase__DecStrong>(apis::libutils::RefBase::Api.DecStrong)(data, this);
        }
    };

    struct SurfaceComposerClientTransaction
    {
        char data[1024];

        SurfaceComposerClientTransaction()
        {
            reinterpret_cast<types::apis::libgui::generic::SurfaceComposerClient__Transaction__Constructor>(apis::libgui::SurfaceComposerClient::Transaction::Api.Constructor)(data);
        }

        void *SetLayer(types::StrongPointer<void> &surfaceControl, int32_t z)
        {
            return reinterpret_cast<types::apis::libgui::generic::SurfaceComposerClient__Transaction__SetLayer>(apis::libgui::SurfaceComposerClient::Transaction::Api.SetLayer)(data, surfaceControl, z);
        }

        void *SetLayerStack(types::StrongPointer<void> &surfaceControl, uint32_t layerStack)
        {
            return reinterpret_cast<types::apis::libgui::generic::SurfaceComposerClient__Transaction__SetLayerStack>(apis::libgui::SurfaceComposerClient::Transaction::Api.SetLayerStack)(data, surfaceControl, layerStack);
        }

        void *SetTrustedOverlay(types::StrongPointer<void> &surfaceControl, bool isTrustedOverlay)
        {
            return reinterpret_cast<types::apis::libgui::generic::SurfaceComposerClient__Transaction__SetTrustedOverlay>(apis::libgui::SurfaceComposerClient::Transaction::Api.SetTrustedOverlay)(data, surfaceControl, isTrustedOverlay);
        }

        void Show(types::StrongPointer<void> &surfaceControl)
        {
            reinterpret_cast<types::apis::libgui::generic::SurfaceComposerClient__Transaction__Show>(apis::libgui::SurfaceComposerClient::Transaction::Api.Show)(data, surfaceControl);
        }

        void Hide(types::StrongPointer<void> &surfaceControl)
        {
            reinterpret_cast<types::apis::libgui::generic::SurfaceComposerClient__Transaction__Hide>(apis::libgui::SurfaceComposerClient::Transaction::Api.Hide)(data, surfaceControl);
        }

        void Reparent(types::StrongPointer<void> &surfaceControl, types::StrongPointer<void> &newParentHandle)
        {
            reinterpret_cast<types::apis::libgui::generic::SurfaceComposerClient__Transaction__Reparent>(apis::libgui::SurfaceComposerClient::Transaction::Api.Reparent)(data, surfaceControl, newParentHandle);
        }

        int32_t Apply(bool synchronous, bool oneWay)
        {
            if (13 > SystemVersion)
                return reinterpret_cast<types::apis::libgui::v8_v12::SurfaceComposerClient__Transaction__Apply>(apis::libgui::SurfaceComposerClient::Transaction::Api.Apply)(data, synchronous);
            else
                return reinterpret_cast<types::apis::libgui::v13_v16::SurfaceComposerClient__Transaction__Apply>(apis::libgui::SurfaceComposerClient::Transaction::Api.Apply)(data, synchronous, oneWay);
        }
    };

    struct SurfaceComposerClient
    {
        char data[1024];

        SurfaceComposerClient()
        {
            reinterpret_cast<types::apis::libgui::generic::SurfaceComposerClient__Constructor>(apis::libgui::SurfaceComposerClient::Api.Constructor)(data);
            reinterpret_cast<types::apis::libutils::generic::RefBase__IncStrong>(apis::libutils::RefBase::Api.IncStrong)(data, this);
        }

        SurfaceControl CreateSurface(const char *name, int32_t width, int32_t height, types::WindowFlags windowFlags = {})
        {
            static void *parentHandle = nullptr;

            parentHandle = nullptr;
            String8 windowName(name);
            types::PixelFormat pixelFormat = types::PixelFormat::RGBA_8888;
            LayerMetadata layerMetadata{};

            types::StrongPointer<void> result{};
            switch (SystemVersion)
            {
            case 5:
            case 6:
            case 7:
            {
                result = reinterpret_cast<types::apis::libgui::v5_v7::SurfaceComposerClient__CreateSurface>(apis::libgui::SurfaceComposerClient::Api.CreateSurface)(data, windowName, width, height, pixelFormat, windowFlags);
                break;
            }
            case 8:
            case 9:
            {
                uint32_t windowType = 0;

                result = reinterpret_cast<types::apis::libgui::v8_v9::SurfaceComposerClient__CreateSurface>(apis::libgui::SurfaceComposerClient::Api.CreateSurface)(data, windowName, width, height, pixelFormat, windowFlags, parentHandle, windowType, 0);
                break;
            }
            case 10:
            {
                result = reinterpret_cast<types::apis::libgui::v10::SurfaceComposerClient__CreateSurface>(apis::libgui::SurfaceComposerClient::Api.CreateSurface)(data, windowName, width, height, pixelFormat, windowFlags, parentHandle, layerMetadata);
                break;
            }
            case 11:
            {
                result = reinterpret_cast<types::apis::libgui::v11::SurfaceComposerClient__CreateSurface>(apis::libgui::SurfaceComposerClient::Api.CreateSurface)(data, windowName, width, height, pixelFormat, windowFlags, parentHandle, layerMetadata, nullptr);
                break;
            }
            case 12:
            case 13:
            {
                result = reinterpret_cast<types::apis::libgui::v12_v13::SurfaceComposerClient__CreateSurface>(apis::libgui::SurfaceComposerClient::Api.CreateSurface)(data, windowName, width, height, pixelFormat, windowFlags, &parentHandle, layerMetadata, nullptr);
            }
            default:
            {
                result = reinterpret_cast<types::apis::libgui::v14_v16::SurfaceComposerClient__CreateSurface>(apis::libgui::SurfaceComposerClient::Api.CreateSurface)(data, windowName, width, height, pixelFormat, windowFlags, &parentHandle, layerMetadata, nullptr);
                break;
            }
            }

            if (12 <= SystemVersion)
            {
                static SurfaceComposerClientTransaction transaction;

                transaction.SetTrustedOverlay(result, true);
                transaction.SetLayer(result, INT_MAX);
                transaction.Apply(false, true);
            }
            else if (8 >= SystemVersion)
            {
                OpenGlobalTransaction();
                SurfaceControl{result.get()}.SetLayer(INT_MAX);
                CloseGlobalTransaction(false);
            }

            return {result.get()};
        }

        SurfaceControl MirrorSurface(SurfaceControl &surface, uint32_t layerStack)
        {
            using mirror_surfaces_t = std::pair<void *, void *>;
            constexpr auto MirrorSurfacesDeleter = [](mirror_surfaces_t *pair)
            {
                SurfaceControl fakeSurface;

                reinterpret_cast<types::apis::libgui::generic::SurfaceControl__DisConnect>(apis::libgui::SurfaceControl::Api.DisConnect)(pair->first);
                reinterpret_cast<types::apis::libutils::generic::RefBase__DecStrong>(apis::libutils::RefBase::Api.DecStrong)(pair->first, fakeSurface.data);

                reinterpret_cast<types::apis::libgui::generic::SurfaceControl__DisConnect>(apis::libgui::SurfaceControl::Api.DisConnect)(pair->second);
                reinterpret_cast<types::apis::libutils::generic::RefBase__DecStrong>(apis::libutils::RefBase::Api.DecStrong)(pair->second, fakeSurface.data);

                delete pair;
            };

            using mirror_surfaces_proxy_t = std::unique_ptr<mirror_surfaces_t, decltype(MirrorSurfacesDeleter)>;

            if (14 > compat::SystemVersion)
                return {};

            auto mirrorSurface = reinterpret_cast<types::apis::libgui::generic::SurfaceComposerClient__MirrorSurface>(apis::libgui::SurfaceComposerClient::Api.MirrorSurface)(data, surface.data);
            if (nullptr == mirrorSurface.get())
            {
                LogError("[-] Failed to mirror surface: %u", layerStack);
                return {};
            }

            int32_t width = -1, height = -1;
            while (-1 == width || -1 == height)
            {
                detail::types::ui::DisplayState displayInfo{};

                if (!GetDisplayInfo(&displayInfo))
                    break;

                width = displayInfo.layerStackSpaceRect.width;
                height = displayInfo.layerStackSpaceRect.height;

                break;
            }
            auto mirrorRootName = "MirrorRoot@" + std::to_string(layerStack);
            auto mirrorRootSurface = CreateSurface(mirrorRootName.data(), width, height, types::WindowFlags::eNoColorFill);
            if (!mirrorRootSurface)
            {
                LogError("[-] Failed to create mirror root surface: %u", layerStack);
                return {};
            }

            static SurfaceComposerClientTransaction transaction;
            static std::vector<mirror_surfaces_proxy_t> mirrorSurfaces;

            transaction.SetLayer(mirrorRootSurface, INT_MAX);
            transaction.SetLayerStack(mirrorRootSurface, layerStack);
            transaction.Apply(false, true);

            transaction.SetLayerStack(mirrorSurface, layerStack);
            transaction.Show(mirrorSurface);
            transaction.Reparent(mirrorSurface, mirrorRootSurface);
            transaction.Apply(false, true);

            mirrorSurfaces.emplace_back(new mirror_surfaces_t{mirrorSurface.get(), mirrorRootSurface.data}, MirrorSurfacesDeleter);

            return {mirrorSurface.get()};
        }

        bool GetDisplayInfo(types::ui::DisplayState *displayInfo)
        {
            types::StrongPointer<void> defaultDisplay;

            if (9 >= SystemVersion)
                defaultDisplay = reinterpret_cast<types::apis::libgui::generic::SurfaceComposerClient__GetBuiltInDisplay__Static>(apis::libgui::SurfaceComposerClient::Api.GetBuiltInDisplay)(types::ui::DisplayType::DisplayIdMain);
            else
            {
                if (14 > SystemVersion)
                    defaultDisplay = reinterpret_cast<types::apis::libgui::generic::SurfaceComposerClient__GetInternalDisplayToken__Static>(apis::libgui::SurfaceComposerClient::Api.GetInternalDisplayToken)();
                else
                {
                    auto displayIds = reinterpret_cast<types::apis::libgui::generic::SurfaceComposerClient__GetPhysicalDisplayIds__Static>(apis::libgui::SurfaceComposerClient::Api.GetPhysicalDisplayIds)();
                    if (displayIds.empty())
                        return false;

                    defaultDisplay = reinterpret_cast<types::apis::libgui::generic::SurfaceComposerClient__GetPhysicalDisplayToken__Static>(apis::libgui::SurfaceComposerClient::Api.GetPhysicalDisplayToken)(displayIds[0]);
                }
            }

            if (nullptr == defaultDisplay.get())
                return false;

            if (11 <= SystemVersion)
                return 0 == reinterpret_cast<types::apis::libgui::generic::SurfaceComposerClient__GetDisplayState__Static>(apis::libgui::SurfaceComposerClient::Api.GetDisplayState)(defaultDisplay, displayInfo);
            else
            {
                types::ui::DisplayInfo realDisplayInfo{};
                if (0 != reinterpret_cast<types::apis::libgui::generic::SurfaceComposerClient__GetDisplayInfo__Static>(apis::libgui::SurfaceComposerClient::Api.GetDisplayInfo)(defaultDisplay, &realDisplayInfo))
                    return false;

                displayInfo->layerStackSpaceRect.width = realDisplayInfo.w;
                displayInfo->layerStackSpaceRect.height = realDisplayInfo.h;
                displayInfo->orientation = static_cast<types::ui::Rotation>(realDisplayInfo.orientation);

                return true;
            }
        }

        void OpenGlobalTransaction()
        {
            reinterpret_cast<types::apis::libgui::v5_v7::SurfaceComposerClient__OpenGlobalTransaction__Static>(apis::libgui::SurfaceComposerClient::Api.OpenGlobalTransaction)();
        }

        void CloseGlobalTransaction(bool synchronous)
        {
            reinterpret_cast<types::apis::libgui::v5_v7::SurfaceComposerClient__CloseGlobalTransaction__Static>(apis::libgui::SurfaceComposerClient::Api.CloseGlobalTransaction)(synchronous);
        }
    };
} // namespace android::detail::compat

namespace android::detail
{
    struct ApiTableDescriptor
    {
        static const auto GetDefaultDescriptors()
        {
            using namespace android::detail::types::apis;

            return std::make_pair(
                // libutils
                std::to_array({
                    // RefBase
                    ApiDescriptor{5, UINT_MAX, &apis::libutils::RefBase::Api.IncStrong, "_ZNK7android7RefBase9incStrongEPKv"},
                    ApiDescriptor{5, UINT_MAX, &apis::libutils::RefBase::Api.DecStrong, "_ZNK7android7RefBase9decStrongEPKv"},

                    // String8
                    ApiDescriptor{5, UINT_MAX, &apis::libutils::String8::Api.Constructor, "_ZN7android7String8C2EPKc"},
                    ApiDescriptor{5, UINT_MAX, &apis::libutils::String8::Api.Destructor, "_ZN7android7String8D2Ev"},
                }),
                // libgui
                std::to_array({
                    // LayerMetadata
                    ApiDescriptor{10, 13, &apis::libgui::LayerMetadata::Api.Constructor, "_ZN7android13LayerMetadataC2Ev"},
                    ApiDescriptor{14, UINT_MAX, &apis::libgui::LayerMetadata::Api.Constructor, "_ZN7android3gui13LayerMetadataC2Ev"},

                    // SurfaceComposerClient
                    ApiDescriptor{5, UINT_MAX, &apis::libgui::SurfaceComposerClient::Api.Constructor, "_ZN7android21SurfaceComposerClientC2Ev"},

                    ApiDescriptor{5, 7, &apis::libgui::SurfaceComposerClient::Api.CreateSurface, "_ZN7android21SurfaceComposerClient13createSurfaceERKNS_7String8Ejjij"},
                    ApiDescriptor{8, 8, &apis::libgui::SurfaceComposerClient::Api.CreateSurface, "_ZN7android21SurfaceComposerClient13createSurfaceERKNS_7String8EjjijPNS_14SurfaceControlEjj"},
                    ApiDescriptor{9, 9, &apis::libgui::SurfaceComposerClient::Api.CreateSurface, "_ZN7android21SurfaceComposerClient13createSurfaceERKNS_7String8EjjijPNS_14SurfaceControlEii"},
                    ApiDescriptor{10, 10, &apis::libgui::SurfaceComposerClient::Api.CreateSurface, "_ZN7android21SurfaceComposerClient13createSurfaceERKNS_7String8EjjijPNS_14SurfaceControlENS_13LayerMetadataE"},
                    ApiDescriptor{11, 11, &apis::libgui::SurfaceComposerClient::Api.CreateSurface, "_ZN7android21SurfaceComposerClient13createSurfaceERKNS_7String8EjjijPNS_14SurfaceControlENS_13LayerMetadataEPj"},
                    ApiDescriptor{12, 13, &apis::libgui::SurfaceComposerClient::Api.CreateSurface, "_ZN7android21SurfaceComposerClient13createSurfaceERKNS_7String8EjjijRKNS_2spINS_7IBinderEEENS_13LayerMetadataEPj"},
                    ApiDescriptor{14, UINT_MAX, &apis::libgui::SurfaceComposerClient::Api.CreateSurface, "_ZN7android21SurfaceComposerClient13createSurfaceERKNS_7String8EjjiiRKNS_2spINS_7IBinderEEENS_3gui13LayerMetadataEPj"},

                    ApiDescriptor{11, UINT_MAX, &apis::libgui::SurfaceComposerClient::Api.MirrorSurface, "_ZN7android21SurfaceComposerClient13mirrorSurfaceEPNS_14SurfaceControlE"},

                    ApiDescriptor{5, 9, &apis::libgui::SurfaceComposerClient::Api.GetBuiltInDisplay, "_ZN7android21SurfaceComposerClient17getBuiltInDisplayEi"},
                    ApiDescriptor{10, 13, &apis::libgui::SurfaceComposerClient::Api.GetInternalDisplayToken, "_ZN7android21SurfaceComposerClient23getInternalDisplayTokenEv"},
                    ApiDescriptor{10, UINT_MAX, &apis::libgui::SurfaceComposerClient::Api.GetPhysicalDisplayIds, "_ZN7android21SurfaceComposerClient21getPhysicalDisplayIdsEv"},
                    ApiDescriptor{12, UINT_MAX, &apis::libgui::SurfaceComposerClient::Api.GetPhysicalDisplayToken, "_ZN7android21SurfaceComposerClient23getPhysicalDisplayTokenENS_17PhysicalDisplayIdE"},

                    ApiDescriptor{5, 11, &apis::libgui::SurfaceComposerClient::Api.GetDisplayInfo, "_ZN7android21SurfaceComposerClient14getDisplayInfoERKNS_2spINS_7IBinderEEEPNS_11DisplayInfoE"},
                    ApiDescriptor{11, UINT_MAX, &apis::libgui::SurfaceComposerClient::Api.GetDisplayState, "_ZN7android21SurfaceComposerClient15getDisplayStateERKNS_2spINS_7IBinderEEEPNS_2ui12DisplayStateE"},

                    // SurfaceComposerClient::Transaction
                    ApiDescriptor{12, UINT_MAX, &apis::libgui::SurfaceComposerClient::Transaction::Api.Constructor, "_ZN7android21SurfaceComposerClient11TransactionC2Ev"},
                    ApiDescriptor{9, UINT_MAX, &apis::libgui::SurfaceComposerClient::Transaction::Api.SetLayer, "_ZN7android21SurfaceComposerClient11Transaction8setLayerERKNS_2spINS_14SurfaceControlEEEi"},
                    ApiDescriptor{12, UINT_MAX, &apis::libgui::SurfaceComposerClient::Transaction::Api.SetTrustedOverlay, "_ZN7android21SurfaceComposerClient11Transaction17setTrustedOverlayERKNS_2spINS_14SurfaceControlEEEb"},

                    ApiDescriptor{9, 12, &apis::libgui::SurfaceComposerClient::Transaction::Api.Apply, "_ZN7android21SurfaceComposerClient11Transaction5applyEb"},
                    ApiDescriptor{13, UINT_MAX, &apis::libgui::SurfaceComposerClient::Transaction::Api.Apply, "_ZN7android21SurfaceComposerClient11Transaction5applyEbb"},

                    ApiDescriptor{13, UINT_MAX, &apis::libgui::SurfaceComposerClient::Transaction::Api.SetLayerStack, "_ZN7android21SurfaceComposerClient11Transaction13setLayerStackERKNS_2spINS_14SurfaceControlEEENS_2ui10LayerStackE"},
                    ApiDescriptor{9, UINT_MAX, &apis::libgui::SurfaceComposerClient::Transaction::Api.Show, "_ZN7android21SurfaceComposerClient11Transaction4showERKNS_2spINS_14SurfaceControlEEE"},
                    ApiDescriptor{9, UINT_MAX, &apis::libgui::SurfaceComposerClient::Transaction::Api.Hide, "_ZN7android21SurfaceComposerClient11Transaction4hideERKNS_2spINS_14SurfaceControlEEE"},
                    ApiDescriptor{12, UINT_MAX, &apis::libgui::SurfaceComposerClient::Transaction::Api.Reparent, "_ZN7android21SurfaceComposerClient11Transaction8reparentERKNS_2spINS_14SurfaceControlEEES6_"},

                    // SurfaceComposerClient::GlobalTransaction
                    ApiDescriptor{5, 8, &apis::libgui::SurfaceComposerClient::Api.OpenGlobalTransaction, "_ZN7android21SurfaceComposerClient21openGlobalTransactionEv"},
                    ApiDescriptor{5, 8, &apis::libgui::SurfaceComposerClient::Api.CloseGlobalTransaction, "_ZN7android21SurfaceComposerClient22closeGlobalTransactionEb"},

                    // SurfaceControl
                    ApiDescriptor{5, 11, &apis::libgui::SurfaceControl::Api.GetSurface, "_ZNK7android14SurfaceControl10getSurfaceEv"},
                    ApiDescriptor{12, UINT_MAX, &apis::libgui::SurfaceControl::Api.GetSurface, "_ZN7android14SurfaceControl10getSurfaceEv"},

                    ApiDescriptor{5, 7, &apis::libgui::Surface::Api.DisConnect, "_ZN7android7Surface10disconnectEi"},
                    ApiDescriptor{7, UINT_MAX, &apis::libgui::SurfaceControl::Api.DisConnect, "_ZN7android14SurfaceControl10disconnectEv"},

                    ApiDescriptor{5, 5, &apis::libgui::SurfaceControl::Api.SetLayer, "_ZN7android14SurfaceControl8setLayerEi"},
                    ApiDescriptor{6, 7, &apis::libgui::SurfaceControl::Api.SetLayer, "_ZN7android14SurfaceControl8setLayerEj"},
                    ApiDescriptor{8, 8, &apis::libgui::SurfaceControl::Api.SetLayer, "_ZN7android14SurfaceControl8setLayerEi"},
                }));
        }
    };

    struct ApiResolver
    {
        struct ResolverImpl
        {
            void *(*Open)(const char *filename, int flag);
            void *(*Resolve)(void *handle, const char *symbol);
            int (*Close)(void *handle);

            ResolverImpl()
                : Open(dlopen),
                  Resolve(dlsym),
                  Close(dlclose)
            {
            }

            ResolverImpl(decltype(Open) open, decltype(Resolve) resolve, decltype(Close) close)
                : Open(open),
                  Resolve(resolve),
                  Close(close)
            {
            }
        };

        static void Resolve(const ResolverImpl &resolver = {})
        {
            static bool resolved = false;

            if (resolved)
                return;

            std::string systemVersionString(128, 0);
            systemVersionString.resize(__system_property_get("ro.build.version.release", systemVersionString.data()));
            if (!systemVersionString.empty())
                compat::SystemVersion = std::stoi(systemVersionString);

            if (compat::SupportedMinVersion > compat::SystemVersion)
            {
                LogError("[!] Unsupported system version: %zu", compat::SystemVersion);
                return;
            }

            LogInfo("[*] Starting to resolve symbols for Android %zu", compat::SystemVersion);

#ifdef __LP64__
            auto libgui = resolver.Open("/system/lib64/libgui.so", RTLD_LAZY);
            auto libutils = resolver.Open("/system/lib64/libutils.so", RTLD_LAZY);
#else
            auto libgui = resolver.Open("/system/lib/libgui.so", RTLD_LAZY);
            auto libutils = resolver.Open("/system/lib/libutils.so", RTLD_LAZY);
#endif
            if (nullptr == libgui || nullptr == libutils)
            {
                LogError("[!] Failed to open libgui.so or libutils.so");
                throw std::runtime_error("Failed to open libgui.so or libutils.so");
            }

            const auto &[libutilsApis, libguiApis] = ApiTableDescriptor::GetDefaultDescriptors();
            for (const auto &descriptor : libutilsApis)
            {
                if (!descriptor.IsSupported(compat::SystemVersion))
                    continue;

                *descriptor.storeToTarget = resolver.Resolve(libutils, descriptor.apiSignature);
                if (nullptr == *descriptor.storeToTarget)
                {
                    LogError("[!] Version[Android %zu] [libutils] failed to resolve symbol: %s", compat::SystemVersion, descriptor.apiSignature);
                    throw std::runtime_error("Failed to resolve symbol");
                }
            }

            for (const auto &descriptor : libguiApis)
            {
                if (!descriptor.IsSupported(compat::SystemVersion))
                    continue;

                *descriptor.storeToTarget = resolver.Resolve(libgui, descriptor.apiSignature);
                if (nullptr == *descriptor.storeToTarget)
                {
                    LogError("[!] Version[Android %zu] [libgui] failed to resolve symbol: %s", compat::SystemVersion, descriptor.apiSignature);
                    throw std::runtime_error("Failed to resolve symbol");
                }
            }

            resolver.Close(libutils);
            resolver.Close(libgui);
            resolved = true;
            LogInfo("[+] Version[Android %zu] resolved all symbols", compat::SystemVersion);
        }
    };

    inline std::vector<std::pair<std::string, std::string>> ParseDisplayInfo(const std::string_view &displayInfo)
    {
        constexpr auto SubStringView = [](const std::string_view &str, std::string_view start, std::string_view end, int startOffset = 0) -> std::string_view
        {
            auto startIt = str.find(start, startOffset);
            if (std::string::npos == startIt)
                return {};

            auto endIt = str.find(end, startIt + start.size());
            if (std::string::npos == endIt)
                return {};

            return str.substr(startIt + start.size(), endIt - startIt - start.size());
        };

        std::vector<std::pair<std::string, std::string>> result;

        // DisplayDeviceInfo
        auto displayInfoIt = std::string_view::npos;
        while (std::string_view::npos != (displayInfoIt = displayInfo.find("DisplayDeviceInfo", displayInfoIt + 1)))
        {
            auto uniqueId = SubStringView(displayInfo, "mUniqueId=", "\n", displayInfoIt);
            auto currentLayerStack = SubStringView(displayInfo, "mCurrentLayerStack=", "\n", displayInfoIt);

            if ("-1" == currentLayerStack)
            {
                LogError("[-] %s -> Current layer stack is -1, skipping", std::string{uniqueId.begin(), uniqueId.end()}.data());

                continue;
            }

            result.emplace_back(std::string{uniqueId.begin(), uniqueId.end()}, std::string{currentLayerStack.begin(), currentLayerStack.end()});
        }

        return result;
    }
} // namespace android::detail

namespace android
{
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
        static void SetupCustomApiResolver(const detail::ApiResolver::ResolverImpl &resolver)
        {
            detail::ApiResolver::Resolve(resolver);
        }

    public:
        static detail::compat::SurfaceComposerClient &GetComposerInstance()
        {
            detail::ApiResolver::Resolve();

            static detail::compat::SurfaceComposerClient surfaceComposerClient;

            return surfaceComposerClient;
        }

        static DisplayInfo GetDisplayInfo()
        {
            auto &surfaceComposerClient = GetComposerInstance();
            detail::types::ui::DisplayState displayInfo{};

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
                detail::types::ui::DisplayState displayInfo{};

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

            m_cachedSurfaceControl[nativeWindow].DestroySurface(reinterpret_cast<detail::compat::Surface *>(nativeWindow));
            m_cachedSurfaceControl.erase(nativeWindow);
        }

        static void ProcessMirrorDisplay()
        {
            static std::chrono::steady_clock::time_point lastTime{};

            if (14 > detail::compat::SystemVersion)
                return;
            if (std::chrono::steady_clock::now() - lastTime < std::chrono::seconds(1))
                return;

            // Run "dumpsys display" and get result
            auto pipe = popen("dumpsys display", "r");
            if (!pipe)
            {
                LogError("[-] Failed to run dumpsys command");
                return;
            }

            char buffer[512]{};
            std::string result;
            while (fgets(buffer, sizeof(buffer), pipe) != nullptr)
                result += buffer;
            pclose(pipe);

            static std::unordered_map<std::string, detail::compat::SurfaceControl> cachedLayerStackMirrorSurfaces;

            auto displayInfo = detail::ParseDisplayInfo(result);
            for (auto &[id, layerStack] : displayInfo)
            {
                if ("0" == layerStack || cachedLayerStackMirrorSurfaces.contains(layerStack))
                    continue;

                LogInfo("[=] New display layerstack detected: [%s] -> %s", id.data(), layerStack.data());

                for (auto &[_, surfaceControl] : m_cachedSurfaceControl)
                {
                    auto mirrorLayer = GetComposerInstance().MirrorSurface(surfaceControl, std::stoi(layerStack));

                    LogInfo("[+] Mirror layer created: %p", mirrorLayer.data);
                    cachedLayerStackMirrorSurfaces.emplace(std::move(layerStack), std::move(mirrorLayer));
                }
            }

            lastTime = std::chrono::steady_clock::now();
        }

    private:
        inline static std::unordered_map<ANativeWindow *, detail::compat::SurfaceControl> m_cachedSurfaceControl;
    };
} // namespace android

#if LOGTAG_DEFINED
#undef LOGTAG

#undef LOGTAG_DEFINED
#endif // LOGTAG_DEFINED

#if LogInfo_DEFINED
#undef LogInfo
#undef LogDebug
#undef LogError

#undef LogInfo_DEFINED
#endif // LogInfo_DEFINED

#endif // !A_NATIVE_WINDOW_CREATOR_H