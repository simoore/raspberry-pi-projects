#include <syslog.h>

#include "image_saver_service.hpp"
#include "rgb_handler.hpp"


void ImageSaverService::start(const Config &cfg)
{
    mConfig = cfg;
    Service::start(Service::staticService<ImageSaverService>, cfg.priority, this);
}


void ImageSaverService::service(void)
{
    syslog(LOG_CRIT, "ImageSaverService: started\n");
    mqd_t mymq = mq_open(mConfig.queue, O_CREAT | O_RDWR, S_IRWXU, &mConfig.mqAttr);

    int count = 0;
    while (count < mConfig.frameCount)
    {
        syslog(LOG_CRIT, "ImageSaverService: awaiting\n");
        RgbHandler handler;
        unsigned int prio;
        int rc = mq_receive(mymq, reinterpret_cast<char *>(&handler), sizeof(RgbHandler), &prio);
        if (rc == -1)
        {
            perror("mq_receive");
            break;
        }
        if (handler.mIsTick || mConfig.saveAll)
        {
            mSaver.processImage(handler.mStart, handler.mSize, mConfig.startTime);
            ++count;
        }
        handler.returnBuffer();
    }
}
