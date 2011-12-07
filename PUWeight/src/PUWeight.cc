///////////////////////////////////////////////////////////////////////
//
//    FILE: PUWeight.cc   $CVS_TAG: V3_2_1
//   CLASS: PUWeight
// AUTHORS: I. Gonzalez Caballero
//    DATE: 09/03/2011
//
///////////////////////////////////////////////////////////////////////
#include "PUWeight.h"

// ROOT Includes
#include "TFile.h"
#include "TCanvas.h"
#include "TString.h"

// C++ includes
#include <iostream>
using namespace std;


//Set DEBUGPUWEIGHT to 1 to get some debug information. Set it to 2 for more
//detail debug information.
//#define DEBUGPUWEIGHT 2

#ifdef DEBUG
#define DEBUGPUWEIGHT 1
#endif


PUWeight::PUWeight(float luminosity, EMCDistribution mcdistr, const char* year):
  fData(0),
  fMC(0),
  fWeight(0) {

  //Load Data histogram
  if (!LoadDataHistogram(luminosity, year))
    return;

  //No MC given. Take ideal MC
  IdealMCHistogram(mcdistr);


  //Calculate Weight
  CalculateWeight();
}


PUWeight::PUWeight(const char* mcfolder, const char* mcproccess, 
		   float luminosity, const char* year):
  fData(0),
  fMC(0),
  fWeight(0) {

  //Load Data histogram
  if (!LoadDataHistogram(luminosity, year))
    return;
  
  //Load MC Histogram
  if (!LoadMCHistogram(mcfolder, mcproccess))
    return;



  //Calculate Weight
  CalculateWeight();
}




TH1F* PUWeight::LoadMCHistogram(const char* mcfolder, const char* mcproccess) {
#ifdef DEBUGPUWEIGHT
  cout << ">> Getting pileup for the MC " << mcproccess 
       << " inside " << mcfolder << "..." << endl;
#endif
  
  TString dsfile;
  dsfile.Form("http://www.hep.uniovi.es/jfernan/PUhistos/%s/%s.root", 
	      mcfolder, mcproccess);
#if (DEBUGPUWEIGHT > 1)
  cout << "   + Opening " << dsfile << endl;
#endif
  
  TFile* fds = TFile::Open(dsfile);
  if (!fds) {
    cerr << "ERROR [PUWeight]: Could not open file " << dsfile << "!"  << endl
	 << "                  Revise dataset name (" << mcproccess 
	 << ") or internet connection" << endl;
    return 0;
  }
  
  //Read dataset histogram...
#if (DEBUGPUWEIGHT > 1)
  cout << "   + Looking for histogram..." << endl;
#endif
  
  fMC = (TH1F*) fds->Get("htemp")->Clone("PU_MC");
  if (!fMC) {
    cerr << "ERROR [PUWeight]: Could not find histogram for dataset " << mcproccess << "!"
	 << endl;
    return 0;
  }
  fMC->SetDirectory(0);

  if (fMC->Integral() != 1) {
    cout << "NOTE [PUWeight]: MC histogram is not normalized to 1! Normalizing..."
	 << endl;
    fMC->Scale(1./fMC->Integral());
  }

  fds->Close();
  return fMC;
  
}


TH1F* PUWeight::LoadDataHistogram(float luminosity, const char* year) {

#ifdef DEBUGPUWEIGHT
  cout << ">> Getting pileup for the " << luminosity << " pb-1 of data..." 
       << endl;
#endif
  
  TString dtfile;
  TFile* fdt = 0;
  if (luminosity > 0) {
    dtfile.Form("http://www.hep.uniovi.es/jfernan/PUhistos/Data%s/PUdata_%.1f.root", 
		year, luminosity);

  
#if (DEBUGPUWEIGHT > 1)
    cout << "   + Opening " << dtfile << endl;
#endif

    fdt = TFile::Open(dtfile);
    if (!fdt) {
      cerr << "NOTE [PUWeight]: Could not find file " << dtfile << "!"  << endl;
      cerr << "                 Trying default PU profile for data..." << endl;
    }
  }

  if (!fdt) {
    dtfile.Form("http://www.hep.uniovi.es/jfernan/PUhistos/Data%s/PUdata.root",
		    year);

#if (DEBUGPUWEIGHT > 1)
    cout << "   + Opening " << dtfile << endl;
#endif

    fdt = TFile::Open(dtfile);
    if (!fdt) {
      cerr << "ERROR [PUWeight]: Could not find default profile in \"" 
	   << dtfile << "\"!"  << endl
	   << "                  Is your internet connection working?" << endl;
      return 0;
    }
  }
  
  //Read data histogram...
  fData = (TH1F*) fdt->Get("pileup")->Clone("PU_Data");
  if (!fData) {
    cerr << "ERROR [PUWeight]: Could not find histogram for data!" << endl;
    return 0;
  }
  
  fData->SetDirectory(0);
  
  if (fData->Integral() != 1) {
    cout << "NOTE [PUWeight]: Data histogram is not normalized to 1! Normalizing..."
	 << endl;
    fData->Scale(1./fData->Integral());
  }

  fdt->Close();

  return fData;
}


