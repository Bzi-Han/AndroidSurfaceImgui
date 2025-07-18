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

#ifndef A_INPUT_EVENT_DISPATCHER_H // !A_INPUT_EVENT_DISPATCHER_H
#define A_INPUT_EVENT_DISPATCHER_H

#include <dlfcn.h>
#include <sys/system_properties.h>

#include <android/log.h>
#include <android/input.h>

#include <cstdint>
#include <memory>
#include <string>
#include <string_view>
#include <array>
#include <vector>
#include <unordered_map>
#include <variant>

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

#define B_PACK_CHARS(c1, c2, c3, c4) ((((c1) << 24)) | (((c2) << 16)) | (((c3) << 8)) | (c4))

namespace android::ainput_event_dispatcher::detail::types
{
    /**
     * The following types and structures are adapted from AOSP.
     */

    /**
     * The type used to return success/failure from frameworks APIs.
     * See the anonymous enum below for valid values.
     */
    typedef int32_t status_t;

    typedef int64_t nsecs_t; // nano-seconds

    struct Rect
    {
        /// Minimum X coordinate of the rectangle.
        int32_t left;
        /// Minimum Y coordinate of the rectangle.
        int32_t top;
        /// Maximum X coordinate of the rectangle.
        int32_t right;
        /// Maximum Y coordinate of the rectangle.
        int32_t bottom;
    };

    enum TransactionCode
    {
        FIRST_CALL_TRANSACTION = 0x00000001,
        LAST_CALL_TRANSACTION = 0x00ffffff,

        PING_TRANSACTION = B_PACK_CHARS('_', 'P', 'N', 'G'),
        START_RECORDING_TRANSACTION = B_PACK_CHARS('_', 'S', 'R', 'D'),
        STOP_RECORDING_TRANSACTION = B_PACK_CHARS('_', 'E', 'R', 'D'),
        DUMP_TRANSACTION = B_PACK_CHARS('_', 'D', 'M', 'P'),
        SHELL_COMMAND_TRANSACTION = B_PACK_CHARS('_', 'C', 'M', 'D'),
        INTERFACE_TRANSACTION = B_PACK_CHARS('_', 'N', 'T', 'F'),
        SYSPROPS_TRANSACTION = B_PACK_CHARS('_', 'S', 'P', 'R'),
        EXTENSION_TRANSACTION = B_PACK_CHARS('_', 'E', 'X', 'T'),
        DEBUG_PID_TRANSACTION = B_PACK_CHARS('_', 'P', 'I', 'D'),
        SET_RPC_CLIENT_TRANSACTION = B_PACK_CHARS('_', 'R', 'P', 'C'),

        // See android.os.IBinder.TWEET_TRANSACTION
        // Most importantly, messages can be anything not exceeding 130 UTF-8
        // characters, and callees should exclaim "jolly good message old boy!"
        TWEET_TRANSACTION = B_PACK_CHARS('_', 'T', 'W', 'T'),

        // See android.os.IBinder.LIKE_TRANSACTION
        // Improve binder self-esteem.
        LIKE_TRANSACTION = B_PACK_CHARS('_', 'L', 'I', 'K'),

        // Corresponds to TF_ONE_WAY -- an asynchronous call.
        FLAG_ONEWAY = 0x00000001,

        // Corresponds to TF_CLEAR_BUF -- clear transaction buffers after call
        // is made
        FLAG_CLEAR_BUF = 0x00000020,

        // Private userspace flag for transaction which is being requested from
        // a vendor context.
        FLAG_PRIVATE_VENDOR = 0x10000000,
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

        template <typename other_t>
        inline operator StrongPointer<other_t>()
        {
            return StrongPointer<other_t>{static_cast<other_t *>(pointer)};
        }
    };
} // namespace android::ainput_event_dispatcher::detail::types

namespace android::ainput_event_dispatcher::detail::types::dummy
{
    struct IAny
    {
        char data[1024]{};

        operator void *()
        {
            return reinterpret_cast<void *>(data);
        }
    };

    struct IServiceManager
    {
    };

    struct IBinder
    {
    };
} // namespace android::ainput_event_dispatcher::detail::types::dummy

namespace android::ainput_event_dispatcher::detail::types::apis::libutils
{
    namespace generic
    {
        using RefBase__IncStrong = void (*)(void *thiz, void *id);
        using RefBase__DecStrong = void (*)(void *thiz, void *id);

        using String8__Constructor = void (*)(void *thiz, const char *const string);
        using String8__Destructor = void (*)(void *thiz);

        using String16__Constructor = void (*)(void *thiz, const char *const string);
        using String16__Destructor = void (*)(void *thiz);
    } // namespace generic
} // namespace android::ainput_event_dispatcher::detail::types::apis::libutils

namespace android::ainput_event_dispatcher::detail::types::apis::libgui
{
    namespace v11
    {
        using WindowInfo__Read = void (*)(void *thiz, void *parcel);
    }

    namespace generic
    {
        using WindowInfo__ReadFromParcel = status_t (*)(void *thiz, void *parcel);
        using WindowInfo__AddTouchableRegion = void (*)(void *thiz, const types::Rect &region);
    } // namespace generic
} // namespace android::ainput_event_dispatcher::detail::types::apis::libgui

namespace android::ainput_event_dispatcher::detail::types::apis::libui
{
    namespace generic
    {
        using Region__Constructor = void (*)(void *thiz);
        using Region__Destructor = void (*)(void *thiz);
        using Region__Clear = void (*)(void *thiz);
    } // namespace generic
} // namespace android::ainput_event_dispatcher::detail::types::apis::libui

namespace android::ainput_event_dispatcher::detail::types::apis::libbinder
{
    namespace v12_infinite
    {
        using ServiceManager__WaitForService = StrongPointer<dummy::IBinder> (*)(void *thiz, void *name16);
    }

    namespace generic
    {
        using DefaultServiceManager = StrongPointer<dummy::IServiceManager> (*)();

        using Parcel__Constructor = void (*)(void *thiz);
        using Parcel__Destructor = void (*)(void *thiz);
        // Write
        using Parcel__WriteInterfaceToken = status_t (*)(void *thiz, void *name16);
        using Parcel__WriteString16 = status_t (*)(void *thiz, void *string16);
        // Read
        using Parcel__ReadInt32 = status_t (*)(void *thiz, int32_t *outValue);
        // Data
        using Parcel__SetData = status_t (*)(void *thiz, const void *data, size_t size);

        using Status__Ok = dummy::IAny (*)();
        using Status__ReadFromParcel = status_t (*)(void *thiz, void *parcel);
        using Status__ToString8 = dummy::IAny (*)(void *thiz);

