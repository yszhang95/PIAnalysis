import ROOT
import numpy as np

def shift_value(h):
    for i in range(h.GetNbinsX()+1):
        for j in range(h.GetNbinsY()+1):
            content = h.GetBinContent(i, j)
            content = content - 1
            if content < 1E-5 and content > -1E-5:
                content = 0
            if content > 1E-5:
                content = ROOT.TMath.Log10(content)
            if content < -1E-5:
                content = -ROOT.TMath.Log10(content)
            if np.isnan(content):
                content = 0
            h.SetBinContent(i, j, content)
            print(h.GetBinContent(i, j))

finputs = {}
finputs["pienu"] = ROOT.TFile("pienu_job_output.root")
finputs["pimue"] = ROOT.TFile("pimue_job_output.root")

hists = {
    "pistop_rec_xy": {},
    "pistop_rec_xy_5hits": {},
    "pistop_rec_xy_eff": {},
    "e_rec_angle": {},
    "e_rec_angle_norm": {},
    "pistop_rec_xy_ratio": None,
    "e_rec_angle_ratio": None
}

for fk, fv in finputs.items():
    for hk in ["pistop_rec_xy", "pistop_rec_xy_5hits", "e_rec_angle"]:
        hists[hk][fk] = fv.Get("h_{}".format(hk))

for fk in finputs.keys():
    hists["e_rec_angle_norm"][fk] = hists["e_rec_angle"][fk].Clone()
    hists["e_rec_angle_norm"][fk].Scale(1./hists["e_rec_angle"][fk].Integral())
    hists["e_rec_angle_norm"][fk].SetTitle("{}, normalized distribution".format(hists["e_rec_angle"][fk].GetTitle()))

for fk in finputs.keys():
    hists["pistop_rec_xy_eff"][fk] = hists["pistop_rec_xy_5hits"][fk].Clone()
    hists["pistop_rec_xy_eff"][fk].Divide(hists["pistop_rec_xy"][fk])
    hists["pistop_rec_xy_eff"][fk].SetTitle("#frac{rec xy of #pi^{+} && 5 hits of e^{+}}{rec xy of #pi^{+}}")
    hists["pistop_rec_xy_eff"][fk].SetStats(0)

hists["pistop_rec_xy_ratio"] = hists["pistop_rec_xy_eff"]["pienu"].Clone()
hists["pistop_rec_xy_ratio"].SetTitle("Acceptance x efficiency ratio, pienu/pimue")
hists["pistop_rec_xy_ratio"].GetZaxis().SetTitle("pienu/pimue")
hists["pistop_rec_xy_ratio"].Divide(hists["pistop_rec_xy_eff"]["pimue"])

hists["e_rec_angle_ratio"] = hists["e_rec_angle_norm"]["pienu"].Clone()
hists["e_rec_angle_ratio"].Divide(hists["e_rec_angle_norm"]["pimue"])
hists["e_rec_angle_ratio"].SetTitle("Distribution ratio, pienu/pimue")
hists["e_rec_angle_ratio"].GetZaxis().SetTitle("pienu/pimue")

canvas = {}
canvas["pienu"] = ROOT.TCanvas("c_pienu",
                               "#pi^{+} #rightarrow e^{+}", 800*2, 600*2)
canvas["pimue"] = ROOT.TCanvas("c_pimue",
                               "#pi^{+} #rightarrow #mu^{+} #rightarrow e^{+}",
                               800*2, 600*2)
canvas["ratio"] = ROOT.TCanvas("c_ratio",
                               "Ratio of variables", 800*2, 600)

canvas["pienu"].Divide(2, 2)
canvas["pimue"].Divide(2, 2)
canvas["ratio"].Divide(2, 1)


def plot_vars(c, k):
    c.cd(1)
    hists["pistop_rec_xy"][k].Draw("COLZ")
    c.cd(2)
    hists["pistop_rec_xy_5hits"][k].Draw("COLZ")
    c.cd(3)
    hists["pistop_rec_xy_eff"][k].Draw("COLZ")
    c.cd(4)
    hists["e_rec_angle_norm"][k].Draw("COLZ")
    c.Print("{}.png".format(c.GetName()))


def plot_ratio(c):
    ROOT.gStyle.SetOptStat(0)
    pad1 = c.cd(1)
    pad1.SetRightMargin(0.15)
    hists["pistop_rec_xy_ratio"].Draw("COLZ")
    pad2 = c.cd(2)
    pad2.SetRightMargin(0.15)
    hists["e_rec_angle_ratio"].Draw("COLZ")
    c.Print("{}.png".format(c.GetName()))
    pad1.SetLogz()
    pad2.SetLogz()
    c.Print("{}_log.png".format(c.GetName()))

    pad1.cd()
    hists["pistop_rec_xy_ratio"].SetMinimum(1-1.1E-4)
    hists["pistop_rec_xy_ratio"].SetMaximum(1+1.1E-4)
    hists["pistop_rec_xy_ratio"].Draw("COLZ")
    c.Print("{}_log_zoom.png".format(c.GetName()))

    # pad1.cd()
    # hh = hists["pistop_rec_xy_ratio"].Clone("{}_shifted".format(hists["pistop_rec_xy_ratio"].GetName()))
    # shift_value(hh)
    # hh.SetStats(0)
    # hh.Draw("COLZ")
    # c.Print("{}_log_zoom.png".format(c.GetName()))


plot_vars(canvas["pienu"], "pienu")
plot_vars(canvas["pimue"], "pimue")
plot_ratio(canvas["ratio"])
