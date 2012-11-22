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
		//! Constructor using cut-based electrons
		LeptonMixingSelection( TreeManager * data, const int & WPlowpt, 
				const int & WPhighpt, 
				const int & nTights, const int & nLeptons,
				const char * runperiod);
		//! Constructor using BDT-based electrons
		LeptonMixingSelection( TreeManager * data,
				const int & nTights, const int & nLeptons,
				const char * runperiod);
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


		//! Get the code names of the selection cuts: as there is no difference
		//! between leptons we can use whatever we want
		virtual std::vector<std::string> GetCodenames() const;
		//virtual std::vector<std::string> GetCodenames(const LeptonTypes & leptontype) const;
		
		//-- Selection
		//---------------------------------------------
		//! Get The lepton type for the i-esim lepton (tight+notight) -- TO BE DEPRECATED
		//virtual LeptonTypes GetLeptonType(const unsigned int & index) const; 
		//! Get The lepton type for the i-esim tight lepton  -- TO BE DEPRECATED
		//virtual LeptonTypes GetTightLeptonType(const unsigned int & index) const;  
		//! Get The lepton type for the i-esim no tight lepton -- TO BE DEPRECATED
		//virtual LeptonTypes GetNoTightLeptonType(const unsigned int & index) const; 

		//! Overloaded Reset method in order to deal with the extra data members
		//! used by this class
		virtual void Reset();

	private:
		//! Pt cuts for both muons and electrons
		bool IsPassPtCuts(const int & nMuons, const int & nElecs) const;

		//! Check if the 'lepton' is found in the vector 'leptonsvector'
		bool isfound(const LeptonRel & lepton, 
				const std::vector<LeptonRel> * const leptonsvector) const;
		
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
		// Loose leptons 
		virtual unsigned int SelectLooseLeptons();

		//! Update fakeables collection, taking into account the lepton type 
		virtual bool WasAlreadyUpdated() { return true; }
		
		//! Update fakeables collections, concrete method for this class
		void UpdateFakeableCollections();
		

		//! The muon cut manager
		MuonSelection * fMuonSelection;
		//! The electron cut manager
		ElecSelection * fElecSelection;		
};

#endif
