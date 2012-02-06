// -*- CCutManagerPackage:    CutManager
// Class:      ElecSelection
// 
/**\class  ElecSelection

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
#include "LeptonMixingSelection.h"

#include<set>

class WPElecID;

class ElecSelection : public CutManager
{
	friend class LeptonMixingSelection;

	public:
		//! Constructor
		ElecSelection( TreeManager * data, const int & WPlowpt, const int & WPhighpt,
				const int & nTights, const int & nLeptons);
		//! Destructor
		virtual ~ElecSelection();

		//! Lock the cuts introduced (function to be call after the last
		//! Fix the cuts extracted from the InputParameters
		virtual void LockCuts(const std::map<LeptonTypes,InputParameters*> & ip, 
				const std::vector<std::string> & cuts);
		
		//! Some special cuts which are use directly from the analysis client
		//! (used as wrapper, see valid codenames inside the implementation)
		virtual bool IsPass(const std::string & codename, 
				const std::vector<double> * auxVar = 0 ) const;

		//-- Acceptance cuts: 
	//	bool IsPassAcceptanceCuts(const unsigned int & i,
	//			const double & pt, const double & eta) const

		//! Get the code names of the selection cuts
		virtual std::vector<std::string> GetCodenames() const;
		
		//-- Selection
		//---------------------------------------------
		//! Get the lepton type of the i-esim good lepton (tight+notight)
		virtual LeptonTypes GetLeptonType(const unsigned int & index) const { return ELECTRON; }
		//! Get the lepton type of the i-esim tight lepton
		virtual LeptonTypes GetTightLeptonType(const unsigned int & index) const { return ELECTRON; }
		//! Get the lepton type of the i-esim no tight lepton
		virtual LeptonTypes GetNoTightLeptonType(const unsigned int & index) const { return ELECTRON; }

	private:
		//-- The effective cuts whose would be called by IsPass
		//   method
		bool IsPassPtCuts() const;
		bool IsPassDeltaRCut(const double & deltaRMuMu) const; 
		bool IsInsideZWindow(const double & invariantMass) const; 
		bool IsPassMETCut(const double & MET) const;
		bool IsPassWP( const unsigned int & index ) const;
		
		//! Syncronize lepton type with indices vector when fake mode active
		virtual void SyncronizeLeptonType() { /* Not neeed for this concrete class */ }
		
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

		//! Working Point for the pt > 20 Gev/c (highPt)
		//  and pt < 20 GeV/c (lowPt)
		WPElecID * pWP_lowPt;
		WPElecID * pWP_highPt;

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

		// Loose
		double kMaxLoosed0;
		double kMaxLooseIso;

	ClassDef(ElecSelection,0);
};
#endif
