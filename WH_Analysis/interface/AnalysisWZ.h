#ifndef ANALYSISWZ_H
#define ANALYSISWZ_H 1

// Blabla description

#include<vector>
#include<map>
#include<cstring>

//#include "CMSAnalysisSelector.h"
#include "AnalysisBase.h"
#include "TreeManager.h"
#include "LeptonTypes.h"
#include "SignatureFS.h"

#include "TString.h"
#include "TLorentzVector.h"


class InputParameters;
class PUWeight;
class TTree;
class CutManager;
class TH1D;


class AnalysisWZ : public AnalysisBase
{
	public: 
		//! For histograms
		enum 
		{
			fHGenFinalState = nextenumerate , //Final state from generation (incl. taus)
			fHGenFinalStateNoTaus,    //Final state from generation (no taus)
			fHNGenWElectrons,	  //Generated leptons coming from a W
			fHNGenWMuons,		  //Generated leptons coming from a W
			fHNGenWLeptons,		  //Generated leptons coming from a W
			fHNRecoLeptons,           //Reconstructed leptons in the event
			fHNSelectedLeptons,       //Leptons passing the basic selection
			fHNSelectedPVLeptons,     //Selected Leptons close to the PV
			fHNSelectedIsoLeptons,    //Selected Isolated leptons
			fHNSelectedIsoGoodLeptons,//Selected Isolated Good Leptons
			fHLeptonCharge,           //Sum charges of leptons
			fHEtJetMatchedLeptonPreSel,//Et of the jet matching a lepton at preselection
			fHEtJetMatchedLeptonAfterZ,//Et of the jet matching a lepton after z candidate
			fHEtJetMatchedLepton,     //Et of the jet matching a lepton after all cuts
			fHLeadingJetPtAfterZCand, // Pt of the leading jet after z candidate
			fHLeadingJetPtAfterWCand, // Pt of the leading jet after W candidate
			fHLeadingJetPt,           // Pt of the leading jet after all cuts
			fHZInvMass,               //Invariant mass of leptons in/out of Z peak after all cust
			fHZInvMassAfterZCand,     //Invariant mass of leptons in/out of Z peak
			fHZInvMassAfterWCand,     //Invariant mass of leptons in/out of Z peak
			fHMET,                    //Missing ET after all cuts
			fHMETAfterZCand,          // MET affer ZCandidate cut
			fHMETAfterWCand,
			fHZPtAfterZCand,          // Pt of the Z (lepton system) after z candidate
			fHZPtAfterWCand,          // Pt of the Z (lepton system) after w candidate
			fHZPt,                    // Pt of the Z (lepton system) after all cuts
			fHPtLeptonZleading,       // Pt of the Z leading lepton after all cuts
			fHPtLeptonZtrailing,      // Pt of the Z trailing lepton after all cuts
			fHPtLeptonWAfterWCand,    // Pt of the W lepton after requiring W 
			fHPtLeptonW,              // Pt of the W lepton after all cuts
			fHWPtAfterWCand,          // Pt of the W system after W Cand
			fHWPt,                    // Pt of the W system after all cuts
			fHNJets,                  // Number of jets after all cuts
			fHNJetsPreSel,            // Number of jets after all cuts
			fHNJetsAfterZCand,        // Number of jets after all cuts
			fHNJetsAfterWCand,        // Number of jets after all cuts
			fHTransversMassAfterWCand,// Transverse mass W lepton and MET after W candidate
			fHTransversMass,          // Transverse mass W lepton and MET after all cuts
			fHDeltaPhiWMETAfterWCand, // DeltaPhi between W lepton and MET after W Candidate
			fHDeltaPhiWMET,           // DeltaPHi between W lepton and MET after all cuts
			fHNPrimaryVerticesAfterZCand, // Number of vertices after Z cand
			fHdRl1Wcand,
			fHdRl2Wcand,
			// REQUESTED FROM PRE-APPROVAL ---> After Z candidate has no sense but...
			fHPtLeptonWAfterWCand_METlowreg,
			fHDeltaPhiWMETAfterWCand_METlowreg,
			fHTransversMassAfterWCand_METlowreg,
			//fHIsoLepton,
			//fHD0Lepton
			// EXTRA PLOTS
			fHRelIsoFakeLeptonAfterZCand
		};
		
		//! Constructor
		AnalysisWZ( TreeManager * data, std::map<LeptonTypes,InputParameters*> ipmap, 
				CutManager * selectorcuts, const unsigned int & finalstate ) ;
		//! Destructor 
		virtual ~AnalysisWZ();

	protected:
		virtual void Initialise();
		virtual std::pair<unsigned int,float> InsideLoop();

	private:
		AnalysisWZ();
		//! Check if Zmass range is the defined when is signal
		const bool isSignalMCInsideZmassRange(const float & masslow,const float & masshigh) const;
		//! Stores histos and datamembers related with MC-truth information, just for signal case
		//! It returns the number of electron and muons leptons (taking into account the 
		//! taus decaying to them)
		const unsigned int setSignalMCInfo(const double & eventweight);
		//! MC Truth check (just for MC samples): a reco lepton matches with a
		//! lepton not coming from a W/Z
		const bool matchedNonPromptLepton(const unsigned int & i, const LeptonTypes & flavor) const;
		// Number of no tight muons, electrons and events (Nti)
		int _nTMuons;
		int _nTElecs;
		int _nTEvents;

		ClassDef(AnalysisWZ,0);

};

#endif