        using BpBinder__Transact = status_t (*)(dummy::IBinder *thiz, int32_t code, void *data, void *reply, uint32_t flags);
    } // namespace generic
} // namespace android::ainput_event_dispatcher::detail::types::apis::libbinder

namespace android::ainput_event_dispatcher::detail::types::apis::libinput
{
    namespace v11
    {
        using InputChannel__Write = status_t (*)(void *thiz, void *parcel);

        using InputConsumer__Constructor = void (*)(void *thiz, const types::StrongPointer<void> &channel);
    } // namespace v11

    namespace v12_infinite
    {
        using InputChannel__Constructor = void (*)(void *thiz, std::string name, int *fd, StrongPointer<dummy::IBinder> binder);
        using InputChannel__Destructor = void (*)(void *thiz);
        using InputChannel__ReadFromParcel = status_t (*)(void *thiz, void *parcel);

        using InputConsumer__Constructor = void (*)(void *thiz, const std::shared_ptr<void> &channel);
    } // namespace v12_infinite

    namespace generic
    {
        using InputChannel__GetConnectionToken = StrongPointer<dummy::IBinder> (*)(void *thiz);
        using InputChannel__OpenInputChannelPair = status_t (*)(const std::string &name, StrongPointer<void> &outServerChannel, StrongPointer<void> &outClientChannel);

        using InputConsumer__Destructor = void (*)(void *thiz);
        using InputConsumer__Consume = status_t (*)(void *thiz, void *factory, bool consumeBatches, types::nsecs_t frameTime, uint32_t *outSeq, AInputEvent **outEvent);
        using InputConsumer__SendFinishedSignal = status_t (*)(void *thiz, uint32_t seq, bool handled);

        using PooledInputEventFactory__Constructor = void (*)(void *thiz, size_t maxPoolSize);
        using PooledInputEventFactory__Destructor = void (*)(void *thiz);
    } // namespace generic
} // namespace android::ainput_event_dispatcher::detail::types::apis::libinput

namespace android::ainput_event_dispatcher::detail::types::apis
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
} // namespace android::ainput_event_dispatcher::detail::types::apis

namespace android::ainput_event_dispatcher::detail::apis
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

        struct String16
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
        struct WindowInfo
        {
            struct OffsetTable
            {
                size_t mo_region;
            };

            struct ApiTable
            {
                void *ReadFromParcel;
                void *AddTouchableRegion;

                void *Read;
            };

            inline static OffsetTable Offset{};
            inline static ApiTable Api;
        };
    } // namespace libgui

    namespace libui
    {
        struct Region
        {
            struct ApiTable
            {
                void *Constructor;
                void *Destructor;
                void *Clear;
            };

            inline static ApiTable Api;
        };
    } // namespace libui

    namespace libbinder
    {
        struct ServiceManager
        {
            struct ApiTable
            {
                void *DefaultServiceManager; // External static function

                void *WaitForService;
            };

            inline static ApiTable Api;
        };

        struct Parcel
        {
            struct ApiTable
            {
                void *Constructor;
                void *Destructor;
                void *WriteInterfaceToken;
                void *WriteString16;
                void *ReadInt32;
                void *SetData;
                void *Data;
                void *DataSize;
                void *DataPosition;
            };

            inline static ApiTable Api;
        };

        struct Status
        {
            struct ApiTable
            {
                void *Ok;
                void *ReadFromParcel;
                void *ToString8;
            };

            inline static ApiTable Api;
        };

        struct BpBinder
        {
            struct ApiTable
            {
                void *Transact;
            };

            inline static ApiTable Api;
        };
    } // namespace libbinder

    namespace libinput
    {
        struct InputChannel
        {
            struct ApiTable
            {
                void *Constructor;
                void *Destructor;
                void *ReadFromParcel;

                void *GetConnectionToken;
                void *OpenInputChannelPair;

                void *Write;
            };

            inline static ApiTable Api;
        };

        struct InputConsumer
        {
            struct ApiTable
            {
                void *Constructor;
                void *Destructor;
                void *Consume;
                void *SendFinishedSignal;
            };

            inline static ApiTable Api;
        };

        struct PooledInputEventFactory
        {
            struct ApiTable
            {
                void *Constructor;
                void *Destructor;
            };

            inline static ApiTable Api;
        };
    } // namespace libinput
} // namespace android::ainput_event_dispatcher::detail::apis

namespace android::ainput_event_dispatcher::detail::compat
{
    constexpr size_t SupportedMinVersion = 11;

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

        if constexpr ("String16::Constructor" == descriptor)
            return reinterpret_cast<types::apis::libutils::generic::String16__Constructor>(apis::libutils::String16::Api.Constructor);
        if constexpr ("String16::Destructor" == descriptor)
            return reinterpret_cast<types::apis::libutils::generic::String16__Destructor>(apis::libutils::String16::Api.Destructor);

        // libgui
        if constexpr ("WindowInfo::ReadFromParcel" == descriptor)
            return reinterpret_cast<types::apis::libgui::generic::WindowInfo__ReadFromParcel>(apis::libgui::WindowInfo::Api.ReadFromParcel);
        if constexpr ("WindowInfo::AddTouchableRegion" == descriptor)
            return reinterpret_cast<types::apis::libgui::generic::WindowInfo__AddTouchableRegion>(apis::libgui::WindowInfo::Api.AddTouchableRegion);
        if constexpr ("WindowInfo::Read" == descriptor)
            return reinterpret_cast<types::apis::libgui::v11::WindowInfo__Read>(apis::libgui::WindowInfo::Api.Read);

        // libui
        if constexpr ("Region::Constructor" == descriptor)
            return reinterpret_cast<types::apis::libui::generic::Region__Constructor>(apis::libui::Region::Api.Constructor);
        if constexpr ("Region::Destructor" == descriptor)
            return reinterpret_cast<types::apis::libui::generic::Region__Destructor>(apis::libui::Region::Api.Destructor);
        if constexpr ("Region::Clear" == descriptor)
            return reinterpret_cast<types::apis::libui::generic::Region__Clear>(apis::libui::Region::Api.Clear);

        // libbinder
        if constexpr ("ServiceManager::DefaultServiceManager" == descriptor)
            return reinterpret_cast<types::apis::libbinder::generic::DefaultServiceManager>(apis::libbinder::ServiceManager::Api.DefaultServiceManager);
        if constexpr ("ServiceManager::WaitForService" == descriptor)
            return reinterpret_cast<types::apis::libbinder::v12_infinite::ServiceManager__WaitForService>(apis::libbinder::ServiceManager::Api.WaitForService);

