#include <iostream>
#include <cstdlib>

#include "TSystem.h"

#if !defined(__CLING__) || defined(__ROOTCLING__)
#include "larcv/core/DataFormat/EventImage2D.h"
#endif

#include "TH2F.h"
#include "TH1D.h"
#include "TTree.h"
#include "TChain.h"

#include "TString.h"

#include "TFile.h"
#include "TStyle.h"
#include "TROOT.h"

//#define DEBUG

using namespace std;

#define FOR(i, size) for (int i = 0; i < size; ++i)

void FindROI(TH2* h, float thldx = 0.2, float thldy = 0.2);
void overThreshold(TH1D* h, float threshold, int& binlow, int& binhi);

void convert(const char* fname, int nevents = 1, int startevt = 0)
{
    const int DOCSV = 1;

    // attach the library
    TString larcvlibdir = gSystem->Getenv("LARCV_LIBDIR");

    gROOT->ProcessLine(".L " + larcvlibdir + "/libLArCVCoreDataFormat.so");

    // set colours
    gStyle->SetPalette(kInvertedDarkBodyRadiator);
    gStyle->SetNumberContours(255);
    gStyle->SetOptStat(0);

    // Get the tree
    auto tree = new TChain("image2d_tpc_tree");
    tree->Add(fname);

    // register the event object
    auto evt = new larcv::EventImage2D();
    tree->SetBranchAddress("image2d_tpc_branch", &evt);

    TString outprefix(fname);
    TString plotprefix(outprefix);
    auto pos = plotprefix.Index("data/");
    if (pos != -1)
	plotprefix.Replace(pos, 5, "plots/");
    outprefix.ReplaceAll(".root", "");
    plotprefix.ReplaceAll(".root", "");

    // Output root file
    auto outf = TFile::Open(outprefix + "_2dhists.root", "recreate");


    // pedestal should now be substracted already in larcvmaker (21 Mar 2023)
    // // pedestal to be subtracted
    // const double pedestal = 490.;

    if (nevents > tree->GetEntries())
	nevents = tree->GetEntries();
    int fiftieth = nevents/50;

    FOR(entry, nevents) {
	if (fiftieth && entry%fiftieth == 0) {
	    cout<<"Done "<<entry<<" entries ("<<(100*entry/nevents)<<"%).\r"<<flush;
	}

	// Get entry
	tree->GetEntry(entry + startevt);

	// Create the output histogram
	int cols = evt->as_vector()[0].meta().cols();
	int rows = evt->as_vector()[0].meta().rows();
	auto h = new TH2F(Form("hCollectionRawDigitsVsWireVsTick_%d", entry+startevt), ";Ticks;Wire",
			  cols, -cols/2, cols/2, // Ticks
			  rows, 0., rows); // Wires

	// Output csv
	//ofstream ofs;
	FILE* outfcsv = 0;
	TString csvname = outprefix + "_" + (entry+startevt) + ".csv";
	if ( DOCSV ) {
	    //ofs.open(csvname);
	    outfcsv = gSystem->OpenPipe(Form("gzip -4 > %s.gz", csvname.Data()), "w");

	    fprintf(outfcsv, "%d x %d\n", rows, cols);
	}

	FOR(i, rows) {
	    FOR(j, cols) {
		//double content = (evt->as_vector()[0].pixel(i, j) > 0)?evt->as_vector()[0].pixel(i, j) - pedestal:0;
		double content = evt->as_vector()[0].pixel(i, j);
		h->SetBinContent(j+1, i+1, content);

		if ( DOCSV )
		    fprintf(outfcsv, "%.0f\n", content);
		//     //ofs<<j<<","<<i<<","<<content<<endl;
		//     ofs<<content<<endl;
	    }
	}

	// determine region of interest
	FindROI(h);
	if ( DOCSV ) {
	    fprintf(outfcsv,
		    "ROI: x - %d %d\n     y - %d %d\n",
		    h->GetXaxis()->GetFirst(),
		    h->GetXaxis()->GetLast(),
		    h->GetYaxis()->GetFirst(),
		    h->GetYaxis()->GetLast());
	}

	// close and gzip output csv
	if ( DOCSV ) {
	    gSystem->ClosePipe(outfcsv);
	}
	//     ofs.close();
	//     gSystem->Exec(Form("gzip -f %s", csvname.Data()));
	// }

	// h->Draw("colz");

	// gPad->SetLogz();

	// gPad->SaveAs(Form(outprefix + "_2dhist_%d.png", entry+startevt));

	h->SetOption("colz");
	h->Write();

	delete h;
    }
    cout<<endl;

    // tar all the compressed output csv files
    if ( DOCSV )
	gSystem->Exec( Form("tar --remove-files -cf %s.csv.gz.tar %s_{%d..%d}.csv.gz",
			    outprefix.Data(), outprefix.Data(), startevt, startevt+nevents-1) );

    // close the output root file
    outf->Close();
}

void FindROI(TH2* h, float thldx, float thldy)
{
    // create projections and find regions over threshold
    auto px = h->ProjectionX();
    auto py = h->ProjectionY();

    int xlow, xhi, ylow, yhi;

    px->Scale(1./py->GetNbinsX());
    py->Scale(1./px->GetNbinsX());

    overThreshold(px, thldx, xlow, xhi);
    overThreshold(py, thldy, ylow, yhi);

#ifdef DEBUG
    cout<<"Found range X: "<<xlow<<" - "<<xhi
	<<" = "<<px->GetXaxis()->GetBinLowEdge(xlow)<<" - "
	<<px->GetXaxis()->GetBinUpEdge(xhi)<<endl
	<<"            Y: "<<ylow<<" - "<<yhi
	<<" = "<<py->GetXaxis()->GetBinLowEdge(ylow)<<" - "
	<<py->GetXaxis()->GetBinUpEdge(yhi)<<endl;
#endif

    int marginx = (xhi-xlow)*0.1;
    int marginy = (yhi-ylow)*0.1;
    if (amrginx == 0) marginx = 1;
    if (amrginy == 0) marginy = 1;
    h->GetXaxis()->SetRange(xlow-marginx, xhi+marginx);
    h->GetYaxis()->SetRange(ylow-marginy, yhi+marginy);
}

void overThreshold(TH1D* h, float threshold, int& binlow, int& binhi)
{
    int nbins = h->GetNbinsX();

    binlow = 1;
    binhi = nbins;

    cout<<"Searching for ROI in "<<nbins<<" bins, from "<<h->GetXaxis()->GetXmin()<<" to "<<h->GetXaxis()->GetXmax()<<endl;

    FOR(i, nbins) {
	if (h->GetBinContent(i+1) > threshold) {
	    binlow = i+1;
	    break;
	}
    }

    FOR(i, nbins) {
	if (h->GetBinContent(nbins - i) > threshold) {
	    binhi = nbins - i;
	    break;
	}
    }

    return;
}



int main(int argc, char **argv)
{
    if (argc < 2) {
	cerr<<"Provide at least 1 argument: input file name."<<endl;
	return -1;
    }

    TString fname(argv[1]);

    int nevents = 1;
    int startevt = 0;

    if (argc > 2)
	nevents = atoi(argv[2]);
    if (argc > 3)
	startevt = atoi(argv[3]);

    convert(fname.Data(), nevents, startevt);

    return 0;
}
