///////////////////////////////////////////////////////////////////////
//
//    FILE: CMSAnalysisSelector.C
//   CLASS: CMSAnalysisSelector
// AUTHORS: I. Gonzalez Caballero, A.Y. Rodriguez Marrero
//    DATE: 2010
//
///////////////////////////////////////////////////////////////////////

#define CMSAnalysisSelector_cxx
// This class is derived
// from the ROOT class TSelector. For more information on the TSelector
// framework see $ROOTSYS/README/README.SELECTOR or the ROOT User Manual.

// The following methods are defined in this file:
//    Begin():        called every time a loop on the tree starts,
//                    a convenient place to create your histograms.
//    SlaveBegin():   called after Begin(), when on PROOF called only on the
//                    slave servers.
//    Process():      called for each event, in this function you decide what
//                    to read and fill your histograms.
//    SlaveTerminate: called at the end of the loop on the tree, when on PROOF
//                    called only on the slave servers.
//    Terminate():    called at the end of the loop on the tree,
//                    a convenient place to draw/fit your histograms.
//

// Header include
#include "CMSAnalysisSelector.h"
#include "TreeManager.h"

// ROOT includes
#include "TTree.h"
#include "TProfile.h"
#include "TH1.h"
#include "TH2.h"

//Standard includes
#include<iostream>
#include<sstream>
#include<fstream>
#include<stdlib.h>
#include<algorithm>

// Uncomment the following line to get some debug information in the output
// Set the value to 2 to get even more debug information
//#define DEBUGCMSANALYSISSELECTOR 1

#ifdef DEBUG
#define DEBUGCMSANALYSISSELECTOR
#endif

CMSAnalysisSelector::~CMSAnalysisSelector()
{
	if( _blacklist != 0 )
	{
		delete _blacklist;
		_blacklist = 0;
	}
}

void CMSAnalysisSelector::SetBlacklistEvents(const char * blfile)
{
	// Sanity check. This function has to be called just once. Otherwise
	// Implies we want to delete the content
	if( _blacklist != 0 )
	{
		/*std::cerr << "\033[33mSetBlacklistEvents WARNING\033[m Blacklist events already"
			<< " filled before this method calling. This method should be called"
			<< " just once. Ignoring the call and using the previous blacklist.. "
			<< std::endl;*/
		return;
	}

	std::ifstream blacklistfile( blfile );
	if( ! blacklistfile.is_open() )
	{
		// There is no events to blacklist
		blacklistfile.close();
		return;
	}
	
	_blacklist = new std::vector<long int>;

	// Parse the list of RUN:EVENTID@entrynumber (each line contain a RUN:EventID@EntryNumber)
	std::string line;
	std::cout << "\033[33mBlacklisting the following events *\033[m" << std::endl;
	while( ! blacklistfile.eof() )
	{	
		getline(blacklistfile,line);
		const size_t colonpos = line.find(":");
		const size_t atpos = line.find("@");
		if( colonpos == line.npos || atpos == line.npos )
		{
			continue;
		}

		long int run    = -1;
		long int evtid  = -1;
		long int entryn = -1;
		std::stringstream runstr(line.substr(0,colonpos));
		runstr >> run;
		std::stringstream evtidstr(line.substr(colonpos+1,std::string::npos));
		evtidstr >> evtid;
		std::stringstream entrynstr(line.substr(atpos+1,std::string::npos));
		entrynstr >> entryn;

		_blacklist->push_back(entryn);
		std::cout << "Run:" << run << " EventID:" << evtid << " Entry number: " 
			<< entryn << std::endl;
	}
	std::cout << "\033[33m***********************************\033[m" << std::endl;
	blacklistfile.close();

	// Some final check,
	if( _blacklist->size() < 1 )
	{
		std::cerr << "\033[33mSetBlacklist WARNING\033[m Found a 'blacklist.evt' file but"
			<< " parsing it returns nothing. Check the format." << std::endl;
		delete _blacklist;
		_blacklist = 0;
	}
}


void CMSAnalysisSelector::Init(TTree *tree )
{
	// Calling the data init
	fData->Init(tree);
}


void CMSAnalysisSelector::SlaveBegin(TTree * /*tree*/)
{
#ifdef DEBUGCMSANALYSISSELECTOR
  std::cout << "DEBUG: ==> CMSAnalysisSelector::SlaveBegin()" << std::endl;
#endif
   // The SlaveBegin() function is called after the Begin() function.
   // When running with PROOF SlaveBegin() is called on each slave server.
   // The tree argument is deprecated (on PROOF 0 is passed).

   TString option = GetOption();

   // Find InputParameters object at the workers
   fInputParameters = ((InputParameters*) FindInput("Set Of Parameters"));

   // Initialise the number of events
   fNEventsProcessed = InitCounterUI("fNEventsProcessed",
				     "Number events processed", 0);


   //Initialise parameters here so they can be used in Initialise()
   InitialiseParameters();

   //  Call the virtual Initialise method where the user should
   //  initialise all the histograms and counters
   Initialise();

#ifdef DEBUGCMSANALYSISSELECTOR
  std::cout << "DEBUG: <== CMSAnalysisSelector::SlaveBegin()" << std::endl;
#endif
}