        if constexpr ("Parcel::Constructor" == descriptor)
            return reinterpret_cast<types::apis::libbinder::generic::Parcel__Constructor>(apis::libbinder::Parcel::Api.Constructor);
        if constexpr ("Parcel::Destructor" == descriptor)
            return reinterpret_cast<types::apis::libbinder::generic::Parcel__Destructor>(apis::libbinder::Parcel::Api.Destructor);
        if constexpr ("Parcel::WriteInterfaceToken" == descriptor)
            return reinterpret_cast<types::apis::libbinder::generic::Parcel__WriteInterfaceToken>(apis::libbinder::Parcel::Api.WriteInterfaceToken);
        if constexpr ("Parcel::WriteString16" == descriptor)
            return reinterpret_cast<types::apis::libbinder::generic::Parcel__WriteString16>(apis::libbinder::Parcel::Api.WriteString16);
        if constexpr ("Parcel::ReadInt32" == descriptor)
            return reinterpret_cast<types::apis::libbinder::generic::Parcel__ReadInt32>(apis::libbinder::Parcel::Api.ReadInt32);
        if constexpr ("Parcel::SetData" == descriptor)
            return reinterpret_cast<types::apis::libbinder::generic::Parcel__SetData>(apis::libbinder::Parcel::Api.SetData);

        if constexpr ("Status::Ok" == descriptor)
            return reinterpret_cast<types::apis::libbinder::generic::Status__Ok>(apis::libbinder::Status::Api.Ok);
        if constexpr ("Status::ReadFromParcel" == descriptor)
            return reinterpret_cast<types::apis::libbinder::generic::Status__ReadFromParcel>(apis::libbinder::Status::Api.ReadFromParcel);
        if constexpr ("Status::ToString8" == descriptor)
            return reinterpret_cast<types::apis::libbinder::generic::Status__ToString8>(apis::libbinder::Status::Api.ToString8);

        if constexpr ("BpBinder::Transact" == descriptor)
            return reinterpret_cast<types::apis::libbinder::generic::BpBinder__Transact>(apis::libbinder::BpBinder::Api.Transact);

        // libinput
        if constexpr (11 < descriptor.version)
        {
            if constexpr ("InputChannel::Constructor" == descriptor)
                return reinterpret_cast<types::apis::libinput::v12_infinite::InputChannel__Constructor>(apis::libinput::InputChannel::Api.Constructor);
            if constexpr ("InputChannel::ReadFromParcel" == descriptor)
                return reinterpret_cast<types::apis::libinput::v12_infinite::InputChannel__ReadFromParcel>(apis::libinput::InputChannel::Api.ReadFromParcel);
        }
        else
        {
            if constexpr ("InputChannel::Write" == descriptor)
                return reinterpret_cast<types::apis::libinput::v11::InputChannel__Write>(apis::libinput::InputChannel::Api.Write);
        }
        if constexpr ("InputChannel::Destructor" == descriptor)
            return reinterpret_cast<types::apis::libinput::v12_infinite::InputChannel__Destructor>(apis::libinput::InputChannel::Api.Destructor);
        if constexpr ("InputChannel::GetConnectionToken" == descriptor)
            return reinterpret_cast<types::apis::libinput::generic::InputChannel__GetConnectionToken>(apis::libinput::InputChannel::Api.GetConnectionToken);
        if constexpr ("InputChannel::OpenInputChannelPair" == descriptor)
            return reinterpret_cast<types::apis::libinput::generic::InputChannel__OpenInputChannelPair>(apis::libinput::InputChannel::Api.OpenInputChannelPair);

        if constexpr ("InputConsumer::Constructor" == descriptor)
        {
            if constexpr (12 > descriptor.version)
                return reinterpret_cast<types::apis::libinput::v11::InputConsumer__Constructor>(apis::libinput::InputConsumer::Api.Constructor);
            else
                return reinterpret_cast<types::apis::libinput::v12_infinite::InputConsumer__Constructor>(apis::libinput::InputConsumer::Api.Constructor);
        }
        if constexpr ("InputConsumer::Destructor" == descriptor)
            return reinterpret_cast<types::apis::libinput::generic::InputConsumer__Destructor>(apis::libinput::InputConsumer::Api.Destructor);
        if constexpr ("InputConsumer::Consume" == descriptor)
            return reinterpret_cast<types::apis::libinput::generic::InputConsumer__Consume>(apis::libinput::InputConsumer::Api.Consume);
        if constexpr ("InputConsumer::SendFinishedSignal" == descriptor)
            return reinterpret_cast<types::apis::libinput::generic::InputConsumer__SendFinishedSignal>(apis::libinput::InputConsumer::Api.SendFinishedSignal);

        if constexpr ("PooledInputEventFactory::Constructor" == descriptor)
            return reinterpret_cast<types::apis::libinput::generic::PooledInputEventFactory__Constructor>(apis::libinput::PooledInputEventFactory::Api.Constructor);
        if constexpr ("PooledInputEventFactory::Destructor" == descriptor)
            return reinterpret_cast<types::apis::libinput::generic::PooledInputEventFactory__Destructor>(apis::libinput::PooledInputEventFactory::Api.Destructor);
    }
} // namespace android::ainput_event_dispatcher::detail::compat

