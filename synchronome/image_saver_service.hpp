#pragma once

#include <mqueue.h>

#include "image_saver.hpp"
#include "service.hpp"


class ImageSaverService final : public Service
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
        unsigned int frameCount;
        const char *queue;
        bool saveAll;
    };

    ///////////////////////////////////////////////////////////////////////////
    // PUBLIC FUNCTIONS
    ///////////////////////////////////////////////////////////////////////////

    /// Starts the service that polls the camera for images.
    void start(const Config &cfg);

    /// The service routine that is executed when the thread is started.
    void service(void);

private:
    ///////////////////////////////////////////////////////////////////////////
    // PRIVATE FIELDS
    ///////////////////////////////////////////////////////////////////////////

    Config mConfig;
    ImageSaver mSaver;
};
