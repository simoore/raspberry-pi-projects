#pragma once

#include <cstddef>
#include <cstdint>
#include <tuple>
#include <mutex>
#include <queue>

#include "rgb_handler.hpp"
#include "buffer_handler.hpp"


class TickDetector final : public RgbHandler::Allocator
{
public:
    ///////////////////////////////////////////////////////////////////////////
    // PUBLIC CONSTANTS
    ///////////////////////////////////////////////////////////////////////////

    static constexpr double sMovingThreshold = 0.0023;
    static constexpr double sStillThreshold = 0.0022;
    static constexpr size_t sBufferSize = 640*480*3;
    static constexpr size_t sNumOfBuffers = 20;

    ///////////////////////////////////////////////////////////////////////////
    // PUBLIC TYPES
    ///////////////////////////////////////////////////////////////////////////

    using RgbBuffer = uint8_t[sBufferSize];

    struct Config
    {
        double startTime;
        bool showDiff;
    };

    enum class ImgState
    {
        Moving,
        Still
    };

    using Pixel = std::tuple<uint8_t, uint8_t, uint8_t>;

    ///////////////////////////////////////////////////////////////////////////
    // PUBLIC FUNCTIONS
    ///////////////////////////////////////////////////////////////////////////

    /// Initialize the available queue to allocate RGB buffers to the applications.
    TickDetector();

    /// COnfiguration setter.
    void setConfig(const Config &cfg);

    /// Places a RGB buffer back on the available queue.
    void returnBuffer(RgbHandler &handler) override;

    /// Get a RGB buffer from the queue. The RgbHandler::mStart field is nullptr if no buffers are available.
    RgbHandler allocate(void);

    /// This is probably the most acceptable conversion from camera YUYV to RGB
    ///
    /// Wikipedia has a good discussion on the details of various conversions and cites good references:
    /// http://en.wikipedia.org/wiki/YUV
    ///
    /// Also http://www.fourcc.org/yuv.php
    ///
    /// What's not clear without knowing more about the camera in question is how often U & V are sampled compared
    /// to Y.
    ///
    /// E.g. YUV444, which is equivalent to RGB, where both require 3 bytes for each pixel
    ///      YUV422, which we assume here, where there are 2 bytes for each pixel, with two Y samples for one U & V,
    ///              or as the name implies, 4Y and 2 UV pairs
    ///      YUV420, where for every 4 Ys, there is a single UV pair, 1.5 bytes for each pixel or 36 bytes for 24
    ///              pixels
    //void colorConvert(const unsigned char *pptr, int size);
    RgbHandler colorConvert(const BufferHandler &bufferHandler);

    /// Sum the difference between the Y (grey) pixels of two YUYV images.
    uint32_t sumDifference(size_t size, const uint8_t *newImg, const uint8_t *oldImg) const;

    /// The technique we use to determine the unique frame for each second is:
    /// 1. Convert image to grey-scale.
    /// 2. Take the difference between this frame and the last frame.
    /// 3. Take the sum of the difference as a percentage of the max difference possible.
    /// 4. Threshold with hystersis the percentage difference to determine when a transition has been made.
    ///
    /// We return true on the transition from moving to still.
    RgbHandler execute(BufferHandler &yuyvHandler);

private:
    ///////////////////////////////////////////////////////////////////////////
    // PRIVATE FIELDS
    ///////////////////////////////////////////////////////////////////////////

    Config mConfig;
    RgbHandler mOldImage;
    double mMaxDiff;
    size_t mCount{0};
    size_t mExpectedSize;
    ImgState mState{ImgState::Still};
    std::mutex mRgbQueueMutex;
    std::queue<RgbHandler> mAvailable;
    RgbBuffer mBuffers[sNumOfBuffers];
};