namespace android::ainput_event_dispatcher::detail::compat
{
    constexpr auto WindowInfoData11 = std::to_array<uint8_t>({0x01, 0x00, 0x00, 0x00, 0x85, 0x2a, 0x62, 0x73, 0x13, 0x01, 0x00, 0x00, 0x10, 0x60, 0x3b, 0x02, 0x49, 0x79, 0x00, 0x00, 0x90, 0x71, 0x3b, 0x12, 0x49, 0x79, 0x00, 0x00, 0x0c, 0x00, 0x00, 0x00, 0x9a, 0x02, 0x00, 0x00, 0x15, 0x00, 0x00, 0x00, 0x41, 0x49, 0x6e, 0x70, 0x75, 0x74, 0x45, 0x76, 0x65, 0x6e, 0x74, 0x44, 0x69, 0x73, 0x70, 0x61, 0x74, 0x63, 0x68, 0x65, 0x72, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xf2, 0x05, 0x2a, 0x01, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x3f, 0x00, 0x00, 0x80, 0x3f, 0x00, 0x00, 0x80, 0x3f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x85, 0x2a, 0x62, 0x73, 0x13, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x14, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x08, 0x00, 0x00, 0x00, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x85, 0x2a, 0x62, 0x73, 0x13, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00});
    constexpr auto WindowInfoData12Plus = std::to_array<uint8_t>({0x01, 0x00, 0x00, 0x00, 0x85, 0x2a, 0x62, 0x73, 0x13, 0x01, 0x00, 0x00, 0x00, 0x87, 0x0a, 0x95, 0x12, 0x73, 0x00, 0x00, 0x90, 0xd5, 0x0a, 0x15, 0x13, 0x73, 0x00, 0x00, 0x0c, 0x00, 0x00, 0x00, 0x00, 0xf2, 0x05, 0x2a, 0x01, 0x00, 0x00, 0x00, 0x9a, 0x02, 0x00, 0x00, 0x15, 0x00, 0x00, 0x00, 0x41, 0x00, 0x49, 0x00, 0x6e, 0x00, 0x70, 0x00, 0x75, 0x00, 0x74, 0x00, 0x45, 0x00, 0x76, 0x00, 0x65, 0x00, 0x6e, 0x00, 0x74, 0x00, 0x44, 0x00, 0x69, 0x00, 0x73, 0x00, 0x70, 0x00, 0x61, 0x00, 0x74, 0x00, 0x63, 0x00, 0x68, 0x00, 0x65, 0x00, 0x72, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x3f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x3f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x3f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff, 0x30, 0x00, 0x00, 0x00, 0x85, 0x2a, 0x62, 0x73, 0x13, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x14, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x08, 0x00, 0x00, 0x00, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x85, 0x2a, 0x62, 0x73, 0x13, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x85, 0x2a, 0x62, 0x73, 0x13, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x85, 0x2a, 0x62, 0x73, 0x13, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00});

    static size_t SystemVersion = 13;

    struct String8 : public types::dummy::IAny
    {
        String8(const char *const string)
        {
            ApiInvoker<"String8::Constructor">()(data, string);
        }
        String8(types::dummy::IAny &&string)
        {
            std::memcpy(data, string.data, sizeof(string.data));
            std::memset(string.data, 0, sizeof(string.data));
        }

        ~String8()
        {
            ApiInvoker<"String8::Destructor">()(data);
        }

        char *Data()
        {
            return *reinterpret_cast<char **>(data);
        }
    };

    struct String16 : public types::dummy::IAny
    {
        String16(const char *const string)
        {
            ApiInvoker<"String16::Constructor">()(data, string);
        }

        ~String16()
        {
            ApiInvoker<"String16::Destructor">()(data);
        }

        char *Data()
        {
            return *reinterpret_cast<char **>(data);
        }
    };

    struct Parcel : public types::dummy::IAny
    {
        Parcel()
        {
            ApiInvoker<"Parcel::Constructor">()(data);
        }

        ~Parcel()
        {
            ApiInvoker<"Parcel::Destructor">()(data);
        }

        types::status_t WriteInterfaceToken(const std::string_view &descriptor)
        {
            String16 descriptor16(descriptor.data());

            return ApiInvoker<"Parcel::WriteInterfaceToken">()(data, descriptor16);
        }

        types::status_t WriteString16(const std::string_view &string)
        {
            String16 string16(string.data());

            return ApiInvoker<"Parcel::WriteString16">()(data, string16);
        }

        types::status_t ReadInt32(int32_t *outValue)
        {
            return ApiInvoker<"Parcel::ReadInt32">()(data, outValue);
        }

        types::status_t SetData(const void *data, size_t size)
        {
            return ApiInvoker<"Parcel::SetData">()(this->data, data, size);
        }

        void *Data()
        {
            return reinterpret_cast<void *(*)(void *)>(apis::libbinder::Parcel::Api.Data)(data);
        }

        size_t DataSize()
        {
            return reinterpret_cast<size_t (*)(void *)>(apis::libbinder::Parcel::Api.DataSize)(data);
        }

        size_t DataPosition()
        {
            return reinterpret_cast<size_t (*)(void *)>(apis::libbinder::Parcel::Api.DataPosition)(data);
        }
    };

    struct Status : public types::dummy::IAny
    {
        static Status Ok()
        {
            return static_cast<Status>(ApiInvoker<"Status::Ok">()());
        }
        Status(types::dummy::IAny &&other)
        {
            std::memcpy(data, other.data, sizeof(other.data));
            std::memset(other.data, 0, sizeof(other.data));
        }

        types::status_t ReadFromParcel(Parcel &parcel)
        {
            return ApiInvoker<"Status::ReadFromParcel">()(data, parcel);
        }

        std::string ToString()
        {
            auto result = static_cast<String8>(ApiInvoker<"Status::ToString8">()(data));

            return result.Data();
        }
    };

    struct BpBinder : public types::dummy::IBinder
    {
        types::status_t Transact(int32_t code, Parcel &data, Parcel &reply, uint32_t flags)
        {
            return ApiInvoker<"BpBinder::Transact">()(this, code, data, reply, flags);
        }
    };

    struct ServiceManager : public types::dummy::IServiceManager
    {
        static ServiceManager *DefaultServiceManager()
        {
            auto result = ApiInvoker<"ServiceManager::DefaultServiceManager">()();

            return static_cast<ServiceManager *>(result.get());
        }

        BpBinder *WaitForService(const std::string_view &name)
        {
            String16 name16(name.data());

            if (nullptr == apis::libbinder::ServiceManager::Api.WaitForService)
            {
                apis::libbinder::ServiceManager::Api.WaitForService = reinterpret_cast<void ***>(this)[0][8];
                LogDebug("[?] Make compatible for ServiceManager::WaitForService[%p]: it might be crashed!", apis::libbinder::ServiceManager::Api.WaitForService);
            }

            auto result = ApiInvoker<"ServiceManager::WaitForService">()(this, name16);

            return static_cast<BpBinder *>(result.get());
        }
    };

    struct InputChannel
    {
        constexpr static int32_t SELF_CREATE_MAGIC = B_PACK_CHARS('S', 'E', 'L', 'F');

        int32_t magic = SELF_CREATE_MAGIC;
        char *data = nullptr; // It's need to dynamic allocation for memory alignment

        static types::status_t OpenInputChannelPair(const std::string &name, InputChannel **outServerChannel, InputChannel **outClientChannel)
        {
            static types::StrongPointer<void> outServerChannelPointer, outClientChannelPointer;

            auto result = ApiInvoker<"InputChannel::OpenInputChannelPair">()(name, outServerChannelPointer, outClientChannelPointer);
            *outServerChannel = static_cast<InputChannel *>(outServerChannelPointer.get());
            *outClientChannel = static_cast<InputChannel *>(outClientChannelPointer.get());

            return result;
        }

        InputChannel()
        {
            if (11 < SystemVersion)
            {
                int fd = -1;

                data = new char[1024]{};

                ApiInvoker<"InputChannel::Constructor@v12">()(Self(), "", &fd, {});
            }
        }

