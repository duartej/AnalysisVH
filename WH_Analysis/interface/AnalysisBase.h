#ifndef ANALYSISBASE_H
#define ANALYSISBASE_H 1

// Blabla description

#include<vector>
#include<map>
#include<cstring>

#include "CMSAnalysisSelector.h"
#include "TreeManager.h"
//#include "CutLevels.h"
#include "LeptonTypes.h"
#include "SignatureFS.h"
#include "WManager.h"

#include "TString.h"
#include "TLorentzVector.h"
// FIXME: PROVISONAL
#include "TH2F.h"


class InputParameters;
class PUWeight;
class TTree;
class CutManager;
class TH1D;
class WManager;



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
                         "Z/W+Jets, WZ+Jets and ttbar",
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
struct EvtInfo
{
        int run;
        int lumi;
        int evt;
        int channel;

        double zmass;
        double zlep1pt;
        double zlep1eta;
        double zlep1phi;
        double zlep2pt;
        double zlep2eta;
        double zlep2phi;

        double wmt;
        double wleppt;
        double wlepeta;
        double wlepphi;

        double metet;
        double metphi;
};





class AnalysisBase : public CMSAnalysisSelector 
{
	public: 
		//! For histograms: Must be common all analysis
		enum 
		{
			fHProcess,                      //Process ID
			fHEventsPerCut,                 //Events passing every cut
			fHEventsPerCut3Lepton,          //Events passing every cut that are 3 lepton from gen
			fHNPrimaryVertices,             //Number of primary vertices just after be PU-weighted 
			fHNPrimaryVerticesAfter3Leptons,//Idem but after exactly NLeptons cut
			nextenumerate

		};

		//! Systematic types
		enum
		{
			LEPTONSYS=10,       // Scale factors related systematics
			FRSYS,		    // Fake rates+-1sigma variation of its value
			MMSSYS,		    // Muon Momentum scale systematic
			EESSYS,             // Electron Energy scale systematic
			METSYS,		    // MET scale and resolution systematic
			PUSYS		    // Pile up variation systematic
		};

		//! Constructor
		AnalysisBase( TreeManager * data, std::map<LeptonTypes,InputParameters*> ipmap, 
				CutManager * selectorcuts, const unsigned int & finalstate );
		//! Destructor
		virtual ~AnalysisBase();

		//! method to store the results to a file
		void SaveOutput( const char * outputname = 0 );

	protected:
		virtual void InitialiseParameters();
		virtual void Initialise() = 0;
		virtual std::pair<unsigned int,float> InsideLoop() = 0;
		//! Method to fill the tree which contain the cut number used per event
		//! (to be fill with the return value of InsideLoop method and inside the
		//! parent method Process)
		virtual void StoresCut(const unsigned int & cut,const float & weight);
		//! Method to fill the tree containing relevant event information for the 
		//! selected events
		virtual void StoresEvtInf(const TLorentzVector & zcand1, const TLorentzVector & zcand2, 
				const TLorentzVector & wcand,
				const double & transversmass, const TLorentzVector & METV);
		virtual void Summary();

		//! Method to extract the trigger weight 
		double GetTriggerWeight(const std::vector<LeptonRel> * const theleptons) const;

		//! Encapsulates the MET object (needed to deal with its systematic)
		//! If the analysis is being done with the METSYS activated calculates
		//! the corrected MET and return it, otherwise uses the branch in the Tree
		const double GetMET(const std::vector<LeptonRel> * const theLeptons) const;

		//! Methods to evaluate the fakeable object method and its PPF estimation 
		//! PPF estimation using the approximations p=1 and f->0
		double GetPPFWeightApprx(const bool & iszjetregion = false);
		//! PPP estimation evaluated using the full calculation
		double GetPPPWeight(const bool & zjetsregion = false);
		//! PPF estimation evaluated using the full calculation
		double GetPPFWeight(const bool & zjetsregion = false);
		//! PFF estimation evaluated using the full calculation
		double GetPFFWeight(const bool & zjetsregion = false);
		//! FFF estimation evaluated using the full calculation
		double GetFFFWeight(const bool & zjetsregion = false);

	private:
		AnalysisBase();
		//! Auxiliary function to set-up the systematic related stuff
		bool initializeSys(const std::string & variation);

	protected: 
		bool IspassHLT() const;
		//! Extract the 4-momenta of object "namep".
		const TLorentzVector GetTLorentzVector( const char * namep, 
				const int & index ) const;
		//! Overloaded to extract the momentum with momentum/energy systematic 
		//const TLorentzVector GetTLorentzVector( const char * namep, 
		//		const int & index, const int & leptontype ) const;

