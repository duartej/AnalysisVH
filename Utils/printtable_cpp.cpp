//  g++ -I`root-config --incdir` -I$VHSYS -L`root-config --libdir` -L$VHSYS/libs `root-config --libs` -lTResultsTable -lInputParameters -o printtable PrintTable.C

#include "CutLevels/interface/CutLevels.h"

#include "TResultsTable/interface/TResultsTable.h"
#include "InputParameters/interface/InputParameters.h"

#include "TFile.h"
#include "TString.h"
#include "TMath.h"
#include "TH1D.h"
#include "TSystem.h"

#include<iostream>
#include<sstream>
#include<vector>
#include<set>

using namespace std;

//#define NBKG 13
#define NSIG 9

// Constant number of total cuts and string with names
unsigned int _iNCuts = -1;
TString * kCutNames = 0;

//////////
// Set the constant value to be shared with some functions
void setcuts(const char * signal)
{
	if( strncmp(signal,"WH",2) == 0 )
	{
		_iNCuts = WHCuts::_iNCuts;
		kCutNames = new TString [_iNCuts+1];
		for(unsigned int i = 0; i < _iNCuts; ++i)
		{
			kCutNames[i] = WHCuts::kCutNames[i];
		}
	}
	else if( strcmp(signal,"WZ") == 0 )
	{
		_iNCuts = WZCuts::_iNCuts;
		kCutNames = new TString [_iNCuts+1];
		for(unsigned int i = 0; i < _iNCuts; ++i)
		{
			kCutNames[i] = WZCuts::kCutNames[i];
		}
	}
	else
	{
		std::cerr << "\033[1;31msetcuts ERROR\033[1;m Signal '" << signal
			<< "' not implemented. Exiting..." << std::endl;
		exit(-1);
	}
}

TString GetFile(TString d) 
{
	TString f;
	f.Form("cluster_%s/Results/%s.root",d.Data(),d.Data());
	return f;
}

////////////////
// Round values over a given limit
double Round(double val, double limit = 1E3) 
{
	if (val > limit) 
	{
		return 1.0* TMath::Nint(val);
	}
	return val;
}

double * GetEvents(TString sample, const double & luminosity, bool showvalues=false) 
{
	//Open file
	std::cout << "   + Opening file for " << sample << "..." << std::endl;
	TFile* file = TFile::Open(GetFile(sample));

	if( file->IsZombie() )
	{
		std::cerr << "\033[1;31mGetEvents ERROR\033[1;m File '" << sample 
			<< "' does not exist. Exiting..." << std::endl;
		exit(-1);
	}
	
	//Get Histogram
	std::cout << "   + Get cuts histogram..." << std::endl;
	TString cname = sample + "_Clone";
	TString histoname("fHEventsPerCut");
	TH1D* histo = (TH1D*)((TH1D*)(file->Get(histoname)))->Clone(cname);
	if( histo == 0 )
	{
		std::cerr << "\033[1;31mGetEvents ERROR\033[1;m File '" << sample
			<< "' have not a recognizable format (histo 'fHEventsPerCut' isn't there)"
			<< std::endl;
		exit(-1);
	}
	histo->SetDirectory(0);
	
	//Loading input parameters
	double weight=0;
	if(sample.Contains("Data"))
	{
		weight = 1;
	}
	else 
	{
		std::cout << "   + Getting Input Parameters..." << std::endl;
		gSystem->Load("libInputParameters.so");
		double xs = 0;
		int    neventssample = 0; //N.Evs. in the whole sample
		int    neventsskim = 0;   //N.Evs. after skimming
		InputParameters* ip = (InputParameters*) file->Get("Set Of Parameters");
		ip->TheNamedDouble("CrossSection", xs);
		ip->TheNamedInt("NEventsSample", neventssample);
		ip->TheNamedInt("NEventsTotal", neventsskim);
		weight = xs * luminosity / neventssample;
		std::cout << "     - XS=" << xs 
			<< " NEv. Sample=" << neventssample 
			<< " NEv. Skim=" << neventsskim
			<< " Weight = " << weight << endl;
	}
	
	//Storing values
	std::cout << "   + Storing values..." << std::endl;
	double* values = new double[_iNCuts];
	for(unsigned int i = 0; i < _iNCuts; i++) 
	{
		values[i] = weight * histo->GetBinContent(i+1);
		if(showvalues)
		{
			std::cout << "     - " << kCutNames[i] << ": " << values[i] << std::endl;
		}
	}
	
	//Close file
	std::cout << "   + Closing file..." << std::endl;
	file->Close();
	
	return values;
}

