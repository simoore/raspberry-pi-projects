# Raspberry PI Projects

## Operating System

Operating System: <https://www.raspberrypi.com/software/>
Hostname: raspberrypi

There are options to setup ssh and wifi when you flash the image onto the micro SD card.

You can connect to the router and see the devices connected to the network to confirm the raspberry pi is connected
to the network. And you can `ping -4 raspberrypi`.

Then login into the device `ssh steven@raspberrypi`. Then you want to copy you ID onto the device to be able to 
login without a password. Copy the contents of your public key into `~/.ssh/authorized_keys` on the device. You 
may have to create the folder and file.

## Commands

* `lscpu` shows information about the CPU
* `htop` shows process and system resourse usage
* `vcgencmd measure_temp` to measure the tempeature of the board

## Links

* <https://github.com/siewertsmooc/RTES-ECEE-5623>

### Posix Threads

* <https://hpc-tutorials.llnl.gov/posix/>
* <https://www.cs.cmu.edu/afs/cs/academic/class/15492-f07/www/pthreads.html>
* <https://randu.org/tutorials/threads/>
