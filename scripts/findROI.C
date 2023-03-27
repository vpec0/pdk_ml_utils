#define FOR(i, size) for (int i = 0; i < size; ++i)

#define OUTPROJECTIONS

void overThreshold(TH1D* h, float threshold, int& binlow, int& binhi);

void findROI(const char* fname, const char* histname)
{
    const float THRESHOLDX = 0.2;
    const float THRESHOLDY = 0.1;


    auto f = TFile::Open(fname, "read");

    auto h = (TH2F*)f->Get(histname);
    if (!h) {
	cout<<"Error: could not find histogram "<<histname<<" in file "<<fname<<endl;
	return;
    }

    // create projections and find regions over threshold
    auto px = h->ProjectionX();
    auto py = h->ProjectionY();

    // output file name
    TString outprefix(fname);
    outprefix.ReplaceAll(".root", "/");
    outprefix.Prepend("plots/");
    gSystem->Exec( Form("mkdir -p $(dirname %s)", outprefix.Data()) );

#ifdef OUTPROJECTIONS
    // Draw and save projection plots
    TH1* hists[2] = {px, py};
    FOR(i, 2) {
	hists[i]->Draw();
	gPad->SaveAs( outprefix + hists[i]->GetName() + "_before_scaling.png");
    }
#endif

    // Rescale projections to normalize to number of bins
    int xlow, xhi, ylow, yhi;

    px->Scale(1./py->GetNbinsX());
    py->Scale(1./px->GetNbinsX());

#ifdef OUTPROJECTIONS
    // Draw and save projection plots after scaling
    FOR(i, 2) {
	hists[i]->Draw();
	gPad->SaveAs( outprefix + hists[i]->GetName() + "_after_scaling.png");
    }
#endif

    // Find region over threshold
    overThreshold(px, THRESHOLDX, xlow, xhi);
    overThreshold(py, THRESHOLDY, ylow, yhi);

    cout<<"Found range X: "<<xlow<<" - "<<xhi
	<<" = "<<px->GetXaxis()->GetBinLowEdge(xlow)<<" - "
	<<px->GetXaxis()->GetBinUpEdge(xhi)<<endl
	<<"            Y: "<<ylow<<" - "<<yhi
	<<" = "<<py->GetXaxis()->GetBinLowEdge(ylow)<<" - "
	<<py->GetXaxis()->GetBinUpEdge(yhi)<<endl;


    // Set 10% margins on each side
    int marginx = (xhi-xlow)*0.1;
    int marginy = (yhi-ylow)*0.1;
    h->GetXaxis()->SetRange(xlow-marginx, xhi+marginx);
    h->GetYaxis()->SetRange(ylow-marginy, yhi+marginy);

    h->SetBinContent(0, -1);

    gStyle->SetNumberContours(255);
    h->Draw();

    TString outfname(outprefix);
    outfname = outfname + histname + ".png";
    gPad->SaveAs(outfname);
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
