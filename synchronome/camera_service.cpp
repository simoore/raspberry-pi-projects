#include <syslog.h>

#include "camera_service.hpp"


void CameraService::start(const Config &cfg)
{
    mConfig = cfg;
    Service::start(Service::staticService<CameraService>, mConfig.priority, this);
}


void CameraService::startCamera(std::string deviceName)
{
    mCamera.openDevice(deviceName);
    mCamera.initDevice();
    mCamera.startCapturing();
}


void CameraService::stopCamera()
{
    mCamera.stopCapturing();
    mCamera.uninitDevice();
    mCamera.closeDevice();
}


void CameraService::service(void)
{
    syslog(LOG_CRIT, "CameraService: Running at 25 frame/sec\n");
    struct timespec readDelay;
    readDelay.tv_sec = 0;
    readDelay.tv_nsec = 40000000;
    unsigned int count = 0;

    mqd_t mymq = mq_open(mConfig.queue, O_CREAT | O_RDWR, S_IRWXU, &mConfig.mqAttr);

    while (!doExit())
    {
        if (!mCamera.waitTilReady())
        {
            continue;
        }

        auto bufferPtr = mCamera.readFrame();
        if (bufferPtr)
        {
            int rc = mq_send(mymq, reinterpret_cast<char *>(bufferPtr.get()), sizeof(BufferHandler), 30U);
            struct timespec timeError;
            if (nanosleep(&readDelay, &timeError) != 0)
            {
                perror("nanosleep");
            }
            else
            {
                double delta = floatTime() - mConfig.startTime;
                double rate = static_cast<double>(count + 1) / delta;
                syslog(LOG_CRIT, "Frame read at %lf, @ %lf FPS\n", delta, rate);
            }
            ++count;
        }
    }
    syslog(LOG_CRIT, "CameraService: exiting");
}
