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

	//std::string mufile(std::string(pkgpath)+"/WManager/data/");
	//std::string elecfile(std::string(pkgpath)+"/WManager/data/");

	// FIXME: Hardcoded filenames
	/*if( _wtype == WManager::SF )
	{
		mufile  += "Muon_OutputScaleFactorMap_MC42X_2011_ALL_Reweighted.root";
		elecfile+= "Electron_OutputScaleFactorMap_MC42X_2011_ALL_Reweighted.root";
	}
	else
	{
		std::cerr << "\033[1;31mWManager::WManager ERROR\033[1;m Not implemented "
			<< "'" << this->GetWTStr(weighttype) << "' weight type." 
			<< std::endl;
		exit(-1);
	}*/

	this->SetWeightFile(MUON,mufile.c_str());
	this->SetWeightFile(ELECTRON,elecfile.c_str());
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
				thefile  += "MuFR_All2011_LPcuts_AND_kink_jet15.root";
			}
		}
		else if( lepton == ELECTRON )
		{
			if( isSystematics )
			{
				thefile  += "ElecFR_all2011_jet15.root";
			}
			else
			{
				thefile += "ElecFR_all2011_jet35.root";
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


void WManager::SetWeightFile(const LeptonTypes & leptontype, const char * filename)
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
			// Check if it was already filled: then removed
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
	// FIXME: SOMETHING IS WRONG HERE, I DIDN'T FIND THE SAME RESULT AS IN THE FOMANAGER. 
	//        TO CHECK!! BY THE MOMENT I DON'T USE IT
	if( _wtype == WManager::FR )
	{
		for(int i = 0; i < _weights[leptontype]->GetNbinsX(); ++i)
		{
			for(int j = 0; j < _weights[leptontype]->GetNbinsY(); ++j)
			{
				const double f = _weights[leptontype]->GetBinContent(i,j);
				double finalweight = f/(1.0-f);
				_weights[leptontype]->SetBinContent(i,j,finalweight);
			}
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



