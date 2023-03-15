#include <cmath>
#include <fcntl.h>
#include <sys/syslog.h>
#include <tuple>
#include <unistd.h>

#include "image_saver.hpp"
#include "util.hpp"


int ImageSaver::dumpPgm(const void *p, int size) const
{
    double fnow = floatTime();

    char filename[32];
    snprintf(filename, sizeof(filename), "frames/test%04llu.pgm", mFrameCount);
    int dumpfd = open(filename, O_WRONLY | O_NONBLOCK | O_CREAT, 00666);
    if (dumpfd == -1)
    {
        errnoExit("Failed to open file");
    }

    char header[64];
    long seconds = std::lround(fnow);
    long milliseconds = std::lround(1000.0 * (fnow - seconds));
    int headerSize =
        snprintf(header, sizeof(header), "P5\n#%010d sec %010d msec \n %d %d \n255\n", seconds, milliseconds, 640, 480);

    write(dumpfd, header, headerSize);
    int total = 0;
    while (total < size)
    {
        total += write(dumpfd, p, size);
    }

    close(dumpfd);
    return total;
}


int ImageSaver::dumpPpm(const void *p, int size) const
{
    double fnow = floatTime();

    char filename[32];
    snprintf(filename, sizeof(filename), "frames/test%04llu.ppm", mFrameCount);
    int dumpfd = open(filename, O_WRONLY | O_NONBLOCK | O_CREAT, 00666);
    if (dumpfd == -1)
    {
        errnoExit("Failed to open file");
    }

    char header[64];
    long seconds = std::lround(fnow);
    long milliseconds = std::lround(1000.0 * (fnow - static_cast<double>(seconds)));
    int headerSize =
        snprintf(header, sizeof(header), "P6\n#%010d sec %010d msec \n %d %d \n255\n", seconds, milliseconds, 640, 480);

    write(dumpfd, header, headerSize);
    int total = 0;
    while (total < size)
    {
        total += write(dumpfd, p, size);
    }

    close(dumpfd);
    return total;
}


void ImageSaver::processImage(const uint8_t *p, int size, double startTime)
{
    double fnow = floatTime();
    mFrameCount++;
    syslog(LOG_CRIT, "ImageSaver: Processing frame %d: ", mFrameCount);
    int total = dumpPpm(p, size);
    syslog(LOG_CRIT, "ImageSaver: Frame written to flash at %lf, %d bytes\n", (floatTime() - startTime), total);
}
