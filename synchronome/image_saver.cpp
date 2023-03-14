#include <cmath>
#include <fcntl.h>
#include <sys/syslog.h>
#include <tuple>
#include <unistd.h>

#include "image_saver.hpp"
#include "util.hpp"


void ImageSaver::dumpPgm(const void *p, int size) const
{
    double fnow = floatTime();
    // syslog(LOG_CRIT, "Start writting to flash at %lf, %d bytes\n", (fnow- mStartTime), size);

    char filename[32];
    snprintf(filename, sizeof(filename), "frames/test%04llu.pgm", mFrameCount);
    int dumpfd = open(filename, O_WRONLY | O_NONBLOCK | O_CREAT, 00666);
    if (dumpfd == -1)
    {
        errnoExit("Failed to open file");
    }

    // syslog(LOG_CRIT, "The file name is %s\n", filename);

    char header[64];
    long seconds = std::lround(fnow);
    long milliseconds = std::lround(1000.0 * (fnow - seconds));
    int headerSize =
        snprintf(header, sizeof(header), "P5\n#%010d sec %010d msec \n %d %d \n255\n", seconds, milliseconds, 640, 480);

    // syslog(LOG_CRIT, "The header is name is %s\n", header);

    write(dumpfd, header, headerSize);
    int total = 0;
    while (total < size)
    {
        total += write(dumpfd, p, size);
    }

    syslog(LOG_CRIT, "Frame written to flash at %lf, %d bytes\n", (floatTime() - mStartTime), total);
    close(dumpfd);
}


void ImageSaver::dumpPpm(const void *p, int size) const
{
    double fnow = floatTime();
    // syslog(LOG_CRIT, "Start writing to flash at %lf, %d bytes\n", (fnow - mStartTime), size);

    char filename[32];
    snprintf(filename, sizeof(filename), "frames/test%04llu.ppm", mFrameCount);
    int dumpfd = open(filename, O_WRONLY | O_NONBLOCK | O_CREAT, 00666);
    if (dumpfd == -1)
    {
        errnoExit("Failed to open file");
    }

    // syslog(LOG_CRIT, "The file name is %s\n", filename);

    char header[64];
    long seconds = std::lround(fnow);
    long milliseconds = std::lround(1000.0 * (fnow - static_cast<double>(seconds)));
    int headerSize =
        snprintf(header, sizeof(header), "P6\n#%010d sec %010d msec \n %d %d \n255\n", seconds, milliseconds, 640, 480);

    // syslog(LOG_CRIT, "The header is name is %s\n", header);

    write(dumpfd, header, headerSize);
    int total = 0;
    while (total < size)
    {
        total += write(dumpfd, p, size);
    }

    syslog(LOG_CRIT, "Frame written to flash at %lf, %d bytes\n", (floatTime() - mStartTime), total);
    close(dumpfd);
}


void ImageSaver::colorConvert(const unsigned char *pptr, int size)
{
    syslog(LOG_CRIT, "Converting pixels %lf\n", floatTime() - mStartTime);
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

    // Pixels are YU and YV alternating, so YUYV which is 4 bytes. We want RGB, so RGBRGB which is 6 bytes.
    for (int i = 0, newi = 0; i < size; i = i + 4, newi = newi + 6)
    {
        std::tie(mBigbuffer[newi], mBigbuffer[newi + 1], mBigbuffer[newi + 2]) =
            yuv2rgb(pptr[i], pptr[i + 1], pptr[i + 3]);
        std::tie(mBigbuffer[newi + 3], mBigbuffer[newi + 4], mBigbuffer[newi + 5]) =
            yuv2rgb(pptr[i + 2], pptr[i + 1], pptr[i + 3]);
    }
    syslog(LOG_CRIT, "Finished converting pixels %lf\n", floatTime() - mStartTime);
}


void ImageSaver::processImage(const void *p, int size, const struct v4l2_format &fmt)
{
    // TODO: size checking,
    double fnow = floatTime();
    mFrameCount++;
    syslog(LOG_CRIT, "Processing frame %d: ", mFrameCount);

    if (mFrameCount == 0)
    {
        mStartTime = fnow;
    }

    if (fmt.fmt.pix.pixelformat == V4L2_PIX_FMT_GREY)
    {
        syslog(LOG_CRIT, "Dump graymap as-is size %d\n", size);
        dumpPgm(p, size);
    }
    else if (fmt.fmt.pix.pixelformat == V4L2_PIX_FMT_YUYV)
    {
        syslog(LOG_CRIT, "Dump YUYV converted to RGB size %d\n", size);
        colorConvert(reinterpret_cast<const unsigned char *>(p), size);
        dumpPpm(mBigbuffer, ((size * 6) / 4));
    }
    else if (fmt.fmt.pix.pixelformat == V4L2_PIX_FMT_RGB24)
    {
        syslog(LOG_CRIT, "Dump RGB as-is size %d\n", size);
        dumpPpm(p, size);
    }
    else
    {
        syslog(LOG_CRIT, "ERROR - unknown dump format\n");
    }
}
