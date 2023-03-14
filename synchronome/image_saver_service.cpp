#include "image_saver_service.hpp"
#include "camera.hpp"


void ImageSaverService::start(const Config &cfg)
{
    mConfig = cfg;
    Service::start(Service::staticService<ImageSaverService>, cfg.priority, this);
}


void ImageSaverService::service(void)
{
    printf("Receiver: started\n");
    mqd_t mymq = mq_open(mConfig.queue, O_CREAT | O_RDWR, S_IRWXU, &mConfig.mqAttr);

    int count = 0;
    while (count < mConfig.frameCount)
    {
        printf("Receiver: awaiting\n");
        BufferHandler handler;
        unsigned int prio;
        int rc = mq_receive(mymq, reinterpret_cast<char *>(&handler), sizeof(BufferHandler), &prio);
        if (rc == -1)
        {
            perror("mq_receive");
            break;
        }
        mSaver.processImage(handler.mStart, handler.mSize, handler.mFmt);
        handler.returnBuffer();
        ++count;
    }
}
