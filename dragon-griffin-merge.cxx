#include <set>
#include <map>
#include <vector>
#include <iostream>
#include <algorithm>
#include "TFile.h"
#include "TTree.h"
#include "TGriffin.h"
#include "Dragon.hxx"
using namespace std;

namespace {
const double us_DRAGON = 1.;
const double us_GRIFFIN = 1e-2;
double MATCH_WINDOW = 20; // us

struct CompareTimestamps {
	bool operator()(double lhs, double rhs) 
		{
			if(fabs(lhs-rhs) < MATCH_WINDOW) { return false; }
			return lhs < rhs;
		}
};

}


int main(int argc, char** argv)
{
	bool use_t3 = false;
	TString grifFile = "", dragFile = "", outFile = "";
	for(int i = 1; i< argc; ++i) {
		if      (i==1) { dragFile = argv[i]; }
		else if (i==2) { grifFile = argv[i]; }
		else if (i==3) { outFile  = argv[i]; }
		else if (string(argv[i]) == "--t3") { 
			use_t3 = true; 
		}
		else if (string(argv[i]).substr(0,15) == "--match-window=") {
			MATCH_WINDOW = atof( string(string(argv[i]).substr(15)).c_str() );
		}
		else {
			cerr << "ERROR: unknown flag: " << argv[i] << "\n";
			return 1;
		}
	}

	if(grifFile == "" || dragFile == "" || outFile == "") {
		cerr << "usage: dragon-griffin-merge <dragon file> <griffin file> <output file> [--t3] [--match-window=<match window>]\n";
		return 1;
	}

	if(use_t3) { cout << "\nSearching for matches in TAIL tree...\n"; }
	else       { cout << "\nSearching for matches in COINCIDENCE tree...\n"; }
	cout << "Using match window of " << MATCH_WINDOW << " us\n\n";

	// Open Analysis Tree
	TFile * analysisfile = new TFile(grifFile, "READ");   
	if (!analysisfile->IsOpen()) {
		printf("Opening file %s failed, aborting\n", grifFile.Data());
		return 1;
	}
 
	printf("File %s opened\n", grifFile.Data());
	TTree * AnalysisTree = (TTree * ) analysisfile->Get("AnalysisTree");
	long analentries = AnalysisTree->GetEntries();
	const char * testval = "NULL";

	TGriffin * grif_data = 0;
	if (AnalysisTree->FindBranch("TGriffin")) {
		AnalysisTree->SetBranchAddress("TGriffin", & grif_data);
	} else {
		cerr << "Branch 'TGriffin' not found! TGriffin variable is NULL pointer" << endl;
		return 1;
	}

	cout << "Bufferring griffin event timestamps...\n\n";
//	map<double, Long64_t, CompareTimestamps> grifMap; // <timestamp/us, entry>

	vector<tuple<double, Long64_t> > grifVector;
	grifVector.reserve(analentries);
	for (Long64_t jentry = 0; jentry < analentries; jentry++) { // loop over events in analysis tree
		AnalysisTree->GetEntry(jentry);
		const auto grifMult = grif_data->GetMultiplicity();
		if(grif_data && grifMult > 0) {

			// set timestamp to time of the earliest hit
			vector<double> ts_hits(grifMult);
			for(size_t jhit = 0; jhit< grifMult; ++jhit) {				
				const double tstamp = grif_data->GetHit(jhit)->GetTimeStampNs() * us_GRIFFIN;
				ts_hits[jhit] = tstamp;
			}
			const double tstamp_min = TMath::MinElement(ts_hits.size(), &ts_hits[0]);
			grifVector.push_back(make_tuple(tstamp_min, jentry));
 		}
	}

	sort(grifVector.begin(), grifVector.end(),
			 [](const tuple<double,Long64_t>& lhs, 
					const tuple<double,Long64_t>& rhs) 
			 { return get<0>(lhs) < get<0>(rhs); } 
		);
	

	// Open DRAGON tree
	TFile * dragonfile = new TFile(dragFile, "READ");   
	if (!dragonfile->IsOpen()) {
		printf("Opening file %s failed, aborting\n", dragFile.Data());
		return 1;
	}
 
	printf("File %s opened\n", dragFile.Data());
	TTree * DragonTree = use_t3 ? 
		(TTree * ) dragonfile->Get("t3") : (TTree * ) dragonfile->Get("t5");

	dragon::Coinc * b_coinc = 0;
	dragon::Tail * b_tail = 0;
	if(use_t3) { DragonTree->SetBranchAddress("tail",&b_tail); }
	else { DragonTree->SetBranchAddress("coinc",&b_coinc); }
	

	// Outputs
	TFile* fout = new TFile(outFile,"recreate");
	TTree* GriffinOutTree = new TTree(AnalysisTree->GetName(), AnalysisTree->GetTitle());
	GriffinOutTree->Branch("TGriffin", "TGriffin", & grif_data);

	TTree* DragonOutTree = new TTree(DragonTree->GetName(), DragonTree->GetTitle());
	if(use_t3) { DragonOutTree->Branch("tail", "dragon::Tail", &b_tail); }
	else { DragonOutTree->Branch("coinc", "dragon::Coinc", & b_coinc); }


	cout << "Looping DRAGON tree and checking for matches...\n\n";
	Long64_t numMatches = 0;
	for(Long64_t i=0; i< DragonTree->GetEntries(); ++i) {
		DragonTree->GetEntry(i);
		dragon::Tail* tail = use_t3 ? b_tail : &(b_coinc->tail);
		const double tstamp = tail->io32.tsc4.trig_time * us_DRAGON;

		auto CompTS = [](const tuple<double,Long64_t>& it, double val)
			{ return get<0>(it) < val; };

		auto itLow = lower_bound(
			grifVector.begin(), 
			grifVector.end(), 
			tstamp - MATCH_WINDOW/2, 
			CompTS);
		if(itLow == grifVector.end()) { continue; }

		auto itHigh = lower_bound(
			grifVector.begin(), 
			grifVector.end(), 
			tstamp + MATCH_WINDOW/2, 
			CompTS);
		
		if(itHigh != itLow) { // matches!
			for(vector<tuple<double, Long64_t> >::iterator it = itLow; it != itHigh; ++it) {
				AnalysisTree->GetEntry(get<1>(*it));
				GriffinOutTree->Fill();
				DragonOutTree->Fill();
				++numMatches;
			}
		}				
	}

	GriffinOutTree->Write();
	DragonOutTree->Write();
	fout->Close();
	analysisfile->Close();
	dragonfile->Close();

	cout << "Done! Number of matches -->> " << numMatches << endl;

	return 0;
}
