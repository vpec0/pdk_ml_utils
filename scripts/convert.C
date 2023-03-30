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

void FindROI(TH2* h, float thldx = 50, float thldy = 50);
void overThreshold(TH1D* h, float threshold, int& binlow, int& binhi);

void convert(const char* fname, int nevents = 1, int startevt = 0)
{
    const float thresholdx = 150.;
    const float thresholdy = 100.;

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

    // Pattern for output csv file
    TString csvname_pat = outprefix + "_plane%d_%d.csv";
    int groupfiles = 20; // group tar files by 20 events

    // pedestal should now be substracted already in larcvmaker (21 Mar 2023)
    // // pedestal to be subtracted
    // const double pedestal = 490.;

    if (nevents == -1 || (nevents + startevt) > tree->GetEntries())
	nevents = tree->GetEntries() - startevt;
    int fiftieth = nevents/50;

    FOR(entry, nevents) {
	if (fiftieth && entry%fiftieth == 0) {
	    if (entry)
		cout<<"\r"<<flush;
	    cout<<"Done "<<entry<<" entries ("<<(100*entry/nevents)<<"%)."<<flush;
	}

	int ievent = entry + startevt;

	outf->cd();
	outf->mkdir(Form("Event_%d",ievent))->cd();

	// Get entry
	tree->GetEntry(ievent);

	// create an image for each plane
	FOR(iplane, 3) {
	    // Create the output histogram
	    int cols = evt->as_vector()[iplane].meta().cols();
	    int rows = evt->as_vector()[iplane].meta().rows();
	    auto h = new TH2F(Form("hRawDigitsVsWireVsTick_plane%d_%d", iplane, ievent), ";Ticks;Wire",
			      cols, (iplane==2)?-cols/2:0, (iplane==2)?cols/2:cols, // Ticks
			      rows, 0., rows); // Wires

	    // Output csv
	    //ofstream ofs;
	    FILE* outfcsv = 0;
	    if ( DOCSV ) {
		//ofs.open(csvname);
		outfcsv = gSystem->OpenPipe( Form("gzip -4 > "+csvname_pat + ".gz", iplane, ievent), "w" );

		fprintf(outfcsv, "%d x %d\n", rows, cols);
	    }

	    FOR(i, rows) {
		FOR(j, cols) {
		    //double content = (evt->as_vector()[iplane].pixel(i, j) > 0)?evt->as_vector()[iplane].pixel(i, j) - pedestal:0;
		    double content = evt->as_vector()[iplane].pixel(i, j);
		    h->SetBinContent(j+1, i+1, content);

		    if ( DOCSV )
			fprintf(outfcsv, "%.0f\n", content);
		    //     //ofs<<j<<","<<i<<","<<content<<endl;
		    //     ofs<<content<<endl;
		}
	    }

	    // determine region of interest
	    FindROI(h, thresholdx, thresholdy);
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

	    // gPad->SaveAs(Form(outprefix + "_2dhist_%d.png", ievent));

	    // write-out the 2d histogram
	    h->SetOption("colz");
	    h->Write();

	    delete h;
	} // FOR iplane
	// Tar single event images together
	if ( DOCSV ) {
	    auto target = csvname_pat;
	    target.ReplaceAll("_plane%d", "") += ".gz.tar";
	    cout<<target<<endl;
	    auto source = csvname_pat;
	    source.ReplaceAll("_plane%d", "_plane{0..2}") += ".gz";
	    cout<<source<<endl;
	    gSystem->Exec( Form("tar --remove-files -cf " + target + " " + source,
				ievent, ievent) );
	}
	// Tar files after every 20 events
	if ( DOCSV && (entry+1)%groupfiles == 0) {
	    auto target = csvname_pat;
	    target.ReplaceAll("_plane%d_", "_grp") += ".gz.tar";
	    cout<<target<<endl;
	    auto source = csvname_pat;
	    source.ReplaceAll("_plane%d_%d", "_{%d..%d}") += ".gz.tar";
	    cout<<source<<endl;
	    gSystem->Exec( Form("tar --remove-files -cf " + target + " " + source,
				entry/groupfiles, ievent-groupfiles+1, ievent) );
	}
    } // FOR entry
    cout<<endl;

    // tar all the compressed output csv files
    if ( DOCSV ) {
	// first group leftover single-event tar files
	if (nevents%groupfiles > 0 && nevents/groupfiles > 0) {
	    auto target = csvname_pat;
	    target.ReplaceAll("_plane%d_", "_grp") += ".gz.tar";
	    cout<<target<<endl;
	    auto source = csvname_pat;
	    source.ReplaceAll("_plane%d_%d", "_{%d..%d}") += ".gz.tar";
	    cout<<source<<endl;
	    gSystem->Exec( Form("tar --remove-files -cf " + target + " " + source,
				nevents/groupfiles, startevt+nevents-groupfiles, startevt + nevents - 1) );
	}

	cout<<"nevents = "<<nevents<<", startevt = "<<startevt<<endl;

	if (nevents > 1 && nevents < groupfiles) {
	    auto target = csvname_pat;
	    target.ReplaceAll("_plane%d_%d", "") += ".gz.tar";
	    cout<<target<<endl;
	    auto source = csvname_pat;
	    source.ReplaceAll("_plane%d_%d", "_{%d..%d}") += ".gz.tar";
	    cout<<source<<endl;
	    gSystem->Exec(Form("tar --remove-files -cf " + target + " " + source, startevt, startevt+nevents-1));
	} else if (nevents > 1) {
	    auto target = csvname_pat;
	    target.ReplaceAll("_plane%d_%d", "") += ".gz.tar";
	    cout<<target<<endl;
	    auto source = csvname_pat;
	    source.ReplaceAll("_plane%d_%d", "_grp*") += ".gz.tar";
	    cout<<source<<endl;
	    gSystem->Exec("tar --remove-files -cf " + target + " " + source);
	}
    }
    // close the output root file
    outf->Close();
}

