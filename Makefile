all: dragon-griffin-merge

dragon-griffin-merge: dragon-griffin-merge.cxx
	c++ dragon-griffin-merge.cxx -o dragon-griffin-merge \
-I$(GRSISYS)/include -L$(GRSISYS)/lib `grsi-config --cflags --all-libs --GRSIData-libs` \
-I$(GRSISYS)/GRSIData/include -L$(GRSISYS)/GRSIData/lib `root-config --cflags --libs` \
-lTreePlayer -lMathMore -lSpectrum -lMinuit -lPyROOT \
-I$(DRAGONSYS)/src -L$(DRAGONSYS)/lib -lDragon

clean:
	rm -f dragon-griffin-merge
