
#ifndef FOR
#define FOR(i, size) for (int i = 0; i < size; ++i)
#endif

#include "color_scale_offset.C"

static int gEvent = 0;
TFile* gF = 0;
const char* gLogfname = 0;

void next(int step = 1);

void drawHistogram(const char* fname, const char* logfname, int startevent = 0)
{
    gEvent = startevent;
    gLogfname = logfname;
    gF = TFile::Open(fname, "read");

    next(0);
    return;
}


void next(int step)
{
    const char* histname = "hCollectionRawDigitsVsWireVsTick_";

    gEvent += step;
    if (gEvent < 0) {
	gEvent = 0;
	return;
    }
    auto h = (TH2F*)gF->Get(Form("%s%d", histname, gEvent));
    if (!h) {
	return;
    }
    My::SetColorScale(h);

    h->Draw("colz");

    // print out event info
    gSystem->Exec(Form("scripts/print_evt_info.sh %d %s", gEvent, gLogfname));

    return;
}


void previous(int step = 1)
{
    next(-step);
    return;
}
