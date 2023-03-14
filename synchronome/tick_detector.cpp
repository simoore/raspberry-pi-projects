#include <syslog.h>

#include "tick_detector.hpp"
#include "util.hpp"


/// Sum the difference between the Y (grey) pixels of two YUYV images.
uint32_t TickDetector::sumDifference(size_t size, const uint8_t *newImg, const uint8_t *oldImg) const
{
    uint32_t sum = 0;
    for (size_t i = 0; i < size; i = i + 4)
    {
        sum += (newImg[i] - oldImg[i]) + (newImg[i + 2] + oldImg[i + 2]);
    }
    return sum;
}

/// The technique we use to determine the unique frame for each second is:
/// 1. Convert image to grey-scale.
/// 2. Take the difference between this frame and the last frame.
/// 3. Take the sum of the difference as a percentage of the max difference possible.
/// 4. Threshold with hystersis the percentage difference to determine when a transition has been made.
///
/// We return true on the transition from moving to still.
bool TickDetector::execute(BufferHandler &newImg)
{
    if (mCount == 0)
    {
        mStartTime = floatTime();
        mExpectedSize = newImg.mSize;
        mOldImage = newImg;
        mMaxDiff = newImg.mSize * 255;
        ++mCount;
        return false;
    }

    if (mExpectedSize != newImg.mSize)
    {
        printf("Expected image size and actual image size don't match");
        exit(EXIT_FAILURE);
    }

    ++mCount;
    double timeNow = floatTime();
    uint32_t sum = sumDifference(
        mExpectedSize, reinterpret_cast<uint8_t *>(newImg.mStart), reinterpret_cast<uint8_t *>(mOldImage.mStart));
    double percentDiff = static_cast<double>(sum) / mMaxDiff;
    mOldImage = newImg;

    syslog(
        LOG_CRIT, "TickDetector: time, %lf, percent diff, %lf, cnt, %u\n", (timeNow - mStartTime), percentDiff, mCount);

    if (mState == ImgState::Still && percentDiff > sMovingThreshold)
    {
        mState = ImgState::Moving;
    }
    else if (mState == ImgState::Moving && percentDiff < sStillThreshold)
    {
        mState = ImgState::Still;
        return true;
    }
    return false;
}
