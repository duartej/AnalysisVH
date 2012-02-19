// -*- C++ -*-
//
// Package:    CutManager
// Class:      CutManager
// 
/**\class  CutManager

 Description: Abstract class in charge of apply cuts

 Implementation: Abstract class us
*/
//
// @author Original Author: Jordi Duarte Campderros  
//         Created:  Sun Oct  30 12:20:31 CET 2011
// 
// @author jordi.duarte.campderros@cern.ch
//
//
#ifndef CUTMANAGER_H
#define CUTMANAGER_H

#include<vector>
#include<map>
#include<iostream>
#include<string>
#include<bitset>

#include"TreeManager.h"
#include "LeptonTypes.h"
#include "InputParameters.h"


class CutManager
{
	public:
		//! Enumerator for the sample MODE (see CutManager::_samplemode)
		//! This code is ready to be used to estimate the fake background,
		//! It is accomplished using the CutManager::FAKEABLESAMPLE mode where
		//! the sample is selected with the definition of fakeable object (see 
		//! @fakeable cuts in configuration files). In this mode is necessary also
		//! to define the number of tight leptons info
		enum
		{
			NORMALSAMPLE,   // 
			FAKEABLESAMPLE  // _selectedbasicLeptons are loose leptons
		};
		
	public:
		//! Constructor: nTights = -1 to enter in NORMALSAMPLE mode
		//!              nTights >= 0 to enter in FAKEABLESAMPLE mode
		CutManager(TreeManager * data, const int & nTights, const int & nLeptons); 
		//! Destructor
		virtual ~CutManager();

		//! Resetting the index vectors before analyse another entry
		virtual void Reset();

		//! Stores the cuts (data members) from an InputParameters
		virtual void InitialiseCuts(const std::map<LeptonTypes,InputParameters*> & ip);

		//! Must be call it after the inclusion of all the cuts
		//! This funtion actually does the initialization of the cuts 
		//! introduced previously and must be implemented in the concrete classes
		virtual void LockCuts(const std::map<LeptonTypes,InputParameters*> & ip, 
				const std::vector<std::string> & cuts) = 0;

		//! Return whether a serie of cuts encoded as 'codename' has been passed
		virtual bool IsPass(const std::string & codename, 
			       const std::vector<double> * auxVar = 0 ) const = 0;
		//! Return a vector of string containing the names of the 'codenams'
		virtual std::vector<std::string> GetCodenames() const = 0;

		
		//! Selection stuff (in parenthesis the meaning when fake mode active)
		//! Number of leptons which pass the basic selection (Loose)
		unsigned int GetNBasicLeptons(); 
		//! Number of leptons closest to PV (tight)
		unsigned int GetNLeptonsCloseToPV();
		//! Number of Isolated leptons (tight)
		unsigned int GetNIsoLeptons();
		//! Number of Good Identified leptons (tight + no tight)
		unsigned int GetNGoodIdLeptons();
		//! Number of Tight Leptons 
		//unsigned int GetNTightLeptons();
		//! Number of no Tight Leptons 
		//unsigned int GetNnoTightLeptons();

		//! Auxiliary methods to deal with fakeables sample
		//! ---------------------------------------------------------------
		//! Return true if exactly have _nLeptons (or _nTight tight leptons
		//! and _nFails no tight leptons in the FAKEABLESAMPLE case), after
		//! all cuts (_selectedGoodIdLeptons must be already fill in)
		bool IspassExactlyN();
		//! Return true if at least have _nLeptons (or _nTight tight leptons
		//! and _nFails no tight leptons in the FAKEABLESAMPLE case), after
		//! all cuts (_selectedGoodIdLeptons must be already fill in)
		bool IspassAtLeastN();
		//! Overloaded method to allow any combination of tights and no tights,
		//! the first argument is the number of leptons we want to be passed, 
		//! the second argument is the number of tight leptons we have (so
		//! the function will add the number of no tight leptons if proceed)
		bool IspassAtLeastN(const unsigned int & nLeptons, const unsigned int & nTight);

		//! Return true if we are in FAKEABLESAMPLE mode
		inline bool IsInFakeableMode() const { return _samplemode == CutManager::FAKEABLESAMPLE ; }

		void UpdateFakeableCollections( const std::vector<int> * finalcol );
		
