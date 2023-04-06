/**
 * Example sctipr to change color poalette for 2D histograms with content > and < 0.
 * The new color palette sets its "median" color to histogram's 0.
 *
 * Vitkor Pec, 2023/02/21
 **/

#ifndef FOR
#define FOR(i, size) for (int i = 0; i < size; ++i)
#endif

namespace My {
    //static int PaletteExists = 0;

    void SetColorScale(TH2* h)
    {
	const int NCONTOURS = 256;
	static int initialized = 0;
	static Int_t full_palette[NCONTOURS] = {};


	h->SetContour(NCONTOURS);

	//if (PaletteExists) return;

	// new full palette - red-white-blue
	// symmetric around 0
	const Int_t Number = 3;
	Double_t Red[Number]    = { 0.00, 1.00, 1.00};
	Double_t Green[Number]  = { 0.00, 1.00, 0.00};
	Double_t Blue[Number]   = { 1.00, 1.00, 0.00};
	Double_t Length[Number] = { 0.00, 0.50, 1.00 };

	Int_t index = full_palette[0];
	if ( !initialized ) { // palette originially not initialized
	    Int_t nb=NCONTOURS;
	    index = TColor::CreateGradientColorTable(Number,Length,Red,Green,Blue,nb);
	    FOR(i, NCONTOURS)
		full_palette[i] = index+i;
	    initialized = 1;
	}

	// Get scale from the histogram
	double low = h->GetMinimum();
	double hi = h->GetMaximum();
	double full_scale = 2*max(abs(hi), abs(low));

	// Get new bounds
	auto n_color_min = int((0.5+low/full_scale)*NCONTOURS);
	auto n_color_max = int((0.5+hi/full_scale) *NCONTOURS - 1);

	TColor::SetPalette(n_color_max - n_color_min + 1, full_palette + n_color_min);
    }



    void color_scale_offset(float xscale = 1., float yscale = 1.)
    {

	// create a 2D histogram
	const int NBINS = 20;
	auto h = new TH2F("h","", NBINS, 0, 100, NBINS, 0, 100);


	// Fill with an anti-symmetric function
	FOR(i, NBINS) {
	    FOR(j, NBINS) {
		float content = TMath::ATan((xscale*i - yscale*j)/100.);
		h->SetBinContent(i+1, j+1, content);
	    }
	}


	SetColorScale(h);

	h->Draw("colz");

	gPad->SaveAs("color_scale_offset.pdf");
    }

};
