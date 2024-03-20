
#ifndef FOR
#define FOR(i, size) for (int i = 0; i < size; ++i)
#endif

#include "color_scale_offset.C"

static int gEvent = 0;
TFile* gF = 0;
TCanvas* gC = 0;
const char* gLogfname = 0;


void next(int step = 1);

void drawHistogram(const char* fname, const char* logfname, int startevent = 0)
{
    gEvent = startevent;
    gLogfname = logfname;
    gF = TFile::Open(fname, "read");

    int padh = 9*34;
    int padw = 16*34;
    gC = new TCanvas("c", "", padw+2, 3*padh+24);
    // Set right margin so that colour scale fits in with its title
    gC->Divide(1,3,1e-4,1e-4);
    FOR(i, 3) {
	auto pad = (TPad*)gC->GetPad(i+1);
	pad->SetRightMargin(1.2*pad->GetRightMargin());
    }

    next(0);
    return;
}


void next(int step)
{
    const char* histname = "Event_%d/hRawDigitsVsWireVsTick_plane%d_%d";

    gEvent += step;
    if (gEvent < 0) {
	gEvent = 0;
	return;
    }


    auto setScale = new TExec("setScale", "change_current_hist_color_scale()");
    FOR(iplane, 3) {
	auto h = (TH2F*)gF->Get(Form(histname, gEvent, iplane, gEvent));
	if (!h) {
	    continue;
	}

	auto pad = gC->cd(iplane+1);
	pad->Clear();
	h->Draw("colz");
	setScale->Draw();
	h->Draw("colz same");
	h->GetZaxis()->SetTitle("Raw ADC");
	h->SetTitle(Form("Plane %d", iplane));
    }

    // print out event info
    gSystem->Exec(Form("scripts/print_evt_info.sh -n %d %s", gEvent, gLogfname));

    return;
}


void previous(int step = 1)
{
    next(-step);
    return;
}

void change_current_hist_color_scale()
{
    if (!gPad) return;
    TIter next(gPad->GetListOfPrimitives());
    while (auto obj = next()) {
	if (obj->InheritsFrom(TH2::Class())) {
	    My::SetColorScale((TH2*)obj);
	    return;
	}
    }
}
