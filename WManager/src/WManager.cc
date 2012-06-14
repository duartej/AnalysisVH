#include "WManager.h"

#include<iostream>
#include<sstream>
#include<cstdlib>
#include<cstring>
#include<cmath>

#include "TFile.h"
#include "TH2F.h"
#include "TKey.h"
#include "TClass.h"
#include "TROOT.h"


WManager::WManager(const unsigned int & weighttype, const bool & isSystematics ) :
	_wtype(weighttype)
{
	_weights[MUON] = 0;
	_weights[ELECTRON] = 0;


	std::string mufile( this->getfile(MUON,isSystematics) );
	std::string elecfile( this->getfile(ELECTRON,isSystematics) ); 

	this->setweightfile(MUON,mufile.c_str());
	this->setweightfile(ELECTRON,elecfile.c_str());
}

WManager::~WManager()
{
	for(std::map<LeptonTypes,TH2F*>::iterator it = _weights.begin(); it != _weights.end(); ++it)
	{
		if( it->second != 0 )
		{
			delete it->second;
			it->second = 0;
		}
	}	
}

std::string WManager::getfile(const unsigned int & lepton, const bool & isSystematics)
{
	char * pkgpath = 0;
	pkgpath = getenv("VHSYS");
	if( pkgpath == 0 )
	{
		std::cerr << "\033[1;31mWManager::getfile ERROR\033[1;m Not set the 'VHSYS'"
			<< " environment varible; this is an inconsistency in the code"
			<< std::endl;
		exit(-1);
	}
	
	std::string thefile(std::string(pkgpath)+"/WManager/data/");

	if( _wtype == WManager::SF )
	{
		if( lepton == MUON )
		{
			thefile  += "Muon_OutputScaleFactorMap_MC42X_2011_ALL_Reweighted.root";
		}
		else if( lepton == ELECTRON )
		{
			thefile  += "Electron_OutputScaleFactorMap_MC42X_2011_ALL_Reweighted.root";
		}
	}
	else if( _wtype == WManager::FR )
	{
		if( lepton == MUON )
		{
			if( isSystematics )
			{
				thefile  += "MuFR_All2011_LPcuts_AND_kink_jet30.root";
			}
			else
			{
				thefile  += "MuFR_all2011_jet50.root";
				//thefile  += "MuFR_All2011_LPcuts_AND_kink_jet30.root"; ZJets
			}
		}
		else if( lepton == ELECTRON )
		{
			if( isSystematics )
			{
				thefile  += "ElecFR_all2011_jet15.root";
				//thefile  += "MuFR_All2011_LPcuts_AND_kink_jet35.root"; 
			}
			else
			{
				thefile  += "ElecFR_all2011_jet50.root";
				//thefile  += "MuFR_All2011_LPcuts_AND_kink_jet35.root"; ZJets

			}
		}
	}
	else
	{
		std::cerr << "\033[1;31mWManager::getfile ERROR\033[1;m Not implemented "
			<< "'" << this->GetWTStr(_wtype) << "' weight type." 
			<< std::endl;
		exit(-1);
	}

	return thefile;
}


void WManager::setweightfile(const LeptonTypes & leptontype, const char * filename)
{
	// Forcing the ownership to WManager instead of TFile,
	// in order not to enter in conflict with the TreeManager TFile
	TH2::AddDirectory(kFALSE);
	
	TFile *f = new TFile(filename);
	if( f->IsZombie() )
	{
		std::cerr << "\033[1;31mWManager::SetWeightFile ERROR\033[1;m Not found the filename '"
			<< filename << "'" << std::endl;
		exit(-1);
	}

	TKey * key;
	TIter nextkey(f->GetListOfKeys());
	while( (key=(TKey*)nextkey()) )
	{
		const char * classname = key->GetClassName();
		TClass * cl = gROOT->GetClass(classname);
		if(cl->InheritsFrom(TH2::Class()))
		{
			std::stringstream ss;
			ss << leptontype;
			std::string name(ss.str()+"_w");
			// Check if it was already filled: if yes, remove it
			// to include the last one
			if( _weights[leptontype] != 0 )
			{
				delete _weights[leptontype];
				_weights[leptontype] = 0;
			}
			_weights[leptontype] = (TH2F*)((TH2F*)f->Get(key->GetName()))->Clone(name.c_str());
		}
	}
	delete key;

	if( _weights[leptontype] == 0)
	{
		std::cerr << "\033[1;31mWManager::SetWeightFile ERROR\033[1;m Not found the TH2F with the"
			<< " rates. File '" << filename << "' is not well defined" << std::endl;
		exit(-1);
	}

	// Some arrangements in order to avoid the calculation inside the loop
	// for the fake rate case the weight per event is f/(1-f)
	if( _wtype == WManager::FR )
	{
		TH2F * prov = (TH2F*)_weights[leptontype]->Clone("prov");
		// Get the Overflow bin in pt in order to assign any lepton
		// higher than the maximum (50)
		for(int i = 1; i <= prov->GetNbinsX()+1; ++i)
		{
			// WARNING: FAKE RATE Matrix for  pt > 35 is not reliable
			//          Following Alicia advice take the values of bin [30,35]
			double ptIn = prov->GetXaxis()->GetBinCenter(i);
			const double ptOut = ptIn;
			if( ptIn >= 35.0 )
			{
				ptIn = 34.;
			}
			
			// Same as pt (w.r.t. the overflow) for the eta case 
			for(int j = 1; j <= prov->GetNbinsY()+1; ++j)
			{
				const double eta = prov->GetYaxis()->GetBinCenter(j);
				const int globalbin = prov->FindBin(ptIn,eta);
				const double f = prov->GetBinContent(globalbin);
				double finalweight = f/(1.0-f);
				const int globalbinOut = prov->FindBin(ptOut,eta);
				_weights[leptontype]->SetBinContent(globalbinOut,finalweight);
			}
		}
		if(prov != 0)
		{
			delete prov;
			prov = 0;
		}
	}
	
	f->Close();
	delete f;
}


const double WManager::GetWeight(const LeptonTypes & lt, const double & pt, const double & eta)
{
	if( _weights[lt] == 0 )
	{
		std::cerr << "\033[1;31mWManager::GetWeight ERROR\033[1;m Not initialized the"
			<< " fake rate histogram!" << std::endl;
		exit(-1);
	}
	
	int bin = _weights[lt]->FindBin(pt,fabs(eta));
	double w = _weights[lt]->GetBinContent(bin);
	
	// FIXME: Also return error --> std::pair
	return w;
}

//! Get name of the weight type 
const char * WManager::GetWTStr(const unsigned int & wt) const
{
	std::string type;
	if( wt == WManager::EFF )
	{
		type = "efficiency";
	}
	else if( wt == WManager::SF )
	{
		type = "scale factor";
	}
	else if( wt == WManager::FR )
	{
		type = "fake rate";
	}
	else
	{
		type = "NOT IMPLEMENTED";
	}

	return type.c_str();
}



