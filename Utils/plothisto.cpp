//////////////////////////////////////////////////////////////////
// plothisto: See below for the signature.
//            Plots a given histogram for all the backgrounds, data
//            and signal together
// 
// Options:
// * histoname: Histogram name
// * rebin: Rebin factor. 1 means no rebin, 2 means joinning every
//          2 bins, etc...
// * plottype: Plot type. Possible options are:
//             - 0: All backgrounds and signal stacked
//             - 1: All backgrounds stacked, signal alone
//             - 2: No stacking at all
// * luminosity: Self explanatory :)
//
//////////////////////////////////////////////////////////////////

#ifndef __CINT__
#include<vector>
#include<set>
#include<iostream>
#include<string>
#include<sstream>
#include<stdlib.h>

#include "TPaveText.h"
#include "TH1F.h"
#include "TFile.h"
#include "TCanvas.h"
#include "TROOT.h"
#include "TSystem.h"
#include "TMath.h"
#include "TStyle.h"
#include "THStack.h"

#include "InputParameters.h"
#endif

#include "TString.h"
#include "TLegend.h"



struct common
{
	int SIGNALFACTOR;
	TString data;
	TString sig; 
	TString dlegend;
	TString slegend;
	std::vector<TString> bkg;
	std::vector<TString> bkgzjets;
	std::vector<TString> blegend;
};

///////////////////////////////
// Helper functions
//
TString GetFile(const TString& d) 
{
	TString f;
	TString namefile = d;
	if(d.Contains("WH")) 
	{
		namefile.Replace(0,2, "WHToWW2L");
	}
	f.Form("cluster_%s/Results/%s.root",namefile.Data(),namefile.Data());
	return f;
}

//Adds up integral + underflow + overflow
double GetRealEntries(TH1* h) 
{
	return (h->Integral()+h->GetBinContent(0)+h->GetBinContent(h->GetNbinsX()+1));
}

double GetNormEntries(TH1* h, double w) 
{
	double n = (GetRealEntries(h) * w);
//	std::cout << n << std::flush;
	return n;
}

TFile* OpenFile(const TString& fname) 
{
	TFile* f = TFile::Open(GetFile(fname));
	if(f->IsZombie()) 
	{
		std::cerr << "\033[1;31mOpenFile ERROR\033[1;m Could not find file " 
			<< fname << std::endl;
		exit(-1);
	}
	return f;
}

TH1D* GetHistogram(const TString& hname, TFile* f, const TString& ds) 
{
	TString cname =  ds + "_Cloned";
	TH1D* h = (TH1D*) ((TH1D*) f->Get(hname))->Clone(cname.Data());
	h->SetDirectory(0);
	return h;
}

double GetWeight(TFile* f, double luminosity) 
{
	InputParameters* ip = (InputParameters*) f->Get("Set Of Parameters");
	double xs = 0;
	int neventssample = 0;
	int neventsskim   = 0;
	ip->TheNamedDouble("CrossSection", xs);
	ip->TheNamedInt("NEventsSample", neventssample);
	ip->TheNamedInt("NEventsTotal", neventsskim);
	double weight = xs * luminosity / neventssample;
//	std::cout << " XS=" << xs 
//		<< " NEv. Sample=" << neventssample 
//		<< " NEv. Skim=" << neventsskim
//		<< " Weight = " << weight << std::endl;
	return weight;
}


