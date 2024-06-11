TH2* rebin(TH2* h, int nx, int ny, const char* label="")
{
    int n1 = h->GetNbinsX();
    int n2 = h->GetNbinsX();
    if (n1 % nx) {
        return nullptr;
    }
    if (n2 % ny) {
        return nullptr;
    }

    TH2* hrebin = new TH2F(::Form("%s_rebin%s_%d_%d", h->GetName(), label, nx, ny),
            ::Form("%s;%s;%s;", h->GetTitle(), h->GetXaxis()->GetTitle(), h->GetYaxis()->GetTitle()),
            nx, h->GetXaxis()->GetBinLowEdge(1), h->GetXaxis()->GetBinLowEdge(n1+1),
            ny, h->GetYaxis()->GetBinLowEdge(1), h->GetYaxis()->GetBinLowEdge(n2+1));
    
    int nxstep = n1/nx;
    int nystep = n2/ny;
    for (int i=0; i<nx; ++i) {
        for (int j=0; j<nx; ++j) {
            int xstart = i*nxstep + 1;
            int xend = (i+1)*nxstep + 1;
            int ystart = j*nystep + 1;
            int yend = (j+1)*nystep + 1;
            double c = h->Integral(xstart, xend, ystart, yend);
            std::cout << xstart << " " << xend << " "<< c << "\n";
            hrebin->SetBinContent(i+1, j+1, c);
        }
    }
    return hrebin;
}
void plot_ratio()
{
    TFile* f1 = new TFile("pienu_job_output.root");
    TFile* f2 = new TFile("pimue_job_output.root");
    TH2* h1 = f1->Get<TH2>("h_e_truemom_angle");
    TH2* h2 = f2->Get<TH2>("h_e_truemom_angle");

    TH2* h1rebin = rebin(h1, 5, 5, "pienu");
    TH2* h2rebin = rebin(h2, 5, 5, "pimue");
    // h1rebin->Print("all");

    TH2* h = (TH2*) h1rebin->Clone("hratio");
    h->Divide(h2rebin);
    h->Print("all");

    TCanvas* c = new TCanvas("c", "ratio", 800, 600);
    h->SetStats(0);
    h->Draw("COLZ");
    c->Draw();
}
