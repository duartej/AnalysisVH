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
			fHZInvMass,               //Invariant mass of leptons in/out of Z peak after all cust
			fHZInvMassAfterZCand,     //Invariant mass of leptons in/out of Z peak
			fHZInvMassAfterWCand,     //Invariant mass of leptons in/out of Z peak
			fHMET,                    //Missing ET after all cuts
			fHMETAfterZCand,          // MET affer ZCandidate cut
			fHMETAfterWCand,
			fHNJets,                   // Number of jets after all cuts
			fHTransversMass,
			fHdRl1Wcand,
			fHdRl2Wcand
			//fHIsoLepton,
			//fHD0Lepton
			//,fHEtJetnoTightLepton
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
		int _nTMuons;
		int _nTElecs;

		ClassDef(AnalysisWZ,0);

};

#endif

