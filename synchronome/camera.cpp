#include "camera.hpp"
#include <cassert>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>


void Camera::openDevice(std::string deviceName, bool forceFormat)
{
    Stat st;
    mDeviceName = deviceName;
    mForceFormat = forceFormat;

    if (-1 == stat(mDeviceName.c_str(), &st))
    {
        errnoExit(std::string{"Cannot identify: "} + mDeviceName);
    }

    if (!S_ISCHR(st.st_mode))
    {
        errnoExit(std::string{"No device: "} + mDeviceName);
    }

    mFd = open(mDeviceName.c_str(), O_RDWR | O_NONBLOCK, 0);

    if (-1 == mFd)
    {
        errnoExit(std::string{"Cannot open: "} + mDeviceName);
    }
}


void Camera::initMmap(void)
{
    V4l2RequestBuffers req;
    clear(req);
    req.count = sRequestBuffers;
    req.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    req.memory = V4L2_MEMORY_MMAP;

    if (-1 == xioctl(mFd, VIDIOC_REQBUFS, &req))
    {
        if (EINVAL == errno)
        {
            errnoExit(std::string{"Device does not support memory mapping: "} + mDeviceName);
        }
        else
        {
            errnoExit("Request for buffers failed");
        }
    }

    if (req.count < 2)
    {
        errnoExit(std::string{"Insufficient buffer memory: "} + mDeviceName);
    }

    mNumBuffers = req.count;
    mBuffers = reinterpret_cast<Buffer *>(std::calloc(req.count, sizeof(Buffer)));

    if (!mBuffers)
    {
        errnoExit("Out of memory");
    }

    for (unsigned int i = 0; i < req.count; ++i)
    {
        V4l2Buffer buf;
        clear(buf);
        buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        buf.memory = V4L2_MEMORY_MMAP;
        buf.index = i;

        if (-1 == xioctl(mFd, VIDIOC_QUERYBUF, &buf))
        {
            errnoExit("Query buffer failed");
        }

        mBuffers[i].size = buf.length;
        mBuffers[i].start = mmap(nullptr, buf.length, PROT_READ | PROT_WRITE, MAP_SHARED, mFd, buf.m.offset);

        if (MAP_FAILED == mBuffers[i].start)
        {
            errnoExit("Failed mmap initialization");
        }
    }
}


void Camera::initDevice(void)
{
    V4l2Capability cap;
    if (-1 == xioctl(mFd, VIDIOC_QUERYCAP, &cap))
    {
        if (EINVAL == errno)
        {
            errnoExit(std::string{"No V4L2 device: "} + mDeviceName);
        }
        else
        {
            errnoExit("VIDIOC_QUERYCAP");
        }
    }

    if (!(cap.capabilities & V4L2_CAP_VIDEO_CAPTURE))
    {
        errnoExit(std::string{"No video capture device: "} + mDeviceName);
    }

    if (!(cap.capabilities & V4L2_CAP_STREAMING))
    {
        errnoExit(std::string{"No streaming support: "} + mDeviceName);
    }

    // Select video input, video standard and tune here.
    V4l2Cropcap cropcap;
    clear(cropcap);
    cropcap.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

    if (0 != xioctl(mFd, VIDIOC_CROPCAP, &cropcap))
    {
        // errnoExit("Cropcap error");
    }

    V4l2Crop crop;
    crop.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    crop.c = cropcap.defrect;

    if (-1 == xioctl(mFd, VIDIOC_S_CROP, &crop))
    {
        // switch (errno)
        // {
        // case EINVAL:
        //     errnoExit("Cropping not supported");
        //     break;
        // default:
        //     errnoExit("Error setting crop");
        //     break;
        // }
    }

    clear(mFmt);
    mFmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

    if (mForceFormat)
    {
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
        if (-1 == xioctl(mFd, VIDIOC_S_FMT, &mFmt))
        {
            errnoExit("VIDIOC_S_FMT");
        }
    }
    else
    {
        if (-1 == xioctl(mFd, VIDIOC_G_FMT, &mFmt))
        {
            errnoExit("VIDIOC_G_FMT");
        }
    }

    #define BIT(n) (0x1U << (n))
    printf("Pixel format: ");
    printf(V4L2_FOURCC_CONV, V4L2_FOURCC_CONV_ARGS(mFmt.fmt.pix.pixelformat));
    printf("\nDimensions W: %d, H: %d\n", mFmt.fmt.pix.width, mFmt.fmt.pix.height);

    // what is this...
    int min = mFmt.fmt.pix.width * 2;
    if (mFmt.fmt.pix.bytesperline < min)
    {
        mFmt.fmt.pix.bytesperline = min;
    }
    min = mFmt.fmt.pix.bytesperline * mFmt.fmt.pix.height;
    if (mFmt.fmt.pix.sizeimage < min)
    {
        mFmt.fmt.pix.sizeimage = min;
    }

    initMmap();
}


void Camera::uninitDevice(void)
{
    for (int i = 0; i < mNumBuffers; ++i)
    {
        if (-1 == munmap(mBuffers[i].start, mBuffers[i].size))
        {
            errnoExit("Uninitialize device failed");
        }
    }
    free(mBuffers);
}


void Camera::closeDevice(void)
{
    if (-1 == close(mFd))
    {
        errnoExit("close");
    }
    mFd = -1;
}


void Camera::startCapturing(void)
{
    for (int i = 0; i < mNumBuffers; ++i)
    {
        printf("Allocated buffer %d\n", i);
        V4l2Buffer buf;
        clear(buf);
        buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        buf.memory = V4L2_MEMORY_MMAP;
        buf.index = i;
        if (-1 == xioctl(mFd, VIDIOC_QBUF, &buf))
        {
            errnoExit("VIDIOC_QBUF");
        }
    }
    enum v4l2_buf_type type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    if (-1 == xioctl(mFd, VIDIOC_STREAMON, &type))
    {
        errnoExit("VIDIOC_STREAMON");
    }
}


void Camera::stopCapturing(void)
{
    enum v4l2_buf_type type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    if (-1 == xioctl(mFd, VIDIOC_STREAMOFF, &type))
    {
        errnoExit("VIDIOC_STREAMOFF");
    }
}


std::unique_ptr<Camera::BufferHandler> Camera::readFrame(void)
{
    std::unique_ptr<BufferHandler> handler = std::make_unique<BufferHandler>(mFmt, mFd);
    clear(handler->mBuf);

    handler->mBuf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    handler->mBuf.memory = V4L2_MEMORY_MMAP;

    if (-1 == xioctl(mFd, VIDIOC_DQBUF, &(handler->mBuf)))
    {
        switch (errno)
        {
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

    assert(handler->mBuf.index < mNumBuffers);
    handler->mStart = mBuffers[handler->mBuf.index].start;
    handler->mSize = handler->mBuf.bytesused;
    return handler;
}


bool Camera::waitTilReady(void)
{
    fd_set fds;
    FD_ZERO(&fds);
    FD_SET(mFd, &fds);

    struct timeval timeout;
    timeout.tv_sec = 2;
    timeout.tv_usec = 0;

    int result = select(mFd + 1, &fds, nullptr, nullptr, &timeout);

    if (-1 == result)
    {
        if (EINTR == errno)
            return false;
        errnoExit("Select error");
    }

    if (0 == result)
    {
        errnoExit("Select timeout");
    }
    return true;
}