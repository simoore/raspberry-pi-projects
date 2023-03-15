#include <syslog.h>

#include "tick_detector.hpp"
#include "util.hpp"


TickDetector::TickDetector()
{
    for (int i = 0; i < sNumOfBuffers; i++)
    {
        mAvailable.emplace(RgbHandler{&mBuffers[i][0], this});
    }
}


void TickDetector::setConfig(const Config &cfg) { mConfig = cfg; }


void TickDetector::returnBuffer(RgbHandler &handler)
{
    std::lock_guard<std::mutex> guard(mRgbQueueMutex);
    if (handler.mStart)
    {
        handler.mIsTick = false;
        mAvailable.push(handler);
    }
    if (mAvailable.size() > sNumOfBuffers)
    {
        syslog(LOG_CRIT, "Available queue has too many elements.");
        exit(EXIT_FAILURE);
    }
}


RgbHandler TickDetector::allocate(void)
{
    std::lock_guard<std::mutex> guard(mRgbQueueMutex);
    if (mAvailable.empty())
    {
        return RgbHandler{};
    }
    RgbHandler rgb = mAvailable.front();
    mAvailable.pop();
    return rgb;
}


RgbHandler TickDetector::colorConvert(const BufferHandler &bufferHandler)
{
    syslog(LOG_CRIT, "TickDetector: Converting pixels %lf\n", floatTime() - mConfig.startTime);
    auto yuv2rgb = [](int y, int u, int v) -> Pixel
    {
        int c = y - 16, d = u - 128, e = v - 128;
        int r1 = (298 * c + 409 * e + 128) >> 8;
        int g1 = (298 * c - 100 * d - 208 * e + 128) >> 8;
        int b1 = (298 * c + 516 * d + 128) >> 8;

        if (r1 > 255)
            r1 = 255;
        if (g1 > 255)
            g1 = 255;
        if (b1 > 255)
            b1 = 255;

        if (r1 < 0)
            r1 = 0;
        if (g1 < 0)
            g1 = 0;
        if (b1 < 0)
            b1 = 0;

        return std::make_tuple(r1, g1, b1);
    };

    RgbHandler rgb = allocate();
    if (!rgb.mStart)
    {
        return rgb;
    }

    // Pixels are YU and YV alternating, so YUYV which is 4 bytes. We want RGB, so RGBRGB which is 6 bytes.
    auto pptr = reinterpret_cast<uint8_t *>(bufferHandler.mStart);
    for (int i = 0, newi = 0; i < bufferHandler.mSize; i = i + 4, newi = newi + 6)
    {
        std::tie(rgb.mStart[newi], rgb.mStart[newi + 1], rgb.mStart[newi + 2]) =
            yuv2rgb(pptr[i], pptr[i + 1], pptr[i + 3]);
        std::tie(rgb.mStart[newi + 3], rgb.mStart[newi + 4], rgb.mStart[newi + 5]) =
            yuv2rgb(pptr[i + 2], pptr[i + 1], pptr[i + 3]);
    }
    rgb.mSize = ((bufferHandler.mSize * 6) / 4);
    syslog(LOG_CRIT, "TickDetector: Finished converting pixels %lf\n", floatTime() - mConfig.startTime);
    return rgb;
}


uint32_t TickDetector::sumDifference(size_t size, const uint8_t *newImg, const uint8_t *oldImg) const
{
    auto absDiff = [](uint8_t a, uint8_t b) -> uint8_t { return a > b ? a - b : b - a; };

    uint32_t sum = 0;
    for (size_t i = 0; i < size; i = i + 3)
    {
        sum += absDiff(newImg[i], oldImg[i]) + absDiff(newImg[i + 1], oldImg[i + 1]) +
               absDiff(newImg[i + 2], oldImg[i + 2]);
    }
    return sum;
}


RgbHandler TickDetector::execute(BufferHandler &yuyvHandler)
{
    RgbHandler rgb = colorConvert(yuyvHandler);
    if (!rgb.mStart)
    {
        syslog(LOG_CRIT, "TickDetector: Failed to allocate rgb buffer.");
        exit(EXIT_FAILURE);
    }

    if (mCount == 0)
    {
        mExpectedSize = rgb.mSize;
        mOldImage = rgb;
        mMaxDiff = static_cast<double>(rgb.mSize) * 255.0;
        syslog(LOG_CRIT, "TickDetector: mMaxDiff is %lf\n", mMaxDiff);
        ++mCount;
        return RgbHandler{};
    }

    if (mExpectedSize != rgb.mSize)
    {
        printf("Expected image size and actual image size don't match");
        exit(EXIT_FAILURE);
    }

    ++mCount;
    double timeNow = floatTime() - mConfig.startTime;
    uint32_t sum = sumDifference(mExpectedSize, rgb.mStart, mOldImage.mStart);
    double percentDiff = static_cast<double>(sum) / mMaxDiff;
    syslog(LOG_CRIT, "TickDetector: time %lf, percent diff %lf, cnt %u, sum %u\n", timeNow, percentDiff, mCount, sum);
    rgb.mIsTick = false;

    if (mState == ImgState::Still && percentDiff > sMovingThreshold)
    {
        mState = ImgState::Moving;
    }
    else if (mState == ImgState::Moving && percentDiff < sStillThreshold)
    {
        mState = ImgState::Still;
        rgb.mIsTick = true;
        syslog(LOG_CRIT, "TickDetector: tick on image %u\n", mCount);
    }

    RgbHandler returnImage = mOldImage;
    mOldImage = rgb;

    if (mConfig.showDiff)
    {
        auto absDiff = [](uint8_t a, uint8_t b) -> uint8_t { return a > b ? a - b : b - a; };

        auto modImg = [absDiff](size_t size, const uint8_t *newImg, uint8_t *oldImg)
        {
            for (size_t i = 0; i < size; i = i + 3)
            {
                oldImg[i] = absDiff(newImg[i], oldImg[i]);
                oldImg[i + 1] = absDiff(newImg[i + 1], oldImg[i + 1]);
                oldImg[i + 2] = absDiff(newImg[i + 2], oldImg[i + 2]);
            }
        };

        modImg(mExpectedSize, mOldImage.mStart, returnImage.mStart);
    }

    return returnImage;
}
