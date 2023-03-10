// For the heap message queue, take this message copy version and create a global heap and queue and dequeue pointers to
// that heap instead of the full data in the messages.
//
// Otherwise, the code should not be substantially different.
//
// You can set up an array in the data segement as a global or use malloc and free for a true heap message queue.
//
// Either way, the queue and dequeue should be ZERO copy.
//
#include <iostream>
#include <mqueue.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <syslog.h>
#include <tuple>
#include <unistd.h>

#include "camera.hpp"
#include "popl.hpp"
#include "image_processor.hpp"

static Camera sCamera;
static ImageProcessor sProcessor;

// On Linux the file systems slash is needed
// #define SNDRCV_MQ "/send_receive_mq"

// #define MAX_MSG_SIZE 128
// #define ERROR (-1)

// struct mq_attr mq_attr;

// pthread_t th_receive, th_send; // create threads
// pthread_attr_t attr_receive, attr_send;
// struct sched_param param_receive, param_send;

// static char canned_msg[] =
//     "This is a test, and only a test, in the event of real emergency, you would be instructed...."; // Message to be
//                                                                                                     // sent

/* receives pointer to heap, reads it, and deallocate heap memory */

// void *receiver(void *arg) {
//     mqd_t mymq;
//     char buffer[MAX_MSG_SIZE];
//     int prio;
//     int rc;

//     printf("receiver - thread entry\n");

//     mymq = mq_open(SNDRCV_MQ, O_CREAT | O_RDWR, S_IRWXU, &mq_attr);

//     /* read oldest, highest priority msg from the message queue until empty */
//     do {
//         printf("receiver - awaiting message\n");
// #if 1
//         if ((rc = mq_receive(mymq, buffer, MAX_MSG_SIZE, &prio)) == ERROR) {
//             perror("mq_receive");
//         } else {
//             buffer[MAX_MSG_SIZE] = '\0';
//             printf("receive: msg %s received with priority = %d, rc = %d\n", buffer, prio, rc);
//         }
// #endif

//     } while (rc != ERROR);
// }

// void main2(void) {
//     int i = 0, rc = 0;
//     /* setup common message q attributes */
//     mq_attr.mq_maxmsg = 10;
//     mq_attr.mq_msgsize = MAX_MSG_SIZE;

//     mq_attr.mq_flags = 0;

//     int rt_max_prio, rt_min_prio;

//     rt_max_prio = sched_get_priority_max(SCHED_FIFO);
//     rt_min_prio = sched_get_priority_min(SCHED_FIFO);

//     // Create two communicating processes right here
//     // receiver((void *)0);
//     // sender((void *)0);

//     // exit(0);

//     // creating prioritized thread

//     // initialize  with default atrribute
//     rc = pthread_attr_init(&attr_receive);
//     // specific scheduling for Receiving
//     rc = pthread_attr_setinheritsched(&attr_receive, PTHREAD_EXPLICIT_SCHED);
//     rc = pthread_attr_setschedpolicy(&attr_receive, SCHED_FIFO);
//     param_receive.sched_priority = rt_min_prio;
//     pthread_attr_setschedparam(&attr_receive, &param_receive);

//     // initialize  with default atrribute
//     rc = pthread_attr_init(&attr_send);
//     // specific scheduling for Receiving
//     rc = pthread_attr_setinheritsched(&attr_send, PTHREAD_EXPLICIT_SCHED);
//     rc = pthread_attr_setschedpolicy(&attr_send, SCHED_FIFO);
//     param_send.sched_priority = rt_max_prio;
//     pthread_attr_setschedparam(&attr_send, &param_send);

//     if ((rc = pthread_create(&th_send, &attr_send, sender, NULL)) == 0) {
//         printf("\n\rSender Thread Created with rc=%d\n\r", rc);
//     } else {
//         perror("\n\rFailed to Make Sender Thread\n\r");
//         printf("rc=%d\n", rc);
//     }

//     if ((rc = pthread_create(&th_receive, &attr_receive, receiver, NULL)) == 0) {
//         printf("\n\r Receiver Thread Created with rc=%d\n\r", rc);
//     } else {
//         perror("\n\r Failed Making Reciever Thread\n\r");
//         printf("rc=%d\n", rc);
//     }

//     printf("pthread join send\n");
//     pthread_join(th_send, NULL);

//     printf("pthread join receive\n");
//     pthread_join(th_receive, NULL);
// }

//

//

// void *sender(void *arg) {
//     mqd_t mymq;
//     int prio;
//     int rc;

//     printf("sender - thread entry\n");

//     mymq = mq_open(SNDRCV_MQ, O_CREAT | O_RDWR, S_IRWXU, &mq_attr); //  ***

//     /* send messages with priority=30 */
//     do {
//         printf("sender - sending message of size=%d\n", sizeof(canned_msg));
// #if 1
//         if ((rc = mq_send(mymq, canned_msg, sizeof(canned_msg), 30)) == ERROR) {
//             perror("mq_send");
//         } else {
//             printf("send: message successfully sent, rc=%d\n", rc);
//         }
// #endif

//     } while (rc != ERROR);
// }

static pthread_t sSenderThread;
static pthread_t sReceiverThread;


std::tuple<std::string, bool, int> processCmdLineArgs(int argc, char **argv)
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
    }
    return std::make_tuple(deviceOpt->value(), forceFormatOpt->is_set(), countOpt->value());
}


void *sender(unsigned int frameCount, double fstart)
{
    printf("Running at 1 frame/sec\n");
    struct timespec readDelay;
    readDelay.tv_sec = 1;
    readDelay.tv_nsec = 0;

    unsigned int count = 0;

    while (count < frameCount)
    {
        if (!sCamera.waitTilReady())
        {
            continue;
        }

        auto bufferPtr = sCamera.readFrame();
        if (bufferPtr)
        {
            sProcessor.processImage(bufferPtr->mStart, bufferPtr->mSize, bufferPtr->mFmt);
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


int main(int argc, char **argv)
{
    const auto [device, forceFormat, count] = processCmdLineArgs(argc, argv);
    std::cout << device << " " << forceFormat << " " << count << std::endl;

    if (count <= 0)
    {
        printf("Count must be positive.\n");
        return 0;
    }

    sCamera.openDevice(device, forceFormat);
    sCamera.initDevice();
    sCamera.startCapturing();

    // Start threads
    double fstart = floatTime();
    sender(count, fstart);
    double fstop = floatTime();

    // Wait for threads to join.
    // pthread_join(sSenderThread, nullptr);
    // pthread_join(sReceiverThread, nullptr);

    sCamera.stopCapturing();
    double rate = static_cast<double>(count) / (fstop - fstart);
    printf("Total capture time=%lf, for %d frames, %lf FPS\n", (fstop - fstart), count, rate);
    sCamera.uninitDevice();
    sCamera.closeDevice();
    return 0;
}
