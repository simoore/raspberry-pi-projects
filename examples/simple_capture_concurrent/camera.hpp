#pragma once

#include <cassert>
#include <cstring>
#include <fcntl.h>
#include <linux/videodev2.h>
#include <memory>
#include <string>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

class Camera {
public:
    ///////////////////////////////////////////////////////////////////////////
    // PUBLIC CONSTANTS
    ///////////////////////////////////////////////////////////////////////////

    static constexpr unsigned int sHorRes = 640;
    static constexpr unsigned int sVerRes = 480;
    static constexpr unsigned int sRequestBuffers = 6;

    ///////////////////////////////////////////////////////////////////////////
    // PUBLIC TYPES
    ///////////////////////////////////////////////////////////////////////////

    using Stat = struct stat;
    using V4l2Capability = struct v4l2_capability;
    using V4l2Cropcap = struct v4l2_cropcap;
    using V4l2Crop = struct v4l2_crop;
    using V4l2RequestBuffers = struct v4l2_requestbuffers;
    using V4l2Format = struct v4l2_format;
    using V4l2Buffer = struct v4l2_buffer;

    struct Buffer {
        void *start;
        size_t size;
    };

    /// This class places the video buffer back on the queue after it has been used.
    class BufferHandler {
    public:
        BufferHandler(const BufferHandler &) = delete;
        BufferHandler &operator=(const BufferHandler &) = delete;
        BufferHandler(void *start, size_t size, const V4l2Format &fmt, int fd, V4l2Buffer buf)
            : mStart(start), mSize(size), mFmt(fmt), mFd(fd), mBuf(buf) {}

        ~BufferHandler() {
            if (-1 == xioctl(mFd, VIDIOC_QBUF, &mBuf)) {
                errnoExit("Buffer re-queueing error");
            }
        }

    private:
        void *mStart;
        size_t mSize;
        const V4l2Format mFmt;
        const int mFd;
        V4l2Buffer mBuf;
    };

    ///////////////////////////////////////////////////////////////////////////
    // PUBLIC FUNCTIONS
    ///////////////////////////////////////////////////////////////////////////

    Camera(std::string deviceName, bool forceFormat) : mDeviceName(deviceName), mForceFormat(forceFormat) {}

    /// Zeros the contents of any type.
    template <typename T> static void clear(T &dest) { std::memset(&dest, 0, sizeof(T)); }

    /// Convience function to print an error message to std::err and then exit.
    static void errnoExit(std::string s) {
        fprintf(stderr, "%s error %d, %s\n", s.c_str(), errno, strerror(errno));
        exit(EXIT_FAILURE);
    }

    static int xioctl(int fh, int request, void *arg) {
        int r;
        do {
            r = ioctl(fh, request, arg);
        } while (-1 == r && EINTR == errno);
        return r;
    }

    /// Opens the camera device and sets the file descriptor to access it.
    void openDevice(void) {
        Stat st;

        if (-1 == stat(mDeviceName.c_str(), &st)) {
            errnoExit(std::string{"Cannot identify: "} + mDeviceName);
        }

        if (!S_ISCHR(st.st_mode)) {
            errnoExit(std::string{"No device: "} + mDeviceName);
        }

        mFd = open(mDeviceName.c_str(), O_RDWR | O_NONBLOCK, 0);

        if (-1 == mFd) {
            errnoExit(std::string{"Cannot open: "} + mDeviceName);
        }
    }

    // The driver allocates buffers for the camera in kernal space and mmap is used to make these available in
    // userspace. This function initializes the buffers and makes handles to them available in this application.
    void initMmap(void) {

        V4l2RequestBuffers req;
        clear(req);
        req.count = sRequestBuffers;
        req.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        req.memory = V4L2_MEMORY_MMAP;

        if (-1 == xioctl(mFd, VIDIOC_REQBUFS, &req)) {
            if (EINVAL == errno) {
                errnoExit(std::string{"Device does not support memory mapping: "} + mDeviceName);
            } else {
                errnoExit("Request for buffers failed");
            }
        }

        if (req.count < 2) {
            errnoExit(std::string{"Insufficient buffer memory: "} + mDeviceName);
        }

        mNumBuffers = req.count;
        mBuffers = reinterpret_cast<Buffer *>(std::calloc(req.count, sizeof(Buffer)));

        if (!mBuffers) {
            errnoExit("Out of memory");
        }

        for (unsigned int i = 0; i < req.count; ++i) {
            V4l2Buffer buf;
            clear(buf);
            buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
            buf.memory = V4L2_MEMORY_MMAP;
            buf.index = i;

            if (-1 == xioctl(mFd, VIDIOC_QUERYBUF, &buf)) {
                errnoExit("Query buffer failed");
            }

            mBuffers[i].size = buf.length;
            mBuffers[i].start = mmap(nullptr, buf.length, PROT_READ | PROT_WRITE, MAP_SHARED, mFd, buf.m.offset);

            if (MAP_FAILED == mBuffers[i].start) {
                errnoExit("Failed mmap initialization");
            }
        }
    }

