#include <fcntl.h>
#include <filesystem>
#include <string>
#include <tuple>
#include <unistd.h>

class ImageProcessor {
public:
    ///////////////////////////////////////////////////////////////////////////
    // PUBLIC TYPES
    ///////////////////////////////////////////////////////////////////////////

    using Pixel = std::tuple<unsigned char, unsigned char, unsigned char>;
    using Timespec = struct timespec;

    ///////////////////////////////////////////////////////////////////////////
    // PUBLIC FUNCTIONS
    ///////////////////////////////////////////////////////////////////////////

    /// Writes the image pointed to by `p` to file using the ppm format.
    /// @param p
    /// @param size
    /// @param tag
    /// @param time
    void dumpPpm(const void *p, int size, unsigned int tag, Timespec *time) const {
        
        std::filesystem::path path = std::format("frames/test{:04d}.ppm", tag);
        int dumpfd = open(path.c_str(), O_WRONLY | O_NONBLOCK | O_CREAT, 00666);

        snprintf(&ppm_header[4], 11, "%010d", (int)time->tv_sec);
        strncat(&ppm_header[14], " sec ", 5);
        snprintf(&ppm_header[19], 11, "%010d", (int)((time->tv_nsec) / 1000000));
        strncat(&ppm_header[29], " msec \n" HRES_STR " " VRES_STR "\n255\n", 19);

        // subtract 1 from sizeof header because it includes the null terminator for the string
        int written = write(dumpfd, ppm_header, sizeof(ppm_header) - 1);
        int total = 0;
        do {
            written = write(dumpfd, p, size);
            total += written;
        } while (total < size);

        Timespec timeNow;
        clock_gettime(CLOCK_MONOTONIC, &timeNow);
        double fnow = (double)timeNow.tv_sec + (double)timeNow.tv_nsec / 1000000000.0;
        printf("Frame written to flash at %lf, %d, bytes\n", (fnow - fstart), total);

        close(dumpfd);
    }

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
    ///      YUV420, where for every 4 Ys, there is a single UV pair, 1.5 bytes for each pixel or 36 bytes for 24 pixels
    static Pixel yuv2rgb(int y, int u, int v) {
        int c = y - 16, d = u - 128, e = v - 128;
        int r1 = (298 * c + 409 * e + 128) >> 8;
        int g1 = (298 * c - 100 * d - 208 * e + 128) >> 8;
        int b1 = (298 * c + 516 * d + 128) >> 8;

        // Computed values may need clipping.
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
    }

    void colorConvert(unsigned char *pptr, int size) {
        int yTemp, y2Temp, uTemp, vTemp;

        // Pixels are YU and YV alternating, so YUYV which is 4 bytes. We want RGB, so RGBRGB which is 6 bytes.
        for (int i = 0, newi = 0; i < size; i = i + 4, newi = newi + 6) {
            yTemp = (int)pptr[i];
            uTemp = (int)pptr[i + 1];
            y2Temp = (int)pptr[i + 2];
            vTemp = (int)pptr[i + 3];
            yuv2rgb(yTemp, uTemp, vTemp, &mBigbuffer[newi], &mBigbuffer[newi + 1], &mBigbuffer[newi + 2]);
            yuv2rgb(y2Temp, uTemp, vTemp, &mBigbuffer[newi + 3], &mBigbuffer[newi + 4], &mBigbuffer[newi + 5]);
        }
    }

    template <bool dumpFrames, bool colorConvert, bool dumpPpm> void processImage(const void *p, int size) {
        int i, newi, newsize = 0;
        struct timespec frameTime;
        unsigned char *pptr = reinterpret_cast<unsigned char *>(p);

        // record when process was called
        clock_gettime(CLOCK_REALTIME, &frameTime);

        mFrameCnt++;
        printf("frame %d: ", mFrameCnt);

        if (mFrameCnt == 0) {
            clock_gettime(CLOCK_MONOTONIC, &time_start);
            fstart = (double)time_start.tv_sec + (double)time_start.tv_nsec / 1000000000.0;
        }

        if constexpr (dumpFrames) {

            // This just dumps the frame to a file now, but you could replace with whatever image
            // processing you wish.
            //

            if (fmt.fmt.pix.pixelformat == V4L2_PIX_FMT_GREY) {
                printf("Dump graymap as-is size %d\n", size);
                dump_pgm(p, size, framecnt, &frame_time);
            } else if (fmt.fmt.pix.pixelformat == V4L2_PIX_FMT_YUYV) {
                if constexpr (colorConvert) {
                    colorConvert(pptr, size);
                }

                if constexpr (dumpPpm) {
                    if (framecnt > -1) {
                        dump_ppm(bigbuffer, ((size * 6) / 4), framecnt, &frame_time);
                        printf("Dump YUYV converted to RGB size %d\n", size);
                    }
                } else {

                    // Pixels are YU and YV alternating, so YUYV which is 4 bytes
                    // We want Y, so YY which is 2 bytes
                    //
                    for (i = 0, newi = 0; i < size; i = i + 4, newi = newi + 2) {
                        // Y1=first byte and Y2=third byte
                        bigbuffer[newi] = pptr[i];
                        bigbuffer[newi + 1] = pptr[i + 2];
                    }

                    if (framecnt > -1) {
                        dump_pgm(bigbuffer, (size / 2), framecnt, &frame_time);
                        // printf("Dump YUYV converted to YY size %d\n", size);
                    }
                }

            }

            else if (fmt.fmt.pix.pixelformat == V4L2_PIX_FMT_RGB24) {
                printf("Dump RGB as-is size %d\n", size);
                dump_ppm(p, size, framecnt, &frame_time);
            } else {
                printf("ERROR - unknown dump format\n");
            }
        }
    }

private:
    ///////////////////////////////////////////////////////////////////////////
    // PRIVATE FIELDS
    ///////////////////////////////////////////////////////////////////////////

    unsigned char mBigbuffer[(1280 * 960)];
    int64_t mFrameCnt{-1};
};