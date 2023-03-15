#pragma once

#include <linux/videodev2.h>

#include "util.hpp"

/// This class contains the information associated with a video buffer from the camera that is needed by different
/// parts of the application to use the information in the buffer. Call `returnBuffer` once the buffer is no longer
/// required to send it back to the driver for re-use.
struct BufferHandler
{
    ///////////////////////////////////////////////////////////////////////////
    // PUBLIC TYPES
    ///////////////////////////////////////////////////////////////////////////

    using V4l2Format = struct v4l2_format;
    using V4l2Buffer = struct v4l2_buffer;

    ///////////////////////////////////////////////////////////////////////////
    // PUBLIC FUNCTIONS
    ///////////////////////////////////////////////////////////////////////////

    BufferHandler() = default;

    BufferHandler(const V4l2Format &fmt, const int fd) : mFmt(fmt), mFd(fd) {}

    void returnBuffer()
    {
        if (-1 == xioctl(mFd, VIDIOC_QBUF, &mBuf))
        {
            errnoExit("Buffer re-queueing error");
        }
    }

    ///////////////////////////////////////////////////////////////////////////
    // PUBLIC FIELDS
    ///////////////////////////////////////////////////////////////////////////

    V4l2Format mFmt;
    V4l2Buffer mBuf;
    void *mStart;
    size_t mSize;
    int mFd;
};