        ~InputChannel()
        {
            ApiInvoker<"InputChannel::Destructor">()(Self());

            if (magic == SELF_CREATE_MAGIC)
                delete[] data;
        }

        inline InputChannel *Self()
        {
            if (magic == SELF_CREATE_MAGIC)
                return reinterpret_cast<InputChannel *>(data);
            else
                return this;
        }

        types::status_t ReadFromParcel(Parcel &parcel)
        {
            if (11 < SystemVersion)
                return ApiInvoker<"InputChannel::ReadFromParcel@v12">()(Self(), parcel);

            throw std::runtime_error("Not implemented");
        }

        types::StrongPointer<BpBinder> GetConnectionToken()
        {
            return ApiInvoker<"InputChannel::GetConnectionToken">()(Self());
        }

        types::status_t Write(Parcel &parcel)
        {
            if (12 > SystemVersion)
                return ApiInvoker<"InputChannel::Write">()(Self(), parcel);

            throw std::runtime_error("Not implemented");
        }
    };

    struct PooledInputEventFactory
    {
        char data[1024]{};

        PooledInputEventFactory(size_t maxPoolSize)
        {
            ApiInvoker<"PooledInputEventFactory::Constructor">()(data, maxPoolSize);
        }

        ~PooledInputEventFactory()
        {
            ApiInvoker<"PooledInputEventFactory::Destructor">()(data);
        }
    };

    struct InputConsumer
    {
        using ChannelHolder_t = std::variant<types::StrongPointer<void>, std::shared_ptr<void>>;

        std::unique_ptr<ChannelHolder_t> channelHolder;
        char data[4096]{};

        InputConsumer(InputChannel *channel)
        {
            if (12 > SystemVersion)
            {
                channelHolder = std::make_unique<ChannelHolder_t>(types::StrongPointer<void>());

                std::get<0>(*channelHolder).pointer = channel->Self();
                ApiInvoker<"InputConsumer::Constructor@v11">()(data, std::get<0>(*channelHolder));
            }
            else
            {
                channelHolder = std::make_unique<ChannelHolder_t>(std::shared_ptr<void>(channel->Self(), [](void *ptr) {}));

                ApiInvoker<"InputConsumer::Constructor@v12">()(data, std::get<1>(*channelHolder));
            }
        }

        ~InputConsumer()
        {
            ApiInvoker<"InputConsumer::Destructor">()(data);
        }

        types::status_t Consume(void *factory, uint32_t *outSequenceNumber, AInputEvent **outEvent)
        {
            return ApiInvoker<"InputConsumer::Consume">()(data, factory, true, -1, outSequenceNumber, outEvent);
        }

        types::status_t SendFinishedSignal(uint32_t outSequenceNumber, bool handled)
        {
            return ApiInvoker<"InputConsumer::SendFinishedSignal">()(data, outSequenceNumber, handled);
        }
    };

    struct WindowInfo
    {
        uint8_t data[1024]{};

        WindowInfo()
        {
            Parcel initializeData;

            if (12 > SystemVersion)
                initializeData.SetData(WindowInfoData11.data(), WindowInfoData11.size());
            else
                initializeData.SetData(WindowInfoData12Plus.data(), WindowInfoData12Plus.size());

            int32_t value{};
            if (initializeData.ReadInt32(&value))
                throw std::runtime_error("Failed to read WindowInfo::initializeData");

            LogDebug("[?] WindowInfo::Initialize[%d]: Version %zu", ReadFromParcel(initializeData), SystemVersion);
        }

        ~WindowInfo()
        {
            ApiInvoker<"Region::Destructor">()(data + apis::libgui::WindowInfo::Offset.mo_region);
        }

        types::status_t ReadFromParcel(Parcel &parcel)
        {
            if (12 > SystemVersion)
            {
                ApiInvoker<"WindowInfo::Read">()(data, parcel); // Simulation member function

                return 0;
            }
            else
            {
                ApiInvoker<"Region::Constructor">()(data + apis::libgui::WindowInfo::Offset.mo_region);

                return ApiInvoker<"WindowInfo::ReadFromParcel">()(data, parcel);
            }
        }

        void SetToken(types::StrongPointer<BpBinder> &token)
        {
            if (12 > SystemVersion)
            {
                reinterpret_cast<void **>(data)[0] = token.get();
                reinterpret_cast<int32_t *>(data)[10] = 0x00000020; // layoutParamsFlags
                reinterpret_cast<int32_t *>(data)[11] = 0x00000004; // layoutParamsType
            }
            else
            {
                reinterpret_cast<void **>(data)[1] = token.get();
            }
        }

        void AddTouchableRegion(const types::Rect &rect)
        {
            ApiInvoker<"WindowInfo::AddTouchableRegion">()(data, rect);
        }

        void ClearTouchableRegion()
        {
            ApiInvoker<"Region::Clear">()(data + apis::libgui::WindowInfo::Offset.mo_region);
        }

        operator void *()
        {
            return reinterpret_cast<void *>(data);
        }
    };

} // namespace android::ainput_event_dispatcher::detail::compat

