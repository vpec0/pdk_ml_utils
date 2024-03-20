
#define FOR(i, size) for (int i = 0; i < size; ++i)

#include "color_scale_offset.h"

const int MIDDLESKIP = 80;

TH2F* CombineHists(TH2F* h1, TH2F* h2);


void CombineInduction(const char* fname, int event)
{
    gROOT->ForceStyle();
    gStyle->SetPadRightMargin(0.12);
    gStyle->SetTickLength(0, "xy");
    // gStyle->SetPadTickX(0);
    // gStyle->SetPadTickY(0);

    auto f = TFile::Open(fname, "read");

    TString dirname = "Event_";
    dirname += event;
    TString hname = "hRawDigitsVsWireVsTick_plane%d_";
    hname += event;
    auto h1 = f->Get<TH2F>(dirname+"/"+Form(hname, 0));
    auto h2 = f->Get<TH2F>(dirname+"/"+Form(hname, 1));

    auto hcol = f->Get<TH2F>(dirname+"/"+Form(hname, 2));

    auto h = CombineHists(h1, h2);

    auto* c = new TCanvas("c", "", 600, 800);
    c->Divide(1,2);

    // My::SetColorScale(h);
    // My::SetColorScale(hcol);


    // h->GetListOfFunctions()->Add(setScale);
    // hcol->GetListOfFunctions()->Add(setScale);

    c->cd(1);
    h->Draw();
    gPad->GetFrame()->SetLineWidth(0);
    My::drawExecChangeScale();
    c->cd(2);
    hcol->Draw();
    gPad->GetFrame()->SetLineWidth(0);
    My::drawExecChangeScale();
    hcol->SetTitle("Collection plane;Time [ticks];Channel;Raw ADC");
}



TH2F* CombineHists(TH2F* h1, TH2F* h2)
// h1 will retain positive X axis; h2 will be added mirrored along Y into negative X values
// Also module shifts h2 along Y axis
{
    int nbinsx = h1->GetNbinsX();
    int nbinsy = h1->GetNbinsY();

    int xmax = h1->GetXaxis()->GetXmax();

    if ( h2->GetNbinsX() != nbinsx || h2->GetNbinsY() != nbinsy ) {
	cerr<<"Histograms "<<h1->GetName()<<" and "<<h2->GetName()<<" don't have the same dimensions."<<endl;
	return 0;
    }

    auto h = new TH2F("hInduction", "Combined U (t > 0) and V (t < 0) planes;Time [ticks]; Channel;Raw ADC",
		      2*nbinsx + MIDDLESKIP, -xmax-MIDDLESKIP/2, xmax+MIDDLESKIP/2,
		      nbinsy, h1->GetYaxis()->GetXmin(), h1->GetYaxis()->GetXmax());

    h->GetXaxis() -> SetRange(nbinsx - h2->GetXaxis()->GetLast() + 1, nbinsx+MIDDLESKIP + h1->GetXaxis()->GetLast() - 1);

    FOR(row, nbinsy) {
	FOR(col, nbinsx) {
	    h->SetBinContent(nbinsx + col + MIDDLESKIP,      row + 1, h1->GetBinContent(col+1, row+1));
	    h->SetBinContent(nbinsx - col, (row+nbinsy/2)%nbinsy + 1, h2->GetBinContent(col+1, row+1));
	}
    }

    h->SetOption("colz");
    h->SetStats(0);

    return h;
}
