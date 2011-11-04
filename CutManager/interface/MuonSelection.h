
///////////////////////////////////////////////////////////////////////
//
//    FILE: MuonSelection.h
//   CLASS: MuonSelection
// AUTHORS: I. Gonzalez Caballero
//    DATE: 09/05/2011
//
// CONTENT: This class for PAF standarizes the selection of good muons 
//          for analysis
//
// Changelog: Incorporates some getters functions to the 
//            CMSAnalysisSelector class in order to avoid the creation
//            of every MuonSelector class for every diferent kind of
//            tree. Now, the responsable to call the current selector
//            to use is the client, not the MuonSelector class.
//                                       J. Duarte Campderros Oct, 2011
///////////////////////////////////////////////////////////////////////
// -*- C++ -*-
//
// Package:    MuonSelection
// Class:      MuonSelection
// 
/**\class  MuonSelection.h src/MuonSelection.cc

 Description: Concrete class to select muons

 Implementation: 
*/
//
// Modifications Author: Jordi Duarte Campderros  
//         Created:  Sun Oct  30 12:20:31 CET 2011
// 
// jordi.duarte.campderros@cern.ch
//
//
#ifndef MUONSELECTION_H
#define MUONSELECTION_H

#include "TROOT.h"
#include "CutManager.h"

#include<set>

// Codenames of the selection cuts:
//   -- 

class MuonSelection : public CutManager
{
	public:
		MuonSelection( TreeManager * data, const int & nLeptons = 3 );
		virtual ~MuonSelection() { }

		// Initialization of datamembers
		virtual void LockCuts();
		
		//-- Some special cuts which are use directly from
		//   the anlysis client (used as wrapper)
		virtual bool IsPass(const std::string & codename, 
				const double auxVar[] = 0 ) const;

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

		// Get the code names of the selection cuts
		virtual std::vector<std::string> GetCodenames() const;


	private:
		//-- The effective cuts whose would be called by IsPass
		//   method
		bool IsPassPtCuts(const unsigned & i, const double & pt,
				const double & eta) const;
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