void FindROI(TH2* h, float thldx, float thldy)
{
    // create projections and find regions over threshold
    auto px = h->ProjectionX();
    auto py = h->ProjectionY();

    int rebinx = 32;
    int rebiny = 8;

    px->Rebin(rebinx);
    py->Rebin(rebiny);

    int xlow, xhi, ylow, yhi;

    // px->Scale(1./px->GetNbinsX());
    // py->Scale(1./py->GetNbinsX());

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

    xhi *= rebinx;
    xlow *= rebinx;
    yhi *= rebiny;
    ylow *= rebiny;

    int marginx = (xhi-xlow)*0.1;
    int marginy = (yhi-ylow)*0.1;
    if (marginx == 0) marginx = 1;
    if (marginy == 0) marginy = 1;
    h->GetXaxis()->SetRange(xlow-marginx, xhi+marginx);
    h->GetYaxis()->SetRange(ylow-marginy, yhi+marginy);
}

void overThreshold(TH1D* h, float threshold, int& binlow, int& binhi)
{
    int nbins = h->GetNbinsX();

    binlow = 1;
    binhi = nbins;

#ifdef DEBUG
    cout<<"Searching for ROI in "<<nbins<<" bins, from "<<h->GetXaxis()->GetXmin()<<" to "<<h->GetXaxis()->GetXmax()<<endl;
#endif

    FOR(i, nbins) {
#ifdef DEBUG
	cout<<" Bin "<<(i+1)<<" content = "<<abs(h->GetBinContent(i+1))<<endl;
#endif

	if (abs(h->GetBinContent(i+1)) > threshold) {
	    binlow = i+1;
#ifdef DEBUG
	    cout<<" Found low limit bin "<<binlow<<endl;
#endif
	    break;
	}
    }

    FOR(i, nbins) {
	if (abs(h->GetBinContent(nbins - i)) > threshold) {
	    binhi = nbins - i;
#ifdef DEBUG
	    cout<<" Found upper limit bin "<<binhi<<endl;
#endif
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
