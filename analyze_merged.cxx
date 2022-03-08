// example script to extract data from merges trees in a loop.
// There are two ways to load and use this:
// 1. >grsisort
//    TGenericDetector d;
//    .L analyze_merged.cxx
//    analyze_merged("/path/to/file.root");
//
//2. >root.exe
//   TFile::Open("/path/to/file.root");
//   analyze_merged("/path/to/file.root");
//
int analyze_merged(const char* filename)
{
	TH2F* hst = new TH2F("hst","DSSSD Energy vs. Griffin Energy", 100,0,50000,50,1800,2400);

	TFile* f = TFile::Open(filename);
	if(!f) { cerr << "BAD FILE\n"; return 1; }

	TTree* t3 = dynamic_cast<TTree*>(f->Get("t3"));
	if(!t3) { cerr << "NO t3\n"; return 1; }
	dragon::Tail* tail = 0;
	t3->SetBranchAddress("tail",&tail);

	TTree* AnalysisTree = dynamic_cast<TTree*>(f->Get("AnalysisTree"));
	if(!AnalysisTree) { cerr << "NO AnalysisTree\n"; return 1; }
	TGenericDetector* det = 0;
	AnalysisTree->SetBranchAddress("TGenericDetector",&det);

	if(AnalysisTree->GetEntries() != t3->GetEntries()) {
		cerr << "Different number of events in trees.\n";
		return 1;
	}

	for(Long64_t ientry=0;ientry< t3->GetEntries(); ++ientry) {
		t3->GetEntry(ientry);
		AnalysisTree->GetEntry(ientry);

		double dsssdE = tail->dsssd.efront;
		double grifE = det->GetHit(0)->GetEnergy();
		hst->Fill(grifE, dsssdE);
	}
	hst->Draw("colz");
	f->Close();
	return 0;
}