		//-- Getters
		//! Get good leptons, i.e., whatever passing the GoodId level, also the no tight 
		//! leptons if fakeable mode is active
		virtual std::vector<int> * GetGoodLeptons() const { return _selectedGoodIdLeptons; }
		//! Get the Tight leptons (passing the PV, Iso and ID cuts)
		inline virtual std::vector<int> * GetTightLeptons() const { return _tightLeptons; }
		//! Get the noTight leptons (not passing the PV, Iso and ID cuts)
		inline virtual std::vector<int> * GetNoTightLeptons() const { return _notightLeptons; }
		//! Get The lepton type for the i-esim good lepton  (tight+notight)
		virtual LeptonTypes GetLeptonType(const unsigned int & index) const = 0;
		//! Get The lepton type for the i-esim Tight lepton 
		virtual LeptonTypes GetTightLeptonType(const unsigned int & index) const = 0;
		//! Get The lepton type for the i-esim no Tight lepton 
		virtual LeptonTypes GetNoTightLeptonType(const unsigned int & index) const = 0;
		//! Get the i-essim index of the NoTight lepton
		const unsigned int GetNoTightIndex(const unsigned int & i) const;
		//! Get the number of total leptons which are considered in this analysis
		inline const unsigned int GetNAnalysisLeptons() { return _nLeptons; }
		//! Get the number of total Tight leptons which are considered in this analysis
		inline const unsigned int GetNAnalysisTightLeptons() { return _nTights; }
		//! Get the number of total No-Tight leptons which are considered in this analysis
		inline const unsigned int GetNAnalysisNoTightLeptons() { return _nFails; }

		//-- Setters
		//! Set the operational MODE
		inline virtual void SetMode( const unsigned int & mode ) { _samplemode = mode; }
		//! Set the number of leptons considered in the analysis client
		inline virtual void SetNLeptons( const unsigned int & nLeptons ) { _nLeptons = nLeptons; }
		//! Set a cut named 'cutname'
		void SetCut(const std::string & cutname, const double & value);

		friend std::ostream & operator<<(std::ostream & out, const CutManager & cm );

	protected:
		//! Selectors: WARNING use GetNWhatever methods instead!! Not to be used by any client
		//! Basic selection: usually consist in some loose kinematical cuts
		//! and some loose id cuts (Loose)
		virtual unsigned int SelectBasicLeptons() = 0; 
		//! Select leptons close to the Primary Vertex 
		virtual unsigned int SelectLeptonsCloseToPV() = 0;
		//! Select isolated leptons 
		virtual unsigned int SelectIsoLeptons() = 0;
		//! Select good Identified leptons (tight, must be used GetNGoodIdLeptons to get
		//! the tight+notight)
		virtual unsigned int SelectGoodIdLeptons() = 0;
		//! Loose leptons: to study fakes background, the iso and id cuts are 
		//! loose generating a fakeable objects sample. Only activated when
		//! mode == CutManager::FAKEABLESAMPLE
		virtual unsigned int SelectLooseLeptons() = 0; 
		
		//! Syncronize lepton type with indices vector when fake mode active
		virtual void SyncronizeLeptonType() = 0;
		
		//! Update fakeables collection, taking into account the lepton type (fake mode active)
		virtual bool WasAlreadyUpdated() = 0;

		//! Container of the data:  FIXME: IT is needed?
		TreeManager * _data;

		//! Mapping name of the cut with its value (must be a double)
		std::map<std::string,double> * _cuts;
		
		//! Number of leptons to be considered in the analysis
		unsigned int _nLeptons;

		//! Sample mode
		unsigned int _samplemode;

		//! Number of Tight leptons, If sample mode is in FAKEABLE: 
		unsigned int _nTights;
		//! Number of no Tight leptons, If sample mode is in FAKEABLE,
		//! note that _nLeptons = _nTights + _nFails
		unsigned int _nFails;

		//! Selection datamembers (in parenthesis the meaning when fake mode active)
		//! Vector of index of leptons which pass the basic selection (Loose)
		std::vector<int> * _selectedbasicLeptons;
		//! Vector of index of leptons closest to PV (tight)
		std::vector<int> * _closeToPVLeptons;
		//! Vector of index of isolated leptons (tight + no tight)
		std::vector<int> * _selectedIsoLeptons;
		//! Vector of index of good identified leptons ( tight + no tight)
		std::vector<int> * _selectedGoodIdLeptons;

		//! Vector of leptons indices which have not passed the tight cuts 
		std::vector<int> * _notightLeptons;
		//! Vector of leptons indices which have pass the tight cuts
		std::vector<int> * _tightLeptons;

	ClassDef(CutManager,0);
};

//! Print method
inline std::ostream & operator<<(std::ostream & out, const CutManager & cutmanager)
{
	out << "|========== CutManager Print ============|" << std::endl;
	out << "| Operational mode:" << cutmanager._samplemode << std::endl;
	if( cutmanager._samplemode == CutManager::FAKEABLESAMPLE )
	{
		out << "| ++ Sample: ";
		for(unsigned int j = 0; j < cutmanager._nTights; ++j)
		{
			out << "Tight ";
		}
		for(unsigned int j = 0; j < cutmanager._nFails; ++j)
		{
			out << "No-Tight ";
		}
		out << std::endl;
	}
	out << "|============ Selection Cuts: " << std::endl;
	if( cutmanager._cuts != 0 )
	{
		for(std::map<std::string,double>::iterator it = cutmanager._cuts->begin();
				it != cutmanager._cuts->end(); ++it)
		{
			out << " + " << it->first << ": " << it->second << std::endl;
		}
	}
	out << "|============ Selection Cuts ============| " << std::endl;

	return out;
}



#endif