namespace android::ainput_event_dispatcher::detail
{
    struct ApiTableDescriptor
    {
        static const auto GetDefaultDescriptors()
        {
            using namespace android::ainput_event_dispatcher::detail::types::apis;

            return std::make_tuple(
                // libutils
                std::to_array({
                    // RefBase
                    ApiDescriptor{5, UINT_MAX, &apis::libutils::RefBase::Api.IncStrong, "_ZNK7android7RefBase9incStrongEPKv"},
                    ApiDescriptor{5, UINT_MAX, &apis::libutils::RefBase::Api.DecStrong, "_ZNK7android7RefBase9decStrongEPKv"},

                    // String8
                    ApiDescriptor{5, UINT_MAX, &apis::libutils::String8::Api.Constructor, "_ZN7android7String8C2EPKc"},
                    ApiDescriptor{5, UINT_MAX, &apis::libutils::String8::Api.Destructor, "_ZN7android7String8D2Ev"},

                    // String16
                    ApiDescriptor{5, UINT_MAX, &apis::libutils::String16::Api.Constructor, "_ZN7android8String16C2EPKc"},
                    ApiDescriptor{5, UINT_MAX, &apis::libutils::String16::Api.Destructor, "_ZN7android8String16D2Ev"},
                }),
                // libgui
                std::to_array({
                    // WindowInfo
                    ApiDescriptor{11, 11, &apis::libgui::WindowInfo::Api.Read, "_ZN7android15InputWindowInfo4readERKNS_6ParcelE"},
                    ApiDescriptor{12, 12, &apis::libgui::WindowInfo::Api.ReadFromParcel, "_ZN7android15InputWindowInfo14readFromParcelEPKNS_6ParcelE"},
                    ApiDescriptor{13, 15, &apis::libgui::WindowInfo::Api.ReadFromParcel, "_ZN7android3gui10WindowInfo14readFromParcelEPKNS_6ParcelE"},

                    ApiDescriptor{11, 12, &apis::libgui::WindowInfo::Api.AddTouchableRegion, "_ZN7android15InputWindowInfo18addTouchableRegionERKNS_4RectE"},
                    ApiDescriptor{13, UINT_MAX, &apis::libgui::WindowInfo::Api.AddTouchableRegion, "_ZN7android3gui10WindowInfo18addTouchableRegionERKNS_4RectE"},
                }),
                // libui
                std::to_array({
                    // Region
                    ApiDescriptor{11, UINT_MAX, &apis::libui::Region::Api.Constructor, "_ZN7android6RegionC2Ev"},
                    ApiDescriptor{11, UINT_MAX, &apis::libui::Region::Api.Destructor, "_ZN7android6RegionD2Ev"},
                    ApiDescriptor{11, UINT_MAX, &apis::libui::Region::Api.Clear, "_ZN7android6Region5clearEv"},
                }),
                // libbinder
                std::to_array({
                    // ServiceManager
                    ApiDescriptor{10, UINT_MAX, &apis::libbinder::ServiceManager::Api.DefaultServiceManager, "_ZN7android21defaultServiceManagerEv"},
                    ApiDescriptor{12, UINT_MAX, &apis::libbinder::ServiceManager::Api.WaitForService, "_ZN7android18ServiceManagerShim14waitForServiceERKNS_8String16E"},

                    // Parcel
                    ApiDescriptor{10, UINT_MAX, &apis::libbinder::Parcel::Api.Constructor, "_ZN7android6ParcelC2Ev"},
                    ApiDescriptor{10, UINT_MAX, &apis::libbinder::Parcel::Api.Destructor, "_ZN7android6ParcelD2Ev"},
                    ApiDescriptor{10, UINT_MAX, &apis::libbinder::Parcel::Api.WriteInterfaceToken, "_ZN7android6Parcel19writeInterfaceTokenERKNS_8String16E"},
                    ApiDescriptor{10, UINT_MAX, &apis::libbinder::Parcel::Api.WriteString16, "_ZN7android6Parcel13writeString16ERKNS_8String16E"},
                    ApiDescriptor{10, UINT_MAX, &apis::libbinder::Parcel::Api.ReadInt32, "_ZNK7android6Parcel9readInt32EPi"},
                    ApiDescriptor{10, UINT_MAX, &apis::libbinder::Parcel::Api.SetData, "_ZN7android6Parcel7setDataEPKhm"},
                    ApiDescriptor{10, UINT_MAX, &apis::libbinder::Parcel::Api.Data, "_ZNK7android6Parcel4dataEv"},
                    ApiDescriptor{10, UINT_MAX, &apis::libbinder::Parcel::Api.DataSize, "_ZNK7android6Parcel8dataSizeEv"},
                    ApiDescriptor{10, UINT_MAX, &apis::libbinder::Parcel::Api.DataPosition, "_ZNK7android6Parcel12dataPositionEv"},

                    // Status
                    ApiDescriptor{10, UINT_MAX, &apis::libbinder::Status::Api.Ok, "_ZN7android6binder6Status2okEv"},
                    ApiDescriptor{10, UINT_MAX, &apis::libbinder::Status::Api.ReadFromParcel, "_ZN7android6binder6Status14readFromParcelERKNS_6ParcelE"},
                    ApiDescriptor{10, UINT_MAX, &apis::libbinder::Status::Api.ToString8, "_ZNK7android6binder6Status9toString8Ev"},

                    // BpBinder
                    ApiDescriptor{10, UINT_MAX, &apis::libbinder::BpBinder::Api.Transact, "_ZN7android8BpBinder8transactEjRKNS_6ParcelEPS1_j"},
                }),
                // libinput
                std::to_array({
                    // InputChannel
                    ApiDescriptor{11, 11, &apis::libinput::InputChannel::Api.Constructor, "_ZN7android12InputChannelC2ERKNSt3__112basic_stringIcNS1_11char_traitsIcEENS1_9allocatorIcEEEENS_4base14unique_fd_implINSA_13DefaultCloserEEENS_2spINS_7IBinderEEE"},
                    ApiDescriptor{12, UINT_MAX, &apis::libinput::InputChannel::Api.Constructor, "_ZN7android12InputChannelC2ENSt3__112basic_stringIcNS1_11char_traitsIcEENS1_9allocatorIcEEEENS_4base14unique_fd_implINS8_13DefaultCloserEEENS_2spINS_7IBinderEEE"},
                    ApiDescriptor{11, UINT_MAX, &apis::libinput::InputChannel::Api.Destructor, "_ZN7android12InputChannelD2Ev"},

                    ApiDescriptor{12, 14, &apis::libinput::InputChannel::Api.ReadFromParcel, "_ZN7android12InputChannel14readFromParcelEPKNS_6ParcelE"},
                    ApiDescriptor{15, 15, &apis::libinput::InputChannel::Api.ReadFromParcel, "_ZN7android2os16InputChannelCore14readFromParcelEPKNS_6ParcelE"},

                    ApiDescriptor{11, UINT_MAX, &apis::libinput::InputChannel::Api.GetConnectionToken, "_ZNK7android12InputChannel18getConnectionTokenEv"},
                    ApiDescriptor{11, 11, &apis::libinput::InputChannel::Api.OpenInputChannelPair, "_ZN7android12InputChannel20openInputChannelPairERKNSt3__112basic_stringIcNS1_11char_traitsIcEENS1_9allocatorIcEEEERNS_2spIS0_EESC_"},

                    ApiDescriptor{11, 11, &apis::libinput::InputChannel::Api.Write, "_ZNK7android12InputChannel5writeERNS_6ParcelE"},

                    // InputConsumer
                    ApiDescriptor{11, 11, &apis::libinput::InputConsumer::Api.Constructor, "_ZN7android13InputConsumerC2ERKNS_2spINS_12InputChannelEEE"},
                    ApiDescriptor{12, UINT_MAX, &apis::libinput::InputConsumer::Api.Constructor, "_ZN7android13InputConsumerC2ERKNSt3__110shared_ptrINS_12InputChannelEEE"},
                    ApiDescriptor{11, UINT_MAX, &apis::libinput::InputConsumer::Api.Destructor, "_ZN7android13InputConsumerD2Ev"},

                    ApiDescriptor{11, UINT_MAX, &apis::libinput::InputConsumer::Api.Consume, "_ZN7android13InputConsumer7consumeEPNS_26InputEventFactoryInterfaceEblPjPPNS_10InputEventE"},
                    ApiDescriptor{11, UINT_MAX, &apis::libinput::InputConsumer::Api.SendFinishedSignal, "_ZN7android13InputConsumer18sendFinishedSignalEjb"},

                    // PooledInputEventFactory
                    ApiDescriptor{11, UINT_MAX, &apis::libinput::PooledInputEventFactory::Api.Constructor, "_ZN7android23PooledInputEventFactoryC2Em"},
                    ApiDescriptor{11, UINT_MAX, &apis::libinput::PooledInputEventFactory::Api.Destructor, "_ZN7android23PooledInputEventFactoryD2Ev"},
                })

            );
        }