void PrintTableSignals(const double & lumi) 
{
	double luminosity = lumi;
	std::cout << ">> Computing for " << luminosity << " pb-1 luminosity" << std::endl; 

	std::cout << ">> Getting events..." << std::endl;
	double* nevents[NSIG];
	for(unsigned int i = 0; i < NSIG; i++) 
	{
		int hmass = 120 + 10*i;
		TString sig = TString("WHToWW2L") + Form("%i",hmass);
		nevents[i] = GetEvents(sig,luminosity);
	}
	
	std::cout << ">> Loading macro TResultsTable.C..." << std::endl;
	gSystem->Load("libTResultsTable.so");
	
	std::cout << ">> Making table..." << std::endl;
	TResultsTable t(_iNCuts,NSIG);
	
	// + Header 
	t.SetRowTitleHeader("Cuts");
	for(unsigned int j = 0; j < NSIG; j++) 
	{
		int hmass = 120 + 10*j;
		TString blegend = TString("WH @ ") + Form("%i",hmass);
		t.SetColumnTitle(j,blegend);
	}
	
	// + Results for each cut
	for(unsigned int i = 0; i < _iNCuts; i++) 
	{
		t.SetRowTitle(i, kCutNames[i]);
		double totalbkg = 0;
		for(unsigned int j = 0; j < NSIG; j++) 
		{
			t[i][j] = Round(nevents[j][i]);
			totalbkg+=nevents[j][i];
		}
	}  
	
	std::cout << ">> Printing table..." << std::endl;
	//t.Print();
	t.Print(kHTML);
	t.Print(kLaTeX);
}