///////////////////////////////
// Main function
//
void PlotAll(const common & cd ,
		TString histoname="fHMET",
		int rebin = 1,
		int plottype = 0, 
		double luminosity=2143.3) 
{
	// Renaming 
	const int SIGNALFACTOR = cd.SIGNALFACTOR;
	const TString data = cd.data;
	const TString sig  = cd.sig;
	const TString dlegend = cd.dlegend;
	const TString slegend = cd.slegend;
	const std::vector<TString> bkg = cd.bkg;
	const std::vector<TString> bkgzjets = cd.bkgzjets;
	const std::vector<TString> blegend = cd.blegend;


	const unsigned int NBKG = bkg.size();
	const unsigned int NBKGZJETS = bkgzjets.size();


	gROOT->SetBatch(1);
	
	std::cout << "\033[1;34mPlotAll INFO\033[1;m Plotting '" << histoname
		<< "' histogram for all the signal and background" << std::endl;
	
	//gSystem->RedirectOutput("log_kkita.txt","a");
	//////
	//Open the files
//	std::cout << ">> Opening files..." << std::endl;
	// + Data
//	std::cout << "   + Data: " << data << std::endl;
	TFile* fdata = OpenFile(data);
	// + Signal
//	std::cout << "   + Signal: " << sig << std::endl;
	TFile* fwh = OpenFile(sig);
	// + Backgrounds
	TFile* fbkg[NBKG];
	TFile* fbkgzjets[NBKGZJETS];
	for(unsigned int i = 0; i < NBKG ; i++) 
	{
//		std::cout << "   + Background: " << bkg[i] << std::endl;
		if(bkg[i] != "ZJets_Powheg") 
		{
			fbkg[i] = OpenFile(bkg[i]);
		}
		else 
		{
			fbkg[i] = 0;
			for(unsigned int j = 0; j < NBKGZJETS; j++) 
			{
//				std::cout << "     - " << bkgzjets[j] << std::endl;
				fbkgzjets[j] = OpenFile(bkgzjets[j]);
			}
		}
	}

	//////
	// Get the Histograms
//	std::cout << ">> Getting histograms..." << std::endl;
	// + Data
//	std::cout << "   + Data: " << data << std::endl;
	TH1D * hdata = GetHistogram(histoname, fdata, "Data");
	// + Signal
//	std::cout << "   + Signal: " << sig << std::endl;
	TH1D* hwh = GetHistogram(histoname, fwh, sig);
	// + Backgrounds
	TH1D* hbkg[NBKG];
	TH1D* hbkgzjets[NBKGZJETS];
	for(unsigned int i = 0; i < NBKG; i++) 
	{
//		std::cout << "   + Background: " << bkg[i] << std::endl;
		if(fbkg[i])
		{
			hbkg[i] = GetHistogram(histoname, fbkg[i], bkg[i]);
		}
		else
		{ 
			//ZJets
			hbkg[i] = 0;
			for(unsigned int j = 0; j < NBKGZJETS; j++) 
			{
//				std::cout << "     - " << bkgzjets[j] << std::endl;
				hbkgzjets[j] = GetHistogram(histoname, fbkgzjets[j], bkgzjets[j]);
			}
		}
	}
	
	//////
	// Get the input parameters that we need: cross sections, n. events...
//	std::cout << ">> Getting Input Parameters..." << std::endl;
	gSystem->Load("libInputParameters.so");
	
	// + Signal
//	std::cout << "   + Signal: " << sig << " -" << std::flush;
	double weightwh = GetWeight(fwh, luminosity);
	
	// + Background
	double weight[NBKG];
	double weightzjets[NBKGZJETS];
	for(unsigned int i = 0; i < NBKG; i++) 
	{
//		std::cout << "   + Background: " << bkg[i]<< " -" << std::flush;
		if(fbkg[i])
		{
			weight[i] = GetWeight(fbkg[i], luminosity);
		}
		else 
		{ 
			//ZJets
			weight[i] = 0;
			for(unsigned int j = 0; j < NBKGZJETS; j++) 
			{
//				std::cout << "     - " << bkgzjets[j] << " -" << std::flush;
				weightzjets[j] = GetWeight(fbkgzjets[j], luminosity);
			}
		}
	}
	
	//////
	// Get the input parameters that we need: cross sections, n. events...
//	std::cout << ">> Counting events..." << std::endl;
	// + Data
//	std::cout << "   + Data: " << data << std::endl;
	double nevdat = hdata->GetEntries();
	// + Signal
//	std::cout << "   + Signal: " << sig << " -" << std::flush;
	double nevwh  = GetNormEntries(hwh, weightwh);
	// + Background
	double nevbkg = 0;
	for(unsigned int i = 0; i < NBKG; i++) 
	{
//		std::cout << "   + Background: " << bkg[i]<< " -" << std::flush;
		if(fbkg[i]) 
		{
			nevbkg += GetNormEntries(hbkg[i], weight[i]); 
//			std::cout << " - Total: " << nevbkg << std::endl;
		}
		else
		{
			double nevzjets = 0;
			for(unsigned int j = 0; j < NBKGZJETS; j++) 
			{
//				std::cout << "     - " << bkgzjets[j] << " -" << std::flush;
				nevzjets += GetNormEntries(hbkgzjets[j], weightzjets[j]); 
//				std::cout << " - Total: " << nevzjets << std::endl;
			}
//			std::cout << "     - Total: " << nevzjets << std::endl;
			nevbkg += nevzjets;
		}
	}
	
	
	//////
	// Scale histograms to luminosity and cross section
//	std::cout << ">> Scaling histograms..." << std::endl;
	int izjets = -1;
	if(plottype==0 || plottype==1) 
	{
//		std::cout << "   + Signal: " << sig << " - " << std::flush;
//		std::cout << weightwh << std::endl;
		hwh->Scale(weightwh*SIGNALFACTOR);
		for(unsigned int i = 0; i < NBKG; i++) 
		{
//			std::cout << "   + Background: " << bkg[i] << " - " << weight[i] << std::endl;
			if(fbkg[i])
			{
				hbkg[i]->Scale(weight[i]);
			}
			else 
			{
				izjets = i;
				for(unsigned int j = 0; j < NBKGZJETS; j++) 
				{
//					std::cout << "     - " << bkgzjets[j] << std::endl;
					hbkgzjets[j]->Scale(weightzjets[j]);
				}
			}
		}
	}
	else if(plottype==2) 
	{
//		std::cout << "   + Signal: " << sig << std::endl;
		hwh->Scale(1./hwh->Integral());
		for(unsigned int i = 0; i < NBKG; i++) 
		{
//			std::cout << "   + Background: " << bkg[i]<< std::endl;
			if(fbkg[i])
			{
				hbkg[i]->Scale(1./hbkg[i]->Integral());
			}
			else 
			{
				izjets = i;
				for(unsigned int j = 0; j < NBKGZJETS; j++) 
				{
//					std::cout << "     - " << bkgzjets[j] << std::endl;
					hbkgzjets[i]->Scale(1./hbkgzjets[j]->Integral());
				}
			}
		}
	}

	if(izjets != -1) 
	{
//		std::cout << "   + Adding Z+Jets histograms" << std::endl;
//		std::cout << "     - " << bkgzjets[0] << std::endl;
		hbkg[izjets] = (TH1D*) hbkgzjets[0]->Clone("ZJets");
		hbkg[izjets]->SetDirectory(0);
		for(unsigned int j = 1; j < NBKGZJETS; j++) 
		{
//			std::cout << "     - " << bkgzjets[j] << std::endl;
			hbkg[izjets]->Add(hbkgzjets[j]);
		}
	}

	
	//////
	// Rebin histograms
//	std::cout << ">> Rebinning histograms by a factor " << rebin << "..." << std::endl;
	hdata->Rebin(rebin);
	hwh->Rebin(rebin);
	for(unsigned int i = 0; i < NBKG; i++)
	{
		hbkg[i]->Rebin(rebin);
	}

	//////
	// Set colors and styles
	std::vector<int> colors;
	colors.push_back(kRed+3);
	colors.push_back(kOrange-6);
	colors.push_back(kOrange+3);
	colors.push_back(kRed+2); 
	colors.push_back(kOrange+5);
	colors.push_back(kCyan-2);
	colors.push_back(kRed);
	colors.push_back(kGreen-3);
	colors.push_back(kBlue+5);

//	std::cout << ">> Setting colors and styles..." << std::endl;
	hdata->SetMarkerStyle(20);
	hdata->SetLineColor(hdata->GetMarkerColor());
	gStyle->SetOptStat(0);
	gStyle->SetLegendBorderSize(0);

	hwh->SetFillColor(kOrange-2);
	hwh->SetLineColor(kOrange-1);
	if(plottype == 1)
	{
		hwh->SetFillStyle(3254);
	}
	for(unsigned int i = 0 ; i < NBKG ; ++i)
	{
		hbkg[i]->SetFillColor(colors[i]);
		hbkg[i]->SetLineColor(colors[i]+1);
	}

	//////
	//Drawing
	TCanvas* canvas = new TCanvas(sig, sig);
	//-- Ration histogram for log plots
	TH1F * ratio = new TH1F("ratio","",hdata->GetNbinsX(),
			hdata->GetXaxis()->GetXmin(),hdata->GetXaxis()->GetXmax());
	ratio->SetMarkerStyle(20);
	ratio->SetLineColor(hdata->GetMarkerColor());
	if(plottype == 0) 
	{
		// 1.- Stack
//		std::cout << ">> Stacking..." << std::endl;
		THStack* hs = new THStack("hs", histoname);
		TH1F * mcratio = (TH1F*)ratio->Clone("mcratio");
		for(unsigned int i = 0; i < NBKG; i++)
		{
			// stack
			hs->Add(hbkg[i]);
			// ratio histo
			mcratio->Add(hbkg[i]); 
		}
		hs->Add(hwh);
		mcratio->Add(hwh);
		
		//2.- Draw data
//		std::cout << ">> Drawing..." << std::endl;
		double hsmax = 1.1 * hs->GetMaximum();
		double hdmax = 1.1 * hdata->GetMaximum();
		hs->SetMaximum(TMath::Max(hsmax, hdmax));
		//   Create canvas
//		std::cout << "   + Stack" << std::endl;
		hs->Draw();
//		std::cout << "   + Data" << std::endl;
		hdata->Draw("E SAME");

		//3.- Set titles and axis
		hs->SetTitle(hdata->GetTitle());
		hs->GetXaxis()->SetTitle(hdata->GetTitle());
		hs->GetYaxis()->SetTitle("Events");///"+
//				TString(hs->GetXaxis()->GetBinWidth(1)));
		
		//4.- ratio plot
		ratio->Divide(hdata,mcratio);
		
	}
	else if(plottype == 1) 
	{
		// 1.- Stack
//		std::cout << ">> Stacking..." << std::endl;
		THStack* hs = new THStack("hs", histoname);
		TH1F * mcratio = (TH1F*)ratio->Clone("mcratio");
		for(unsigned int i = 0; i < NBKG; i++)
		{
			hs->Add(hbkg[i]);
			mcratio->Add(hbkg[i]); 
		}
		
		//2.- Draw signal and data
//		std::cout << ">> Drawing..." << std::endl;
		double hmax = 1.1 * hs->GetMaximum();
		hmax = TMath::Max(hmax, 1.1 * hdata->GetMaximum());
		hmax = TMath::Max(hmax, 1.1 * hwh->GetMaximum());
		hs->SetMaximum(hmax);
		//   Create canvas
	//	TCanvas* canvas = new TCanvas(sig, sig);
//		std::cout << "   + Stack" << std::endl;
		hs->Draw();
//		std::cout << "   + Signal" << std::endl;
		hwh->Draw("SAME");
//		std::cout << "   + Data" << std::endl;
		hdata->Draw("E SAME");

		//3.- Set titles and axis
		hs->SetTitle(hdata->GetTitle());
		hs->GetXaxis()->SetTitle(hdata->GetTitle());
		hs->GetYaxis()->SetTitle("Events");///"+
//				TString(hs->GetXaxis()->GetBinWidth(1)));
	
		//4.- ratio plot
		ratio->Divide(hdata,mcratio);
	}
	else if(plottype == 2) 
	{
//		std::cout << ">> Drawing..." << std::endl;
		double max=1.1*hwh->GetMaximum();
		for(unsigned int i = 0; i < NBKG; i++) 
		{
			max = TMath::Max(max,1.1*(hbkg[i]->GetMaximum()));
		}
		hbkg[0]->SetMaximum(max);
		hbkg[0]->Draw();
		for(unsigned int i = 1; i < NBKG; i++) 
		{
			if(hbkg[i]->Integral() > 0)
			{
				hbkg[i]->Draw("SAME");
			}
		}
		hwh->Draw("SAMES");
	}
	
	//////
	// Setting the "stats"
//	std::cout << ">> Printing stats..." << std::endl;
	TString datstats = Form("Data: %.0f", nevdat);
	int ndec = 0;
	if(nevbkg < 1000)
	{
		ndec = 1;
	}
	if(nevbkg < 10)
	{
		ndec = 2;    
	}
	TString lumstats = Form("Lumi: %.1f fb^{-1}",luminosity/1000);
	TString bkgstats = Form("Bkg.: %.*f",ndec, nevbkg);
	TString sigstats = Form("Sig.: %.4f", nevwh);
	//TPaveText* stats = new TPaveText(0.78,0.78,0.98,0.98, "NDC");
	TPaveText* stats = new TPaveText(0.72,0.78,0.92,0.98, "NDC");
	stats->SetTextAlign(12);
	stats->AddText(lumstats);
	stats->AddText(datstats);
	stats->AddText(bkgstats);
	stats->AddText(sigstats);
	stats->SetFillColor(10);
	stats->SetBorderSize(0);
	stats->Draw();
	//////
	// Setting the legend
//	std::cout << ">> Legend..." << std::endl;
	TString format;
	if(plottype == 0 || plottype == 1)
	{
		format="f";
	}
	else if (plottype == 2)
	{
		format="l";
	}
	
	//TLegend *legend = new TLegend(0.78,0.50,0.98,0.75);
	TLegend *legend = new TLegend(0.72,0.65,0.98,0.775);
	legend->SetNColumns(2);
	legend->AddEntry(hdata, dlegend, "p");
	TString llegend;
	llegend += SIGNALFACTOR;
	llegend += TString(" #times ") + slegend;
	legend->AddEntry(hwh,   llegend, format);
	for(unsigned int i = 0; i < NBKG; i++) 
	{
		legend->AddEntry(hbkg[i],blegend[i],format);
	}
	legend->SetFillColor(10);
	legend->Draw();
	
	gSystem->MakeDirectory("Plots");
	canvas->SaveAs("Plots/"+histoname+".eps");
	//canvas->SetLogy();
	if( plottype != 2 )
	{
		//=====  Ratio plot
		// Recovering the content of the canvas
		TH1D * _hdata = new TH1D();
		THStack * _hstack = new THStack();
		TIter next(canvas->GetListOfPrimitives());
		TObject * obj = 0;
		while( (obj = next()) )
		{
			if( obj->InheritsFrom(TH1D::Class()) )
			{
				_hdata = (TH1D*)obj;
			}
			else if( obj->InheritsFrom(THStack::Class()) )
			{
				_hstack = (THStack*)obj;
			}
		}
		if( _hdata == 0 || _hstack == 0 )
		{
			std::cerr << "\033[1;31mPlotAll ERROR\033[1;m Some inconsistency"
				<< " found. Trying to generate the ratio MC-data plot it"
				<< " is not instantiate the data and MC histograms. Exiting"
				<< std::endl;
			exit(-1);
		}

		//----- Up pad where the log plot goes in
		TPad * pad1 = new TPad("pad1_"+histoname,"pad1",0,0.3,1,1);
		//pad1->SetBottomMargin(-0.5);
		pad1->SetBottomMargin(0);
		pad1->SetLogy();
		pad1->Draw();
		pad1->cd();
		_hstack->Draw();
		_hdata->Draw("E SAME");
		
		canvas->cd();
		//---- Down pad where the ratio plot goes in
		TPad * pad2 = new TPad("pad2_"+histoname,"pad2",0,0,1,0.3);
		pad2->SetTopMargin(0);
		pad2->SetBottomMargin(0.3);
		pad2->Draw();
		pad2->cd();
		//---- Ratio histogram
		ratio->SetXTitle(_hdata->GetTitle());
		ratio->GetXaxis()->SetTitleSize(0.08);
		ratio->GetXaxis()->SetLabelSize(0.08);
		ratio->GetYaxis()->SetNdivisions(205);
		ratio->GetYaxis()->SetTitle("N_{data}/N_{MC}");
		ratio->GetYaxis()->SetTitleSize(0.08);
		ratio->GetYaxis()->SetTitleOffset(0.4);
		ratio->GetYaxis()->SetLabelSize(0.08);
		ratio->Draw("E");
		TLine * line =  new TLine(0,1,ratio->GetXaxis()->GetXmax(),1);
		line->SetLineColor(46);
		line->SetLineStyle(8);
		line->Draw("SAME");

		canvas->cd();
		legend->SetY1(legend->GetY1()+0.2);
		legend->SetY2(legend->GetY2()+0.2);
		legend->Draw();
		stats->SetY1(stats->GetY1()+0.2);
		stats->SetY2(stats->GetY2()+0.2);
		stats->Draw();
	}

	canvas->SaveAs("Plots/"+histoname+"_log.eps");
	canvas->SetLogy(0);
	
	//////
	// Close files
	fdata->Close();
	fwh->Close();
	for(unsigned int i = 0; i < NBKG ; i++) 
	{
		if(fbkg[i])
		{
			fbkg[i]->Close();
		}
		else 
		{ 
			//ZJets
			for (unsigned int j = 0; j < NBKGZJETS; j++) 
			{
				fbkgzjets[j]->Close();
			}
		}
	}
}

