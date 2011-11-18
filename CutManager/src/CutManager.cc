
#include<stdlib.h>

#include "CutManager.h"
#include "TreeManager.h"

CutManager::CutManager( TreeManager * data, const int & nLeptons  ) :
	_data(data), 
	_cuts(0),
	//_undefcuts(0),
	_nLeptons(nLeptons),
	_selectedbasicLeptons(0),
	_closeToPVLeptons(0),
	_selectedIsoLeptons(0),
	_selectedGoodIdLeptons(0),
	_idxLeptons(0)
{
	_cuts = new std::map<std::string,double>;
	//_undefcuts = new std::map<int,std::vector<double> *>;
}

CutManager::~CutManager()
{
	// Freeing memory
	if( _cuts != 0 )
	{
		delete _cuts;
		_cuts = 0;
	}

	if( _idxLeptons != 0 )
	{
		delete _idxLeptons;
		_idxLeptons = 0;
	}

	if( _selectedbasicLeptons != 0)
	{
		delete _selectedbasicLeptons;
		_selectedbasicLeptons = 0;
	}
	if( _closeToPVLeptons != 0)
	{
		delete _closeToPVLeptons;
		_closeToPVLeptons = 0;
	}
	if( _selectedIsoLeptons != 0)
	{
		delete _selectedIsoLeptons;
		_selectedIsoLeptons = 0;
	}
	if( _selectedGoodIdLeptons != 0)
	{
		delete _selectedGoodIdLeptons;
		_selectedGoodIdLeptons = 0;
	}
}

// Es una funcion de esta classe quien se deberia encargar de esto o es
// mejor que lo haga la clase analisis? Depende de lo generales que 
// queremos que sean los corte...
void CutManager::InitialiseCuts(const std::map<LeptonTypes,InputParameters*> & ipmap)
{
	//---- FIXME: Recupera las explicaciones
	std::vector<std::string> cuts;
	//   - Pt and Eta of muons
	cuts.push_back("MinMuPt1");
	cuts.push_back("MinMuPt2");
	cuts.push_back("MinMuPt3");
	cuts.push_back("MaxAbsEta");
	//   - IP and DeltaZ of track associated with muon w.r.t PV
	cuts.push_back("MaxMuIP2DInTrackR1");
	cuts.push_back("MaxMuIP2DInTrackR2");
	cuts.push_back("MaxDeltaZMu") ;
  	//   - Isolation: (PTtraks + ETcalo)/PTmuon: different regions
	cuts.push_back("MaxPTIsolationR1");
	cuts.push_back("MaxPTIsolationR2");
	cuts.push_back("MaxPTIsolationR3");
	cuts.push_back("MaxPTIsolationR4");
	cuts.push_back("MaxIsoMu");  // OBSOLETE--> Now in regions
	//   - Quality and Identification
	cuts.push_back("MinNValidHitsSATrk");
	cuts.push_back("MaxNormChi2GTrk");
	cuts.push_back("MinNumOfMatches");
	cuts.push_back("MinNValidPixelHitsInTrk");
	cuts.push_back("MinNValidHitsInTrk");
	cuts.push_back("MaxDeltaPtMuOverPtMu");
	//   - Max DeltaR between muons
	cuts.push_back("MaxDeltaRMuMu");
  	//   - Min MET of the event
	cuts.push_back("MinMET");

	// Now including all the cuts to the manager
	this->LockCuts(ipmap,cuts);
}

// Method to be called each time finalize a entry
void CutManager::Reset()
{
	if( _idxLeptons != 0 )
	{
		delete _idxLeptons;
		_idxLeptons = 0;
	}

	if( _selectedbasicLeptons != 0)
	{
		delete _selectedbasicLeptons;
		_selectedbasicLeptons = 0;
	}
	if( _closeToPVLeptons != 0)
	{
		delete _closeToPVLeptons;
		_closeToPVLeptons = 0;
	}
	if( _selectedIsoLeptons != 0)
	{
		delete _selectedIsoLeptons;
		_selectedIsoLeptons = 0;
	}
	if( _selectedGoodIdLeptons != 0)
	{
		delete _selectedGoodIdLeptons;
		_selectedGoodIdLeptons = 0;
	}
}

//
unsigned int CutManager::GetNBasicLeptons()
{
	int size = 0;
	if( _selectedbasicLeptons == 0)
	{
		size = this->SelectBasicLeptons();
	}
	else
	{
		size = _selectedbasicLeptons->size();
	}

	return size;
}

unsigned int CutManager::GetNLeptonsCloseToPV()
{
	int size = 0;
	if( _closeToPVLeptons == 0)
	{
		size = this->SelectLeptonsCloseToPV();
	}
	else
	{
		size = _closeToPVLeptons->size();
	}

	return size;
}

unsigned int CutManager::GetNIsoLeptons()
{
	int size = 0;
	if( _selectedIsoLeptons == 0)
	{
		size = this->SelectIsoLeptons();
	}
	else
	{
		size = _selectedIsoLeptons->size();
	}

	return size;
}

unsigned int CutManager::GetNGoodIdLeptons()
{
	int size = 0;
	if( _selectedGoodIdLeptons == 0)
	{
		size = this->SelectGoodIdLeptons();
	}
	else
	{
		size = _selectedGoodIdLeptons->size();
	}

	return size;
}

// Setters
void CutManager::SetCut(const std::string & cutname, const double & value)
{
	//FIXME: Check the field is already not used
	(*_cuts)[cutname] = value;
}


