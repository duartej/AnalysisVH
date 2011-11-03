#ifndef ANALYSISVH_H
#define ANALYSISVH_H 1

// Blabla description

#include<vector>
#include<cstring>

#include "CMSAnalysisSelector.h"
#include "TreeManager.h"
#include "CutLevels.h"

#include "TString.h"
#include "TLorentzVector.h"


class InputParameters;
class PUWeight;
class TTree;
class CutManager;
class TH1D;




// FIXME (millor lloc es aquest?)
//---------------------------------------------------------------------
// Some helper enums and consts
//---------------------------------------------------------------------
enum {_iWH,        //0 Higgs
      _iZH,        //1 Higgs
      _ittH,       //2 Higgs

      _iZZ,        //3 Diboson
      _iWZ,        //4 Diboson
      _iWW,        //5 Diboson

      _iVarious,   //6 Z + jets/W + Jets/Ttbar

      _iOther,     //N-1

      _iNProcesses //N
};

// + Gen Final States (incl taus)
// CODE: Nelectrons*1000+Nmuons*100+Ntaus*10
enum {
  _iFSeee, // = 3000,   // 3 electrons
  _iFSmmm, // = 300,    // 3 muons
  _iFSttt, // = 30,     // 3 taus
  _iFSeem, // = 2100,   // 2 electrons 1 muon
  _iFSeet, // = 2010,   // 2 electrons 1 tau
  _iFSmme, // = 1200,   // 1 electron  2 muons
  _iFSmmt, // = 210,    // 2 muons     1 tau
  _iFStte, // = 1020,   // 1 electron  2 taus
  _iFSttm, // = 120,    // 1 muon      2 taus
  _iFSemt, // = 1110,   // 1 electron  1 muon 1 tau
  _iFSunknown, // = 0,  // Something went wrong
  _iFStotal // = 11    // Remember to change everytime you add/remove on
};


const TString kProcesses[] = { "WH",
                         "ZH",
                         "ttH",
                         "ZZ",
                         "WZ",
                         "WW",
                         "Z/W+Jets and ttbar",
                         "Other" };

const TString kFinalStates[] = {
  "e e e",
  "#mu #mu #mu",
  "#tau #tau #tau",
  "e e #mu",
  "e e #tau",
  "#mu #mu e",
  "#mu #mu #tau",
  "#tau #tau e",
  "#tau #tau #mu",
  "e #mu #tau",
  "Unknown"
};


class AnalysisVH : public CMSAnalysisSelector 
{
	public: 
		//! For histograms
		enum { fHProcess , fHGenFinalState , fHGenFinalStateNoTaus, fHNGenWMuons,
			fHEventsPerCut, fHEventsPerCut3Mu, fHNRecoMuons, fHNSelectedMuons,
			fHNSelectedPVMuons, fHNSelectedIsoMuons, fHNSelectedIsoGoodMuons 
		};

		// State prepare analisis
		AnalysisVH( TreeManager * data, InputParameters * ip, 
				CutManager * selectorcuts, TTree * tree );
		// State runanalisis
		//AnalysisVH( CMSAnalysisSelector * consel) : _cmsselector(conselr) { } 
		virtual ~AnalysisVH();

	protected:
	//	TreeManager * _data; 
		
		virtual void InitialiseParameters();
		virtual void Initialise();
		virtual void InsideLoop();
		virtual void Summary();

	private:
		AnalysisVH();

		unsigned int GetFSID( const unsigned int & nelecs, const unsigned int & nmuons,
				const unsigned int & ntaus ) const ;
		//! Methods to fill histograms
		void FillHistoPerCut(const ECutLevel & cut,const double & puw, 
				const unsigned int & fs);
		void FillGenPlots(ECutLevel cut, double puw); 

		// Number of final state leptons
		unsigned int _nLeptons;
		
		// Variables describing dataset
		// -----------------------------------------------------------------------
		TString fDataName; // Dataset Name
		bool    fIsData;   // True if it should be treated as data
		bool    fIsWH;     // True if it should be treated as signal (WH)
		std::vector<std::string> _datafiles; // Complete path to the files
		
		// Luminosity: FIXME: Possibly to the mother since has to be used for each
		//----------------------------------------------------------------------------
		double fLuminosity;

		// Leptons at generation
		//----------------------------------------------------------------------------
		unsigned int fNGenElectrons;             //Number of generated electrons from W or tau
		unsigned int fNGenMuons;                 //Number of generated muons from W or tau
		std::vector<TLorentzVector> * fGenMuon;  //TLorentzVector with the 3 muons from W or tau


		// Cut selections --> FIXME: This has to be included in the CMSAnalysisSelector
		CutManager * _selectioncuts;
		
		// PU Weight utility
		//----------------------------------------------------------------------------
		PUWeight* fPUWeight;		

		// FIXME: NECESARIO???
		TTree * _tree;

		// Histograms FIXME: 3 --> nLeptons
		//----------------------------------------------------------------------------
		//TH1D* fHProcess;               //Process ID
		//TH1D* fHGenFinalState;         //Final state from generation (incl. taus)
		//TH1D* fHGenFinalStateNoTaus;   //Final state from generation (no taus)
		//TH1D* fHNGenWMuons;            //Generated muons coming from a W
		TH1D* fHGenPtMu[3][_iNCuts];   //PT 1st/2nd/3rd energetic gen muon from W or tau
		TH1D* fHGenEtaMu[3][_iNCuts];  //Eta 1st/2nd/3rd energetic gen muons from W or tau
		//TH1D* fHEventsPerCut;          //Events passing every cut
		//TH1D* fHEventsPerCut3Mu;       //Events passing every cut that are 3 mu from gen
		//TH1D* fHNRecoMuons;            //Reconstructed muons in the event
		//TH1D* fHNSelectedMuons;        //Muons passing the basic selection
		//TH1D* fHNSelectedPVMuons;      //Selected Muons close to the PV
		//TH1D* fHNSelectedIsoMuons;     //Selected Isolated Muons
		//TH1D* fHNSelectedIsoGoodMuons; //Selected Isolated Good Muons
		TH1D* fHPtMu[3];               //Pt of 1st/2nd/3rd good isolated muon
		TH1D* fHEtaMu[3];              //Eta of 1st/2nd/3rd good isolated muon
		TH1D* fHDeltaRGenRecoMu[3];    //DeltaR between reco and gen muons
		TH1D* fHMinDeltaRMuMu;         //Smallest DeltaR between 2 opp. sign leptons
		TH1D* fHMaxDeltaRMuMu;         //Largest DeltaR between 2 opp. sign leptons
		TH1D* fHMinDeltaPhiMuMu;       //Smallest Delta phi between 2 opp. sign leptons
		TH1D* fHMaxDeltaPhiMuMu;       //Largest Delta phi between 2 opp. sign leptons
		TH1D* fHMuonCharge;            //Sum charges of muons
		TH1D* fHHInvMass;              //Invariant mass of leptons supposedly from H
		TH1D* fHZInvMass;              //Invariant mass of leptons in/out of Z peak
		TH1D* fHMET;                   //Missing ET after inv mass cut

		// FIXME ----
		std::map<int,TH1D*> _histos;


	public:
		ClassDef(AnalysisVH,0);

};

#endif

