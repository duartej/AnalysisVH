///////////////////////////////////////////////////////////////////////
//
//    FILE: CMSAnalysisSelector.h
//   CLASS: CMSAnalysisSelector
// AUTHORS: A.Y. Rodriguez Marrero, I. Gonzalez Caballero
//    DATE: 2010
//
// CONTENT: Extensions to the TSelector Class to deal with our trees
//
///////////////////////////////////////////////////////////////////////

#ifndef CMSAnalysisSelector_h
#define CMSAnalysisSelector_h

// sytem
#include<vector>

// ROOT includes
#include <TROOT.h>
#include <TChain.h>
#include <TFile.h>
#include <TSelector.h>

// Forward references for ROOT classes
class TH1F;
class TH1D;
class TH2F;
class TTree;
class TProfile;


// Framework includes that might be also moved to the
// implementation file
#include "TCounterUI.h"
#include "InputParameters.h"

class CMSAnalysisSelector : public TSelector {
 public :
  /////////////////////////////////////////////////////////
  //Methods from TSelector
  CMSAnalysisSelector(TTree * /*tree*/ =0):fNEventsProcessed(0) { }
  virtual ~CMSAnalysisSelector() { }
  virtual Int_t   Version() const { return 2; }
  virtual void    Begin(TTree *tree);
  virtual void    SlaveBegin(TTree *tree);
  virtual Bool_t  Notify();
  virtual Bool_t  Process(Long64_t entry);
  virtual Int_t   GetEntry(Long64_t entry, Int_t getall = 0) { return fChain ? fChain->GetTree()->GetEntry(entry, getall) : 0; }
  virtual void    SetOption(const char *option) { fOption = option; }
  virtual void    SetObject(TObject *obj) { fObject = obj; }
  virtual void    SetInputList(TList *input) { fInput = input; }
  virtual TList  *GetOutputList() const { return fOutput; }
  virtual void    SlaveTerminate();
  virtual void    Terminate();
  //
  //////////////////////////////////////////////////////////////
  
  //////////////////////////////////////////////////////////////
  // Methods to create counters and histograms
  TCounterUI *InitCounterUI(const char* name, const char* title,
			    unsigned int init);

  //TTree
  TTree* CreateTree(const char* name, const char* title);

  //TH1F
  TH1F* CreateH1F(const char* name, const char* title,
                  Int_t nbinsx, Axis_t xlow, Axis_t xup);
  TH1F* CreateH1F(const char* name, const char* title,
                  Int_t nbinsx, Float_t* bins);
  
  //TH1D
  TH1D* CreateH1D(const char* name, const char* title,
                  Int_t nbinsx, Axis_t xlow, Axis_t xup);
  TH1D* CreateH1D(const char* name, const char* title,
                  Int_t nbinsx, Double_t* bins);

  //TH2F
  TH2F* CreateH2F(const char* name, const char* title,
                  Int_t nbinsx, Axis_t xlow, Axis_t xup,
                  Int_t nbinsy, Axis_t ylow, Axis_t yup);
  TH2F* CreateH2F(const char* name, const char* title,
		  Int_t nbinsx, Float_t* xbins,
		  Int_t nbinsy, Float_t* ybins);

  //TProfile
  TProfile* CreateProfile(const char* name, const char* title,
			  Int_t nbinsx, Axis_t xlow, Axis_t xup,
			  Axis_t ylow, Axis_t yup);

  //
  //////////////////////////////////////////////////////////////
  
