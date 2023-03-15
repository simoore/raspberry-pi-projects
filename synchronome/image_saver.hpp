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

    using Timespec = struct timespec;

    ///////////////////////////////////////////////////////////////////////////
    // PUBLIC FUNCTIONS
    ///////////////////////////////////////////////////////////////////////////

    /// Writes the image pointed to by `p` to file using the pgm format. `size` is the number of bytes and fnow is
    /// a time in seconds to add to the header of the image.
    int dumpPgm(const void *p, int size) const;

    /// Writes the image pointed to by `p` to file using the ppm format. `size` is the number of bytes and fnow is
    /// a time in seconds to add to the header of the image.
    int dumpPpm(const void *p, int size) const;

    void processImage(const uint8_t *p, int size, double startTime);

private:
    ///////////////////////////////////////////////////////////////////////////
    // PRIVATE FIELDS
    ///////////////////////////////////////////////////////////////////////////

    int64_t mFrameCount{-1};
};