void display_usage()
{
	std::cout << "\033[1;37musage:\033[1;m plothistos histoname [options]" << std::endl;
	std::cout << "" << std::endl;
	std::cout << "Options:" << std::endl;
	std::cout << "    -s <WZ|WH#>       Signal name, for Higgs subsituing # for mass" << std::endl;
	std::cout << "    -r N              Rebin the histos a factor N [default: 0]" << std::endl;
	std::cout << "    -p <0|1|2>        Plot type. Possible options are:" << std::endl;
	std::cout << "                      - 0: All backgrounds and signal stacked" << std::endl;
	std::cout << "                      - 1: All backgrounds stacked, signal alone [default]" << std::endl;
	std::cout << "                      - 2: No stacking at all" << std::endl;
	std::cout << "    -l L              Luminosity in pb^-1 [default: 2143.3 pb^-1]" << std::endl;
	std::cout << "    -h                displays this help message and exits " << std::endl;
	std::cout << "" << std::endl;
}


int main(int argc, char *argv[])
{
	const char * signal    = "WHToWW2L120";
	const char * rebin     = "1";
	const char * plottype  = "0";
	const char * luminosity= "2143.3";
	const char * histoname = 0;

	// Arguments used
	std::set<int> usedargs;
	//Parsing input options
	if(argc == 1)
	{
		display_usage();
		return -1;
	}
	else
	{
		//Argument 1 must be a valid input fileName
		//dataName = argv[1];
		for(int i = 1; i < argc; i++) 
		{
			if( strcmp(argv[i],"-h") == 0 )
			{
				display_usage();
				return 0;
			}
			
			if( strcmp(argv[i],"-r") == 0 )
			{
				rebin = argv[i+1];
				usedargs.insert(i);
				usedargs.insert(i+1);
				i++;
			}
			if( strcmp(argv[i],"-p") == 0 )
			{
				plottype = argv[i+1];
				usedargs.insert(i);
				usedargs.insert(i+1);
				i++;
			}
			if( strcmp(argv[i],"-l") == 0 )
			{
				luminosity = argv[i+1];
				usedargs.insert(i);
				usedargs.insert(i+1);
				i++;
			}
			if( strcmp(argv[i],"-s") == 0 )
			{
				signal = argv[i+1];
				usedargs.insert(i);
				usedargs.insert(i+1);
				i++;
			}
		}
	}
	// Extracting the signal name
        for(int i=1; i < argc; i++)
	{
		if(usedargs.find(i) == usedargs.end())
		{
			histoname = argv[i];
			break;
		}
	}
	
	if( histoname == 0 )
	{
		std::cerr << "\033[1;31mplothistos ERROR:\033[1;m The 'histoname' argument is mandatory!"
			<< std::endl;
		display_usage();
		return -1;
	}
	
	// Some constants
	//
	// + The following values are used to construct the name of the files 
	//   to look for the histograms
	const TString data      = "Data";
	const TString sig       = signal;
	std::vector<TString> bkg;
	//bkg.push_back("WZ");
	bkg.push_back("ZZ");
	bkg.push_back("WW");
	bkg.push_back("TTbar_Madgraph");
	bkg.push_back("ZJets_Powheg");
	bkg.push_back("WJets_Madgraph");

	std::vector<TString> bkgzjets;
	bkgzjets.push_back("DYee_Powheg");
	bkgzjets.push_back("DYmumu_Powheg");
	bkgzjets.push_back("DYtautau_Powheg"); 
	bkgzjets.push_back("Zee_Powheg");
	bkgzjets.push_back("Zmumu_Powheg");
	bkgzjets.push_back("Ztautau_Powheg");
	
	const TString dlegend   = "Data";
	const TString slegend   = signal;//"WH @ 120";
	std::vector<TString> blegend; 
	//blegend.push_back("WZ");
	blegend.push_back("ZZ");
	blegend.push_back("WW");
	blegend.push_back("t#bar{t} (MG)");
	blegend.push_back("Z+Jets (incl. Zbb)");
	blegend.push_back("W+Jets (MG)");
	
	// + Value to scale the signal
	int SIGNALFACTOR=1;
	
	if( strcmp(sig,"WZ") != 0 )
	{
		bkg.push_back("WZ");
		blegend.push_back("WZ");
		bkg.push_back("TW_DR");
		blegend.push_back("tW");
		bkg.push_back("TbarW_DR");
		blegend.push_back("tbarW");
		SIGNALFACTOR = 100;
	}

	// + Number of backgrounds
	//const unsigned int NBKG=bkg.size();
	//const unsigned int NBKGZJETS=bkgzjets.size();

	common cd;
	cd.SIGNALFACTOR = SIGNALFACTOR;
	cd.data = data;
	cd.sig  = sig;
	cd.dlegend = dlegend;
	cd.slegend = slegend;
	cd.bkg = bkg;
	cd.bkgzjets = bkgzjets;
	cd.blegend = blegend;

	int rebinI = -1;
	std::stringstream ss(rebin);
	ss >> rebinI;

	int plottypeI = -1;
	std::stringstream ss1(plottype);
	ss1 >> plottypeI;
	
	double lumi = -1;
	std::stringstream ss2(luminosity);
	ss2 >> lumi;


	PlotAll(cd,histoname,rebinI,plottypeI,lumi);	
}
