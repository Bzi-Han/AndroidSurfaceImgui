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
#include <unordered_set>

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

    enum class WindowFlags : uint32_t
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

    enum class MetadataType : uint32_t
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

    enum
    {
        WINDOW_TYPE_DONT_SCREENSHOT = 441731
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

    template <typename enum_t>
    constexpr enum_t operator|(enum_t lhs, enum_t rhs)
        requires std::is_enum_v<enum_t>
    {
        using underlying_t = std::underlying_type_t<enum_t>;

        return static_cast<enum_t>(static_cast<underlying_t>(lhs) | static_cast<underlying_t>(rhs));
    }

    template <typename enum_t>
    constexpr enum_t operator|=(enum_t &lhs, enum_t rhs)
        requires std::is_enum_v<enum_t>
    {
        return (lhs = lhs | rhs);
    }
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

    namespace v14_infinite
    {
        // SurfaceComposerClient::createSurface(const String8& name, uint32_t w, uint32_t h,
        //         PixelFormat format, int32_t flags,
        //         const sp<IBinder>& parentHandle,
        //         LayerMetadata metadata,
        //         uint32_t* outTransformHint)
        using SurfaceComposerClient__CreateSurface = StrongPointer<void> (*)(void *thiz, void *name, uint32_t w, uint32_t h, PixelFormat format, WindowFlags flags, void **parentHandle, void *metadata, uint32_t *outTransformHint);
    } // namespace v14_infinite

    namespace v8_v12
    {
        using SurfaceComposerClient__Transaction__Apply = int32_t (*)(void *thiz, bool synchronous);
    } // namespace v8_v12

    namespace v13_infinite
    {
        using SurfaceComposerClient__Transaction__Apply = int32_t (*)(void *thiz, bool synchronous, bool oneWay);
    } // namespace v13_infinite

    namespace generic
    {
        using LayerMetadata__Constructor = void (*)(void *thiz);
        using LayerMetadata__SetInt32 = void (*)(void *thiz, MetadataType key, int32_t value);

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
        using SurfaceComposerClient__Transaction__SetMatrix = void *(*)(void *thiz, StrongPointer<void> &surfaceControl, float dsdx, float dtdx, float dtdy, float dsdy);
        using SurfaceComposerClient__Transaction__SetPosition = void *(*)(void *thiz, StrongPointer<void> &surfaceControl, float x, float y);

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
                void *SetInt32;
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
                void *SetMatrix;
                void *SetPosition;
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

    template <size_t length>
    struct ApiInvokeDescriptor
    {
        size_t api;
        size_t version;

        consteval size_t DataHash(const std::string_view &data) const
        {
            constexpr size_t fnvOffsetBasis = 0x811C9DC5ull;
            constexpr size_t fnvPrime = 0x1000193ull;
            size_t hash = fnvOffsetBasis;

            for (size_t i = 0; i < data.size(); ++i)
            {
                hash ^= static_cast<size_t>(data[i]);
                hash *= fnvPrime;
            }

            return hash;
        }

        consteval bool operator==(const char *rhs) const
        {
            return api == DataHash(rhs);
        }

        consteval ApiInvokeDescriptor(const char (&apiInvokeName)[length])
        {
            const std::string_view apiInvokeNameView{apiInvokeName};
            const auto invokeNameDelimiter = apiInvokeNameView.find("@");

            if (std::string_view::npos == invokeNameDelimiter)
            {
                api = DataHash(apiInvokeName);
                version = SupportedMinVersion;
            }
            else
            {
                if ('v' != apiInvokeNameView[invokeNameDelimiter + 1])
                    throw std::invalid_argument("Invalid version string, must start with 'v'");

                const auto invokeName = apiInvokeNameView.substr(0, invokeNameDelimiter);
                const auto invokeVersion = apiInvokeNameView.substr(invokeNameDelimiter + 2);

                api = DataHash(invokeName);

                version = 0;
                for (char c : invokeVersion)
                {
                    if (c < '0' || c > '9')
                        throw std::invalid_argument("Invalid version string, must be digits only");

                    version = version * 10 + (c - '0');
                }
            }
        }
    };

    template <ApiInvokeDescriptor descriptor>
    constexpr auto ApiInvoker()
    {
        // libutils
        if constexpr ("RefBase::IncStrong" == descriptor)
            return reinterpret_cast<types::apis::libutils::generic::RefBase__IncStrong>(apis::libutils::RefBase::Api.IncStrong);
        if constexpr ("RefBase::DecStrong" == descriptor)
            return reinterpret_cast<types::apis::libutils::generic::RefBase__DecStrong>(apis::libutils::RefBase::Api.DecStrong);

        if constexpr ("String8::Constructor" == descriptor)
            return reinterpret_cast<types::apis::libutils::generic::String8__Constructor>(apis::libutils::String8::Api.Constructor);
        if constexpr ("String8::Destructor" == descriptor)
            return reinterpret_cast<types::apis::libutils::generic::String8__Destructor>(apis::libutils::String8::Api.Destructor);

        // libgui
        if constexpr ("LayerMetadata::Constructor" == descriptor)
            return reinterpret_cast<types::apis::libgui::generic::LayerMetadata__Constructor>(apis::libgui::LayerMetadata::Api.Constructor);
        if constexpr ("LayerMetadata::SetInt32" == descriptor)
            return reinterpret_cast<types::apis::libgui::generic::LayerMetadata__SetInt32>(apis::libgui::LayerMetadata::Api.SetInt32);

        if constexpr ("SurfaceControl::GetSurface" == descriptor)
            return reinterpret_cast<types::apis::libgui::generic::SurfaceControl__GetSurface>(apis::libgui::SurfaceControl::Api.GetSurface);
        if constexpr ("SurfaceControl::DisConnect" == descriptor)
            return reinterpret_cast<types::apis::libgui::generic::SurfaceControl__DisConnect>(apis::libgui::SurfaceControl::Api.DisConnect);
        if constexpr ("SurfaceControl::SetLayer" == descriptor)
            return reinterpret_cast<types::apis::libgui::v5_v7::SurfaceControl__SetLayer>(apis::libgui::SurfaceControl::Api.SetLayer);

        if constexpr ("Surface::DisConnect" == descriptor)
            return reinterpret_cast<types::apis::libgui::v5_v7::Surface__DisConnect>(apis::libgui::Surface::Api.DisConnect);

        if constexpr ("SurfaceComposerClient::Transaction::Constructor" == descriptor)
            return reinterpret_cast<types::apis::libgui::generic::SurfaceComposerClient__Transaction__Constructor>(apis::libgui::SurfaceComposerClient::Transaction::Api.Constructor);
        if constexpr ("SurfaceComposerClient::Transaction::SetLayer" == descriptor)
            return reinterpret_cast<types::apis::libgui::generic::SurfaceComposerClient__Transaction__SetLayer>(apis::libgui::SurfaceComposerClient::Transaction::Api.SetLayer);
        if constexpr ("SurfaceComposerClient::Transaction::SetLayerStack" == descriptor)
            return reinterpret_cast<types::apis::libgui::generic::SurfaceComposerClient__Transaction__SetLayerStack>(apis::libgui::SurfaceComposerClient::Transaction::Api.SetLayerStack);
        if constexpr ("SurfaceComposerClient::Transaction::SetTrustedOverlay" == descriptor)
            return reinterpret_cast<types::apis::libgui::generic::SurfaceComposerClient__Transaction__SetTrustedOverlay>(apis::libgui::SurfaceComposerClient::Transaction::Api.SetTrustedOverlay);
        if constexpr ("SurfaceComposerClient::Transaction::Show" == descriptor)
            return reinterpret_cast<types::apis::libgui::generic::SurfaceComposerClient__Transaction__Show>(apis::libgui::SurfaceComposerClient::Transaction::Api.Show);
        if constexpr ("SurfaceComposerClient::Transaction::Hide" == descriptor)
            return reinterpret_cast<types::apis::libgui::generic::SurfaceComposerClient__Transaction__Hide>(apis::libgui::SurfaceComposerClient::Transaction::Api.Hide);
        if constexpr ("SurfaceComposerClient::Transaction::Reparent" == descriptor)
            return reinterpret_cast<types::apis::libgui::generic::SurfaceComposerClient__Transaction__Reparent>(apis::libgui::SurfaceComposerClient::Transaction::Api.Reparent);
        if constexpr ("SurfaceComposerClient::Transaction::SetMatrix" == descriptor)
            return reinterpret_cast<types::apis::libgui::generic::SurfaceComposerClient__Transaction__SetMatrix>(apis::libgui::SurfaceComposerClient::Transaction::Api.SetMatrix);
        if constexpr ("SurfaceComposerClient::Transaction::SetPosition" == descriptor)
            return reinterpret_cast<types::apis::libgui::generic::SurfaceComposerClient__Transaction__SetPosition>(apis::libgui::SurfaceComposerClient::Transaction::Api.SetPosition);
        if constexpr ("SurfaceComposerClient::Transaction::Apply" == descriptor)
        {
            if constexpr (8 <= descriptor.version && 12 >= descriptor.version)
                return reinterpret_cast<types::apis::libgui::v8_v12::SurfaceComposerClient__Transaction__Apply>(apis::libgui::SurfaceComposerClient::Transaction::Api.Apply);
            else if constexpr (13 <= descriptor.version)
                return reinterpret_cast<types::apis::libgui::v13_infinite::SurfaceComposerClient__Transaction__Apply>(apis::libgui::SurfaceComposerClient::Transaction::Api.Apply);
        }

        if constexpr ("SurfaceComposerClient::Constructor" == descriptor)
            return reinterpret_cast<types::apis::libgui::generic::SurfaceComposerClient__Constructor>(apis::libgui::SurfaceComposerClient::Api.Constructor);
        if constexpr ("SurfaceComposerClient::CreateSurface" == descriptor)
        {
            if constexpr (5 <= descriptor.version && 7 >= descriptor.version)
                return reinterpret_cast<types::apis::libgui::v5_v7::SurfaceComposerClient__CreateSurface>(apis::libgui::SurfaceComposerClient::Api.CreateSurface);
            else if constexpr (8 <= descriptor.version && 9 >= descriptor.version)
                return reinterpret_cast<types::apis::libgui::v8_v9::SurfaceComposerClient__CreateSurface>(apis::libgui::SurfaceComposerClient::Api.CreateSurface);
            else if constexpr (10 == descriptor.version)
                return reinterpret_cast<types::apis::libgui::v10::SurfaceComposerClient__CreateSurface>(apis::libgui::SurfaceComposerClient::Api.CreateSurface);
            else if constexpr (11 == descriptor.version)
                return reinterpret_cast<types::apis::libgui::v11::SurfaceComposerClient__CreateSurface>(apis::libgui::SurfaceComposerClient::Api.CreateSurface);
            else if constexpr (12 <= descriptor.version && 13 >= descriptor.version)
                return reinterpret_cast<types::apis::libgui::v12_v13::SurfaceComposerClient__CreateSurface>(apis::libgui::SurfaceComposerClient::Api.CreateSurface);
            else if constexpr (14 <= descriptor.version)
                return reinterpret_cast<types::apis::libgui::v14_infinite::SurfaceComposerClient__CreateSurface>(apis::libgui::SurfaceComposerClient::Api.CreateSurface);
        }
        if constexpr ("SurfaceComposerClient::MirrorSurface" == descriptor)
            return reinterpret_cast<types::apis::libgui::generic::SurfaceComposerClient__MirrorSurface>(apis::libgui::SurfaceComposerClient::Api.MirrorSurface);

        if constexpr ("SurfaceComposerClient::GetBuiltInDisplay" == descriptor)
            return reinterpret_cast<types::apis::libgui::generic::SurfaceComposerClient__GetBuiltInDisplay__Static>(apis::libgui::SurfaceComposerClient::Api.GetBuiltInDisplay);
        if constexpr ("SurfaceComposerClient::GetInternalDisplayToken" == descriptor)
            return reinterpret_cast<types::apis::libgui::generic::SurfaceComposerClient__GetInternalDisplayToken__Static>(apis::libgui::SurfaceComposerClient::Api.GetInternalDisplayToken);
        if constexpr ("SurfaceComposerClient::GetPhysicalDisplayIds" == descriptor)
            return reinterpret_cast<types::apis::libgui::generic::SurfaceComposerClient__GetPhysicalDisplayIds__Static>(apis::libgui::SurfaceComposerClient::Api.GetPhysicalDisplayIds);
        if constexpr ("SurfaceComposerClient::GetPhysicalDisplayToken" == descriptor)
            return reinterpret_cast<types::apis::libgui::generic::SurfaceComposerClient__GetPhysicalDisplayToken__Static>(apis::libgui::SurfaceComposerClient::Api.GetPhysicalDisplayToken);
        if constexpr ("SurfaceComposerClient::GetDisplayState" == descriptor)
            return reinterpret_cast<types::apis::libgui::generic::SurfaceComposerClient__GetDisplayState__Static>(apis::libgui::SurfaceComposerClient::Api.GetDisplayState);
        if constexpr ("SurfaceComposerClient::GetDisplayInfo" == descriptor)
            return reinterpret_cast<types::apis::libgui::generic::SurfaceComposerClient__GetDisplayInfo__Static>(apis::libgui::SurfaceComposerClient::Api.GetDisplayInfo);

        if constexpr ("SurfaceComposerClient::OpenGlobalTransaction" == descriptor)
            return reinterpret_cast<types::apis::libgui::v5_v7::SurfaceComposerClient__OpenGlobalTransaction__Static>(apis::libgui::SurfaceComposerClient::Api.OpenGlobalTransaction);
        if constexpr ("SurfaceComposerClient::CloseGlobalTransaction" == descriptor)
            return reinterpret_cast<types::apis::libgui::v5_v7::SurfaceComposerClient__CloseGlobalTransaction__Static>(apis::libgui::SurfaceComposerClient::Api.CloseGlobalTransaction);
    }

} // namespace android::detail::compat

namespace android::detail::compat
{
    static size_t SystemVersion = 13;

    struct String8
    {
        char data[1024];

        String8(const char *const string)
        {
            ApiInvoker<"String8::Constructor">()(data, string);
        }

        ~String8()
        {
            ApiInvoker<"String8::Destructor">()(data);
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
                ApiInvoker<"LayerMetadata::Constructor@v10">()(data);
        }

        void SetInt32(types::MetadataType key, int32_t value)
        {
            if (9 < SystemVersion)
                ApiInvoker<"LayerMetadata::SetInt32@v10">()(data, key, value);
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
        int32_t width, height;
        bool skipScreenshot;

        SurfaceControl(void *data = nullptr)
            : data(data),
              width(0),
              height(0),
              skipScreenshot(false)
        {
        }
        SurfaceControl(void *data, int32_t width, int32_t height, bool skipScreenshot)
            : data(data),
              width(width),
              height(height),
              skipScreenshot(skipScreenshot)
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

            auto result = ApiInvoker<"SurfaceControl::GetSurface">()(data);

            return reinterpret_cast<Surface *>(reinterpret_cast<size_t>(result.pointer) + sizeof(std::max_align_t) / 2);
        }

        void DisConnect()
        {
            if (nullptr == data)
                return;

            ApiInvoker<"SurfaceControl::DisConnect">()(data);
        }

        void SetLayer(int32_t z)
        {
            if (nullptr == data || 8 < SystemVersion)
                return;

            ApiInvoker<"SurfaceControl::SetLayer@v8">()(data, z);
        }

        void DestroySurface(Surface *surface)
        {
            if (nullptr == data || nullptr == surface)
                return;

            ApiInvoker<"RefBase::DecStrong">()(reinterpret_cast<Surface *>(reinterpret_cast<size_t>(surface) - sizeof(std::max_align_t) / 2), this);
            if (7 > SystemVersion)
                ApiInvoker<"Surface::DisConnect@v6">()(reinterpret_cast<Surface *>(reinterpret_cast<size_t>(surface) - sizeof(std::max_align_t) / 2), -1);
            else
                DisConnect();
            ApiInvoker<"RefBase::DecStrong">()(data, this);
        }
    };

    struct SurfaceComposerClientTransaction
    {
        char data[1024];

        SurfaceComposerClientTransaction()
        {
            ApiInvoker<"SurfaceComposerClient::Transaction::Constructor">()(data);
        }

        void *SetLayer(types::StrongPointer<void> &surfaceControl, int32_t z)
        {
            return ApiInvoker<"SurfaceComposerClient::Transaction::SetLayer">()(data, surfaceControl, z);
        }

        void *SetLayerStack(types::StrongPointer<void> &surfaceControl, uint32_t layerStack)
        {
            return ApiInvoker<"SurfaceComposerClient::Transaction::SetLayerStack">()(data, surfaceControl, layerStack);
        }

        void *SetTrustedOverlay(types::StrongPointer<void> &surfaceControl, bool isTrustedOverlay)
        {
            return ApiInvoker<"SurfaceComposerClient::Transaction::SetTrustedOverlay">()(data, surfaceControl, isTrustedOverlay);
        }

        void Show(types::StrongPointer<void> &surfaceControl)
        {
            ApiInvoker<"SurfaceComposerClient::Transaction::Show">()(data, surfaceControl);
        }

        void Hide(types::StrongPointer<void> &surfaceControl)
        {
            ApiInvoker<"SurfaceComposerClient::Transaction::Hide">()(data, surfaceControl);
        }

        void Reparent(types::StrongPointer<void> &surfaceControl, types::StrongPointer<void> &newParentHandle)
        {
            ApiInvoker<"SurfaceComposerClient::Transaction::Reparent">()(data, surfaceControl, newParentHandle);
        }

        void SetMatrix(types::StrongPointer<void> &surfaceControl, float dsdx, float dtdx, float dsdy, float dtdy)
        {
            ApiInvoker<"SurfaceComposerClient::Transaction::SetMatrix">()(data, surfaceControl, dsdx, dtdx, dsdy, dtdy);
        }

        void SetPosition(types::StrongPointer<void> &surfaceControl, float x, float y)
        {
            ApiInvoker<"SurfaceComposerClient::Transaction::SetPosition">()(data, surfaceControl, x, y);
        }

        int32_t Apply(bool synchronous, bool oneWay)
        {
            if (13 > SystemVersion)
                return ApiInvoker<"SurfaceComposerClient::Transaction::Apply@v12">()(data, synchronous);
            else
                return ApiInvoker<"SurfaceComposerClient::Transaction::Apply@v13">()(data, synchronous, oneWay);
        }
    };

    struct SurfaceComposerClient
    {
        char data[1024];

        SurfaceComposerClient()
        {
            ApiInvoker<"SurfaceComposerClient::Constructor">()(data);
            ApiInvoker<"RefBase::IncStrong">()(data, this);
        }

        SurfaceControl CreateSurface(const char *name, int32_t width, int32_t height, types::WindowFlags windowFlags = {}, bool skipScreenshot = false)
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
                result = ApiInvoker<"SurfaceComposerClient::CreateSurface@v7">()(data, windowName, width, height, pixelFormat, windowFlags);
                break;
            }
            case 8:
            case 9:
            {
                uint32_t windowType = skipScreenshot ? types::WINDOW_TYPE_DONT_SCREENSHOT : 0;

                result = ApiInvoker<"SurfaceComposerClient::CreateSurface@v9">()(data, windowName, width, height, pixelFormat, windowFlags, parentHandle, windowType, 0);
                break;
            }
            case 10:
            {
                if (skipScreenshot)
                    layerMetadata.SetInt32(types::MetadataType::WINDOW_TYPE, types::WINDOW_TYPE_DONT_SCREENSHOT);

                result = ApiInvoker<"SurfaceComposerClient::CreateSurface@v10">()(data, windowName, width, height, pixelFormat, windowFlags, parentHandle, layerMetadata);
                break;
            }
            case 11:
            {
                if (skipScreenshot)
                    layerMetadata.SetInt32(types::MetadataType::WINDOW_TYPE, types::WINDOW_TYPE_DONT_SCREENSHOT);

                result = ApiInvoker<"SurfaceComposerClient::CreateSurface@v11">()(data, windowName, width, height, pixelFormat, windowFlags, parentHandle, layerMetadata, nullptr);
                break;
            }
            case 12:
            case 13:
            {
                using types::operator|=;

                if (skipScreenshot)
                    windowFlags |= types::WindowFlags::eSkipScreenshot;

                result = ApiInvoker<"SurfaceComposerClient::CreateSurface@v13">()(data, windowName, width, height, pixelFormat, windowFlags, &parentHandle, layerMetadata, nullptr);
            }
            default:
            {
                using types::operator|=;

                if (skipScreenshot)
                    windowFlags |= types::WindowFlags::eSkipScreenshot;

                result = ApiInvoker<"SurfaceComposerClient::CreateSurface@v14">()(data, windowName, width, height, pixelFormat, windowFlags, &parentHandle, layerMetadata, nullptr);
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

            return {result.get(), width, height, skipScreenshot};
        }

        SurfaceControl MirrorSurface(SurfaceControl &surface, uint32_t layerStack)
        {
            using mirror_surfaces_t = std::pair<void *, void *>;
            constexpr auto MirrorSurfacesDeleter = [](mirror_surfaces_t *pair)
            {
                SurfaceControl fakeSurface;

                ApiInvoker<"SurfaceControl::DisConnect@v14">()(pair->first);
                ApiInvoker<"RefBase::DecStrong">()(pair->first, fakeSurface.data);

                ApiInvoker<"SurfaceControl::DisConnect@v14">()(pair->second);
                ApiInvoker<"RefBase::DecStrong">()(pair->second, fakeSurface.data);

                delete pair;
            };

            using mirror_surfaces_proxy_t = std::unique_ptr<mirror_surfaces_t, decltype(MirrorSurfacesDeleter)>;

            if (14 > compat::SystemVersion)
                return {};
            if (surface.skipScreenshot)
            {
                LogInfo("[=] Surface not need mirror: skipScreenshot is true");
                return {};
            }
            if (0 == surface.width || 0 == surface.height)
            {
                LogInfo("[=] Surface not need mirror: width or height is 0");
                return {};
            }

            auto mirrorSurface = ApiInvoker<"SurfaceComposerClient::MirrorSurface@v14">()(data, surface.data);
            if (nullptr == mirrorSurface.get())
            {
                LogError("[-] Failed to mirror surface: %u", layerStack);
                return {};
            }

            auto mirrorRootName = "MirrorRoot@" + std::to_string(layerStack);
            auto mirrorRootSurface = CreateSurface(mirrorRootName.data(), surface.width, surface.height, types::WindowFlags::eNoColorFill);
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

            return mirrorRootSurface;
        }

        void ZoomSurface(SurfaceControl &surface, float scaleX, float scaleY)
        {
            static SurfaceComposerClientTransaction transaction;

            transaction.SetMatrix(surface, scaleX, 0.f, 0.f, scaleY);
            transaction.Apply(false, true);
        }

        void MoveSurface(SurfaceControl &surface, float x, float y)
        {
            static SurfaceComposerClientTransaction transaction;

            transaction.SetPosition(surface, x, y);
            transaction.Apply(false, true);
        }

        bool GetDisplayInfo(types::ui::DisplayState *displayInfo)
        {
            types::StrongPointer<void> defaultDisplay;

            if (9 >= SystemVersion)
                defaultDisplay = ApiInvoker<"SurfaceComposerClient::GetBuiltInDisplay@v9">()(types::ui::DisplayType::DisplayIdMain);
            else
            {
                if (14 > SystemVersion)
                    defaultDisplay = ApiInvoker<"SurfaceComposerClient::GetInternalDisplayToken@v13">()();
                else
                {
                    auto displayIds = ApiInvoker<"SurfaceComposerClient::GetPhysicalDisplayIds@v14">()();
                    if (displayIds.empty())
                        return false;

                    defaultDisplay = ApiInvoker<"SurfaceComposerClient::GetPhysicalDisplayToken@v14">()(displayIds[0]);
                }
            }

            if (nullptr == defaultDisplay.get())
                return false;

            if (11 <= SystemVersion)
                return 0 == ApiInvoker<"SurfaceComposerClient::GetDisplayState@v11">()(defaultDisplay, displayInfo);
            else
            {
                types::ui::DisplayInfo realDisplayInfo{};
                if (0 != ApiInvoker<"SurfaceComposerClient::GetDisplayInfo@v10">()(defaultDisplay, &realDisplayInfo))
                    return false;

                displayInfo->layerStackSpaceRect.width = realDisplayInfo.w;
                displayInfo->layerStackSpaceRect.height = realDisplayInfo.h;
                displayInfo->orientation = static_cast<types::ui::Rotation>(realDisplayInfo.orientation);

                return true;
            }
        }

        void OpenGlobalTransaction()
        {
            ApiInvoker<"SurfaceComposerClient::OpenGlobalTransaction@v7">()();
        }

        void CloseGlobalTransaction(bool synchronous)
        {
            ApiInvoker<"SurfaceComposerClient::CloseGlobalTransaction@v7">()(synchronous);
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

                    ApiDescriptor{10, 13, &apis::libgui::LayerMetadata::Api.SetInt32, "_ZN7android13LayerMetadata8setInt32Eji"},
                    ApiDescriptor{14, UINT_MAX, &apis::libgui::LayerMetadata::Api.SetInt32, "_ZN7android3gui13LayerMetadata8setInt32Eji"},

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

                    ApiDescriptor{9, UINT_MAX, &apis::libgui::SurfaceComposerClient::Transaction::Api.SetMatrix, "_ZN7android21SurfaceComposerClient11Transaction9setMatrixERKNS_2spINS_14SurfaceControlEEEffff"},
                    ApiDescriptor{9, UINT_MAX, &apis::libgui::SurfaceComposerClient::Transaction::Api.SetPosition, "_ZN7android21SurfaceComposerClient11Transaction11setPositionERKNS_2spINS_14SurfaceControlEEEff"},

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

    struct DumpDisplayInfo
    {
        std::string uniqueId;
        uint32_t currentLayerStack;
        struct
        {
            int32_t left;
            int32_t top;
            int32_t right;
            int32_t bottom;
        } currentLayerStackRect;

        static DumpDisplayInfo MakeFromRawDumpInfo(const std::string_view &uniqueId, const std::string_view &currentLayerStack, const std::string_view &currentLayerStackRect)
        {
            DumpDisplayInfo result;

            result.uniqueId = std::string{uniqueId.begin(), uniqueId.end()};
            result.currentLayerStack = static_cast<uint32_t>(std::stoul(std::string{currentLayerStack.begin(), currentLayerStack.end()}));

            auto leftPos = currentLayerStackRect.find("(") + 1;
            auto topPos = currentLayerStackRect.find(", ", leftPos);
            auto rightPos = currentLayerStackRect.find(" - ", topPos + 2);
            auto bottomPos = currentLayerStackRect.find(", ", rightPos + 3);
            auto endPos = currentLayerStackRect.find(")", bottomPos + 2);

            // Don't check it, even though it might cause a crash.
            result.currentLayerStackRect.left = std::stoi(std::string{currentLayerStackRect.begin() + leftPos, currentLayerStackRect.begin() + topPos});
            result.currentLayerStackRect.top = std::stoi(std::string{currentLayerStackRect.begin() + topPos + 2, currentLayerStackRect.begin() + rightPos});
            result.currentLayerStackRect.right = std::stoi(std::string{currentLayerStackRect.begin() + rightPos + 3, currentLayerStackRect.begin() + bottomPos});
            result.currentLayerStackRect.bottom = std::stoi(std::string{currentLayerStackRect.begin() + bottomPos + 2, currentLayerStackRect.begin() + endPos});

            return result;
        }
    };

    struct MirrorLayerTransform
    {
        bool isAspectRatioSimilar;
        float widthScale;
        float heightScale;
        float offsetX;
        float offsetY;
    };

    inline std::vector<DumpDisplayInfo> ParseDumpDisplayInfo(const std::string_view &dumpDisplayInfo)
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

        std::vector<DumpDisplayInfo> result;

        // DisplayDeviceInfo
        auto dumpDisplayInfoIt = std::string_view::npos;
        while (std::string_view::npos != (dumpDisplayInfoIt = dumpDisplayInfo.find("DisplayDeviceInfo", dumpDisplayInfoIt + 1)))
        {
            auto uniqueId = SubStringView(dumpDisplayInfo, "mUniqueId=", "\n", dumpDisplayInfoIt);
            auto currentLayerStack = SubStringView(dumpDisplayInfo, "mCurrentLayerStack=", "\n", dumpDisplayInfoIt);
            auto currentLayerStackRect = SubStringView(dumpDisplayInfo, "mCurrentLayerStackRect=", "\n", dumpDisplayInfoIt);

            if ("-1" == currentLayerStack)
            {
                LogError("[-] %s -> Current layer stack is -1, skipping", std::string{uniqueId.begin(), uniqueId.end()}.data());

                continue;
            }

            result.push_back(DumpDisplayInfo::MakeFromRawDumpInfo(uniqueId, currentLayerStack, currentLayerStackRect));
        }

        return result;
    }

    inline MirrorLayerTransform CalcMirrorLayerTransform(float targetWidth, float targetHeight, float sourceWidth, float sourceHeight, float epsilon = 0.002)
    {
        if (0.f == targetHeight || 0.f == sourceHeight)
            throw std::runtime_error("[-] Invalid height");

        MirrorLayerTransform result{
            .isAspectRatioSimilar = std::abs(targetWidth / targetHeight - sourceWidth / sourceHeight) < epsilon,
            .widthScale = sourceWidth / targetWidth,
            .heightScale = sourceHeight / targetHeight,
        };
        if (result.isAspectRatioSimilar)
            return result;

        if (result.widthScale > result.heightScale)
        {
            result.offsetX = (sourceWidth - targetWidth * result.heightScale) / 2;
            result.widthScale = result.heightScale;
        }
        else
        {
            result.offsetY = (sourceHeight - targetHeight * result.widthScale) / 2;
            result.heightScale = result.widthScale;
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

        struct CreateOptions
        {
            const char *name;
            int32_t width;
            int32_t height;
            bool skipScreenshot;
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

        static ANativeWindow *Create(const CreateOptions &options = {.name = "AImGui"})
        {
            auto &surfaceComposerClient = GetComposerInstance();

            int32_t width = options.width;
            int32_t height = options.height;
            while (0 == width || 0 == height)
            {
                detail::types::ui::DisplayState displayInfo{};

                if (!surfaceComposerClient.GetDisplayInfo(&displayInfo))
                    break;

                width = displayInfo.layerStackSpaceRect.width;
                height = displayInfo.layerStackSpaceRect.height;

                break;
            }

            auto surfaceControl = surfaceComposerClient.CreateSurface(options.name, width, height, {}, options.skipScreenshot);
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

            // Check if have surfaces need to mirror
            static std::vector<detail::compat::SurfaceControl *> surfacesNeedToMirror;

            surfacesNeedToMirror.clear();
            for (auto &[_, surfaceControl] : m_cachedSurfaceControl)
            {
                if (!surfaceControl.skipScreenshot)
                    surfacesNeedToMirror.push_back(&surfaceControl);
            }
            if (surfacesNeedToMirror.empty())
                return;

            // Run "dumpsys display" and get result
            auto pipe = popen("dumpsys display", "r");
            if (!pipe)
            {
                LogError("[-] Failed to run dumpsys command");
                return;
            }

            char buffer[512]{};
            std::string dumpDisplayResult;
            while (fgets(buffer, sizeof(buffer), pipe) != nullptr)
                dumpDisplayResult += buffer;
            pclose(pipe);

            static std::unordered_map<uint32_t, std::vector<detail::compat::SurfaceControl>> cachedLayerStackMirrorSurfaces;
            static std::unordered_set<uint32_t> cachedLayerStackScales;

            auto dumpDisplayInfos = detail::ParseDumpDisplayInfo(dumpDisplayResult);
            for (auto &displayInfo : dumpDisplayInfos)
            {
                // Update multi display layer scale
                static int32_t builtinDisplayWidth = -1, builtinDisplayHeight = -1;
                if (0 == displayInfo.currentLayerStack)
                {
                    builtinDisplayWidth = displayInfo.currentLayerStackRect.right;
                    builtinDisplayHeight = displayInfo.currentLayerStackRect.bottom;
                }

                // Process mirror display
                if (0 == displayInfo.currentLayerStack)
                    continue;

                if (!cachedLayerStackMirrorSurfaces.contains(displayInfo.currentLayerStack))
                {
                    LogInfo("[=] New display layerstack detected: [%s] -> %u", displayInfo.uniqueId.data(), displayInfo.currentLayerStack);

                    for (auto surfaceControl : surfacesNeedToMirror)
                    {
                        auto mirrorLayer = GetComposerInstance().MirrorSurface(*surfaceControl, displayInfo.currentLayerStack);

                        LogInfo("[+] Mirror layer created: %p", mirrorLayer.data);
                        cachedLayerStackMirrorSurfaces[displayInfo.currentLayerStack].emplace_back(std::move(mirrorLayer));
                    }
                }

                if (-1 != builtinDisplayWidth && -1 != builtinDisplayHeight && cachedLayerStackMirrorSurfaces.contains(displayInfo.currentLayerStack))
                {
                    if ((displayInfo.currentLayerStackRect.right != builtinDisplayWidth || displayInfo.currentLayerStackRect.bottom != builtinDisplayHeight) && !cachedLayerStackScales.contains(displayInfo.currentLayerStack))
                    {
                        LogInfo("[=] Display layerstack size changed[%d x %d]: %u -> %d x %d",
                                builtinDisplayWidth,
                                builtinDisplayHeight,
                                displayInfo.currentLayerStack,
                                displayInfo.currentLayerStackRect.right,
                                displayInfo.currentLayerStackRect.bottom);

                        auto &composerInstance = GetComposerInstance();
                        auto &mirrorLayers = cachedLayerStackMirrorSurfaces.at(displayInfo.currentLayerStack);
                        auto transformParams = detail::CalcMirrorLayerTransform(
                            builtinDisplayWidth,
                            builtinDisplayHeight,
                            displayInfo.currentLayerStackRect.right,
                            displayInfo.currentLayerStackRect.bottom);

                        for (auto &mirrorLayer : mirrorLayers)
                        {
                            composerInstance.ZoomSurface(mirrorLayer, transformParams.widthScale, transformParams.heightScale);

                            if (!transformParams.isAspectRatioSimilar)
                                composerInstance.MoveSurface(mirrorLayer, transformParams.offsetX, transformParams.offsetY);

                            cachedLayerStackScales.emplace(displayInfo.currentLayerStack);
                            LogInfo("[+] Transform mirror layer:%p similar:%d width:%f height:%f x:%f y:%f",
                                    mirrorLayer.data,
                                    transformParams.isAspectRatioSimilar,
                                    transformParams.widthScale,
                                    transformParams.heightScale,
                                    transformParams.offsetX,
                                    transformParams.offsetY);
                        }
                    }
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