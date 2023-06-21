#include "ATouchEvent.h"

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

        touchEvent->type = EventType::move;
        touchEvent->x = lastTouchPointX;
        touchEvent->y = lastTouchPointY;
        for (const auto &processEvent : cachedEventQueue)
        {
            switch (processEvent.type)
            {
            case EV_KEY:
            {
                if (BTN_TOUCH != processEvent.code)
                    break;

                touchEvent->type = 1 == processEvent.value ? EventType::touchDown : EventType::touchUp;
                break;
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