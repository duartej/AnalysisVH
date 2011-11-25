// -*- C++ -*-
//
// Package:    CutManager
// Class:      LeptonMixingSelection
// 
/**\class  LeptonMixingSelection

 Description: Concrete class to select muons and electrons
 final state

 Implementation: 
*/
//
// \author: Jordi Duarte Campderros  
//         Created:  Fri Nov  30 10:28:31 CET 2011
// 
// jorge.duarte.campderros@cern.ch
//
//
#ifndef LEPTONMIXINGSELECTION_H
#define LEPTONMIXINGSELECTION_H

#include "CutManager.h"
#include "LeptonTypes.h"
#include "CutManager.h"

#include<set>

class ElecSelection;
class MuonSelection;

class LeptonMixingSelection : public CutManager
{
	public:
		//! Constructor
		LeptonMixingSelection( TreeManager * data, const int & WPlowpt, const int & WPhighpt, 
				const int & nLeptons = 3 );
		//! Destructor
		virtual ~LeptonMixingSelection();
		
		//! Lock the cuts introduced (function to be call after the last
		//! SetCut call)
		virtual void LockCuts(const std::map<LeptonTypes,InputParameters*> & ip,
				const std::vector<std::string> & cuts);
		
		//! Some special cuts which are use directly from the analysis client
		//! (used as wrapper, see valid codenames inside the implementation)
		virtual bool IsPass(const std::string & codename, 
				const std::vector<double> * auxVar = 0 ) const;


		//-- Selection
		//---------------------------------------------
		//! Select basic muons: 
		//! - with pt > MinPt and fabs(eta) < eta 
		virtual unsigned int SelectBasicLeptons();
		//! Select close to PV muons: 
		//! - Depends on kMaxMuIP2DInTrack and kMaxDeltaZMu
		virtual unsigned int SelectLeptonsCloseToPV();
		//! Select Iso Leptons: 
		//! - Depends on MaxPTIsolationR# dependent of the region
		virtual unsigned int SelectIsoLeptons();
		//! Select Good Identified Leptons: 
		//! - Depends on ...
		virtual unsigned int SelectGoodIdLeptons();

		//! Get the code names of the selection cuts: as there is no difference
		//! between leptons we can use whatever we want
		virtual std::vector<std::string> GetCodenames() const;
		//virtual std::vector<std::string> GetCodenames(const LeptonTypes & leptontype) const;
		
		//! Get The lepton type for the i-esim lepton // FIXME check if _lept.. != 0
		virtual LeptonTypes GetLeptonType(const unsigned int & index) const; 

	private:
		//! Pt cuts for both muons and electrons
		bool IsPassPtCuts(const int & nMuons, const int & nElecs) const;

		//! Check if the 'index' is found in the vector 'leptonsvector'
		bool isfoundindex(const std::vector<int> * const leptonsvector, 
				const int & index) const;

		//! The muon cut manager
		MuonSelection * fMuonSelection;
		//! The electron cut manager
		ElecSelection * fElecSelection;
		
		// The list of the selection chain codenames 
		//std::set<std::string> _codenames;

		//! Vector containing the type of lepton which
		//! corresponds to the counterpart index vector (from
		//! the mother class). It could be done directly by a map
		//! but then is needed modify the mother class.. (Maybe it's useful..)
		std::vector<LeptonTypes> * _leptontypebasicLeptons;
		std::vector<LeptonTypes> * _leptontypecloseToPVLeptons;
		std::vector<LeptonTypes> * _leptontypeIsoLeptons;
		std::vector<LeptonTypes> * _leptontypeGoodIdLeptons;
};

#endif
