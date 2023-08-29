#include "ATouchEvent.h"

int g_scanCodeMapping[] = {
    AKEYCODE_UNKNOWN, // Make scan codes mapping array start index with 1
    AKEYCODE_ESCAPE,
    AKEYCODE_1,
    AKEYCODE_2,
    AKEYCODE_3,
    AKEYCODE_4,
    AKEYCODE_5,
    AKEYCODE_6,
    AKEYCODE_7,
    AKEYCODE_8,
    AKEYCODE_9,
    AKEYCODE_0,
    AKEYCODE_MINUS,
    AKEYCODE_EQUALS,
    AKEYCODE_DEL,
    AKEYCODE_TAB,
    AKEYCODE_Q,
    AKEYCODE_W,
    AKEYCODE_E,
    AKEYCODE_R,
    AKEYCODE_T,
    AKEYCODE_Y,
    AKEYCODE_U,
    AKEYCODE_I,
    AKEYCODE_O,
    AKEYCODE_P,
    AKEYCODE_LEFT_BRACKET,
    AKEYCODE_RIGHT_BRACKET,
    AKEYCODE_ENTER,
    AKEYCODE_CTRL_LEFT,
    AKEYCODE_A,
    AKEYCODE_S,
    AKEYCODE_D,
    AKEYCODE_F,
    AKEYCODE_G,
    AKEYCODE_H,
    AKEYCODE_J,
    AKEYCODE_K,
    AKEYCODE_L,
    AKEYCODE_SEMICOLON,
    AKEYCODE_APOSTROPHE,
    AKEYCODE_GRAVE,
    AKEYCODE_SHIFT_LEFT,
    AKEYCODE_BACKSLASH,
    AKEYCODE_Z,
    AKEYCODE_X,
    AKEYCODE_C,
    AKEYCODE_V,
    AKEYCODE_B,
    AKEYCODE_N,
    AKEYCODE_M,
    AKEYCODE_COMMA,
    AKEYCODE_PERIOD,
    AKEYCODE_SLASH,
    AKEYCODE_SHIFT_RIGHT,
    AKEYCODE_NUMPAD_MULTIPLY,
    AKEYCODE_ALT_LEFT,
    AKEYCODE_SPACE,
    AKEYCODE_CAPS_LOCK,
    AKEYCODE_F1,
    AKEYCODE_F2,
    AKEYCODE_F3,
    AKEYCODE_F4,
    AKEYCODE_F5,
    AKEYCODE_F6,
    AKEYCODE_F7,
    AKEYCODE_F8,
    AKEYCODE_F9,
    AKEYCODE_F10,
    AKEYCODE_NUM_LOCK,
    AKEYCODE_SCROLL_LOCK,
    AKEYCODE_NUMPAD_7,
    AKEYCODE_NUMPAD_8,
    AKEYCODE_NUMPAD_9,
    AKEYCODE_NUMPAD_SUBTRACT,
    AKEYCODE_NUMPAD_4,
    AKEYCODE_NUMPAD_5,
    AKEYCODE_NUMPAD_6,
    AKEYCODE_NUMPAD_ADD,
    AKEYCODE_NUMPAD_1,
    AKEYCODE_NUMPAD_2,
    AKEYCODE_NUMPAD_3,
    AKEYCODE_NUMPAD_0,
    AKEYCODE_NUMPAD_DOT,
    AKEYCODE_UNKNOWN, //(undefined)
    AKEYCODE_ZENKAKU_HANKAKU,
    AKEYCODE_BACKSLASH,
    AKEYCODE_F11,
    AKEYCODE_F12,
    AKEYCODE_RO,
    AKEYCODE_UNKNOWN, //"KEY_KATAKANA"
    AKEYCODE_UNKNOWN, //"KEY_HIRAGANA"
    AKEYCODE_HENKAN,
    AKEYCODE_KATAKANA_HIRAGANA,
    AKEYCODE_MUHENKAN,
    AKEYCODE_NUMPAD_COMMA,
    AKEYCODE_NUMPAD_ENTER,
    AKEYCODE_CTRL_RIGHT,
    AKEYCODE_NUMPAD_DIVIDE,
    AKEYCODE_SYSRQ,
    AKEYCODE_ALT_RIGHT,
    AKEYCODE_UNKNOWN, //"KEY_LINEFEED"
    AKEYCODE_MOVE_HOME,
    AKEYCODE_DPAD_UP,
    AKEYCODE_PAGE_UP,
    AKEYCODE_DPAD_LEFT,
    AKEYCODE_DPAD_RIGHT,
    AKEYCODE_MOVE_END,
    AKEYCODE_DPAD_DOWN,
    AKEYCODE_PAGE_DOWN,
    AKEYCODE_INSERT,
    AKEYCODE_FORWARD_DEL,
    AKEYCODE_UNKNOWN, //"KEY_MACRO"
    AKEYCODE_VOLUME_MUTE,
    AKEYCODE_VOLUME_DOWN,
    AKEYCODE_VOLUME_UP,
    AKEYCODE_POWER,
    AKEYCODE_NUMPAD_EQUALS,
    AKEYCODE_UNKNOWN, //"KEY_KPPLUSMINUS"
    AKEYCODE_BREAK,
    AKEYCODE_UNKNOWN, //(undefined)
    AKEYCODE_NUMPAD_COMMA,
    AKEYCODE_KANA,
    AKEYCODE_EISU,
    AKEYCODE_YEN,
    AKEYCODE_META_LEFT,
    AKEYCODE_META_RIGHT,
    AKEYCODE_MENU,
    AKEYCODE_MEDIA_STOP,
    AKEYCODE_UNKNOWN, //"KEY_AGAIN"
    AKEYCODE_UNKNOWN, //"KEY_PROPS"
    AKEYCODE_UNKNOWN, //"KEY_UNDO"
    AKEYCODE_UNKNOWN, //"KEY_FRONT"
    AKEYCODE_COPY,
    AKEYCODE_UNKNOWN, //"KEY_OPEN"
    AKEYCODE_PASTE,
    AKEYCODE_UNKNOWN, //"KEY_FIND"
    AKEYCODE_CUT,
    AKEYCODE_UNKNOWN, //"KEY_HELP"
    AKEYCODE_MENU,
    AKEYCODE_CALCULATOR,
    AKEYCODE_UNKNOWN, //"KEY_SETUP"
    AKEYCODE_SLEEP,
    AKEYCODE_WAKEUP,
    AKEYCODE_UNKNOWN, //"KEY_FILE"
    AKEYCODE_UNKNOWN, //"KEY_SENDFILE"
    AKEYCODE_UNKNOWN, //"KEY_DELETEFILE"
    AKEYCODE_UNKNOWN, //"KEY_XFER"
    AKEYCODE_UNKNOWN, //"KEY_PROG1"
    AKEYCODE_UNKNOWN, //"KEY_PROG2"
    AKEYCODE_EXPLORER,
    AKEYCODE_UNKNOWN, //"KEY_MSDOS"
    AKEYCODE_POWER,
    AKEYCODE_UNKNOWN, //"KEY_DIRECTION"
    AKEYCODE_UNKNOWN, //"KEY_CYCLEWINDOWS"
    AKEYCODE_ENVELOPE,
    AKEYCODE_BOOKMARK,
    AKEYCODE_UNKNOWN, //"KEY_COMPUTER"
    AKEYCODE_BACK,
    AKEYCODE_FORWARD,
    AKEYCODE_MEDIA_CLOSE,
    AKEYCODE_MEDIA_EJECT,
    AKEYCODE_MEDIA_EJECT,
    AKEYCODE_MEDIA_NEXT,
    AKEYCODE_MEDIA_PLAY_PAUSE,
    AKEYCODE_MEDIA_PREVIOUS,
    AKEYCODE_MEDIA_STOP,
    AKEYCODE_MEDIA_RECORD,
    AKEYCODE_MEDIA_REWIND,
    AKEYCODE_CALL,
    AKEYCODE_UNKNOWN, //"KEY_ISO"
    AKEYCODE_MUSIC,
    AKEYCODE_HOME,
    AKEYCODE_REFRESH,
    AKEYCODE_UNKNOWN, //"KEY_EXIT"
    AKEYCODE_UNKNOWN, //"KEY_MOVE"
    AKEYCODE_UNKNOWN, //"KEY_EDIT"
    AKEYCODE_PAGE_UP,
    AKEYCODE_PAGE_DOWN,
    AKEYCODE_NUMPAD_LEFT_PAREN,
    AKEYCODE_NUMPAD_RIGHT_PAREN,
    AKEYCODE_UNKNOWN, //"KEY_NEW"
    AKEYCODE_UNKNOWN, //"KEY_REDO"
    AKEYCODE_UNKNOWN, // F13
    AKEYCODE_UNKNOWN, // F14
    AKEYCODE_UNKNOWN, // F15
    AKEYCODE_UNKNOWN, // F16
    AKEYCODE_UNKNOWN, // F17
    AKEYCODE_UNKNOWN, // F18
    AKEYCODE_UNKNOWN, // F19
    AKEYCODE_UNKNOWN, // F20
    AKEYCODE_UNKNOWN, // F21
    AKEYCODE_UNKNOWN, // F22
    AKEYCODE_UNKNOWN, // F23
    AKEYCODE_UNKNOWN, // F24
    AKEYCODE_UNKNOWN, //(undefined)
    AKEYCODE_UNKNOWN, //(undefined)
    AKEYCODE_UNKNOWN, //(undefined)
    AKEYCODE_UNKNOWN, //(undefined)
    AKEYCODE_UNKNOWN, //(undefined)
    AKEYCODE_MEDIA_PLAY,
    AKEYCODE_MEDIA_PAUSE,
    AKEYCODE_UNKNOWN, //"KEY_PROG3"
    AKEYCODE_UNKNOWN, //"KEY_PROG4"
    AKEYCODE_NOTIFICATION,
    AKEYCODE_UNKNOWN, //"KEY_SUSPEND"
    AKEYCODE_UNKNOWN, //"KEY_CLOSE"
    AKEYCODE_MEDIA_PLAY,
    AKEYCODE_MEDIA_FAST_FORWARD,
    AKEYCODE_UNKNOWN, //"KEY_BASSBOOST"
    AKEYCODE_UNKNOWN, //"KEY_PRINT"
    AKEYCODE_UNKNOWN, //"KEY_HP"
    AKEYCODE_CAMERA,
    AKEYCODE_MUSIC,
    AKEYCODE_UNKNOWN, //"KEY_QUESTION"
    AKEYCODE_ENVELOPE,
    AKEYCODE_UNKNOWN, //"KEY_CHAT"
    AKEYCODE_SEARCH,
    AKEYCODE_UNKNOWN, //"KEY_CONNECT"
    AKEYCODE_UNKNOWN, //"KEY_FINANCE"
    AKEYCODE_UNKNOWN, //"KEY_SPORT"
    AKEYCODE_UNKNOWN, //"KEY_SHOP"
    AKEYCODE_UNKNOWN, //"KEY_ALTERASE"
    AKEYCODE_UNKNOWN, //"KEY_CANCEL"
    AKEYCODE_BRIGHTNESS_DOWN,
    AKEYCODE_BRIGHTNESS_UP,
    AKEYCODE_HEADSETHOOK,
};

