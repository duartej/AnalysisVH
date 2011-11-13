#ifndef ANALYSISVH_H
#define ANALYSISVH_H 1

// Blabla description

#include<vector>
#include<cstring>

#include "CMSAnalysisSelector.h"
#include "TreeManager.h"
#include "CutLevels.h"
#include "LeptonTypes.h"
#include "SignatureFS.h"

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
/*enum {
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
};*/


const TString kProcesses[] = { "WH",
                         "ZH",
                         "ttH",
                         "ZZ",
                         "WZ",
                         "WW",
                         "Z/W+Jets and ttbar",
                         "Other" };

/*const TString kFinalStates[] = {
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
*/

class AnalysisVH : public CMSAnalysisSelector 
{
	public: 
		//! For histograms
		enum {  fHProcess,                //Process ID
			fHGenFinalState, 	  //Final state from generation (incl. taus)
			fHGenFinalStateNoTaus,    //Final state from generation (no taus)
			fHNGenWLeptons,		  //Generated leptons coming from a W
			fHEventsPerCut,           //Events passing every cut
			fHEventsPerCut3Lepton,    //Events passing every cut that are 3 lepton from gen
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
			fHMET                     //Missing ET after inv mass cut
		};

		//! Constructor (TO BE DEPRECATED: tree)
		AnalysisVH( TreeManager * data, InputParameters * ip, 
				CutManager * selectorcuts, const unsigned int & finalstate );
		//! Destructor
		virtual ~AnalysisVH();

	protected:
		virtual void InitialiseParameters();
		virtual void Initialise();
		virtual void InsideLoop(); // = 0;
		virtual void Summary();

	private:
		AnalysisVH();


	protected: // TO BE CHANGED --> PRIVATE
		const TLorentzVector GetTLorentzVector( const int & index ) const;
		// Overloaded for other objects than leptons
		const TLorentzVector GetTLorentzVector( const char * namep, 
				const int & index ) const;

		//unsigned int GetFSID( const unsigned int & nelecs, const unsigned int & nmuons,
		//		const unsigned int & ntaus ) const ;
		//! Methods to fill histograms
		virtual void FillHistoPerCut(const ECutLevel & cut,const double & puw, 
				const unsigned int & fs); // = 0;  //FIXME de momento
		virtual void FillGenPlots(ECutLevel cut, double puw); //= 0; //FIXME. de momento 

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

		//! Signature of the analysis (giving by Signature::EFS enum)
		unsigned int fFS;

		//! Type of leptons in the analysis
		LeptonTypes fLeptonType;                
		const char * fLeptonName;             // Name used in the TTree (Muon, Elec)

		// Leptons at generation
		//----------------------------------------------------------------------------
		unsigned int fNGenElectrons;           //Number of generated electrons from W or tau
		unsigned int fNGenMuons;               //Number of generated muons from W or tau
		unsigned int * fNGenLeptons;           //A reference to the Gen Lepton of the analysis (one of the two last data members)
		std::vector<TLorentzVector> fGenLepton;//TLorentzVector with the 3 muons from W or tau

		// PU Weight utility
		//----------------------------------------------------------------------------
		PUWeight* fPUWeight;		

		// FIXME: NECESARIO???
	//	TTree * _tree;

		// Histograms FIXME: 3 --> nLeptons and to a vector or map: { # id : TH1D }
		//                         y map: { #id : { # corte: TH1D } }
		//----------------------------------------------------------------------------
		TH1D* fHGenPtLepton[3][_iNCuts];   //PT 1st/2nd/3rd energetic gen muon from W or tau
		TH1D* fHGenEtaLepton[3][_iNCuts];  //Eta 1st/2nd/3rd energetic gen muons from W or tau
		TH1D* fHPtLepton[3];               //Pt of 1st/2nd/3rd good isolated muon
		TH1D* fHEtaLepton[3];              //Eta of 1st/2nd/3rd good isolated muon
		TH1D* fHDeltaRGenRecoLepton[3];    //DeltaR between reco and gen muons

		// -- Histograms
		std::map<int,TH1D*> _histos;
		//std::map<int,std::map<int,TH1D*> > _histos3Leptons; / { nombre_enum: { #lepton: TH1D* , .. } ..
		//std::map<int,std::map<int, std::map<int,TH1D*> > > _histos3LeptonsPerCut; / { nombre_enum: { #lepton: { # corte: TH1D* ,  .. } .. } 


	public:
		ClassDef(AnalysisVH,0);

};

#endif

