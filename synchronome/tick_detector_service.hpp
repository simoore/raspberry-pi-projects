#pragma once

#include <cstdint>
#include <mqueue.h>

#include "service.hpp"
#include "tick_detector.hpp"


class TickDetectorService final : public Service
{
public:
    ///////////////////////////////////////////////////////////////////////////
    // PUBLIC CONSTANTS
    ///////////////////////////////////////////////////////////////////////////

    static constexpr uint32_t sOutQueuePrio = 29U;

    ///////////////////////////////////////////////////////////////////////////
    // PUBLIC TYPES
    ///////////////////////////////////////////////////////////////////////////

    struct Config
    {
        TickDetector::Config tickDetectorConfig;
        struct mq_attr inMqAttr;
        struct mq_attr outMqAttr;
        unsigned int priority;
        const char *inQueue;
        const char *outQueue;
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
    TickDetector mTickDetector;
};