        static const bool PatchDefaultOffsetTable()
        {
#if defined(__aarch64__)
            for (size_t i = 0; i < 4; ++i)
            {
                auto instruction = reinterpret_cast<uint32_t *>(apis::libgui::WindowInfo::Api.AddTouchableRegion)[i];
                LogInfo("[=] Instruction: %08X", instruction);

                if (0x11000000 != (instruction & 0x7F800000))
                {
                    LogError("[!] Instruction is invalid!");
                    continue;
                }

                apis::libgui::WindowInfo::Offset.mo_region = (instruction & 0x3FFC00) >> 10;
                break;
            }
#elif defined(__x86_64__)
            auto instruction = *reinterpret_cast<uint32_t *>(apis::libgui::WindowInfo::Api.AddTouchableRegion);
            LogInfo("[=] Instruction: %08X", instruction);

            if ((12 > compat::SystemVersion ? 0x00C78348 : 0x00C78148) != (instruction & 0x00FFFFFF))
            {
                LogError("[!] Instruction is invalid!");
                return false;
            }

            apis::libgui::WindowInfo::Offset.mo_region = *reinterpret_cast<uint8_t *>(reinterpret_cast<uint8_t *>(apis::libgui::WindowInfo::Api.AddTouchableRegion) + 3);
#else
#error "Unsupported architecture!"
#endif

            LogDebug("[=] mo_region: %zu", apis::libgui::WindowInfo::Offset.mo_region);

            if (0 == apis::libgui::WindowInfo::Offset.mo_region)
                return false;

            return true;
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
            auto libui = resolver.Open("/system/lib64/libui.so", RTLD_LAZY);
            auto libbinder = resolver.Open("/system/lib64/libbinder.so", RTLD_LAZY);
            auto libinput = resolver.Open("/system/lib64/libinput.so", RTLD_LAZY);
#else
            auto libgui = resolver.Open("/system/lib/libgui.so", RTLD_LAZY);
            auto libutils = resolver.Open("/system/lib/libutils.so", RTLD_LAZY);
            auto libui = resolver.Open("/system/lib/libui.so", RTLD_LAZY);
            auto libbinder = resolver.Open("/system/lib/libbinder.so", RTLD_LAZY);
            auto libinput = resolver.Open("/system/lib/libinput.so", RTLD_LAZY);
#endif
            if (nullptr == libgui || nullptr == libutils)
            {
                LogError("[!] Failed to open libgui.so or libutils.so");
                throw std::runtime_error("Failed to open libgui.so or libutils.so");
            }

            std::unordered_map<std::string_view, void **> checkFailedToResolveMap;

            const auto &[libutilsApis,
                         libguiApis,
                         libuiApis,
                         libbinderApis,
                         libinputApis] = ApiTableDescriptor::GetDefaultDescriptors();
            for (const auto &descriptor : libutilsApis)
            {
                if (!descriptor.IsSupported(compat::SystemVersion) || nullptr != *descriptor.storeToTarget)
                    continue;

                *descriptor.storeToTarget = resolver.Resolve(libutils, descriptor.apiSignature);
                if (nullptr == *descriptor.storeToTarget)
                {
                    LogDebug("[?] Version[Android %zu] [libutils] failed to resolve symbol: %s", compat::SystemVersion, descriptor.apiSignature);
                    checkFailedToResolveMap[descriptor.apiSignature] = descriptor.storeToTarget;
                }
            }

            for (const auto &descriptor : libguiApis)
            {
                if (!descriptor.IsSupported(compat::SystemVersion) || nullptr != *descriptor.storeToTarget)
                    continue;

                *descriptor.storeToTarget = resolver.Resolve(libgui, descriptor.apiSignature);
                if (nullptr == *descriptor.storeToTarget)
                {
                    LogDebug("[?] Version[Android %zu] [libgui] failed to resolve symbol: %s", compat::SystemVersion, descriptor.apiSignature);
                    checkFailedToResolveMap[descriptor.apiSignature] = descriptor.storeToTarget;
                }
            }

            for (const auto &descriptor : libuiApis)
            {
                if (!descriptor.IsSupported(compat::SystemVersion) || nullptr != *descriptor.storeToTarget)
                    continue;

                *descriptor.storeToTarget = resolver.Resolve(libui, descriptor.apiSignature);
                if (nullptr == *descriptor.storeToTarget)
                {
                    LogDebug("[?] Version[Android %zu] [libui] failed to resolve symbol: %s", compat::SystemVersion, descriptor.apiSignature);
                    checkFailedToResolveMap[descriptor.apiSignature] = descriptor.storeToTarget;
                }
            }

            for (const auto &descriptor : libbinderApis)
            {
                if (!descriptor.IsSupported(compat::SystemVersion) || nullptr != *descriptor.storeToTarget)
                    continue;

                *descriptor.storeToTarget = resolver.Resolve(libbinder, descriptor.apiSignature);
                if (nullptr == *descriptor.storeToTarget)
                {
                    LogDebug("[?] Version[Android %zu] [libbinder] failed to resolve symbol: %s", compat::SystemVersion, descriptor.apiSignature);
                    checkFailedToResolveMap[descriptor.apiSignature] = descriptor.storeToTarget;
                }
            }

            for (const auto &descriptor : libinputApis)
            {
                if (!descriptor.IsSupported(compat::SystemVersion) || nullptr != *descriptor.storeToTarget)
                    continue;

                *descriptor.storeToTarget = resolver.Resolve(libinput, descriptor.apiSignature);
                if (nullptr == *descriptor.storeToTarget)
                {
                    LogDebug("[?] Version[Android %zu] [libinput] failed to resolve symbol: %s", compat::SystemVersion, descriptor.apiSignature);
                    checkFailedToResolveMap[descriptor.apiSignature] = descriptor.storeToTarget;
                }
            }

            for (const auto &[signature, storeToTarget] : checkFailedToResolveMap)
            {
                if (nullptr != *storeToTarget)
                    continue;

                LogError("[!] Version[Android %zu] Unresolved — No workaround available symbol: %s", compat::SystemVersion, signature.data());
                throw std::runtime_error("Failed to resolve symbol");
            }

            if (!ApiTableDescriptor::PatchDefaultOffsetTable())
            {
                LogError("[!] Version[Android %zu] failed to patch offset table", compat::SystemVersion);
                throw std::runtime_error("Failed to patch default table");
            }

            resolver.Close(libinput);
            resolver.Close(libbinder);
            resolver.Close(libui);
            resolver.Close(libutils);
            resolver.Close(libgui);
            resolved = true;
            LogInfo("[+] Version[Android %zu] resolved all symbols", compat::SystemVersion);
        }
    };
} // namespace android::ainput_event_dispatcher::detail

