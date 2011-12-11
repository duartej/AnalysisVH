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
class TreeManager;


// Framework includes that might be also moved to the
// implementation file
#include "TCounterUI.h"
#include "InputParameters.h"
#include "CutManager.h"

class CMSAnalysisSelector : public TSelector {
 public :
  /////////////////////////////////////////////////////////
  //Methods from TSelector
  CMSAnalysisSelector(TreeManager * data): 
	  TSelector(), fData(data), fInputParameters(0), 
	  fNEventsProcessed(0), fLeptonSelection(0) { }
  virtual ~CMSAnalysisSelector() { }
  virtual Int_t   Version() const { return 2; }
  //virtual void    Begin(TTree *tree);
  virtual void    Init(TTree *tree);
  virtual void    SlaveBegin(TTree *tree);
  virtual Bool_t  Notify();
  virtual Bool_t  Process(Long64_t entry);
  //virtual Int_t   GetEntry(Long64_t entry, Int_t getall = 0) { return fChain ? fChain->GetTree()->GetEntry(entry, getall) : 0; }
  virtual void    SetOption(const char *option) { fOption = option; }
  virtual void    SetObject(TObject *obj) { fObject = obj; }
  virtual void    SetInputList(TList *input) { fInput = input; }
  virtual TList  *GetOutputList() const { return fOutput; }
  virtual TList  *GetInputList() const { return fInput; }
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
  virtual unsigned int InsideLoop() = 0;
  virtual void StoresCut(const unsigned int & cut) = 0 ;
  virtual void Summary() = 0;
  //
  /////////////////////////////////////////////////////////////
  



 protected:
  // From TSelector...
 // TTree          *fChain;   //!pointer to the analyzed TTree or TChain
  TreeManager   *fData;    //!pointer to the data


 protected:
  //! type of the leptons -> InputParameters
  InputParameters * fInputParameters;
  TCounterUI*      fNEventsProcessed;
  CutManager *     fLeptonSelection;   // FIXME: Podria ser un map<enum lepton type,manager>??



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
