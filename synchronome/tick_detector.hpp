#pragma once

#include <cstddef>
#include <cstdint>

#include "buffer_handler.hpp"


class TickDetector final
{
public:
    ///////////////////////////////////////////////////////////////////////////
    // PUBLIC CONSTANTS
    ///////////////////////////////////////////////////////////////////////////

    static constexpr double sMovingThreshold = 5.0;
    static constexpr double sStillThreshold = 0.1;

    ///////////////////////////////////////////////////////////////////////////
    // PUBLIC TYPES
    ///////////////////////////////////////////////////////////////////////////

    enum class ImgState
    {
        Moving,
        Still
    };

    ///////////////////////////////////////////////////////////////////////////
    // PUBLIC FUNCTIONS
    ///////////////////////////////////////////////////////////////////////////

    /// Sum the difference between the Y (grey) pixels of two YUYV images.
    uint32_t sumDifference(size_t size, const uint8_t *newImg, const uint8_t *oldImg) const;

    /// The technique we use to determine the unique frame for each second is:
    /// 1. Convert image to grey-scale.
    /// 2. Take the difference between this frame and the last frame.
    /// 3. Take the sum of the difference as a percentage of the max difference possible.
    /// 4. Threshold with hystersis the percentage difference to determine when a transition has been made.
    ///
    /// We return true on the transition from moving to still.
    bool execute(BufferHandler &newImg);

private:
    ///////////////////////////////////////////////////////////////////////////
    // PRIVATE FIELDS
    ///////////////////////////////////////////////////////////////////////////

    BufferHandler mOldImage;
    double mMaxDiff;
    double mStartTime;
    size_t mCount{0};
    size_t mExpectedSize;
    ImgState mState{ImgState::Still};
};
