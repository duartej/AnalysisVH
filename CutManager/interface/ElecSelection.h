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

	enum 
	{
		CUTBASED,           //cut based: using WPElecID class
		BDTBASED,           //BDT type 
		_nTYPE              //Internal use
	};

	public:
		//! Constructor for Cut based electrons
		ElecSelection( TreeManager * data, const int & WPlowpt, const int & WPhighpt,
				const int & nTights, const int & nLeptons,
				const char * runperiod);
		//! Constructor for BDT electrons
		ElecSelection( TreeManager * data, const int & nTights, const int & nLeptons, 
				const char * runperiod);
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
		//virtual LeptonTypes GetLeptonType(const unsigned int & index) const { return ELECTRON; }
		//! Get the lepton type of the i-esim tight lepton
		//virtual LeptonTypes GetTightLeptonType(const unsigned int & index) const { return ELECTRON; }
		//! Get the lepton type of the i-esim no tight lepton
		//virtual LeptonTypes GetNoTightLeptonType(const unsigned int & index) const { return ELECTRON; }

	private:
		//-- The effective cuts whose would be called by IsPass
		//   method
		bool IsPassPtCuts() const;
		bool IsPassDeltaRCut(const double & deltaRMuMu) const; 
		bool IsInsideZWindow(const double & invariantMass) const; 
		bool IsPassMETCut(const double & MET) const;
		//! Check if pass the Cut-based Working Point selected
		bool IsPassWP( const unsigned int & index ) const;
		//! Check if pass the BDT Working Point selected
		bool IsPassBDT( const unsigned int & index ) const;
		
		//! Update fakeables collection, taking into account the lepton type 
		virtual bool WasAlreadyUpdated() { return false; }
		
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
		//  and pt < 20 GeV/c (lowPt) in Cut-based electrons
		WPElecID * pWP_lowPt;
		WPElecID * pWP_highPt;

		//!the electron type used: Cut-based or BDT
		int _ElecType;

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

                double kMinMVAValueR1     ;
                double kMinMVAValueR2     ;
                double kMinMVAValueR3     ;
                double kMinMVAValueR4     ;
                double kMinMVAValueR5     ;
                double kMinMVAValueR6     ;
		
		//! Id related var
	        double kMaxSigmaietaietaR1;
	       	double kMaxdeltaPhiInR1; 
	       	double kMaxdeltaEtaInR1;
	       	double kMaxHtoER1;       
	       	double kMaxSigmaietaietaR2;
	       	double kMaxdeltaPhiInR2; 
	       	double kMaxdeltaEtaInR2;
	       	double kMaxHtoER2;  
	       	double kMaxdr03TkSumPtOverPt;
	       	double kMaxdr03EcalSumPtOverPt;
	       	double kMaxdr03HcalSumPtOverPt;
		
	ClassDef(ElecSelection,0);
};
#endif