  // Reflexion: to extract methods needed to be implemented
  // in the concrete selector
  inline static std::vector<std::string> getmethods()
  {
	  std::vector<std::string> methods;

          methods.push_back("virtual std::vector<float>* GetMuonPt() const");
          methods.push_back("virtual std::vector<float>* GetMuondeltaPt() const");
          methods.push_back("virtual std::vector<float>* GetMuonPx() const");
          methods.push_back("virtual std::vector<float>* GetMuonPy() const");
          methods.push_back("virtual std::vector<float>* GetMuonPz() const");
          methods.push_back("virtual std::vector<float>* GetMuonEnergy() const");
         
          // Isolation
          methods.push_back("virtual std::vector<float>* GetMuonSumIsoTrack() const");
          methods.push_back("virtual std::vector<float>* GetMuonSumIsoCalo() const");
         
          // Position
          methods.push_back("virtual std::vector<float>* GetMuonvx() const");
          methods.push_back("virtual std::vector<float>* GetMuonvy() const");
          methods.push_back("virtual std::vector<float>* GetMuonvz() const");
          
          // IP, PV stuff
          methods.push_back("virtual std::vector<float>* GetMuonIP2DInTrack() const");
          methods.push_back("virtual std::vector<float>* GetMuonIP2DBiasedPV() const");
          methods.push_back("virtual std::vector<float>* GetMuonIP2DUnBiasedPV() const");
          methods.push_back("virtual std::vector<float>* GetMuondzPVBiasedPV() const");
          methods.push_back("virtual std::vector<float>* GetMuondzPVUnBiasedPV() const");
         
          // Identification 
          methods.push_back("virtual std::vector<bool>* IsGlobalMuon() const");
          methods.push_back("virtual std::vector<bool>* IsAllTrackerMuons() const");
          methods.push_back("virtual std::vector<bool>* IsAllStandAloneMuons() const");
          methods.push_back("virtual std::vector<bool>* IsTMLastStationTight() const");
          
          // Quality
          methods.push_back("virtual std::vector<int>* GetMuonNValidHitsSATrk() const");
          methods.push_back("virtual std::vector<int>* GetMuonNumOfMatches() const");
          methods.push_back("virtual std::vector<int>* GetMuonNValidPixelHitsInTrk() const");
          methods.push_back("virtual std::vector<int>* GetMuonNValidHitsInTrk() const");
          methods.push_back("virtual std::vector<float>* GetMuonNormChi2GTrk() const");
         
          //-- Vertex
          methods.push_back("virtual std::vector<float>* GetVertexz() const");
         
          // General
          methods.push_back("virtual int GetEventEventNumber() const");

	  return methods;
  }

/*
  // ### TO BE IMPLEMENTED IN CONCRETE SELECTOR ###
  //////////////////////////////////////////////////////////////
  // Getters methods for the datamembers
  //-- Muons
  // Energy and momentum
  virtual std::vector<float>* GetMuonPt() const = 0;
  virtual std::vector<float>* GetMuondeltaPt() const = 0;
  virtual std::vector<float>* GetMuonPx() const = 0;
  virtual std::vector<float>* GetMuonPy() const = 0;
  virtual std::vector<float>* GetMuonPz() const = 0;
  virtual std::vector<float>* GetMuonEnergy() const = 0;

  // Isolation
  virtual std::vector<float>* GetMuonSumIsoTrack() const = 0;
  virtual std::vector<float>* GetMuonSumIsoCalo() const = 0;

  // Position
  virtual std::vector<float>* GetMuonvx() const = 0;
  virtual std::vector<float>* GetMuonvy() const = 0;
  virtual std::vector<float>* GetMuonvz() const = 0;
  
  // IP, PV stuff
  virtual std::vector<float>* GetMuonIP2DInTrack() const = 0;
  virtual std::vector<float>* GetMuonIP2DBiasedPV() const = 0;
  virtual std::vector<float>* GetMuonIP2DUnBiasedPV() const = 0;
  virtual std::vector<float>* GetMuondzPVBiasedPV() const = 0;
  virtual std::vector<float>* GetMuondzPVUnBiasedPV() const = 0;

  // Identification 
  virtual std::vector<bool>* IsGlobalMuon() const = 0;
  virtual std::vector<bool>* IsAllTrackerMuons() const = 0;
  virtual std::vector<bool>* IsAllStandAloneMuons() const = 0;
  virtual std::vector<bool>* IsTMLastStationTight() const = 0;
  
  // Quality
  virtual std::vector<int>* GetMuonNValidHitsSATrk() const = 0;
  virtual std::vector<int>* GetMuonNumOfMatches() const = 0;
  virtual std::vector<int>* GetMuonNValidPixelHitsInTrk() const = 0;
  virtual std::vector<int>* GetMuonNValidHitsInTrk() const = 0;
  virtual std::vector<float>* GetMuonNormChi2GTrk() const = 0;

  //-- Vertex
  virtual std::vector<float>* GetVertexz() const = 0;

  // General
  virtual int GetEventEventNumber() const = 0;
  // ### END TO BE IMPLEMENTED IN CONCRETE SELECTOR ### */
  
  //////////////////////////////////////////////////////////////
  // Methods to retrieve the objects at the end of the run
  TObject* FindOutput(TString name, TString classname = "");
  TObject* FindInput(TString name, TString classname = "");
  
  TObject* FindOutputByClassName(TString classname);
  //
  /////////////////////////////////////////////////////////////
  
  //////////////////////////////////////////////////////////////
  // Methods related to input parameters
  InputParameters* GetInputParameters() { return fInputParameters;}
  //
  /////////////////////////////////////////////////////////////
  


 protected: 
  //////////////////////////////////////////////////////////////
  // To be implemented in the user analysis...
  //
  virtual void InitialiseParameters() {}
  virtual void Initialise() = 0;
  virtual void InsideLoop() = 0;
  virtual void SetDataMembersAtTermination() {} //OBSOLETE
  virtual void Summary() = 0;
  //
  /////////////////////////////////////////////////////////////
  



 public:
  // From TSelector...
  TTree          *fChain;   //!pointer to the analyzed TTree or TChain


 protected:
  InputParameters* fInputParameters;
  TCounterUI*      fNEventsProcessed;



  ClassDef(CMSAnalysisSelector,0);

};

#endif


#ifdef CMSAnalysisSelector_cxx
Bool_t CMSAnalysisSelector::Notify()
{
   // The Notify() function is called when a new file is opened. This
   // can be either for a new TTree in a TChain or when when a new TTree
   // is started when using PROOF. It is normally not necessary to make changes
   // to the generated code, but the routine can be extended by the
   // user if needed. The return value is currently not used.

   return kTRUE;
}

#endif // #ifdef CMSAnalysisSelector_cxx
