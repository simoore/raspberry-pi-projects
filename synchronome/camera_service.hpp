#pragma once

#include <mqueue.h>
#include <syslog.h>

#include "camera.hpp"
#include "service.hpp"


class CameraService final : public Service
{
public:
    ///////////////////////////////////////////////////////////////////////////
    // PUBLIC TYPES
    ///////////////////////////////////////////////////////////////////////////

    struct Config
    {
        struct mq_attr mqAttr;
        double startTime;
        unsigned int priority;
        const char *queue;
    };

    ///////////////////////////////////////////////////////////////////////////
    // PUBLIC FUNCTIONS
    ///////////////////////////////////////////////////////////////////////////

    /// Starts the service that polls the camera for images.
    void start(const Config &cfg);

    /// Initializes the camera device.
    void startCamera(std::string deviceName, bool forceFormat);

    /// De-initializes the camera device.
    void stopCamera();

    /// The service routine that is executed when the thread is started.
    void service(void);

private:
    ///////////////////////////////////////////////////////////////////////////
    // PRIVATE FIELDS
    ///////////////////////////////////////////////////////////////////////////

    Config mConfig;
    Camera mCamera;
};
