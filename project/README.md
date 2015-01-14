An interface for accesing statistics about the /dev/random device
========

The project includes a kernel module to gather all information about the device 
and a CLI program to access this stored information.

Gathered statistics:

1. How many bytes were read / currenly remain.
2. Which programs are currently using /dev/random.
3. By specifing the flag -v, statistics on all 256 bytes that can be produced by the device 
will be displayed (i.e. how many times a 0 or 141 was produced). This can be used to check
if /dev/random is truly random.
