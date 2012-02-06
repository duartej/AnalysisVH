#include "FOManager.h"

#include<iostream>
#include<sstream>
#include<cstdlib>
#include<cstring>
#include<cmath>

#include "TFile.h"
#include "TKey.h"
#include "TClass.h"
#include "TROOT.h"


FOManager::FOManager() 
{
	_fakerate[MUON] = 0;
	_fakerate[ELECTRON] = 0;
	_promptrate[MUON] = 0;
	_promptrate[ELECTRON] = 0;

	// FIXME: Hardcoded filenames
	char * pkgpath = 0;
	pkgpath = getenv("VHSYS");
	if( pkgpath == 0 )
	{
		std::cerr << "\033[1;31mFOManager::SetFR ERROR\033[1;m Not set the 'VHSYS'"
			<< " environment varible; this is an inconsistency in the code"
			<< std::endl;
		exit(-1);
	}

	std::string mufile(std::string(pkgpath)+"/FOManager/data/MuFR_All2011_LPcuts_AND_kink_jet15.root");
	std::string elecfile(std::string(pkgpath)+"/FOManager/data/ElecFR_all2011_jet35.root");

	this->SetFR(MUON,mufile.c_str());
	this->SetFR(ELECTRON,elecfile.c_str());


}

FOManager::~FOManager()
{
	for(std::map<LeptonTypes,TH2F*>::iterator it = _fakerate.begin(); it != _fakerate.end(); ++it)
	{
		if( it->second != 0 )
		{
			delete it->second;
			it->second = 0;
		}
	}
	
	for(std::map<LeptonTypes,TH2F*>::iterator it = _promptrate.begin(); it != _promptrate.end(); ++it)
	{
		if( it->second != 0 )
		{
			delete it->second;
			it->second = 0;
		}
	}
}

void FOManager::SetFR(const LeptonTypes & leptontype, const char * filename)
{
	// Forcing the ownership to FOManager instead of TFile,
	// in order not to enter in conflict with the TreeManager TFile
	TH2::AddDirectory(kFALSE);
	
	TFile *f = new TFile(filename);
	if( f->IsZombie() )
	{
		std::cerr << "\033[1;31mFOManager::SetFR ERROR\033[1;m Not found the filename '"
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
			std::string name(ss.str()+"_fr");
			_fakerate[leptontype] = (TH2F*)((TH2F*)f->Get(key->GetName()))->Clone(name.c_str());
		}
	}
	delete key;

	if( _fakerate[leptontype] == 0)
	{
		std::cerr << "\033[1;31mFOManager::SetFR ERROR\033[1;m Not found the TH2F with the"
			<< " rates. File '" << filename << "' not well defined" << std::endl;
		exit(-1);
	}

	f->Close();
	delete f;
}


void FOManager::SetPR(const LeptonTypes & leptontype, const char * filename)
{
	// Forcing the ownership to FOManager instead of TFile,
	// in order not to enter in conflict with the TreeManager TFile
	TH2::AddDirectory(kFALSE);

	TFile * f = new TFile(filename);
	if( f->IsZombie() )
	{
		std::cerr << "\033[1;31mFOManager::SetPR ERROR\033[1;m Not found the filename '"
			<< filename << "'" << std::endl;
		exit(-1);
	}

	// HARDCODED NAMES FOR THE HISTOGRAMS!! FIXME?
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
			std::string name(ss.str()+"_fr");
			_promptrate[leptontype] = (TH2F*)((TH2F*)f->Get(key->GetName()))->Clone(name.c_str());
		}
	}
	delete key;

	if( _promptrate[leptontype] == 0)
	{
		std::cerr << "\033[1;31mFOManager::SetFR ERROR\033[1;m Not found the TH2F with the"
			<< " rates. File '" << filename << "' not well defined" << std::endl;
		exit(-1);
	}

	f->Close();
	delete f;
}

const double FOManager::GetWeight(const LeptonTypes & lt, const double & pt, const double & eta)
{
	if( _fakerate[lt] == 0 )
	{
		std::cerr << "\033[1;31mFOManager::SetGetWeight ERROR\033[1;m Not initialized the "
			<< " fake rate histogram!" << std::endl;
		exit(-1);
	}
	
	int bin = _fakerate[lt]->FindBin(pt,fabs(eta));
	double f = _fakerate[lt]->GetBinContent(bin);
	
	// FIXME: Also return error --> std::pair
	return f/(1.0-f);
}

	