namespace android
{
    class AInputEventDispatcher
    {
    public:
        using Rect = ainput_event_dispatcher::detail::types::Rect;

    public:
        static void SetupCustomApiResolver(const ainput_event_dispatcher::detail::ApiResolver::ResolverImpl &resolver)
        {
            ainput_event_dispatcher::detail::ApiResolver::Resolve(resolver);
        }

    public:
        void *GetWindowInfo()
        {
            return m_windowInfo.get();
        }

        void UpdateTouchableRegions(const std::vector<Rect> &touchableRegion)
        {
            m_windowInfo->ClearTouchableRegion();

            for (const auto &region : touchableRegion)
                m_windowInfo->AddTouchableRegion(region);
        }

        int64_t GetNextInputEvent(AInputEvent **event)
        {
            uint32_t sequenceNumber = 0;

            if (0 != m_consumer->Consume(m_eventFactory->data, &sequenceNumber, event))
                return -1;

            return sequenceNumber;
        }

        void FinishInputEvent(int64_t eventId)
        {
            if (0 != m_consumer->SendFinishedSignal(eventId, true))
            {
                LogError("[!] Failed to finish input event: %ld", eventId);
                return;
            }
        }

    public:
        ~AInputEventDispatcher()
        {
        }
        AInputEventDispatcher(const std::vector<Rect> &touchableRegion)
            : AInputEventDispatcher()
        {
            UpdateTouchableRegions(touchableRegion);
        }
        AInputEventDispatcher()
        {
            using namespace ainput_event_dispatcher::detail::compat;

            ainput_event_dispatcher::detail::ApiResolver::Resolve();

            auto serviceManager = ServiceManager::DefaultServiceManager();
            if (nullptr == serviceManager)
            {
                LogError("[!] Version[Android %zu] failed to get default service manager", SystemVersion);
                throw std::runtime_error("Failed to get default service manager");
            }

            auto inputflinger = serviceManager->WaitForService("inputflinger");
            if (nullptr == inputflinger)
            {
                LogError("[!] Version[Android %zu] failed to get inputflinger service", SystemVersion);
                throw std::runtime_error("Failed to get inputflinger service");
            }

            // ============================================================
            InputChannel *serverChannel = nullptr, *clientChannel = nullptr;
            Parcel data, replay;

            if (12 > SystemVersion)
            {
                if (InputChannel::OpenInputChannelPair("AInputEventDispatcher", &serverChannel, &clientChannel))
                {
                    LogError("[!] Version[Android %zu] failed to open input channel pair", SystemVersion);
                    throw std::runtime_error("Failed to open input channel pair");
                }

                data.WriteInterfaceToken("android.input.IInputFlinger");
                serverChannel->Write(data);
            }
            else
            {
                m_clientChannel = std::make_unique<InputChannel>();

                data.WriteInterfaceToken("android.os.IInputFlinger");
                data.WriteString16("AInputEventDispatcher");
            }

            ainput_event_dispatcher::detail::types::status_t transactionResult = -1;
            if (13 > SystemVersion)
                transactionResult = inputflinger->Transact(ainput_event_dispatcher::detail::types::TransactionCode::FIRST_CALL_TRANSACTION + 1, data, replay, 0);
            else
                transactionResult = inputflinger->Transact(ainput_event_dispatcher::detail::types::TransactionCode::FIRST_CALL_TRANSACTION + 0, data, replay, 0);
            if (transactionResult)
            {
                LogError("[!] Version[Android %zu] failed to transaction with inputflinger service", SystemVersion);
                throw std::runtime_error("Failed to transaction with inputflinger service");
            }

            if (11 < SystemVersion)
            {
                auto transactionStatus = Status::Ok();
                if (transactionStatus.ReadFromParcel(replay))
                {
                    LogError("[!] Version[Android %zu] failed to read transaction status", SystemVersion);
                    throw std::runtime_error("Failed to read transaction status");
                }
                LogInfo("[=] Transaction status: %s", transactionStatus.ToString().data());

                int32_t callResultCode = 0;
                if (replay.ReadInt32(&callResultCode) || !callResultCode)
                {
                    LogError("[!] Version[Android %zu] failed to read call result: %d", SystemVersion, callResultCode);
                    throw std::runtime_error("Failed to read call result");
                }

                if (m_clientChannel->ReadFromParcel(replay))
                {
                    LogError("[!] Version[Android %zu] failed to read client channel proxy", SystemVersion);
                    throw std::runtime_error("Failed to read client channel proxy");
                }
                clientChannel = m_clientChannel.get();
            }

            // ============================================================
            m_windowInfo = std::make_unique<WindowInfo>();

            auto connectionToken = clientChannel->GetConnectionToken();
            m_windowInfo->SetToken(connectionToken);

            LogDebug("[?] AInputEventDispatcher::WindowInfo[%p]", m_windowInfo.get());

            // ============================================================
            m_eventFactory = std::make_unique<PooledInputEventFactory>(20);
            m_consumer = std::make_unique<InputConsumer>(clientChannel);
        }

    private:
        std::unique_ptr<ainput_event_dispatcher::detail::compat::InputChannel> m_clientChannel;
        std::unique_ptr<ainput_event_dispatcher::detail::compat::WindowInfo> m_windowInfo;
        std::unique_ptr<ainput_event_dispatcher::detail::compat::PooledInputEventFactory> m_eventFactory;
        std::unique_ptr<ainput_event_dispatcher::detail::compat::InputConsumer> m_consumer;
    };
} // namespace android

#undef B_PACK_CHARS

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

#endif // !A_INPUT_EVENT_DISPATCHER_H