Bool_t CMSAnalysisSelector::Process(Long64_t entry)
{
#if (DEBUGCMSANALYSISSELECTOR >= 2 )
  std::cout << "DEBUG: ==> CMSAnalysisSelector::Process() [Entry:" << entry 
	  << "]" << std::endl;
#endif
   // The Process() function is called for each entry in the tree (or possibly
   // keyed object in the case of PROOF) to be processed. The entry argument
   // specifies which entry in the currently loaded tree is to be processed.
   // It can be passed to either CMSAnalysisSelector::GetEntry() or TBranch::GetEntry()
   // to read either all or the required parts of the data. When processing
   // keyed objects with PROOF, the object is already loaded and is available
   // via the fObject pointer.
   //
   // This function should contain the "body" of the analysis. It can contain
   // simple or elaborate selection criteria, run algorithms on the data
   // of the event and typically fill histograms.
   //
   // The processing can be stopped by calling Abort().
   //
   // Use fStatus to set the return value of TTree::Process().
   //
   // The return value is currently not used.
 
   // Check if the event is blacklisted
   if( _blacklist != 0 )
   {
	   const long int globalentry = fData->GetGlobalEntryNumber(entry);
	   if( std::find(_blacklist->begin(),_blacklist->end(),globalentry) != _blacklist->end() )
	   {
		   return kFALSE;
	   }
   }
   fData->GetEntry(entry);
   
   // Increment the number of events
   (*fNEventsProcessed)++;


   // Run the user function
   const std::pair<unsigned int,float> cut_weight= this->InsideLoop();
   
   // Stores the cut used to reject/accept the event
   this->StoresCut(cut_weight.first,cut_weight.second);

   // Resetting the used vectors
   fLeptonSelection->Reset();

#if (DEBUGCMSANALYSISSELECTOR >= 2 )
  std::cout << "DEBUG: <== CMSAnalysisSelector::Process()" << std::endl;
#endif
   return kTRUE;
}


void CMSAnalysisSelector::SlaveTerminate()
{
#ifdef DEBUGCMSANALYSISSELECTOR
  std::cout << "DEBUG: ==> CMSAnalysisSelector::SlaveTerminate()" << std::endl;
#endif
   // The SlaveTerminate() function is called after all entries or objects
   // have been processed. When running with PROOF SlaveTerminate() is called
   // on each slave server.

#ifdef DEBUGCMSANALYSISSELECTOR
  std::cout << "DEBUG: <== CMSAnalysisSelector::SlaveTerminate()" << std::endl;
#endif
}

void CMSAnalysisSelector::Terminate()
{
#ifdef DEBUGCMSANALYSISSELECTOR
  std::cout << "DEBUG: ==> CMSAnalysisSelector::Terminate()" << std::endl;
#endif

   // The Terminate() function is the last function to be called during
   // a query. It always runs on the client, it can be used to present
   // the results graphically or save the results to file.

   //Initialise parameters here so they can be used in Initialise()
   // InitialiseParameters(); ---> FIXED: BUG!!!

   // Get the number of events processed
   //fNEventsProcessed = ((TCounterUI*) FindOutput("fNEventsProcessed"));

   // Now call the method that sets the user variables at the end
   //SetDataMembersAtTermination();

   // Print the summary
   std::cout << "===================================================" << std::endl;
   std::cout << "||                    SUMMARY                    ||" << std::endl;
   std::cout << "===================================================" << std::endl;
   std::cout << "[ Analisys::Summary:  INPUT PARAMETERS ]" << std::endl << std::endl;
   if( this->fInputParameters )
   {
	   fInputParameters->DumpParms();
   }
   std::cout << std::endl << "[ Analisys::Summary: RESULTS ]" << std::endl << std::endl;
   std::cout << "------------------------------------------------- " << std::endl;
   std::cout << "+ Events processed: " << *fNEventsProcessed << std::endl;
   
   // AnalysisBase summary 
   Summary();
#ifdef DEBUGCMSANALYSISSELECTOR
   std::cout << "DEBUG: <== CMSAnalysisSelector::Terminate()" << std::endl;
#endif
}

