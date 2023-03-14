#pragma once

#include <linux/videodev2.h>
#include <tuple>

#include "service.hpp"
#include "util.hpp"

class ImageSaver
{
public:
    ///////////////////////////////////////////////////////////////////////////
    // PUBLIC TYPES
    ///////////////////////////////////////////////////////////////////////////

    using Pixel = std::tuple<unsigned char, unsigned char, unsigned char>;
    using Timespec = struct timespec;

    ///////////////////////////////////////////////////////////////////////////
    // PUBLIC FUNCTIONS
    ///////////////////////////////////////////////////////////////////////////

    /// Writes the image pointed to by `p` to file using the pgm format. `size` is the number of bytes and fnow is
    /// a time in seconds to add to the header of the image.
    void dumpPgm(const void *p, int size) const;

    /// Writes the image pointed to by `p` to file using the ppm format. `size` is the number of bytes and fnow is
    /// a time in seconds to add to the header of the image.
    void dumpPpm(const void *p, int size) const;

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
    void colorConvert(const unsigned char *pptr, int size);

    void processImage(const void *p, int size, const struct v4l2_format &fmt);

private:
    ///////////////////////////////////////////////////////////////////////////
    // PRIVATE FIELDS
    ///////////////////////////////////////////////////////////////////////////

    int64_t mFrameCount{-1};
    double mStartTime;
    unsigned char mBigbuffer[(1280 * 960)];
};
