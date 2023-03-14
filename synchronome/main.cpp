#include "popl.hpp"

#include "camera_service.hpp"
#include "image_saver_service.hpp"
#include "tick_detector_service.hpp"

///////////////////////////////////////////////////////////////////////////////
// SYSTEM CONFIGURATION
///////////////////////////////////////////////////////////////////////////////

static constexpr size_t sNumMessages = 40;
static constexpr size_t sMaxMessageSize = sizeof(BufferHandler);
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


int main(int argc, char **argv)
{
    const auto [device, forceFormat, count] = processCmdLineArgs(argc, argv);

    sCameraService.startCamera(device, forceFormat);

    // Service configuration.
    double startTime = floatTime();

    struct mq_attr mqAttr;
    mqAttr.mq_maxmsg = sNumMessages;
    mqAttr.mq_msgsize = sMaxMessageSize;
    mqAttr.mq_flags = 0;

    CameraService::Config cameraServiceCfg;
    cameraServiceCfg.mqAttr = mqAttr;
    cameraServiceCfg.startTime = startTime;
    cameraServiceCfg.priority = sched_get_priority_max(SCHED_FIFO);
    cameraServiceCfg.queue = sCameraQueue;

    TickDetectorService::Config tickDetectorServiceCfg;
    tickDetectorServiceCfg.mqAttr = mqAttr;
    tickDetectorServiceCfg.startTime = startTime;
    tickDetectorServiceCfg.priority = sched_get_priority_max(SCHED_FIFO) - 1;
    tickDetectorServiceCfg.inQueue = sCameraQueue;
    tickDetectorServiceCfg.outQueue = sTickQueue;

    ImageSaverService::Config imageSaverServiceCfg;
    imageSaverServiceCfg.mqAttr = mqAttr;
    imageSaverServiceCfg.startTime = startTime;
    imageSaverServiceCfg.priority = sched_get_priority_max(SCHED_FIFO) - 2;
    imageSaverServiceCfg.frameCount = count;
    imageSaverServiceCfg.queue = sTickQueue;

    // Start services.
    sCameraService.start(cameraServiceCfg);
    sTickDetectorService.start(tickDetectorServiceCfg);
    sImageSaverService.start(imageSaverServiceCfg);

    // Wait for the image saver service to join, then tell other services to terminate.
    sImageSaverService.join();
    sCameraService.flagExit();
    sTickDetectorService.flagExit();
    sCameraService.join();
    sTickDetectorService.join();

    double stopTime = floatTime();
    double total = stopTime - startTime;
    double rate = static_cast<double>(count) / total;
    syslog(LOG_CRIT, "Total capture time=%lf, for %d frames, %lf FPS\n", total, count, rate);

    sCameraService.stopCamera();
    return 0;
}
