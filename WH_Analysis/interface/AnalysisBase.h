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
#include "FOManager.h"

#include "TString.h"
#include "TLorentzVector.h"


class InputParameters;
class PUWeight;
class TTree;
class CutManager;
class TH1D;
class FOManager;




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

class AnalysisBase : public CMSAnalysisSelector 
{
	public: 
		//! For histograms: Must be common all analysis
		enum 
		{
			fHProcess,                //Process ID
			fHEventsPerCut,           //Events passing every cut
			fHEventsPerCut3Lepton,    //Events passing every cut that are 3 lepton from gen
			fHNPrimaryVertices,       //Number of primary vertices
			nextenumerate

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
		virtual unsigned int InsideLoop() = 0;
		//! Method to fill the tree which contain the cut number used per event
		//! (to be fill with the return value of InsideLoop method and inside the
		//! parent method Process)
		virtual void StoresCut(const unsigned int & cut);
		virtual void Summary();

	private:
		AnalysisBase();


	protected: 
		bool IspassHLT() const;
		//! Extract the 4-momenta of the good leptons. WARNING only can be used with
		//! the good leptons
		//const TLorentzVector GetTLorentzVector( const int & index ) const;
		// Overloaded for other objects than leptons
		const TLorentzVector GetTLorentzVector( const char * namep, 
				const int & index ) const;

		//unsigned int GetFSID( const unsigned int & nelecs, const unsigned int & nmuons,
		//		const unsigned int & ntaus ) const ;
		//! Methods to fill histograms
		virtual void FillHistoPerCut(const unsigned int & cut,const double & puw, 
				const unsigned int & fs); // = 0;  //FIXME de momento
		virtual void FillGenPlots(const unsigned int & cut, double puw); 




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

		// PU Weight utility
		//----------------------------------------------------------------------------
		PUWeight* fPUWeight;		

		// Fakeable object manager, if it is needed...
		FOManager * fFO;

		// Tree containing the last cut used in a given event
		TTree * _cuttree;
		// Value of the cut
		unsigned int _cutvalue;

		int _eventnumber;

		// Histograms FIXME: 3 --> nLeptons and to a vector or map: { # id : TH1D }
		//                         y map: { #id : { # corte: TH1D } }
		//----------------------------------------------------------------------------
		// PROVISONAL : FIXME
		std::map<int,std::vector<TH1D*> > fHGenPtLepton;   //PT 1st/2nd/3rd energetic gen muon from W or tau
		std::map<int,std::vector<TH1D*> > fHGenEtaLepton;  //Eta 1st/2nd/3rd energetic gen muons from W or tau
		TH1D * fHPtLepton[3];               //Pt of 1st/2nd/3rd good isolated muon
		TH1D * fHEtaLepton[3];              //Eta of 1st/2nd/3rd good isolated muon
		TH1D * fHDeltaRGenRecoLepton[3];    //DeltaR between reco and gen muons

		// -- Histograms
		std::map<int,TH1D*> _histos;
		
		// -- It is was stored the ouput
		bool fWasStored;


	public:
		ClassDef(AnalysisBase,0);

};

#endif

