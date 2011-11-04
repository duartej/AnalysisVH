
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


