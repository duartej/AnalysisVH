#include "FOManager.h"

#include<iostream>
#include<sstream>
#include<stdlib.h>
#include<cstring>

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
	
	for(std::map<LeptonTypes,TH2F*>::iterator it = _fakerate.begin(); it != _fakerate.end(); ++it)
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
	TFile * f = new TFile(filename);
	if( f->IsZombie() )
	{
		std::cerr << "\033[1;31mFOManager::SetFR ERROR[1;m Not found the filename '"
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
			_fakerate[leptontype] = new TH2F;
			std::stringstream ss;
			ss << leptontype;
			std::string name(ss.str()+"_fr");
			_fakerate[leptontype] = (TH2F*)((TH2F*)f->Get(key->GetName()))->Clone(name.c_str());
		}
		
		delete cl;
	}
	delete key;

	if( _fakerate[leptontype] == 0)
	{
		std::cerr << "\033[1;31mFOManager::SetFR ERROR[1;m Not found the TH2F with the"
			<< " rates. File '" << filename << "' not well defined" << std::endl;
		exit(-1);
	}

	f->Close();
	delete f;
}


void FOManager::SetPR(const LeptonTypes & leptontype, const char * filename)
{
	TFile * f = new TFile(filename);
	if( f->IsZombie() )
	{
		std::cerr << "\033[1;31mFOManager::SetPR ERROR[1;m Not found the filename '"
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
			_promptrate[leptontype] = new TH2F;
			std::stringstream ss;
			ss << leptontype;
			std::string name(ss.str()+"_fr");
			_promptrate[leptontype] = (TH2F*)((TH2F*)f->Get(key->GetName()))->Clone(name.c_str());
		}
		
		delete cl;
	}
	delete key;

	if( _promptrate[leptontype] == 0)
	{
		std::cerr << "\033[1;31mFOManager::SetFR ERROR[1;m Not found the TH2F with the"
			<< " rates. File '" << filename << "' not well defined" << std::endl;
		exit(-1);
	}

	f->Close();
	delete f;
}

const double FOManager::GetWeight(const LeptonTypes & lt, const double & pt, const double & eta) const
{
	// FIXME:  
	// _fakerate[lt]->FindBin(pt,eta);

	double f = 0.15; // MUON
	if( lt == ELECTRON )
	{
		f = 0.20;
	}
	
	return f/(1.0-f);
}

	
