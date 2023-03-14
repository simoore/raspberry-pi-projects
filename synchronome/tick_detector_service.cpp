#include <cstdio>
#include <syslog.h>

#include "buffer_handler.hpp"
#include "tick_detector_service.hpp"
#include "util.hpp"


void TickDetectorService::start(const Config &cfg)
{
    mConfig = cfg;
    Service::start(Service::staticService<TickDetectorService>, cfg.priority, this);
}


void TickDetectorService::service(void)
{
    syslog(LOG_CRIT, "TickDetectorService: started\n");
    mqd_t inmq = mq_open(mConfig.inQueue, O_CREAT | O_RDWR, S_IRWXU, &mConfig.mqAttr);
    mqd_t outmq = mq_open(mConfig.outQueue, O_CREAT | O_RDWR, S_IRWXU, &mConfig.mqAttr);

    int count = 0;
    while (!doExit())
    {
        syslog(LOG_CRIT, "TickDetectorService: awaiting frame\n");
        BufferHandler handler;
        unsigned int prio;
        int rc = mq_receive(inmq, reinterpret_cast<char *>(&handler), sizeof(BufferHandler), &prio);
        if (rc == -1)
        {
            perror("TickDetectorService: receive");
            break;
        }

        bool isTick = mTickDetector.execute(handler);
        if (isTick)
        {
            int rc = mq_send(outmq, reinterpret_cast<char *>(&handler), sizeof(BufferHandler), sOutQueuePrio);
            if (rc == -1)
            {
                errnoExit("TickDetectorService: send");
            }
        }
        else
        {
            handler.returnBuffer();
        }
        ++count;
    }
}