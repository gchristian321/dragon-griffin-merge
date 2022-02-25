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

## Compiling
To compile (on astro02), after completing the above setup, just type
```
make
```

## Running
### Creating Input Files
Running the code requires two impot ROOT files. One created with GRSISORT containing
Griffin data and the other created with mid2root containing DRAGON data.

To create the Griffin ROOT file from an original MIDAS file, do the following:
```
grsisort --recommended --reconstruct-timestamp /path/to/run*.mid /data/astro/gchristian/dragon-griffin-test/GRIFFIN/Sceptar2.cal --word-count-offset=-1 --write-clients=1
```
(where * is the run number). Please note that this uses the calibration file `/data/astro/gchristian/dragon-griffin-test/GRIFFIN/Sceptar2.cal`.
If another calibration is desired, replace that argument with its path.

Running the command above creates two ROOT files in the current directory:
```
fragment*.root
analysis*.root
```
(again * is the run number). Ensure to move them to an appropriate directory for long-term data storage.

To create the DRAGON ROOT file from an original MIDAS file, do the following:
```
mid2root /path/to/run*.mid -o /path/to/run*.root
```
This creates the ROOT file run*.root in the specified path.

### Running the code
To run the merging code, do:
```
./dragon-griffin-merge <dragon root file> <griffin root file> <output root file>
```

Where the dragon and griffin root files are created in the previous steps.

There are also optional arguments:
```
--t3  --->>> perform matching on the dragon tail (heavy-ion) tree
      	     default is to match on the coincidence tree
--match-window=<window>  --->>> specify a specific coincidence matching window
			 	in microseconds; default is 20 us
```

## Using the Output
To view the output data, load the merged root file into grsisort:
```
grsisort run*-merged.root
```

The output ROOT file contains two TTrees, one with DRAGON data (called t3 or t5), and
the other with Griffin data (callad AnalysisTree). To look at correlated events, use AddFriend:
```
AnalysisTree->AddFriend(t3);
```
(or t5, depending on which option was used for the DRAGON tree).

After doing this, events from either GRIFFIN or DRAGON can be accessed as they normally would.