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


WManager::WManager(const unsigned int & weighttype, const std::string & runperiod, 
		const std::string & muonid,
		const int & systematics ) :
	_runperiod(runperiod),
	_muonid(muonid),
	_wtype(weighttype)
{
	this->init(systematics,false);
}

WManager::WManager(const unsigned int & weighttype, const std::string & runperiod, 
		const std::string & muonid,
		const int & systematics,
		const bool & isZJetsRegion ) :
	_runperiod(runperiod),
	_muonid(muonid),
	_wtype(weighttype)
{
	// Check coherence
	if( systematics != WManager::UP && systematics != WManager::DOWN && systematics != 0)
	{
		std::cerr << "\033[1;31mWManager ERROR\033[1;m Not understood '"
			<< " sytematic variation '" << systematics << "'. Valid"
			<< " values are 'UP' and 'DOWN'"
			<< std::endl;
		exit(-1);
	}
	this->init(systematics,isZJetsRegion);
}

void WManager::init(const int & systematic,const bool & isZJetsRegion )
{
	// Check coherence
	if( _runperiod.find("2011") != std::string::npos &&
		_runperiod.find("2012") != std::string::npos )
	{
		std::cerr << "\033[1;31mWManager ERROR\033[1;m The run period '"
			<< _runperiod << "' is not coded. Exiting..."
			<< std::endl;
		exit(-1);
	}
	
	// Check for valid keys
	if(  (_muonid != "") and (_muonid != "hwwid" and _muonid != "vbtf") )
	{
		std::cerr << "\033[1;31mWManager ERROR\033[1;m Valid values are"
			<< " 'hwwid' and 'vbtf'. Instead, found the value: '" << _muonid << "'"
			<< std::endl;
		exit(-1);
	}

	_weights[MUON] = 0;
	_weights[ELECTRON] = 0;
	_filesnames[MUON] = "";
	_filesnames[ELECTRON] = "";

	_filesnames[MUON] = this->getfile(MUON,isZJetsRegion);
	_filesnames[ELECTRON] = this->getfile(ELECTRON,isZJetsRegion);

	this->setweightfile(MUON,_filesnames[MUON].c_str(),systematic);
	this->setweightfile(ELECTRON,_filesnames[ELECTRON].c_str(),systematic);
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

std::string WManager::getfile(const unsigned int & lepton, const bool & isZJetsRegion)
{
	/* Notation of the weighting files: LepWT_YEARP1P2_EXTRAINFO.root
	Where: 
	       Lep       -- Lepton name -- Mu|Ele 
	       WT        -- Weight type -- FR|PR|SF
	       YEAR      -- Run year    -- 2011|2012
	       ID        -- ID and ISO  -- hwwid|vbtf (just for muons of 2011)
	       PI        -- Run Period  -- A|B|C (Nothing is equivalent to all the run periods of the year)
	       EXTRAINFO -- Some extra information
	*/

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
	std::string jetzregion;

	// Build the name given the inputs
	// 1. Lepton type
	if( lepton == MUON )
	{
		thefile += "Mu";
		jetzregion = "jet30";
	}
	else
	{
		thefile += "Ele";
		jetzregion = "jet35";
	}
	
	// 2. the weight type
	thefile += this->getstrtype(this->_wtype);
	thefile += "_";

	// 3. the Run year and periods  (notation is yearABC ..)
	thefile += this->_runperiod;

	// 3!. If muons and 2011, check the  id
	if( lepton == MUON and this->_runperiod.find("2011") != std::string::npos )
	{
		// Coherence
		if( _muonid == "" )
		{
			std::cerr << "\033[1;31mWManager::getfile ERROR\033[1;m For the run period 2011 is "
			<< "mandatory the inclusion of the muon id (vbtf or hwwid)."
			<< std::endl;
			exit(-1);
		}
		
		thefile += "_";
		thefile += this->_muonid;
	}


	// 4. if fake rate, check whether we use the ZRegion fake matrices
	if( this->_wtype == WManager::FR )
	{
		thefile += "_";
		if( isZJetsRegion )
		{
			thefile += jetzregion;
		}
		else
		{
			thefile += "jet50";
		}
	}

	// And extension
	thefile += ".root";

	return thefile;
}

void WManager::setweightfile(const LeptonTypes & leptontype, const char * filename, const int & systematic)
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

	// Get the Overflow bin in pt in order to assign any lepton
	// higher than the maximum
	TH2F * prov = (TH2F*)_weights[leptontype]->Clone("prov");
	// Get the Overflow bin in pt in order to assign any lepton
	// higher than the maximum 
	// Extract the maximum pt
	const double ptMax = prov->GetXaxis()->GetBinLowEdge(prov->GetNbinsX()+1);
	const double ptLowMax = prov->GetXaxis()->GetBinCenter(prov->GetNbinsX());
	for(int i = 1; i <= prov->GetNbinsX()+1; ++i)
	{
		double ptIn = prov->GetXaxis()->GetBinCenter(i);
		const double ptOut = ptIn;
		if( ptIn >= ptMax )  // Note this is a fixed bug!! Afecting Scale factors and Prompt rates
		{
			ptIn = ptLowMax;
		}

		// WARNING: FAKE RATE Matrix for  pt > 35 is not reliable
		//          Following Alicia advice take the values of bin [30,35]
		if( _wtype == WManager::FR ) // FIXME: Solo para 2011? o para todos?
		{
			if( ptIn >= 35.0 )
			{
				ptIn = 34.;
			}
		}
		// Same as pt (w.r.t. the overflow) for the eta case 
		for(int j = 1; j <= prov->GetNbinsY()+1; ++j)
		{
			const double eta = prov->GetYaxis()->GetBinCenter(j);
			const int globalbin = prov->FindBin(ptIn,eta);
			// Some arrangements in order to avoid the calculation inside the loop
			// for the simple calculation of the fake rate, the weight per event is f/(1-f)
			// Otherwise (using the full calculation) comment lines below and change 
			const double f = prov->GetBinContent(globalbin);
			double finalweight = f;
			// Building systematics if proceed, adding the error contribution
			// so when call getWeight the value is already the (value+-error)
			if( systematic )
			{
				if( systematic == WManager::UP )
				{
					finalweight += prov->GetBinError(globalbin);
				}
				else if( systematic == WManager::DOWN )
				{
					finalweight -= prov->GetBinError(globalbin);
				}
			}
			/*if( _wtype == WManager::FR )  ---> Comment when full calculation
			{
			        finalweight = finalweight/(1.0-finalweight);
			}*/
			const int globalbinOut = prov->FindBin(ptOut,eta);
			_weights[leptontype]->SetBinContent(globalbinOut,finalweight);
		}
	}
	if(prov != 0)
	{
		delete prov;
		prov = 0;
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

//! Get the internal string name of the weight type
const std::string WManager::getstrtype( const unsigned int & wt) const
{
	std::string type;
	if( wt == WManager::EFF )
	{
		type = "EF";
	}
	else if( wt == WManager::SF )
	{
		type = "SF";
	}
	else if( wt == WManager::FR )
	{
		type = "FR";
	}
	else if( wt == WManager::PR )
	{
		type = "PR";
	}
	else if( wt == WManager::TR_LEADING )
	{
		type = "TR_leading";
	}
	else if( wt == WManager::TR_TRAILING )
	{
		type = "TR_trailing";
	}
	else
	{
		std::cerr << "\033[1;31mWManager::getstrtype ERROR\033[1;m Type not implemented "
			<< std::endl;
		exit(-1);
	}

	return type;
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
	else if( wt == WManager::PR )
	{
		type = "prompt rate";
	}
	else if( wt == WManager::TR_LEADING )
	{
		type = "trigger efficiency (leading leg)";
	}
	else if( wt == WManager::TR_TRAILING )
	{
		type = "trigger efficiency (trailing leg)";
	}
	else
	{
		type = "NOT IMPLEMENTED";
	}

	return type.c_str();
}


// Just extract the name of the file used
const std::string WManager::GetFilename(const LeptonTypes & lt)
{
	size_t wmanagerfound = _filesnames[lt].find("WManager/data");
	return _filesnames[lt].substr(wmanagerfound);
}


