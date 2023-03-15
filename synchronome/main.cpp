#include "popl.hpp"

#include "camera_service.hpp"
#include "image_saver_service.hpp"
#include "tick_detector_service.hpp"

///////////////////////////////////////////////////////////////////////////////
// SYSTEM CONFIGURATION
///////////////////////////////////////////////////////////////////////////////

static constexpr size_t sNumMessages = 40;
static constexpr const char *sCameraQueue = "/camera_mq";
static constexpr const char *sTickQueue = "/tick_mq";

///////////////////////////////////////////////////////////////////////////////
// SYSTEM COMPONENTS
///////////////////////////////////////////////////////////////////////////////

static CameraService sCameraService;
static TickDetectorService sTickDetectorService;
static ImageSaverService sImageSaverService;

///////////////////////////////////////////////////////////////////////////////
// TOP LEVEL FUNCTIONS
///////////////////////////////////////////////////////////////////////////////

static std::tuple<std::string, int> processCmdLineArgs(int argc, char **argv)
{
    using namespace popl;
    OptionParser op("Allowed options");

    auto deviceOpt = op.add<Value<std::string>>("d", "device", "Camera device, eg. \"/dev/video0\"", "/dev/video0");
    auto helpOpt = op.add<Switch>("h", "help", "Show help message");
    auto countOpt = op.add<Value<int>>("c", "count", "Number of frames to grab", 100);

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

    return std::make_tuple(deviceOpt->value(), countOpt->value());
}


int main(int argc, char **argv)
{
    const auto [device, count] = processCmdLineArgs(argc, argv);

    mq_unlink(sCameraQueue);
    mq_unlink(sTickQueue);
    sCameraService.startCamera(device);

    // Service configuration.
    double startTime = floatTime();

    struct mq_attr cameraMqAttr;
    cameraMqAttr.mq_maxmsg = sNumMessages;
    cameraMqAttr.mq_msgsize = sizeof(BufferHandler);
    cameraMqAttr.mq_flags = 0;

    struct mq_attr tickMqAttr;
    tickMqAttr.mq_maxmsg = sNumMessages;
    tickMqAttr.mq_msgsize = sizeof(RgbHandler);
    tickMqAttr.mq_flags = 0;

    CameraService::Config cameraServiceCfg;
    cameraServiceCfg.mqAttr = cameraMqAttr;
    cameraServiceCfg.startTime = startTime;
    cameraServiceCfg.priority = sched_get_priority_max(SCHED_FIFO) - 2;
    cameraServiceCfg.queue = sCameraQueue;

    TickDetectorService::Config tickDetectorServiceCfg;
    tickDetectorServiceCfg.inMqAttr = cameraMqAttr;
    tickDetectorServiceCfg.outMqAttr = tickMqAttr;
    tickDetectorServiceCfg.priority = sched_get_priority_max(SCHED_FIFO) - 1;
    tickDetectorServiceCfg.inQueue = sCameraQueue;
    tickDetectorServiceCfg.outQueue = sTickQueue;
    tickDetectorServiceCfg.tickDetectorConfig.showDiff = false;
    tickDetectorServiceCfg.tickDetectorConfig.startTime = startTime;

    ImageSaverService::Config imageSaverServiceCfg;
    imageSaverServiceCfg.mqAttr = tickMqAttr;
    imageSaverServiceCfg.startTime = startTime;
    imageSaverServiceCfg.priority = sched_get_priority_max(SCHED_FIFO);
    imageSaverServiceCfg.frameCount = count;
    imageSaverServiceCfg.saveAll = false;
    imageSaverServiceCfg.queue = sTickQueue;

    // Start services.
    sCameraService.start(cameraServiceCfg);
    sTickDetectorService.start(tickDetectorServiceCfg);
    sImageSaverService.start(imageSaverServiceCfg);

    // Wait for the image saver service to join, then tell other services to terminate.
    sImageSaverService.join();

    double stopTime = floatTime();
    double total = stopTime - startTime;
    double rate = static_cast<double>(count) / total;
    syslog(LOG_CRIT, "Total capture time=%lf, for %d frames, %lf FPS\n", total, count, rate);

    sCameraService.flagExit();
    sTickDetectorService.flagExit();
    sCameraService.join();
    sTickDetectorService.join();

    sCameraService.stopCamera();
    mq_unlink(sCameraQueue);
    mq_unlink(sTickQueue);
    return 0;
}
