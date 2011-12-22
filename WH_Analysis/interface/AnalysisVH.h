#ifndef ANALYSISVH_H
#define ANALYSISVH_H 1

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

class AnalysisVH : public AnalysisBase
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
			fHMinDeltaRLp1Lp2,        //Smallest DeltaR between 2 opp. sign leptons    |
			fHMaxDeltaRLp1Lp2,         //Largest DeltaR between 2 opp. sign leptons    |_ After JetVeto cut
			fHMinDeltaPhiLp1Lp2,      //Smallest Delta phi between 2 opp. sign leptons |
			fHMaxDeltaPhiLp1Lp2,      //Largest Delta phi between 2 opp. sign leptons  |
			fHLeptonCharge,           //Sum charges of leptons before 2 opp. sign leptons cut
			fHHInvMassAfterJetVeto,   //Invariant mass of leptons from H after JetVeto cut
			fHHInvMass,               //Invariant mass of leptons supposedly from H after all cuts
			fHMET,                    //Missing ET after all cuts
			fHMETAfterJetVeto,        // MET after jet veto cut
			fHNJets,                  // Number of Jets before the jet veto
			fHTransversMass,            // Transverse mass between all leptons and MET after all cuts
			fHTransversMassAfterJetVeto,// Transverse mass between all leptons and MET after jet veto cut
			fHTrileptonMass,            //Invariant mass of the 3 leptons after all cuts
			fHTrileptonMassAfterJetVeto,//Invariant mass of the 3 leptons after jet veto
			fHHT,                        //HT = sum of all transverse energy in the event
			fHHTAfterJetVeto            //HT after jet veto
		};

		//! Constructor (TO BE DEPRECATED: tree)
		AnalysisVH( TreeManager * data, std::map<LeptonTypes,InputParameters*> ipmap, 
				CutManager * selectorcuts, const unsigned int & finalstate );
		//! Destructor
		virtual ~AnalysisVH();


	protected:
		virtual void Initialise();
		virtual unsigned int InsideLoop(); // = 0;

	private:
		AnalysisVH();
		
		ClassDef(AnalysisVH,0);

};

#endif