		//! Methods to fill histograms
		virtual void FillHistoPerCut(const unsigned int & cut,const double & puw, 
				const unsigned int & fs); // = 0;  //FIXME de momento
		virtual void FillGenPlots(const unsigned int & cut, double puw); 


		//! Number of final state leptons
		unsigned int _nLeptons;
		
		//! Variables describing dataset
		// -----------------------------------------------------------------------
		TString fDataName; // Dataset Name
		bool    fIsData;   // True if it should be treated as data
		bool    fIsWH;     // True if it is WH MC sample
		bool    fIsWZ;     // True if it is WZ MC sample
		bool    fIsSignal; // True if it should be treated as signal (MC-like)
		std::vector<std::string> _datafiles; // Complete path to the files
		
		//! Luminosity: 
		//! FIXME: Possibly to the mother since has to be used for each
		//----------------------------------------------------------------------------
		double fLuminosity;

		//! Run Period 
		std::string fRunPeriod;

		//! Signature of the analysis (giving by Signature::EFS enum)
		unsigned int fFS;

		//! Number of total cuts in the analysis
		unsigned int fNCuts;
		std::vector<TString> fCutNames;

		//! Type of leptons in the analysis
		LeptonTypes fLeptonType;                
		std::vector<std::string> fLeptonName;

		// Leptons at generation
		//----------------------------------------------------------------------------
		unsigned int fNGenElectrons;           //Number of generated electrons from W or tau
		unsigned int fNGenMuons;               //Number of generated muons from W or tau
		unsigned int fNGenLeptons;             //A reference to the two Gen Lepton of the analysis (one of the two last data members)
		std::vector<TLorentzVector> fGenLepton;//TLorentzVector with the 3 muons from W or tau
		std::vector<std::string> fGenLeptonName;

		//! PU Weight utility
		//----------------------------------------------------------------------------
		PUWeight* fPUWeight;		

		//! Fake Rate weights
		WManager * fFO;
		
		//! Fake Rate weights for ZJets region, i.e. all the analysis up to the MET cut
		WManager * fFOZJetsRegion;
		
		//! Prompt Rate weights
		WManager * fPO;

		//! Scale factors
		WManager * fSF;

		//! Trigger efficiencies: leading leg
		WManager * fTRL;
		//! Trigger efficiencies: trailing leg
		WManager * fTRT;

		//! Tree containing the last cut used in a given event
		TTree * _cuttree;
		//! Weight of a given event
		float _cutweight;
		//! Value of the cut
		unsigned int _cutvalue;
		int _eventnumber;
		int _runnumber;

		//! Tree to store some useful info for the selected events
		TTree * _evtlisttree;
		EvtInfo _evtinfo;

		//! W Charge sign, when want to distinguish between them. 
		int _wcharge;
		
		//! Jet name (it depends of the run period)
		std::string _jetname;

		//! Systematics info
		bool _issysrun;
		std::string _namesys;
		int _typesys; 
		int _modesys;
		//! systematic pt
		float _mptsys;
		float _eptbarrelsys;
		float _epteesys;
		//! systematic met (1 for up, -1 for down)
		int _metsysmode;
		float _metmuonpt;
		float _metelecpt;
		float _metjetspt;
		//! systematic pu
		float _pusys;


		// Histograms FIXME: 3 --> nLeptons and to a vector or map: { # id : TH1D }
		//                         y map: { #id : { # corte: TH1D } }
		//----------------------------------------------------------------------------
		// PROVISONAL : FIXME
		std::map<int,std::vector<TH1D*> > fHGenPtLepton;   //PT 1st/2nd/3rd energetic gen muon from W or tau
		std::map<int,std::vector<TH1D*> > fHGenEtaLepton;  //Eta 1st/2nd/3rd energetic gen muons from W or tau
		TH1D * fHPtLepton[3];               //Pt of 1st/2nd/3rd good isolated muon
		TH1D * fHEtaLepton[3];              //Eta of 1st/2nd/3rd good isolated muon
		TH1D * fHDeltaRGenRecoLepton[3];    //DeltaR between reco and gen muons

		//! Histograms
		std::map<int,TH1D*> _histos;

		// ! if it was stored the ouput
		bool fWasStored;

		// FIXME: Provisional
		TH2F * fHPTETA_NOTIGHTS;
		TH2F * fHPTETA_NOTIGHTS_WEIGHTED;
		

	public:
		ClassDef(AnalysisBase,0);

};

#endif

