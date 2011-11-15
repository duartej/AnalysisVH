// -*- C++ -*-
//
// Package:    ElecSelection
// Class:      ElecSelection
// 
/**\class  ElecSelection.h src/ElecSelection.cc

 Description: Concrete class to select electrons

 Implementation: 
*/
//
// Modifications Author: Jordi Duarte Campderros  
//         Created:  Sun Oct  30 12:20:31 CET 2011
// 
// jordi.duarte.campderros@cern.ch
//
//
#ifndef ELECSELECTION_H
#define ELECSELECTION_H

#include "TROOT.h"
#include "CutManager.h"

#include<set>


class ElecSelection : public CutManager
{
	public:
		//! Constructor
		ElecSelection( TreeManager * data, const int & nLeptons = 3 );
		//! Destructor
		virtual ~ElecSelection() { }

		//! Lock the cuts introduced (function to be call after the last
		//! SetCut call)
		virtual void LockCuts();
		
		//! Some special cuts which are use directly from the analysis client
		//! (used as wrapper, see valid codenames inside the implementation)
		virtual bool IsPass(const std::string & codename, 
				const std::vector<double> * auxVar = 0 ) const;

		//-- Acceptance cuts: 
	//	bool IsPassAcceptanceCuts(const unsigned int & i,
	//			const double & pt, const double & eta) const

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

		//! Get the code names of the selection cuts
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

	ClassDef(ElecSelection,0);
};
#endif
