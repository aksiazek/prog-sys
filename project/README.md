An interface for accesing more statistics about the /dev/random device
for Linux Kernel >= 2.6
========

The project includes a kernel module (stats_of_random) to gather additional 
information about the device by substituting existing random device
kernel functions and proc entries to access this stored information.

Info: the directory /proc/sys/kernel/random/ already holds some important data
e.g. entropy_avail - the number of bits available

Additional gathered statistics by stats_of_random:

1. How many bytes were read from /dev/random (since the module was inserted).
2. Which programs are currently using /dev/random.
3. By specifing the flag -v, statistics on all 256 bytes that can be produced by the device 
will be displayed (i.e. how many times a 0 or 141 was produced). This can be used to check
if /dev/random is truly random.


DISCLAIMER: the module uses the GPL code of the TPE Linux Kernel Module, available at:
https://github.com/cormander/tpe-lkm to hijack running kernel functions (to change them).
Please refer to it's documentation to see if this works for you.