namespace android
{
    int ATouchEvent::transformScalerX = -1;
    int ATouchEvent::transformScalerY = -1;

    ATouchEvent::ATouchEvent()
    {
        std::error_code errorCode;
        if (!std::filesystem::exists("/dev/input", errorCode))
        {
            if (errorCode)
                LogDebug("[-] Could not run filesystem::exists() due to error %d : %s.", errorCode.value(), errorCode.message().data());

            return;
        }

        for (const auto &devicePath : std::filesystem::directory_iterator("/dev/input"))
        {
            if (std::string::npos == devicePath.path().filename().string().find("event"))
                continue;

            m_deviceFd = open(devicePath.path().c_str(), O_RDONLY | O_SYNC | O_NONBLOCK);
            if (-1 == m_deviceFd)
            {
                LogDebug("[-] Could not open file %s due to error %d : %s", devicePath.path().c_str(), errno, strerror(errno));
                continue;
            }

            ioctl(m_deviceFd, EVIOCGBIT(EV_KEY, m_keyBitmask.bytes()), m_keyBitBuffer.data());
            m_keyBitmask.loadFromBuffer(m_keyBitBuffer);

            if (m_keyBitmask.test(BTN_TOUCH) || m_keyBitmask.test(BTN_TOOL_FINGER))
                break;

            close(m_deviceFd);
            m_deviceFd = -1;
        }

        int buffer[6]{};
        ioctl(m_deviceFd, EVIOCGABS(ABS_MT_POSITION_X), buffer);
        transformScalerX = buffer[2];

        ioctl(m_deviceFd, EVIOCGABS(ABS_MT_POSITION_Y), buffer);
        transformScalerY = buffer[2];
    }

