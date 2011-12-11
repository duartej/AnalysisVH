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
			fHMinDeltaRLp1Lp2,        //Smallest DeltaR between 2 opp. sign leptons
			fHMaxDeltaRLp1Lp2,         //Largest DeltaR between 2 opp. sign leptons
			fHMinDeltaPhiLp1Lp2,      //Smallest Delta phi between 2 opp. sign leptons
			fHMaxDeltaPhiLp1Lp2,      //Largest Delta phi between 2 opp. sign leptons
			fHLeptonCharge,           //Sum charges of leptons
			fHHInvMass,               //Invariant mass of leptons supposedly from H
			fHZInvMass,               //Invariant mass of leptons in/out of Z peak
			fHMET,                     //Missing ET after inv mass cut
			fHZInvMassAfterZCand,
			fHHInvMassAfterZCand,
			fHMETAfterZCand,
			fHNJets,
			fHTransversMass,
			fHTrileptonMass,
			fHTrileptonMassAfterZCand

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

