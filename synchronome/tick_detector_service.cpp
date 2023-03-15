#include <cstdio>
#include <syslog.h>

#include "buffer_handler.hpp"
#include "rgb_handler.hpp"
#include "tick_detector_service.hpp"
#include "util.hpp"


void TickDetectorService::start(const Config &cfg)
{
    mConfig = cfg;
    mTickDetector.setConfig(cfg.tickDetectorConfig);
    Service::start(Service::staticService<TickDetectorService>, cfg.priority, this);
}


void TickDetectorService::service(void)
{
    syslog(LOG_CRIT, "TickDetectorService: started\n");
    mqd_t inmq = mq_open(mConfig.inQueue, O_CREAT | O_RDWR, S_IRWXU, &mConfig.inMqAttr);
    mqd_t outmq = mq_open(mConfig.outQueue, O_CREAT | O_RDWR, S_IRWXU, &mConfig.outMqAttr);

    while (!doExit())
    {
        syslog(LOG_CRIT, "TickDetectorService: awaiting frame\n");
        BufferHandler handler;
        unsigned int prio;
        struct timespec receiveTimeout;
        clock_gettime(CLOCK_REALTIME, &receiveTimeout);
        receiveTimeout.tv_sec += 1;
        int rc =
            mq_timedreceive(inmq, reinterpret_cast<char *>(&handler), sizeof(BufferHandler), &prio, &receiveTimeout);
        if (rc == -1)
        {
            if (errno == ETIMEDOUT)
            {
                continue;
            }
            else
            {
                perror("TickDetectorService: receive");
                break;
            }
        }

        RgbHandler rgbHandler = mTickDetector.execute(handler);
        handler.returnBuffer();

        if (rgbHandler.mStart)
        {
            int rc = mq_send(outmq, reinterpret_cast<char *>(&rgbHandler), sizeof(RgbHandler), sOutQueuePrio);
            if (rc == -1)
            {
                errnoExit("TickDetectorService: send");
            }
        }
    }
    syslog(LOG_CRIT, "TickDetectorService: exiting");
}