TH1F* PUWeight::CalculateWeight() {
#ifdef DEBUGPUWEIGHT
  cout << ">> Calculating weights..." 
       << endl;
#endif
  if (fData && fMC) {
    unsigned int nbins = fData->GetXaxis()->GetNbins();
    float xmin  = fData->GetXaxis()->GetXmin();
    float xmax  = fData->GetXaxis()->GetXmax();
    fWeight = new TH1F("PUWeight", "PU Weight", nbins, xmin, xmax);
    fWeight->SetDirectory(0);
    fWeight->Divide(fData, fMC);
  }
  else {
    cerr << "ERROR [PUWeight]: Something weird happened when trying to calculate the weights."
	 << endl 
	 << "                  I could not find the data and/or mc histograms!"
	 << endl;
  }

#ifdef DEBUGPUWEIGHT
  cout << ">> Done weight calculation..." << endl;
#endif
  return fWeight;
}

TH1F* PUWeight::IdealMCHistogram(EMCDistribution mcdistr) {
#ifdef DEBUGPUWEIGHT
  cout << ">> Building ideal MC profile..." 
       << endl;
#endif
  unsigned int ndbins = 25;
  float xmin = -0.5;
  float xmax = 24.5;

  if (fData) {
    ndbins = fData->GetXaxis()->GetNbins();
    xmin   = fData->GetXaxis()->GetXmin();
    xmax   = fData->GetXaxis()->GetXmax();
  }


#if (DEBUGPUWEIGHT > 1)
  cout << " + Data histogram has " << ndbins << " bins - [ " 
       << xmin << " - " << xmax << " ]" << endl;
#endif

  fMC = new TH1F("PU_MC", "PU^{MC} Weight", ndbins, xmin, xmax);
  double bins[50] = {
    0, 1, 2, 3, 4, 5, 6, 7, 8, 9,
    10,11,12,13,14,15,16,17,18,19,
    20,21,22,23,24,25,26,27,28,29,
    30,31,32,33,34,35,36,37,38,39,
    40,41,42,43,44,45,46,47,48,49
  };

  unsigned int nbins = 25;
  
  if (mcdistr == Spring11) {
    double idealpu[] = {0.0698146584, 0.0698146584, 0.0698146584, 
		       0.0698146584, 0.0698146584, 0.0698146584,
		       0.0698146584, 0.0698146584, 0.0698146584,
		       0.0698146584, 0.0698146584, 0.0630151648,
		       0.0526654164, 0.0402754482, 0.0292988928,
		       0.0194384503, 0.0122016783, 0.007207042,
		       0.004003637,  0.0020278322, 0.0010739954,
		       0.0004595759, 0.0002229748, 0.0001028162,
		       4.58337152809607E-05};
    fMC->FillN(nbins, bins, idealpu);
  }
  else if (mcdistr == Summer11) {
    double idealpu[] = {
      0.104109,
      0.0703573,
      0.0698445,
      0.0698254,
      0.0697054,
      0.0697907,
      0.0696751,
      0.0694486,
      0.0680332,
      0.0651044,
      0.0598036,
      0.0527395,
      0.0439513,
      0.0352202,
      0.0266714,
      0.019411,
      0.0133974,
      0.00898536,
      0.0057516,
      0.00351493,
      0.00212087,
      0.00122891,
      0.00070592,
      0.000384744,
      0.000219377     
    };
    fMC->FillN(nbins, bins, idealpu);
  }
  else if (mcdistr == Summer11InTime) {
    double idealpu[] = {
      0.14551,
      0.0644453,
      0.0696412,
      0.0700311,
      0.0694257,
      0.0685655,
      0.0670929,
      0.0646049,
      0.0609383,
      0.0564597,
      0.0508014,
      0.0445226,
      0.0378796,
      0.0314746,
      0.0254139,
      0.0200091,
      0.0154191,
      0.0116242,
      0.00846857,
      0.00614328,
      0.00426355,
      0.00300632,
      0.00203485,
      0.00133045,
      0.000893794
    };
    fMC->FillN(nbins, bins, idealpu);
  }
  else if (mcdistr == Summer11ITSmear) {
    nbins = 35;
    double idealpu[] = {
      1.45346E-01,
      6.42802E-02,
      6.95255E-02,
      6.96747E-02,
      6.92955E-02,
      6.84997E-02,
      6.69528E-02,
      6.45515E-02,
      6.09865E-02,
      5.63323E-02,
      5.07322E-02,
      4.44681E-02,
      3.79205E-02,
      3.15131E-02,
      2.54220E-02,
      2.00184E-02,
      1.53776E-02,
      1.15387E-02,
      8.47608E-03,
      6.08715E-03,
      4.28255E-03,
      2.97185E-03,
      2.01918E-03,
      1.34490E-03,
      8.81587E-04,
      5.69954E-04,
      3.61493E-04,
      2.28692E-04,
      1.40791E-04,
      8.44606E-05,
      5.10204E-05,
      3.07802E-05,
      1.81401E-05,
      1.00201E-05,
      5.80004E-06
    };
    fMC->FillN(nbins, bins, idealpu);
  }
  else if (mcdistr == Summer11True) {
    double idealpu[] = {
      0.104109,
      0.0703573,
      0.0698445,
      0.0698254,
      0.0697054,
      0.0697907,
      0.0696751,
      0.0694486,
      0.0680332,
      0.0651044,
      0.0598036,
      0.0527395,
      0.0439513,
      0.0352202,
      0.0266714,
      0.019411,
      0.0133974,
      0.00898536,
      0.0057516,
      0.00351493,
      0.00212087,
      0.00122891,
      0.00070592,
      0.000384744,
      0.000219377 
    };
    fMC->FillN(nbins, bins, idealpu);
  }
  else if (mcdistr == Fall11) {
    nbins = 50;
    double idealpu[] = {
      0.003388501,
      0.010357558,
      0.024724258,
      0.042348605,
      0.058279812,
      0.068851751,
      0.072914824,
      0.071579609,
      0.066811668,
      0.060672356,
      0.054528356,
      0.04919354,
      0.044886042,
      0.041341896,
      0.0384679,
      0.035871463,
      0.03341952,
      0.030915649,
      0.028395374,
      0.025798107,
      0.023237445,
      0.020602754,
      0.0180688,
      0.015559693,
      0.013211063,
      0.010964293,
      0.008920993,
      0.007080504,
      0.005499239,
      0.004187022,
      0.003096474,
      0.002237361,
      0.001566428,
      0.001074149,
      0.000721755,
      0.000470838,
      0.00030268,
      0.000184665,
      0.000112883,
      6.74043E-05,
      3.82178E-05,
      2.22847E-05,
      1.20933E-05,
      6.96173E-06,
      3.4689E-06,
      1.96172E-06,
      8.49283E-07,
      5.02393E-07,
      2.15311E-07,
      9.56938E-08     
    };
    fMC->FillN(nbins, bins, idealpu);
  }

  else if (mcdistr == Fall11True){
     nbins =50;
     double idealpu[]= {

	0.00875488,
	0.0188779,
	0.0314413,
	0.0439477,
	0.053954,
	0.060907,
	0.0629538,
	0.0614203,
	0.0601845,
	0.0573943,
	0.0528803,
	0.0490987,
	0.0453003,
	0.041755,
	0.0387835,
	0.0356998,
	0.0329536,
	0.0305912,
	0.0279545,
	0.025328,
	0.0229131,
	0.0205869,
	0.0182821,
	0.0160806,
	0.0140798,
	0.0121316,
	0.0103717,
	0.00875817,
	0.00729464,
	0.00606236,
	0.00501249,
	0.00404522,
	0.00322428,
	0.00257927,
	0.0020242,
	0.00159807,
	0.001214,
	0.000921139,
	0.000675573,
	0.000528562,
	0.00039809,
	0.000290443,
	0.000210844,
	0.00014498,
	0.000113643,
	8.07592e-05,
	6.04485e-05,
	4.23623e-05,
	2.74678e-05,
	1.73125e-05
//	1.39273e-05
    };
    fMC->FillN(nbins, bins, idealpu);
  }


  return fMC;
}
