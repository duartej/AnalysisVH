// -*- C++ -*-
//
// Package:    CutManager
// Class:      MuonSelection
// 
/**\class  MuonSelection

 Description: Concrete class to select muons

 Implementation: 
*/
//
// Author: Jordi Duarte Campderros  
//         Created:  Sun Oct  30 12:20:31 CET 2011
//         Based in the PROOF version of the code created by
//         I. Gonzalez Caballero (May 9 2011)
// 
// jordi.duarte.campderros@cern.ch
//
//
#ifndef MUONSELECTION_H
#define MUONSELECTION_H

#include "TROOT.h"
#include "CutManager.h"
#include "LeptonMixingSelection.h"

#include<set>

// Codenames of the selection cuts:
//   -- 

class MuonSelection : public CutManager
{
	//! Granting access to the vector datamembers
	friend class LeptonMixingSelection;

	public:
		MuonSelection( TreeManager * data, const int & nLeptons = 3 );
		virtual ~MuonSelection() { }

		// Initialization of datamembers
		virtual void LockCuts(const std::map<LeptonTypes,InputParameters*> & ip,
				const std::vector<std::string> & cuts);
		
		//-- Some special cuts which are use directly from
		//   the anlysis client (used as wrapper)
		virtual bool IsPass(const std::string & codename, 
				const std::vector<double> * auxVar = 0 ) const;

		//-- Acceptance cuts: 
	//	bool IsPassAcceptanceCuts(const unsigned int & i,
	//			const double & pt, const double & eta) const

		//-- Selection
		//---------------------------------------------
		// Select basic muons: 
		// - with pt > MinPt and fabs(eta) < eta 
		//   (see IsPassAcceptanceCuts function)
		virtual unsigned int SelectBasicLeptons();
		// Select close to PV muons: 
		// - Depends on kMaxMuIP2DInTrack and kMaxDeltaZMu
		virtual unsigned int SelectLeptonsCloseToPV();
		// Select Iso Leptons: 
		// - Depends on MaxPTIsolationR# dependent of the region
		virtual unsigned int SelectIsoLeptons();
		// Select Good Identified Leptons: 
		// - Depends on 
		virtual unsigned int SelectGoodIdLeptons();
		virtual LeptonTypes GetLeptonType(const unsigned int & index) const { return MUON; }

		// Get the code names of the selection cuts
		virtual std::vector<std::string> GetCodenames() const;

	private:
		//-- The effective cuts whose would be called by IsPass
		//   method
		bool IsPassPtCuts() const;
		bool IsPassDeltaRCut(const double & deltaRMuMu) const; 
		bool IsInsideZWindow(const double & invariantMass) const; 
		bool IsPassMETCut(const double & MET) const;

		// The list of the selection chain codenames 
		std::set<std::string> _codenames;
		// The cuts
		double kMinMuPt1          ;
		double kMinMuPt2          ;
		double kMinMuPt3          ;
		double kMaxAbsEta         ;
	
		double kMaxMuIP2DInTrackR1;
		double kMaxMuIP2DInTrackR2;
	
		double kMaxDeltaZMu       ;
		double kMaxDeltaRMuMu     ;
		double kMinMET            ;
		
		double kDeltaZMass        ;
		double kMaxZMass	  ;
		double kMinZMass	  ;
	
		double kMaxPTIsolationR1  ;
		double kMaxPTIsolationR2  ;
		double kMaxPTIsolationR3  ;
		double kMaxPTIsolationR4  ;
		
		int    kMinNValidHitsSATrk     ;
		double kMaxNormChi2GTrk        ;
		int    kMinNumOfMatches        ;
		int    kMinNValidPixelHitsInTrk;
		int    kMinNValidHitsInTrk     ;
		double kMaxDeltaPtMuOverPtMu   ;
		
	ClassDef(MuonSelection,0);
};
#endif