    ATouchEvent::~ATouchEvent()
    {
        if (-1 != m_deviceFd)
            close(m_deviceFd);
    }

    bool ATouchEvent::GetRawEvent(input_event *event)
    {
        if (-1 == m_deviceFd)
            return false;

        if (0 >= read(m_deviceFd, event, sizeof(input_event)))
            return false;

        return true;
    }

    bool ATouchEvent::GetTouchEvent(TouchEvent *touchEvent)
    {
        static std::vector<input_event> cachedEventQueue;
        static int lastTouchPointX = 0, lastTouchPointY = 0;

        if (-1 == m_deviceFd)
            return false;

        input_event event{};
        if (0 >= read(m_deviceFd, &event, sizeof(input_event)))
            return false;

        // Check if event is not a submit event
        if (EV_SYN != event.type || SYN_REPORT != event.code || 0 != event.value)
        {
            cachedEventQueue.push_back(event);
            return false;
        }
        else if (cachedEventQueue.empty())
            return false;

        touchEvent->type = EventType::Move;
        touchEvent->x = lastTouchPointX;
        touchEvent->y = lastTouchPointY;
        touchEvent->keyCode = 0;
        for (const auto &processEvent : cachedEventQueue)
        {
            switch (processEvent.type)
            {
            case EV_KEY:
            {
                if (BTN_TOUCH == processEvent.code)
                {
                    touchEvent->type = 1 == processEvent.value ? EventType::TouchDown : EventType::TouchUp;
                    break;
                }

                touchEvent->scanCode = processEvent.code;
                touchEvent->keyCode = g_scanCodeMapping[touchEvent->scanCode];
                touchEvent->type = 1 == processEvent.value ? EventType::KeyDown : EventType::KeyUp;
                break;
            }
            case EV_REL:
            {
                switch (processEvent.code)
                {
                case 6:
                    touchEvent->y = processEvent.value;
                    touchEvent->type = EventType::Wheel;
                    break;
                case 8:
                    touchEvent->x = processEvent.value;
                    break;
                default:
                    break;
                }
            }
            case EV_ABS:
            {
                if (ABS_MT_SLOT == processEvent.code && 0 != processEvent.value) // Filter multi touch(Unsupported)
                {
                    cachedEventQueue.clear();
                    return false;
                }

                if (ABS_MT_POSITION_X == processEvent.code)
                {
                    touchEvent->x = processEvent.value;
                    lastTouchPointX = touchEvent->x;
                }
                else if (ABS_MT_POSITION_Y == processEvent.code)
                {
                    touchEvent->y = processEvent.value;
                    lastTouchPointY = touchEvent->y;
                }
            }
            default:
                break;
            }
        }
        cachedEventQueue.clear();

        return true;
    }
}