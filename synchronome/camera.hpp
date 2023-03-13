#pragma once

#include <linux/videodev2.h>
#include <memory>
#include <sys/select.h>

#include "util.hpp"

class Camera
{
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

    struct Buffer
    {
        void *start;
        size_t size;
    };

    /// This class places the video buffer back on the queue after it has been used.
    struct BufferHandler
    {
        BufferHandler() = default;
        BufferHandler(const V4l2Format &fmt, const int fd) : mFmt(fmt), mFd(fd) {}
        void returnBuffer();

        void *mStart;
        size_t mSize;
        V4l2Format mFmt;
        int mFd;
        V4l2Buffer mBuf;
    };

    ///////////////////////////////////////////////////////////////////////////
    // PUBLIC FUNCTIONS
    ///////////////////////////////////////////////////////////////////////////

    /// Opens the camera device and sets the file descriptor to access it.
    void openDevice(std::string deviceName, bool forceFormat);

    // The driver allocates buffers for the camera in kernal space and mmap is used to make these available in
    // userspace. This function initializes the buffers and makes handles to them available in this application.
    void initMmap(void);

    /// Initializes various properties of the video driver.
    void initDevice(void);

    /// Uninitializing the device unmaps the drives kernal space buffers from the application and de-allocates the
    /// handlers used to reference them.
    void uninitDevice(void);

    /// Closes the file descriptor associated with the camera.
    void closeDevice(void);

    /// Queue buffers and tell the video driver to start streaming.
    void startCapturing(void);

    /// Tell the video driver to turn the stream off.
    void stopCapturing(void);

    /// Reads a frame from the video driver. When the unique ptr is destroyed, the destructor fo the buffer handler
    /// object places the buffer back on the queue.
    std::unique_ptr<BufferHandler> readFrame(void);

    /// Wait til the file descriptor is ready for a read/write without blocking.
    bool waitTilReady(void);

private:
    ///////////////////////////////////////////////////////////////////////////
    // PRIVATE FIELDS
    ///////////////////////////////////////////////////////////////////////////

    std::string mDeviceName;
    bool mForceFormat;
    int mFd{-1};
    unsigned int mNumBuffers{0};
    Buffer *mBuffers{nullptr};
    V4l2Format mFmt;
};
