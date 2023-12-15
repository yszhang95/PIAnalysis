#include <iostream>

#include "TCanvas.h"
#include "TView.h"
#include "TFile.h"
#include "TPolyLine3D.h"
#include "TGraph2D.h"
#include "TH3D.h"
#include "TLegend.h"
#include <RtypesCore.h>

void draw(Long64_t entry)
{
  TFile* f = TFile::Open("output.root");
  // auto rec_line = f->Get<TPolyLine3D>(::Form("rec_event%lld", entry));
  // auto gen_line = f->Get<TPolyLine3D>(::Form("gen_event%lld", entry));

  auto e_rec_line = f->Get<TGraph2D>(::Form("e_rec_event%lld", entry));
  auto e_gen_line = f->Get<TGraph2D>(::Form("e_gen_event%lld", entry));

  auto pi_rec_line = f->Get<TGraph2D>(::Form("pi_rec_event%lld", entry));
  auto pi_gen_line = f->Get<TGraph2D>(::Form("pi_gen_event%lld", entry));


  auto pi_fit = f->Get<TPolyLine3D>(::Form("pi_fit_event%lld", entry));
  auto e_fit = f->Get<TPolyLine3D>(::Form("e_fit_event%lld", entry));

  const bool ok = e_rec_line && e_gen_line && pi_rec_line && e_rec_line;
  if (!ok) {
    std::cout << "At least one of input graphs is null\n";
    return;
  }

  e_gen_line->SetLineColor(kRed);
  e_gen_line->SetMarkerColor(kRed);
  e_gen_line->SetMarkerStyle(20);
  //  e_gen_line->SetMarkerSize(1.5);
  e_rec_line->SetLineColor(kBlue);
  e_rec_line->SetMarkerColor(kBlue);
  e_rec_line->SetMarkerStyle(kOpenCircle);
  //  e_rec_line->SetMarkerSize(1.5);

  pi_gen_line->SetLineColor(kRed);
  pi_gen_line->SetMarkerColor(kRed);
  pi_gen_line->SetMarkerStyle(kFullSquare);
  // pi_gen_line->SetMarkerSize(1.5);
  pi_rec_line->SetLineColor(kBlue);
  pi_rec_line->SetMarkerColor(kBlue);
  pi_rec_line->SetMarkerStyle(kOpenSquare);
  // pi_rec_line->SetMarkerSize(1.5);

  pi_fit->SetLineColor(kGreen);
  e_fit->SetLineColor(kCyan+3);

  TLegend* leg = new TLegend(0.65, 0.76, 0.95, 0.95);
  leg->AddEntry(pi_gen_line, "pion truth", "p");
  leg->AddEntry(pi_rec_line, "pion reco", "p");
  leg->AddEntry(e_gen_line, "positron truth", "p");
  leg->AddEntry(e_rec_line, "positron reco", "p");
  leg->AddEntry(pi_fit, "pion fit", "l");
  leg->AddEntry(e_fit, "positron fit", "l");

  TCanvas *c1 = new TCanvas("c1","c1",500,500);
  TH3D *h = new TH3D("h", "", 10, -25, 25, 10, -25, 25, 80, -1, 7);
  h->SetTitle("ATAR tracks/hits;X (mm);Y (mm); Z(mm)");
  h->SetStats(0);
  h->Draw();
  e_gen_line->Draw("PSAME");
  e_rec_line->Draw("PSAME");
  pi_gen_line->Draw("PSAME");
  pi_rec_line->Draw("PSAME");

  pi_fit->Draw("LSAME");
  e_fit->Draw("LSAME");

  leg->Draw();
}
