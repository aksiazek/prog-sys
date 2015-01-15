An interface for accesing more statistics about the /dev/random device
for Linux Kernel >= 2.6

===============================================================================
Improved /dev/random statistics module (stats_of_random)
===============================================================================

The project includes a kernel module to gather additional 
information about the device by substituting existing random device
kernel functions and /proc/random/ entries to access this stored information.

Info: the directory /proc/sys/kernel/random/ already holds some important data
e.g. entropy_avail - the number of bits available

Additional gathered statistics by stats_of_random:

1. How many bytes were read from /dev/random (since the module was inserted).
Stored in /proc/random/bytes_read .
2. Which programs are currently using /dev/random and how much have they read.


DISCLAIMER: the module uses the GPL code of the TPE Linux Kernel Module, available at:
https://github.com/cormander/tpe-lkm to hijack running kernel functions (to change them).
Please refer to it's documentation to see if this works for you.


===============================================================================
Install:

make
sudo insmod stats_of_random.ko

that's it!
(assuming /dev/random exists)

===============================================================================
Cleanup:

sudo rmmod stats_of_random

or make clean to previous remove compilation results
