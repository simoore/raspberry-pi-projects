#include <mqueue.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <syslog.h>
#include <tuple>
#include <unistd.h>

#include "popl.hpp"

#include "camera.hpp"
#include "image_processor.hpp"

static constexpr size_t sMaxMessageSize = sizeof(Camera::BufferHandler);
static constexpr const char *sQueue = "/send_receive_mq";

using StartRoutine = void *(*)(void *);

static struct mq_attr sMqAttr;
static pthread_t sSenderThread;
static pthread_t sReceiverThread;
static Camera sCamera;
static ImageProcessor sProcessor;


static std::tuple<std::string, bool, int> processCmdLineArgs(int argc, char **argv)
{
    using namespace popl;
    OptionParser op("Allowed options");

    auto deviceOpt = op.add<Value<std::string>>("d", "device", "Camera device, eg. \"/dev/video0\"", "/dev/video0");
    auto helpOpt = op.add<Switch>("h", "help", "Show help message");
    auto forceFormatOpt = op.add<Switch>("f", "format", "Force format to 640x480 GREY");
    auto countOpt = op.add<Value<int>>("c", "count", "Number of frames to grab", 3);

    op.parse(argc, argv);

    if (helpOpt->is_set())
    {
        std::cout << op << std::endl;
        exit(EXIT_SUCCESS);
    }
    if (countOpt->value() <= 0)
    {
        printf("Count must be positive.\n");
        exit(EXIT_SUCCESS);
    }

    return std::make_tuple(deviceOpt->value(), forceFormatOpt->is_set(), countOpt->value());
}


static void *receiver(void *args)
{
    unsigned int frameCount = reinterpret_cast<std::pair<unsigned int, double> *>(args)->first;
    double fstart = reinterpret_cast<std::pair<unsigned int, double> *>(args)->second;
    printf("Receiver: started\n");
    mqd_t mymq = mq_open(sQueue, O_CREAT | O_RDWR, S_IRWXU, &sMqAttr);

    int count = 0;
    while (count < frameCount)
    {
        printf("Receiver: awaiting\n");
        Camera::BufferHandler handler;
        unsigned int prio;
        int rc = mq_receive(mymq, reinterpret_cast<char *>(&handler), sizeof(Camera::BufferHandler), &prio);
        if (rc == -1)
        {
            perror("mq_receive");
            break;
        }
        sProcessor.processImage(handler.mStart, handler.mSize, handler.mFmt);
        handler.returnBuffer();
        ++count;
    }
    return nullptr;
}


static void *sender(void *args)
{
    unsigned int frameCount = reinterpret_cast<std::pair<unsigned int, double> *>(args)->first;
    double fstart = reinterpret_cast<std::pair<unsigned int, double> *>(args)->second;

    printf("Running at 1 frame/sec\n");
    struct timespec readDelay;
    readDelay.tv_sec = 1;
    readDelay.tv_nsec = 0;
    unsigned int count = 0;

    mqd_t mymq = mq_open(sQueue, O_CREAT | O_RDWR, S_IRWXU, &sMqAttr);

    while (count < frameCount)
    {
        if (!sCamera.waitTilReady())
        {
            continue;
        }

        auto bufferPtr = sCamera.readFrame();
        if (bufferPtr)
        {
            int rc = mq_send(mymq, reinterpret_cast<char *>(bufferPtr.get()), sizeof(Camera::BufferHandler), 30U);
            struct timespec timeError;
            if (nanosleep(&readDelay, &timeError) != 0)
            {
                perror("nanosleep");
            }
            else
            {
                double fnow = floatTime();
                double rate = static_cast<double>(count + 1) / (fnow - fstart);
                syslog(LOG_CRIT, "Frame read at %lf, @ %lf FPS\n", (fnow - fstart), rate);
            }
            ++count;
        }
    }
    return nullptr;
}


static void initAndStartThread(pthread_t *thread, int priority, StartRoutine routine, void *args)
{
    pthread_attr_t pthreadAttr;
    static struct sched_param schedParam;
    pthread_attr_init(&pthreadAttr);
    pthread_attr_setinheritsched(&pthreadAttr, PTHREAD_EXPLICIT_SCHED);
    pthread_attr_setschedpolicy(&pthreadAttr, SCHED_FIFO);
    schedParam.sched_priority = priority;
    pthread_attr_setschedparam(&pthreadAttr, &schedParam);

    int rc = pthread_create(thread, &pthreadAttr, routine, args);
    if (rc != 0)
    {
        perror("Failed to Make Thread\n");
        printf("return value=%d\n", rc);
        exit(EXIT_FAILURE);
    }
}


int main(int argc, char **argv)
{
    const auto [device, forceFormat, count] = processCmdLineArgs(argc, argv);

    sMqAttr.mq_maxmsg = 10;
    sMqAttr.mq_msgsize = sMaxMessageSize;
    sMqAttr.mq_flags = 0;

    sCamera.openDevice(device, forceFormat);
    sCamera.initDevice();
    sCamera.startCapturing();

    // Start threads
    double fstart = floatTime();
    std::pair<unsigned int, double> args = std::make_pair(count, fstart);
    initAndStartThread(&sSenderThread, sched_get_priority_max(SCHED_FIFO), sender, &args);
    initAndStartThread(&sReceiverThread, sched_get_priority_min(SCHED_FIFO), receiver, &args);

    // Wait for threads to join.
    pthread_join(sSenderThread, nullptr);
    pthread_join(sReceiverThread, nullptr);

    double fstop = floatTime();
    double rate = static_cast<double>(count) / (fstop - fstart);
    printf("Total capture time=%lf, for %d frames, %lf FPS\n", (fstop - fstart), count, rate);

    sCamera.stopCapturing();
    sCamera.uninitDevice();
    sCamera.closeDevice();
    return 0;
}
