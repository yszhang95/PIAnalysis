#include "RooDataSet.h"
#include "RooRealVar.h"
#include "RooExponential.h"
#include "RooRealConstant.h"
#include "TCanvas.h"
#include "TH1D.h"
#include "TF1.h"
#include "RooPlot.h"
#include "TLatex.h"
#include "TLegend.h"

double return_error_inverse(const double x, const double xerr) {
  // 1./x --> dx/x/x;
  return std::abs(xerr/x/x);
}
void toymc()
{

  const auto tau_pi = 26.033;
  TLatex* tex = new TLatex();
  RooRealVar *t = new RooRealVar("t", "time [ns]", 0, 200);
  RooRealVar *tau_pi_inverse = new RooRealVar("tau_pi_inverse", "-1/#tau_{#pi}", -1/tau_pi, -1./(tau_pi-1), -1./(tau_pi+1));
  RooExponential* decay = new RooExponential("decay", "decay", *t, *tau_pi_inverse);
  auto mydataset = decay->generate(*t, 5000);
  auto h =
      mydataset->createHistogram("binned_histogram", *t, RooFit::Binning(200));

  TF1 *f = new TF1("f", "[0]*exp(x*[1])*[1]", 0, 200);
  f->SetParameter(0, 5000);
  f->SetParameter(1, tau_pi_inverse->getVal());
  TF1 *f2 = new TF1("f2", "[0]*exp(x*[1])*[1]", 0, 200);
  f2->SetParameter(0, 5000);
  f2->SetParameter(1, tau_pi_inverse->getVal());

  TCanvas *c = new TCanvas("c", "canvas", 800 * 2, 600);
  c->Divide(2, 1);
  c->cd(1);
  auto tframe = t->frame();
  decay->fitTo(*mydataset);
  mydataset->plotOn(tframe, RooFit::Binning(200));
  mydataset->statOn(tframe, "M");
  decay->plotOn(tframe);
  // tex->DrawLatex(0.5, 0.7, ::Form("mean = %.3f +/- %.3f", mydataset->mean(*t)), mydataset->mean);
  tframe->Draw();
  tex->DrawLatexNDC(0.5, 0.61,
                    ::Form("#tau_{#pi}^{input} = %.3f", tau_pi));
  tex->DrawLatexNDC(0.5, 0.55,
                    ::Form("#tau_{#pi} = %.3f +/- %.3f",
                           -1. / tau_pi_inverse->getVal(),
                           return_error_inverse(tau_pi_inverse->getVal(),
                                                tau_pi_inverse->getError())));
  c->cd(2);
  h->Fit(f, "N");
  h->Fit(f2, "PN");
  f->SetLineColor(kRed);
  f2->SetLineColor(kBlue);
  TLegend *leg = new TLegend(0.5, 0.4, 0.85, 0.65);
  leg->AddEntry(f, "Neyman #chi^2", "L");
  // leg->AddEntry(f2, "Pearson #chi^2", "L");
  h->GetListOfFunctions()->Add(f);
  // h->GetListOfFunctions()->Add(f2);
  h->Draw();
  leg->Draw();
  tex->DrawLatexNDC(
      0.5, 0.75,
      ::Form("#tau_{#pi}^{Neyman} = %.3f +/- %.3f", -1. / f->GetParameter(1),
             return_error_inverse(f->GetParameter(1), f->GetParError(1))));
  // tex->DrawLatexNDC(0.5, 0.82,
  //                   ::Form("#tau_{#pi^{Pearson}} = %.3f +/- %.3f",
  //                          -1. / f2->GetParameter(1),
  //                          return_error_inverse(f2->GetParameter(1),
  //                                               f2->GetParError(1))));


  c->Draw();
}

int main() {
  return 0;
}
