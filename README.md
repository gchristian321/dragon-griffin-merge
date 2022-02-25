A program to merge GRIFFIN analysis trees with DRAGON trees, based on timestamps.


# Instructions

## Environment
This program is only designed to work on the astro02 server at TRIUMF.
Porting it elsewhere is the responsibility of the user.

To set up your user environment on astro02, issue the following commands:
```
module load grsisort/4.0.0.2_root-6.18 dragon-analyzer/1.3.0_root-6.18
export LD_LIBRARY_PATH=$GRSISYS/GRSIData/lib:$LD_LIBRARY_PATH
export ROOTSYS=/usr/share/root
```
If you will be using this often, it is a good idea to put these lines
in your `~/.bashrc` or other login file.

Also ensure that you have not already loaded a different versio of ROOT before
doing the above.

## 