void PrintTableBackgrounds(const char * signal,const char * filename, const double & lumi) 
{
	const TString data = "Data";
	TString sig        = signal;
	TString hmassstr   = signal;
	int hmass          = -1;
	TString slegend    = signal;
	TString dlegend    = "Data";
	if( sig.Contains("WH") )
	{
		sig.Replace(0,2,"WHToWW2L");
		hmassstr.Replace(0,2,"");
		std::stringstream ss(hmassstr.Data());
		ss >> hmass;
		slegend = TString("WH @ ") + hmassstr;
	}

	std::vector<TString> bkg;
	bkg.push_back("ZZ");
	bkg.push_back("WW");
	bkg.push_back("TTbar_Madgraph");
	//bkg.push_back("ZJets_Powheg");
	bkg.push_back("WJets_Madgraph");
	bkg.push_back("DYee_Powheg");
	bkg.push_back("DYmumu_Powheg");
	bkg.push_back("DYtautau_Powheg"); 
	bkg.push_back("Zee_Powheg");
	bkg.push_back("Zmumu_Powheg");
	bkg.push_back("Ztautau_Powheg");
	
	std::vector<TString> blegend; 
	blegend.push_back("ZZ");
	blegend.push_back("WW");
	blegend.push_back("t#bar{t} (MG)");
	//blegend.push_back("Z+Jets (incl. Zbb)");
	blegend.push_back("W+Jets (MG)");
	blegend.push_back("DY ee");
	blegend.push_back("DY mumu");
	blegend.push_back("DY tautau");
	blegend.push_back("DYee > 20");
	blegend.push_back("DYmm > 20");
	blegend.push_back("DYtt > 20");
	
	if( strcmp(signal,"WZ") != 0 )
	{
		bkg.push_back("WZ");
		blegend.push_back("WZ");
		bkg.push_back("TW_DR");
		blegend.push_back("tW");
		bkg.push_back("TbarW_DR");
		blegend.push_back("tbarW");
	}

  	std::cout << ">> Computing for " << lumi << " pb-1 luminosity" << std::endl; 

	const unsigned int NBKG = bkg.size();
	// Get Events
	std::cout << ">> Getting events..." << std::endl;
	double* neventsd = GetEvents(data,lumi); //Data
	double* neventss = GetEvents(sig,lumi);  //Signal
	std::vector<double *> neventsb;     //Background
	for (unsigned int i = 0; i < NBKG; i++)
	{
		neventsb.push_back( GetEvents(bkg[i],lumi) );
	}

	// Text
	//std::cout << ">> Loading macro TResultsTable.C..." << std::endl;
	//gSystem->Load("libTResultsTable.so");
	
	std::cout << ">> Making table..." << std::endl;
	TResultsTable t(_iNCuts,NBKG+3);
	
	// + Header 
	t.SetRowTitleHeader("Cuts");
	for (unsigned int j = 0; j < NBKG; j++)
	{
		t.SetColumnTitle(j,blegend[j]);
	}
	t.SetColumnTitle(NBKG, "TotBkg");
	t.SetColumnTitle(NBKG + 1, dlegend);
	t.SetColumnTitle(NBKG + 2, slegend);
	
	// + Results for each cut
	for(unsigned int i = 0; i < _iNCuts; i++) 
	{
		t.SetRowTitle(i, kCutNames[i]);
		double totalbkg = 0;
		for (unsigned int j = 0; j < NBKG; j++) 
		{     
			t[i][j] = Round(neventsb[j][i]);		
			totalbkg+=neventsb[j][i];
		}
		t[i][NBKG]   = Round(totalbkg); 
		t[i][NBKG+1] = neventsd[i];
		t[i][NBKG+2] = neventss[i];
	}  
	std::cout << ">> Printing table..." << std::endl;
	t.SaveAs(filename);
}


//void PrintTable(const char* sample, const double & luminosity) 
//{
//	GetEvents(sample, luminosity,true);
//}

void PrintTable(const double &  lumi = -1) 
{
	PrintTableSignals(lumi);
}

void PrintTable(const char * signal,const char * format, const double & lumi=-1) 
{
	PrintTableBackgrounds(signal,format, lumi);
}

void display_usage()
{
	std::cout << "\033[1;37musage:\033[1;m printtable signal [options]" << std::endl;
	std::cout << "" << std::endl;
	std::cout << "Options:" << std::endl;
	std::cout << "    -l L              Luminosity in pb^-1 [default: 2143.3 pb^-1]" << std::endl;
	std::cout << "    -f                Output table filename [default: table.tex]" << std::endl;
	std::cout << "    -h                displays this help message and exits " << std::endl;
	std::cout << std::endl;
	std::cout << "Valid 'signal' argument: WZ WH# (# stands for mass hypothesis)" << std::endl;
	std::cout << "" << std::endl;
}

int main(int argc, char *argv[])
{
	const char * signal    = 0;
	const char * filename  = "table.tex";
	const char * luminosity= 0; //"2143.3";

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
			
			if( strcmp(argv[i],"-f") == 0 )
			{
				filename = argv[i+1];
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
		}
	}
	// Extracting the signal name
        for(int i=1; i < argc; i++)
	{
		if(usedargs.find(i) == usedargs.end())
		{
			signal = argv[i];
			break;
		}
	}
	
	if( signal == 0 )
	{
		std::cerr << "\033[1;31mprinttable ERROR:\033[1;m The 'signal' argument is mandatory!"
			<< std::endl;
		display_usage();
		return -1;
	}
	
	
	double lumi = -1;
	std::stringstream ss2(luminosity);
	ss2 >> lumi;
	
	// Set the cuts
	setcuts(signal);

	PrintTable(signal,filename,lumi);	

	return 0;
}
