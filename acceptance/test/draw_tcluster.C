#include "TFile.h"
#include "TH1D.h"
#include "TF1.h"
#include "TCanvas.h"
#include "TStyle.h"

void draw_tcluster_pienu()
{
  gStyle->SetOptFit(1);
	TFile* fin = TFile::Open("pienu-output.root", "READ");
	TH1D* hcategories;
	TH1D* htcluster;
	TH1D* hdelayed;
	fin->GetObject("hcategories", hcategories);
	fin->GetObject("htcluster", htcluster);
	fin->GetObject("hdelayed", hdelayed);
	TCanvas* c = new TCanvas("ce", "delayed signal of pienu", 800, 600);
	c->cd();
	auto ntotal = htcluster->Integral();
	std::cout << ntotal << "\n";
	TF1* f = new TF1("fpitoe", "[0]*exp(-x/[1])/[1]", 0, 1000);
	f->SetParameter(1, 26);
	f->SetParameter(0, ntotal);
	std::cout << f->Eval(0) << "\n";
	std::cout << f->Eval(1) << "\n";
	hdelayed->GetXaxis()->SetRangeUser(0, 150);
	hdelayed->Draw("");
	// f->Draw("SAME");
	TF1* f2 = new TF1("fpitoefit", "[0]*exp(-x/[1])/[1]", 5, 1000);
	f2->SetParameter(1, 26);
	f2->SetParameter(0, ntotal);
        hdelayed->Fit(f2, "R", "", 5, 1000);
        hdelayed->Fit(f2, "ER", "", 5, 1000);
        c->SetLogy(1);
        c->Draw();
        std::cout << "pienu " << ntotal << "\n";

        TH1D *hdecay_true;
        fin->GetObject("hdecay_true", hdecay_true);
        TCanvas *cdecay = new TCanvas("cdecay", "cdecay in truth", 800, 600);
        TF1 *f3 = new TF1("fpitoefit_true", "[0]*exp(-x/[1])/[1]", 0, 1000);
        TF1 *f4 = new TF1("fpitoefit_true_sub", "[0]*exp(-x/[1])/[1]", 5, 1000);
        f3->SetParameter(1, 26);
        f3->SetParameter(0, hdecay_true->Integral());
        f4->SetParameter(1, 26);
        f4->SetParameter(0, hdecay_true->Integral());
        hdecay_true->Fit(f3, "RN", "", 0, 1000);
        hdecay_true->Fit(f3, "ERN", "", 0, 1000);
        hdecay_true->Fit(f4, "RN", "", 5, 1000);
        hdecay_true->Fit(f4, "ERN", "", 5, 1000);
        f3->SetLineColor(kGreen+2);
        f3->SetLineStyle(2);
        hdecay_true->GetListOfFunctions()->Add(f3);
        hdecay_true->GetListOfFunctions()->Add(f4);
        hdecay_true->GetXaxis()->SetRangeUser(0, 150);
        hdecay_true->Draw();
        cdecay->SetLogy(1);
        cdecay->Draw();
}

void draw_tcluster_pimunu()
{
	gStyle->SetOptFit(1);
	TFile* fin = TFile::Open("pimunu-output.root", "READ");
	TH1D* hcategories;
	TH1D* htcluster;
	TH1D* hdelayed;
	fin->GetObject("hcategories", hcategories);
	fin->GetObject("htcluster", htcluster);
	fin->GetObject("hdelayed", hdelayed);
	TCanvas* c = new TCanvas("cmu", "delayed signal of pimunu", 800, 600);
	c->cd();
	auto ntotal = htcluster->Integral();
	std::cout << ntotal << "\n";
	TF1* f1 = new TF1("fpitomu", "[0]*exp(-x/[1])/[1]", 0, 1000);
	f1->SetParameter(1, 26);
	f1->SetParameter(0, ntotal);

	TF1* f2 = new TF1("fmutoe", "[0]/([2]-[1])*(exp((5-x)/[2])*exp(-5/[1]) - exp(-5/[2])*exp((5-x)/[1]))", 0, 1000);
	f2->SetParameter(1, 26);
	f2->SetParameter(2, 2197);
	f2->SetParameter(0, ntotal);

	// std::cout << f->Eval(0) << "\n";
	// std::cout << f->Eval(1) << "\n";
	hdelayed->GetXaxis()->SetRangeUser(0, 1000);
	hdelayed->Draw("");
	TF1* f3 = new TF1("fsum", "[0]*exp(-x/[1])/[1] + [0]/([2]-[1])*(exp((5-x)/[2])*exp(-5/[1]) - exp(-5/[2])*exp((5-x)/[1]))", 0, 1000);

	f3->SetParameter(1, 26);
	f3->SetParameter(2, 2197);
	f3->SetParameter(0, ntotal);

	f3->SetLineStyle(2);
	// f3->Draw("SAME");
	//auto f4 = new TF1(*f3);
	TF1* f4 = new TF1("fsumfit", "[0]*exp(-x/[1])/[1] + [0]/([2]-[1])*(exp((5-x)/[2])*exp(-5/[1]) - exp(-5/[2])*exp((5-x)/[1]))", 0, 1000);
	f4->SetParameter(1, 26);
	f4->SetParameter(2, 2197);
	f4->SetParameter(0, ntotal);
	hdelayed->Fit(f4, "R", "", 10, 1000);

	c->Draw();

	std::cout << "pimue " << ntotal << "\n";
}

void draw_tcluster()
{
	// draw_tcluster_pimunu();
	draw_tcluster_pienu();
}
