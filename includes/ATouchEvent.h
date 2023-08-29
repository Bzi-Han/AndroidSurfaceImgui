#ifndef A_TOUCH_EVENT_H // !A_TOUCH_EVENT_H
#define A_TOUCH_EVENT_H

#include "Global.h"

#include <fcntl.h>
#include <unistd.h>
#include <linux/input.h>
#include <android/keycodes.h>

#include <array>
#include <bitset>
#include <filesystem>
#include <vector>

namespace android
{
    namespace detail
    {
        template <std::size_t BITS>
        class BitArray
        {
            /* Array element type and vector of element type. */
            using Element = std::uint32_t;
            /* Number of bits in each BitArray element. */
            static constexpr size_t WIDTH = sizeof(Element) * CHAR_BIT;
            /* Number of elements to represent a bit array of the specified size of bits. */
            static constexpr size_t COUNT = (BITS + WIDTH - 1) / WIDTH;

        public:
            /* BUFFER type declaration for BitArray */
            using Buffer = std::array<Element, COUNT>;
            /* To tell if a bit is set in array, it selects an element from the array, and test
             * if the relevant bit set.
             * Note the parameter "bit" is an index to the bit, 0 <= bit < BITS.
             */
            inline bool test(size_t bit) const
            {
                return (bit < BITS) ? mData[bit / WIDTH].test(bit % WIDTH) : false;
            }
            /* Returns total number of bytes needed for the array */
            inline size_t bytes() { return (BITS + CHAR_BIT - 1) / CHAR_BIT; }
            /* Returns true if array contains any non-zero bit from the range defined by start and end
             * bit index [startIndex, endIndex).
             */
            bool any(size_t startIndex, size_t endIndex)
            {
                if (startIndex >= endIndex || startIndex > BITS || endIndex > BITS + 1)
                {
                    LogDebug("Invalid start/end index. start = %zu, end = %zu, total bits = %zu", startIndex,
                             endIndex, BITS);
                    return false;
                }
                size_t se = startIndex / WIDTH; // Start of element
                size_t ee = endIndex / WIDTH;   // End of element
                size_t si = startIndex % WIDTH; // Start index in start element
                size_t ei = endIndex % WIDTH;   // End index in end element
                // Need to check first unaligned bitset for any non zero bit
                if (si > 0)
                {
                    size_t nBits = se == ee ? ei - si : WIDTH - si;
                    // Generate the mask of interested bit range
                    Element mask = ((1 << nBits) - 1) << si;
                    if (mData[se++].to_ulong() & mask)
                    {
                        return true;
                    }
                }
                // Check whole bitset for any bit set
                for (; se < ee; se++)
                {
                    if (mData[se].any())
                    {
                        return true;
                    }
                }
                // Need to check last unaligned bitset for any non zero bit
                if (ei > 0 && se <= ee)
                {
                    // Generate the mask of interested bit range
                    Element mask = (1 << ei) - 1;
                    if (mData[se].to_ulong() & mask)
                    {
                        return true;
                    }
                }
                return false;
            }
            /* Load bit array values from buffer */
            void loadFromBuffer(const Buffer &buffer)
            {
                for (size_t i = 0; i < COUNT; i++)
                {
                    mData[i] = std::bitset<WIDTH>(buffer[i]);
                }
            }

        private:
            std::array<std::bitset<WIDTH>, COUNT> mData;
        };
    }

    class ATouchEvent
    {
    public:
        enum class EventType : uint32_t
        {
            Move,
            TouchDown,
            TouchUp,
            KeyDown,
            KeyUp,
            Wheel,
        };

        struct TouchEvent
        {
            EventType type;
            int x;
            int y;
            int scanCode;
            int keyCode;

            void TransformToScreen(int width, int height, int theta = 0)
            {
                auto k = x, l = width;
                if (90 == theta)
                {
                    x = y;
                    y = transformScalerX - k;
                    width = height;
                    height = l;
                }
                else if (180 == theta)
                {
                    x = transformScalerX - x;
                    y = transformScalerY - y;
                }
                else if (270 == theta)
                {
                    x = transformScalerY - y;
                    y = k;
                    width = height;
                    height = l;
                }

                x = x * width / transformScalerX;
                y = y * height / transformScalerY;
            }
        };

    public:
        ATouchEvent();
        ~ATouchEvent();

        bool GetRawEvent(input_event *event);

        bool GetTouchEvent(TouchEvent *touchEvent);

    public:
        static int transformScalerX, transformScalerY;

    private:
        int m_deviceFd = -1;

        typename detail::BitArray<KEY_MAX>::Buffer m_keyBitBuffer;
        detail::BitArray<KEY_MAX> m_keyBitmask;
    };
}

#endif // !A_TOUCH_EVENT_H
