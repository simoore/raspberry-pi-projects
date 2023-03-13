# Raspberry PI Projects

## Operating System

Operating System: [Raspberry Pi OS](https://www.raspberrypi.com/software/)
Hostname: raspberrypi

There are options to setup ssh and wifi when you flash the image onto the micro SD card.

You can connect to the router and see the devices connected to the network to confirm the raspberry pi is connected
to the network. And you can `ping -4 raspberrypi`.

Then login into the device `ssh steven@raspberrypi`. Then you want to copy you ID onto the device to be able to 
login without a password. Copy the contents of your public key into `~/.ssh/authorized_keys` on the device. You 
may have to create the folder and file.

## gprof, gcov

Add `-pg` compile flags. Run your application and it will generate a gmon.out. Execute the following command to get
a text report: `gprof raidtest gmon.out > gprof.txt`.

For gcov add the `-coverage` compile flag. Run your application and it generates a report that tells you what lines of 
code have and have not been executed. gcov or lcov turn the generated files into a report that is human readable.

Other tools profiling and tracing tools include is sysprof, kernalshark, and wireshark.

## Commands

* `lscpu` shows information about the CPU
* `htop` shows process and system resourse usage
* `vcgencmd measure_temp` to measure the tempeature of the board
* `v4l2-ctl --list-devices` shows which device is mapped to the camera
* `iostat` to monitor IO usage
* `free -m` to monitor memory usage
* `v4l2-ctl --list-formats` lists the image formats that the video devices use

A user must be part of the video group to use the camera.

## Links

* [Real Time Example Code](https://github.com/siewertsmooc/RTES-ECEE-5623)
* [Linux Real Time Documentation](https://wiki.linuxfoundation.org/realtime/start)
* [SimSo - Simulation of Multiprocessor Scheduling with Overheads](https://github.com/MaximeCheramy/simso)
* [Writing a Linux Kernel Module — Part 1: Introduction](http://derekmolloy.ie/writing-a-linux-kernel-module-part-1-introduction/)
* [Writing a Linux Kernel Module — Part 2: A Character Device](http://derekmolloy.ie/writing-a-linux-kernel-module-part-2-a-character-device/)
* [Writing a Linux Kernel Module — Part 3: Buttons and LEDs](http://derekmolloy.ie/kernel-gpio-programming-buttons-and-leds/)
* [Getting Started With XRDP On Raspberry Pi](https://raspberrytips.com/xrdp-on-raspberry-pi/)
* [GCC compiler optimization for ARM-based systems](https://gist.github.com/fm4dd/c663217935dc17f0fc73c9c81b0aa845)

### Posix Threads

* [POSIX Threads Programming](https://hpc-tutorials.llnl.gov/posix/)
* [POSIX thread (pthread) libraries](https://www.cs.cmu.edu/afs/cs/academic/class/15492-f07/www/pthreads.html)
* [Multithreaded Programming (POSIX pthreads Tutorial)](https://randu.org/tutorials/threads/)

### ARM Synchronization

* [ARM Synchronization Primitives Development Article](https://developer.arm.com/documentation/dht0008/a/arm-synchronization-primitives/practical-uses/implementing-a-semaphore)

### Camera UVC Driver

* [Part I - Video for Linux API](https://linuxtv.org/downloads/v4l-dvb-apis-new/userspace-api/v4l/v4l2.html)
* [The Linux USB Video Class (UVC) driver](https://www.kernel.org/doc/html/v4.13/media/v4l-drivers/uvcvideo.html)
