
#include "CutManager.h"
#include "TreeManager.h"

CutManager::CutManager( TreeManager * data, const int & nLeptons  ) :
	_data(data), 
	_cuts(0),
	_undefcuts(0),
	_nLeptons(nLeptons),
	_idxLeptons(0)
{
	_cuts = new std::map<cuttype,std::vector<double> *>;
	_undefcuts = new std::map<int,std::vector<double> *>;
}

CutManager::~CutManager()
{
	// Freeing memory
	if( _cuts != 0 )
	{
		for(std::map<cuttype,std::vector<double> *>::iterator it = _cuts->begin();
				it != _cuts->end(); ++it)
		{
			if( it->second != 0)
			{
				delete it->second;
				it->second = 0;
			}
		}

		delete _cuts;
		_cuts = 0;
	}
	if( _undefcuts != 0 )
	{
		for(std::map<int,std::vector<double> *>::iterator it = _undefcuts->begin();
				it != _undefcuts->end(); ++it)
		{
			if( it->second != 0)
			{
				delete it->second;
				it->second = 0;
			}
		}

		delete _undefcuts;
		_undefcuts = 0;
	}

	if( _idxLeptons != 0 )
	{
		delete _idxLeptons;
		_idxLeptons = 0;
	}
}

// Setters
void CutManager::setcut( const cuttype & cutclass, const std::vector<double> & cuts )
{
	//FIXME: Con los insert, find, etc...?
	if( (*_cuts)[cutclass] == 0 )
	{
		(*_cuts)[cutclass] = new std::vector<double>;
	}

	for(unsigned int i = 0; i < cuts.size(); ++i)
	{
		(*_cuts)[cutclass]->push_back(cuts[i]);
	}
}


void CutManager::SetTriggerCuts( const std::vector<double> & cuts)
{
	setcut(_triggerCuts,cuts);
}

void CutManager::SetEventCuts( const std::vector<double> & cuts)
{
	setcut(_eventCuts,cuts);
}

void CutManager::SetPtMinCuts( const std::vector<double> & cuts)
{
	setcut(_ptCuts,cuts);
}
void CutManager::SetABSEtaMaxCuts( const std::vector<double> & cuts)
{
	setcut(_etaCuts,cuts);
}
void CutManager::SetIsoCuts( const std::vector<double> & cuts)
{
	setcut(_IsoCuts,cuts);
}
void CutManager::SetIdCuts( const std::vector<double> & cuts)
{
	setcut(_IdCuts,cuts);
}
void CutManager::SetQualityCuts( const std::vector<double> & cuts)
{
	setcut(_qualityCuts,cuts);
}

void CutManager::SetUndefCuts( const std::vector<double> & cuts, const int & cutindex )
{
	//FIXME: Con los insert, find, etc...?
	if( (*_undefcuts)[cutindex] == 0 )
	{
		(*_undefcuts)[cutindex] = new std::vector<double>;
	}

	for(unsigned int i = 0; i < cuts.size(); ++i)
	{
		(*_undefcuts)[cutindex]->push_back(cuts[i]);
	}
}