    /// Initializes various properties of the video driver.
    void initDevice(void) {

        V4l2Capability cap;
        if (-1 == xioctl(mFd, VIDIOC_QUERYCAP, &cap)) {
            if (EINVAL == errno) {
                errnoExit(std::string{"No V4L2 device: "} + mDeviceName);
            } else {
                errnoExit("VIDIOC_QUERYCAP");
            }
        }

        if (!(cap.capabilities & V4L2_CAP_VIDEO_CAPTURE)) {
            errnoExit(std::string{"No video capture device: "} + mDeviceName);
        }

        if (!(cap.capabilities & V4L2_CAP_STREAMING)) {
            errnoExit(std::string{"No streaming support: "} + mDeviceName);
        }

        // Select video input, video standard and tune here.
        V4l2Cropcap cropcap;
        clear(cropcap);

        cropcap.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

        if (0 != xioctl(mFd, VIDIOC_CROPCAP, &cropcap)) {
            errnoExit("Cropcap error");
        }

        V4l2Crop crop;
        crop.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        crop.c = cropcap.defrect; /* reset to default */

        if (-1 == xioctl(mFd, VIDIOC_S_CROP, &crop)) {
            switch (errno) {
            case EINVAL:
                errnoExit("Cropping not supported");
                break;
            default:
                errnoExit("Error setting crop");
                break;
            }
        }

        clear(mFmt);

        mFmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

        if (mForceFormat) {
            printf("Forcing Format\n");
            mFmt.fmt.pix.width = sHorRes;
            mFmt.fmt.pix.height = sVerRes;

            // Specify the Pixel Coding Formate here
            mFmt.fmt.pix.pixelformat = V4L2_PIX_FMT_YUYV; // This one works for Logitech C200
            // fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_UYVY;
            // fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_VYUY;
            // fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_GREY;
            // fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_RGB24;

            // fmt.fmt.pix.field       = V4L2_FIELD_INTERLACED;
            mFmt.fmt.pix.field = V4L2_FIELD_NONE;

            // Note VIDIOC_S_FMT may change width and height.
            if (-1 == xioctl(mFd, VIDIOC_S_FMT, &mFmt)) {
                errnoExit("VIDIOC_S_FMT");
            }
        } else {
            // Preserve original settings.
            printf("Assuming format\n");
            if (-1 == xioctl(mFd, VIDIOC_G_FMT, &mFmt)) {
                errnoExit("VIDIOC_G_FMT");
            }
        }

        int min = mFmt.fmt.pix.width * 2;
        if (mFmt.fmt.pix.bytesperline < min) {
            mFmt.fmt.pix.bytesperline = min;
        }
        min = mFmt.fmt.pix.bytesperline * mFmt.fmt.pix.height;
        if (mFmt.fmt.pix.sizeimage < min) {
            mFmt.fmt.pix.sizeimage = min;
        }

        initMmap();
    }

    /// Uninitializing the device unmaps the drives kernal space buffers from the application and de-allocates the
    /// handlers used to reference them.
    void uninitDevice(void) {
        for (int i = 0; i < mNumBuffers; ++i) {
            if (-1 == munmap(mBuffers[i].start, mBuffers[i].size)) {
                errnoExit("Uninitialize device failed");
            }
        }
        free(mBuffers);
    }

    /// Closes the file descriptor associated with the camera.
    void closeDevice(void) {
        if (-1 == close(mFd)) {
            errnoExit("close");
        }
        mFd = -1;
    }

    /// Queue buffers and tell the video driver to start streaming.
    void startCapturing(void) {
        for (int i = 0; i < mNumBuffers; ++i) {
            printf("allocated buffer %d\n", i);
            V4l2Buffer buf;
            clear(buf);
            buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
            buf.memory = V4L2_MEMORY_MMAP;
            buf.index = i;
            if (-1 == xioctl(mFd, VIDIOC_QBUF, &buf)) {
                errnoExit("VIDIOC_QBUF");
            }
        }
        enum v4l2_buf_type type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        if (-1 == xioctl(mFd, VIDIOC_STREAMON, &type)) {
            errnoExit("VIDIOC_STREAMON");
        }
    }

    /// Tell the video driver to turn the stream off.
    void stopCapturing(void) {
        enum v4l2_buf_type type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        if (-1 == xioctl(mFd, VIDIOC_STREAMOFF, &type)) {
            errnoExit("VIDIOC_STREAMOFF");
        }
    }

    std::unique_ptr<BufferHandler> readFrame(void) {
        V4l2Buffer buf;
        clear(buf);

        buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        buf.memory = V4L2_MEMORY_MMAP;

        if (-1 == xioctl(mFd, VIDIOC_DQBUF, &buf)) {
            switch (errno) {
            case EAGAIN:
                return nullptr;
            case EIO:
                // Could ignore EIO, but drivers should only set for serious errors, although some
                // set for non-fatal errors too.
                return nullptr;
            default:
                errnoExit("Read frame failure");
            }
        }

        assert(buf.index < mNumBuffers);

        // I feel I could store a list of V4l2Buffer objects rather than my custom buffer handler and then I'd just
        // return a reference to it.
        return std::make_unique<BufferHandler>(mBuffers[buf.index].start, buf.bytesused, mFmt, mFd, buf);
    }

private:
    ///////////////////////////////////////////////////////////////////////////
    // PRIVATE FIELDS
    ///////////////////////////////////////////////////////////////////////////

    const std::string mDeviceName;
    const bool mForceFormat;

    int mFd{-1};
    unsigned int mNumBuffers{0};
    Buffer *mBuffers{nullptr};
    V4l2Format mFmt;
};