//
///////////////////////////////////////////////////////////////////
//
TObject* CMSAnalysisSelector::FindOutput(TString name, TString classname) {
  TObject* object = 0;
  TObject* tmpobj = 0 ;
  for (int i = 0; i < fOutput->GetEntries(); i++) {
    tmpobj = fOutput->At(i);
    if (name == tmpobj->GetName())
      if (classname == "" || classname == tmpobj->IsA()->GetName()) {
        object = tmpobj;
        break;
      }
  }
  return object;
}

TObject* CMSAnalysisSelector::FindInput(TString name, TString classname) {
  TObject* object = 0;
  TObject* tmpobj = 0 ;
  if( fInput == 0 )
  {
	  std::cerr << "CMSAnalysisSelector::FindInput ERROR: "
		  << "Not initialized the input list of objects! Exiting..." << std::endl;
	  exit(-1);
  }
  for (int i = 0; i < fInput->GetEntries(); i++) {
    tmpobj = fInput->At(i);
    if (name == tmpobj->GetName())
      if (classname == "" || classname == tmpobj->IsA()->GetName()) {
        object = tmpobj;
        break;
      }
  }
  return object;
}

TObject* CMSAnalysisSelector::FindOutputByClassName(TString classname) {
  TObject* object = 0;
  TObject* tmpobj = 0 ;
  if( fOutput == 0 )
  {
	  std::cerr << "CMSAnalysisSelector::FindOutputByClass ERROR: "
		  << "Not initialized the output list of objects! Exiting..." << std::endl;
	  exit(-1);
  }
  for (int i = 0; i < fOutput->GetEntries(); i++) {
    tmpobj = fOutput->At(i);
    if (classname == tmpobj->IsA()->GetName()) {
      object = tmpobj;
      break;
    }
  }
  return object;
}
//
///////////////////////////////////////////////////////////////////
//
// + TCounterUI
TCounterUI* CMSAnalysisSelector::InitCounterUI(const char* name, const char* title,
                                        unsigned int init) {
  TCounterUI *counter = new TCounterUI(name, title, init);
  fOutput->Add(counter);
  return counter;
}


// + TTree
TTree* CMSAnalysisSelector::CreateTree(const char* name, const char* title) {

  TTree* h = new TTree(name, title);

  fOutput->Add(h);
  return h;
}

// + TH1
TH1F* CMSAnalysisSelector::CreateH1F(const char* name, const char* title,
                                     Int_t nbinsx, Axis_t xlow, Axis_t xup) {
  TH1F* h = new TH1F(name, title, nbinsx, xlow, xup);
  fOutput->Add(h);
  return h;
}

TH1F* CMSAnalysisSelector::CreateH1F(const char* name, const char* title,
                                     Int_t nbinsx, Float_t* bins){
  TH1F* h = new TH1F(name, title, nbinsx, bins);
  fOutput->Add(h);
  return h;
}

TH1D* CMSAnalysisSelector::CreateH1D(const char* name, const char* title,
                                     Int_t nbinsx, Axis_t xlow, Axis_t xup) {
  TH1D* h = new TH1D(name, title, nbinsx, xlow, xup);
  fOutput->Add(h);
  return h;
}

TH1D* CMSAnalysisSelector::CreateH1D(const char* name, const char* title,
                                     Int_t nbinsx, Double_t* bins){
  TH1D* h = new TH1D(name, title, nbinsx, bins);
  fOutput->Add(h);
  return h;
}

// + TH2
TH2F* CMSAnalysisSelector::CreateH2F(const char* name, const char* title,
                                     Int_t nbinsx, Axis_t xlow, Axis_t xup,
                                     Int_t nbinsy, Axis_t ylow, Axis_t yup) {
  TH2F* h = new TH2F(name, title,
                     nbinsx, xlow, xup,
                     nbinsy, ylow, yup);
  fOutput->Add(h);
  return h;
}

TH2F* CMSAnalysisSelector::CreateH2F(const char* name, const char* title,
                                     Int_t nbinsx, Float_t* xbins,
                                     Int_t nbinsy, Float_t* ybins){
  TH2F* h = new TH2F(name, title,nbinsx, xbins,nbinsy, ybins);
  fOutput->Add(h);
  return h;
}


// + TProfile
TProfile* CMSAnalysisSelector::CreateProfile(const char* name, const
					     char* title, Int_t nbinsx, 
					     Axis_t xlow, Axis_t xup,
					     Axis_t ylow, Axis_t yup) {
  TProfile* h = new TProfile(name, title, nbinsx, xlow, xup, ylow, yup);
  fOutput->Add(h);
  return h;
}

//
/////////////////////////////////////////////////////////////////////
