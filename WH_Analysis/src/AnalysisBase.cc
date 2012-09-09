#include<sstream>
#include<stdlib.h>
#include<cstring>
#include<iostream>
#include<functional>
#include<algorithm>

#include "AnalysisBase.h"
#include "InputParameters.h"
#include "CutManager.h"
#include "PUWeight.h"
#include "WManager.h"

#include "TTree.h"
#include "TH1D.h"
#include "TMath.h"

// To be changed: FIXME
const double kZMass = 91.1876;
//const unsigned int kNMuons = 3; 
const unsigned int kNMuons = 2; 
const unsigned int kWPID   = 24; //Found with TDatabasePDG::Instance()->GetParticle("W+")->PdgCode()
const unsigned int kTauPID = 15; //Found with TDatabasePDG::Instance()->GetParticle("tau-")->PdgCode()



// Prepare analysis Constructor
AnalysisBase::AnalysisBase(TreeManager * data, std::map<LeptonTypes,InputParameters*> ipmap, 
		CutManager * selectioncuts, const unsigned int & finalstate ) :
	CMSAnalysisSelector(data),
	_nLeptons(3), //FIXME: argumento de entrada ? --> No dependera de la SignatureFS
	fIsData(false),
	fIsWH(false),
	fIsWZ(false),
	fIsSignal(false),
	fLuminosity(0),
	fRunPeriod(""),
	fFS(999),
	//fLeptonName(0),
	fNGenElectrons(0),
	fNGenMuons(0),
	fNGenLeptons(0),
	fPUWeight(0),
	fFO(0),
	fPO(0),
	fSF(0),
	_cuttree(0),
	_cutweight(1),
	_cutvalue(-1),
	_eventnumber(-1),
	_runnumber(-1),
	_evtlisttree(0),
	_wcharge(0),
	_jetname(""),
	fWasStored(false)
{
	// FIXME: Check that the data is attached to the selector manager
	fLeptonSelection = selectioncuts;
	// Initialize the cuts for the cut manager
	fLeptonSelection->InitialiseCuts(ipmap);
	
	// Just to use the general values fLuminosity, cross section and so on....
	// it doesn't matters which one pick up
	// The others already can be deleted as they have already used to input the cuts
	int ksize = ipmap.size();
	for(std::map<LeptonTypes,InputParameters*>::iterator ipI = ipmap.begin();
			ipI != ipmap.end(); ++ipI)
	{
		if( ksize == 1 )
		{
			fInputParameters = ipI->second;
			break;
		}

		if( ipI->second != 0 )
		{
			delete ipI->second;
			ipI->second = 0;
		}
		--ksize;
	}
	
	// Extract the run period
	fRunPeriod = fInputParameters->TheNamedString("RunPeriod");

	// Set jet name
	_jetname = "JetAKPFNoPU";
	if( fRunPeriod.find("2012") != std::string::npos )
	{
		_jetname = "JetAKCHS";
	}
	
	// Initialize the scale factors
	fSF = new WManager( WManager::SF );

	// Are in fake sample mode?
	if( fLeptonSelection->IsInFakeableMode() ) 
	{
		// Fake rate Matrix for Z Jets, when proceed
		int iszjetsFRMatrixint = 0;
		fInputParameters->TheNamedInt("FRMatrixZJETS",iszjetsFRMatrixint);
		const bool iszjetsFRMatrix = (bool)iszjetsFRMatrixint;
		fFO = new WManager( WManager::FR, iszjetsFRMatrix );
		fPO = new WManager( WManager::PR );
	}

	// The Inputparameters have to be initialized before, just to complete it
	// introducing the set of datasets: 
	//  --> extract filenames: datafilenames_[index]
	std::string filenames( "datafilenames_" );
	// -- Checking is there
	if( fInputParameters == 0 )
	{
		std::cerr << "\033[1;31mAnalysisBase::AnalysisBase ERROR:\033[1;m The 'InputParameters' argument "
			<< "cannot be passed as NULL pointer, initialize first!! Exiting... " << std::endl;
		exit(-1);
	}

	if( fInputParameters->TheNamedString( filenames+"0" ) == 0 )
	{
		std::cerr << "\033[1;31mAnalysisBase::AnalysisBase ERROR:\033[1;m The 'InputParameters' argument "
			<< "must contain a 'datafilenames_0' value!! Exiting..." << std::endl;
		exit(-1);
	}
	std::stringstream istr;
	int id = 0;
	istr << id;
	const char * filename  = 0;
	while( (filename = fInputParameters->TheNamedString(filenames+istr.str())) )
	{
		_datafiles.push_back( filename );
		id++;
		istr << id;
	}

	// Including to the list of fInputs the InputParameter
	TList * inputlist = new TList;
	inputlist->Add(fInputParameters);
	this->SetInputList(inputlist);

	// The signature of the analysis
	fFS = finalstate;
	// And the link to the Gen we want to use
	if( fFS == SignatureFS::_iFSmmm )
	{
		fLeptonType = MUON;
		fLeptonName.push_back("Muon");
	}
	else if( fFS == SignatureFS::_iFSeee )
	{
		fLeptonType = ELECTRON;
		fLeptonName.push_back("Elec");
	}
	else if( fFS == SignatureFS::_iFSmme )
	{
		fLeptonType = MIX2MU1ELE;
		fLeptonName.push_back("Muon");
		fLeptonName.push_back("Elec");
	}
	else if( fFS == SignatureFS::_iFSeem )
	{
		fLeptonType = MIX2ELE1MU;
		fLeptonName.push_back("Muon");
		fLeptonName.push_back("Elec");
	}

	// else if fFS == SignatrueFS::iFSmee --> fGenLeptonIndex = vector(MUON ELECTRON ELECTRON)
	// else ???? Ya deberia estar controlado antes
	// anyway FIXME: meter aqui un exit--->>

	// The tree for cuts
	_cuttree = new TTree("cuts","Last cut used");
	_cuttree->Branch("cuts",&_cutvalue);
	_cuttree->Branch("weight",&_cutweight);
	_cuttree->Branch("EventNumber",&_eventnumber);
	_cuttree->Branch("RunNumber",&_runnumber);

	// The tree for Event info
	_evtlisttree = new TTree("evtlist","Event selected info");
	_evtlisttree->Branch("evtinfo",&_evtinfo.run,"run/I:lumi/I:evt/I:channel/I:zmass/D:zlep1pt/D:zlep1eta/D:zlep1phi/D:zlep2pt/D:zlep2eta/D:zlep2phi/D:wmt/D:wleppt/D:wlepeta/D:wlepphi/D:metet/D:metphi/D");
}

AnalysisBase::~AnalysisBase()
{
	// Checking if already was stored the results,
	// otherwise do it
	if( ! fWasStored )
	{
		this->SaveOutput();
	}

	/*if( fData != 0)
	{
		delete fData; //WARNING: Somebody deleting-- > YES!!
	}*/
	if( fInputParameters != 0)
	{
		delete fInputParameters; 
		fInputParameters=0;
	}
	if( fInput != 0 )
	{
		delete fInput;  // new TList deleted
		fInput = 0;
	}
	if( _cuttree != 0)
	{
		delete _cuttree;
		_cuttree = 0;
	}
	if( _evtlisttree != 0)
	{
		delete _evtlisttree;
		_evtlisttree = 0;
	}

	if( fFO != 0 )
	{
		delete fFO;
		fFO = 0;
	}
	
	if( fPO != 0 )
	{
		delete fPO;
		fPO = 0;
	}
	
	if( fSF != 0 )
	{
		delete fSF;
		fSF = 0;
	}

}

void AnalysisBase::SaveOutput( const char * outputname )
{
	// Create the output file and fill it
	// Putting the outputfile name per default if not defined
	std::string outputfile;

	if( outputname == 0 )
	{
		// Extract the name of the file and get the last 
		std::string filename( fInputParameters->TheNamedString("datafilenames_0") );
		size_t barlastpos = filename.rfind("/")+1;
		if( barlastpos == filename.npos )
		{
			// all the string is valid
			barlastpos = 0;
		}
		// Extracting the .root suffix
		const size_t rootpos = filename.find(".root");
		const size_t length  = rootpos-barlastpos;
		std::string almostfinalname = filename.substr(barlastpos,length);
		// And extract the Tree_
		size_t prefix = almostfinalname.rfind("Tree_")+5;
		if( prefix == almostfinalname.npos )
		{
			prefix = 0;
		}
		std::string finalname = almostfinalname.substr(prefix);;

		outputfile = "Results/"+std::string(finalname+".root");
	}
	else
	{
		outputfile = outputname;
	}

	std::cout << ">> Saving results to " << outputfile << " ..." << std::endl;
	TString outputfileTS = TString(outputfile);
	if(gSystem->FindFile(".", outputfileTS)) 
	{
		std::cout << "WARNING: File " << outputfile << " already exits!" << std::endl;
		TString outputFileBak = outputfile + ".bak";
		std::cout << "         Moving it to " << outputFileBak << std::endl;
		gSystem->CopyFile(outputfile.c_str(), outputFileBak.Data(), kTRUE);
		gSystem->Unlink(outputfile.c_str());
	}
	TFile histoAnalysis(outputfile.c_str(), "NEW");
	if (histoAnalysis.IsOpen()) 
	{
		TList* li = 0;
		TList* lo = 0;
		li = this->GetInputList();
		lo = this->GetOutputList();
		li->Write();
		lo->Write();
		// And the tree for cuts
		_cuttree->Write();
		// Also the event info tree
		_evtlisttree->Write();

		histoAnalysis.Close();
	}

	this->fWasStored = true;
}

void AnalysisBase::InitialiseParameters()
{
	InputParameters * ip = this->GetInputParameters();

	// FIXME: I need a method to checked that the cut
	// is really in the configuration file
	//Cuts ---> Reorganized!! See CutManager src
	
	// Variables describing dataset...
	//--------------------------------
	fDataName = ip->TheNamedString("DataName");
	if(fDataName.Contains("WH"))
	{
		fIsWH = true;
	}
	else if(fDataName.Contains("WZ"))
	{
		fIsWZ  = true;
	}
	else if(fDataName.Contains("Data"))
	{
		fIsData = true;
	}
	else if(fDataName.Contains("Fake"))
	{
		fIsData = true;
	}
	
	// Luminosity
	//--------------------------------
	ip->TheNamedDouble("Luminosity", fLuminosity);
	
#ifdef DEBUGANALYSIS
	std::cout << "DEBUG: IsWH   = " << fIsWH << std::endl;
	std::cout << "DEBUG: IsWZ   = " << fIsWZ << std::endl;
	std::cout << "DEBUG: IsData = " << fIsData << std::endl;
#endif
	
	// PU Weight
	//----------------------------------------------------------------------------
	EMCDistribution MCdist; 
	if( fRunPeriod == "2011A" )
	{
		MCdist = Summer11;
	}
	else if( fRunPeriod == "2011B" )
	{
		MCdist = Summer11;
	}
	else if( fRunPeriod == "2011" )
	{
		//MCdist = Fall11True;
		MCdist = Fall11;
	}
	else if( fRunPeriod == "2012A" )
	{
		MCdist = Summer12; 
	}
	else if( fRunPeriod == "2012B" )
	{
		MCdist = Summer12;
	}
	else if( fRunPeriod == "2012" )
	{
		MCdist = Summer12;
	}
	else
	{
		std::cerr << "\033[1;31mAnalysisBase::AnalysisBase ERROR:\033[1;m The Run Period '"
			<< fRunPeriod << "' is not recognized" << std::endl;
		exit(-1);
	}
	fPUWeight = new PUWeight(fLuminosity, MCdist, fRunPeriod.c_str());

	// The W-charge if needed (if there are anything is going to return -1 but without
	// touching the initial value 0
	ip->TheNamedInt("WCharge",_wcharge);
}


// Function to extract 4-moments of objects
const TLorentzVector AnalysisBase::GetTLorentzVector( const char * name, const int & index) const
{
	std::string namestr = name;
	const std::string px("T_"+namestr+"_Px");
	const std::string py("T_"+namestr+"_Py");
	const std::string pz("T_"+namestr+"_Pz");
	const std::string energy("T_"+namestr+"_Energy");

	return TLorentzVector( fData->Get<float>(px.c_str(),index),
			fData->Get<float>(py.c_str(),index),
			fData->Get<float>(pz.c_str(),index),
			fData->Get<float>(energy.c_str(),index)
			);
}

void AnalysisBase::Summary()
{
	std::cout << " + Channel evaluated: " << SignatureFS::GetFSID(fFS) << std::endl;
	if( ! fIsData )
	{
		std::cout << " + Scale Factors used: |" << std::endl;
		if( fFS == SignatureFS::_iFSmmm ||
				fFS == SignatureFS::_iFSeem || SignatureFS::_iFSmme )
		{
			std::cout << "                       + MUON: " << fSF->GetFilename(MUON) << std::endl;
		}
		
		if( fFS == SignatureFS::_iFSeee ||
				fFS == SignatureFS::_iFSeem || SignatureFS::_iFSmme )
		{
			std::cout << "                       + ELECTRON: " << fSF->GetFilename(ELECTRON) << std::endl;
		}
	}
	if( fLeptonSelection->IsInFakeableMode() ) 
	{
		std::cout << " + FAKEABLE MODE ENABLED: ";
		int iszjetsFRMatrixint = 0;
		// Fake rate Matrix for Z Jets, when proceed
		fInputParameters->TheNamedInt("FRMatrixZJETS",iszjetsFRMatrixint);
		const bool iszjetsFRMatrix = (bool)iszjetsFRMatrixint;
		if( iszjetsFRMatrix ) 
		{
			std::cout << "Z+Jets Region" << std::endl;
		}
		else
		{
			std::cout << "ttbar Region" << std::endl;
		}
	}
	// FIXME---> SYSTEMATICS MODE ?? -> IsSystematicMode() ??
	std::cout << " ------------------------------------------------- " << std::endl;
	std::cout << std::endl;
  
	std::cout << "N. events by process ID:" << std::endl;
	std::cout << "------------------------" << std::endl;
	for(unsigned int i = 0; i < _iNProcesses; i++)
	{
		std::cout << kProcesses[i] << ":\t" << _histos[fHProcess]->GetBinContent(i+1) << " events ("
			<< 100.0*_histos[fHProcess]->GetBinContent(i+1)/_histos[fHProcess]->GetEntries() << "%)" << std::endl;
	}
	std::cout << std::endl << std::endl;
	std::cout << "N. events selected at each stage:" << std::endl;
	std::cout << "---------------------------------" << std::endl;
	for(unsigned int i = 0; i < fNCuts; i++)
	{
		std::cout << _histos[fHEventsPerCut]->GetBinContent(i+1) << " ["
			<< 100.0*_histos[fHEventsPerCut]->GetBinContent(i+1)/_histos[fHEventsPerCut]->GetBinContent(1)
			<< "%] selected events (" << fCutNames[i] << ")" << std::endl;
	}
	std::cout << std::endl << std::endl;
}




//---------------------------------------------------------------------
// Other helper methods
//---------------------------------------------------------------------
//

bool AnalysisBase::IspassHLT() const
{
	//FIXME
	return  true;
	/*// Only data by the moment
	if( ! fIsData )
	{
		return true;
	}

	bool passtrigger = false;
	if( fFS == SignatureFS::_iFSmmm )
	{
		passtrigger = fData->Get<bool>("T_passTriggerDoubleMu");
	}
	else if( fFS == SignatureFS::_iFSeee )
	{
		passtrigger = fData->Get<bool>("T_passTriggerDoubleEl");
	}
	else if( fFS == SignatureFS::_iFSmme || fFS == SignatureFS::_iFSeem )
	{
		passtrigger = fData->Get<bool>("T_passTriggerElMu");
	}
	else if( fFS == SignatureFS::_iFSlll )
	{
		// Note that here we are going to mix DoubleElectron samples with 
		// DoubleMuon Samples: so to avoid double counting:
		// -- 
		passtrigger = fData->Get<bool>("T_passTriggerElMu");
		if( ! passtrigger )
		{
			passtrigger = pass->Get<bool>("T_passTriggerMu");
		}
		if( ! passtrigger )
		{
			passtrigger = pass->Get<bool>("T_passTriggerEl");
		}
	}

	return passtrigger;*/
}


void AnalysisBase::StoresCut(const unsigned int & cut, const float & weight)
{
	_cutweight = weight;
	_cutvalue = cut;
	_eventnumber = fData->Get<int>("T_Event_EventNumber");
	_runnumber   = fData->Get<int>("T_Event_RunNumber");
	_cuttree->Fill();
	_cutvalue = -1;
	_cutweight = 1;
}

void AnalysisBase::StoresEvtInf(const std::vector<TLorentzVector> & lepton, 
		const int & iZ1, const int & iZ2,
		const int & iW, const double & transversmass, const TLorentzVector & METV)
{
        _evtinfo.run = fData->Get<int>("T_Event_RunNumber");
        _evtinfo.lumi = fData->Get<int>("T_Event_LuminosityBlock");
        _evtinfo.evt  = fData->Get<int>("T_Event_EventNumber");
        _evtinfo.channel = fFS;

        _evtinfo.zmass = (lepton[iZ1]+lepton[iZ2]).M();
        _evtinfo.zlep1pt = lepton[iZ1].Pt();
        _evtinfo.zlep1eta = lepton[iZ1].Eta();
        _evtinfo.zlep1phi = lepton[iZ1].Phi();

        _evtinfo.zlep2pt = lepton[iZ2].Pt();
        _evtinfo.zlep2eta = lepton[iZ2].Eta();
        _evtinfo.zlep2phi = lepton[iZ2].Phi();

        _evtinfo.wmt = transversmass;
        _evtinfo.wleppt = lepton[iW].Pt();
        _evtinfo.wlepeta = lepton[iW].Eta();
        _evtinfo.wlepphi = lepton[iW].Phi();

        _evtinfo.metet = METV.Pt();
        _evtinfo.metphi= METV.Phi();

	_evtlisttree->Fill();
}


void AnalysisBase::FillHistoPerCut(const unsigned int & cut,const double & puw, const unsigned int & fs) 
{
	_histos[fHEventsPerCut]->Fill(cut, puw);
	if(fs == fFS)
	{
		_histos[fHEventsPerCut3Lepton]->Fill(cut, puw);
	}
}

void AnalysisBase::FillGenPlots(const unsigned int & cut, double puw) 
{
	if(fIsSignal && fNGenLeptons == _nLeptons) 
	{
		for(unsigned int i = 0; i < fNGenLeptons; i++) 
		{
			fHGenPtLepton[i][cut]->Fill(fGenLepton[i].Pt(), puw);
			fHGenEtaLepton[i][cut]->Fill(fGenLepton[i].Eta(), puw);
		}
	}
}

// Fake rate estimations
//===============================================================

// Simplistic calculation using full approximations p\sim 1, f->0
double AnalysisBase::GetPPFWeightApprx()
{
	// As we are using the approximation PromptRate=1, then
	// PPF (3,2) = fF0->GetWeight
	// PFF (3,1) = (fFO->GetWeight)^2
	// FFF (3,0) = (fFO->GetWeight)^3
	// Where (N,T) are actually the number of Total leptons and PROMPT leptons. 
	// This equivalence between tight-prompt can be done because of the approximations
	// used. So, each no-tight lepton is weighted in order to get its probability to be
	// fake.
	double puw = 1.0;
	for(unsigned int k = 0; k < fLeptonSelection->GetNAnalysisNoTightLeptons(); ++k)
	{
		const unsigned int i = fLeptonSelection->GetNoTightIndex(k);
		const LeptonTypes ileptontype= fLeptonSelection->GetNoTightLeptonType(k);
		const char * name = 0;
		if( ileptontype == MUON )
		{
			name = "Muon";
		}
		else
		{
			name = "Elec";
		}
		TLorentzVector lvec = this->GetTLorentzVector(name,i);
		const double pt  = lvec.Pt();
		const double eta = lvec.Eta();
		puw *= fFO->GetWeight(ileptontype,pt,eta);
	}

	return puw;
}


// Full calculation for PPF estimation
// Nt3 term
double AnalysisBase::GetPPFWeightNt3()
{
	// Weighting rules:
	//  PROMPT ESTIMATED:     p(1-f)  for each passing 
	//                        pf      for each failing
	//  FAKE ESTIMATED:       f(1-p)  for each passing
	//                        pf      for each failing
	//  Plus the common factor: 1/(p-f)^N  (N: number of analysis leptons)
	
	const unsigned int ntight = fLeptonSelection->GetNAnalysisTightLeptons();
	std::vector<double> p; 
	std::vector<double> f;
	// 1. All Tights
	double kterm = 1.0;
	for(unsigned int k = 0; k < ntight; ++k)
	{
		const unsigned int i = fLeptonSelection->GetTightIndex(k);
		const LeptonTypes ileptontype= fLeptonSelection->GetTightLeptonType(k);
		const char * name = 0;
		if( ileptontype == MUON )
		{
			name = "Muon";
		}
		else
		{
			name = "Elec";
		}
		TLorentzVector lvec = this->GetTLorentzVector(name,i);
		const double pt  = lvec.Pt();
		const double eta = lvec.Eta();
		p.push_back(fPO->GetWeight(ileptontype,pt,eta));
		f.push_back(fFO->GetWeight(ileptontype,pt,eta));
		// Taking advantatge of the loop
		kterm *= 1.0/(p[k]-f[k]);
	}
	// combinations of the tights: PPF PFP FPP
	const double PPF = kterm*p[0]*(1.0-f[0])*p[1]*(1.0-f[1])*f[2]*(1.0-p[2]);
	const double PFP = kterm*p[0]*(1.0-f[0])*p[2]*(1.0-f[2])*f[1]*(1.0-p[1]);
	const double FPP = kterm*p[2]*(1.0-f[2])*p[1]*(1.0-f[1])*f[0]*(1.0-p[0]);

/*std::cout << " ===================== " << std::endl;
std::cout << "Prompt rates: ";
for(unsigned int k = 0; k < p.size(); ++k)
{
std::cout << "[" << k << "]=" << p[k] << " ";
}
std::cout << std::endl << "Fake rates: ";
for(unsigned int k = 0; k < f.size(); ++k)
{
std::cout << "[" << k << "]=" << f[k] << " ";
}
std::cout << std::endl << "PPF:" << PPF << " || PFP:" << PFP << " || FPP:" << FPP <<
"   TOTAL weight: " << (PPF+PFP+FPP) << std::endl;*/

	return PPF+PFP+FPP;	
}

// Nt2 term
double AnalysisBase::GetPPFWeightNt2()
{
	// Weighting rules:
	//  PROMPT ESTIMATED:     p(1-f)  for each passing 
	//                        pf      for each failing
	//  FAKE ESTIMATED:       f(1-p)  for each passing
	//                        pf      for each failing
	//  Plus the common factor: 1/(p-f)^N  (N: number of analysis leptons)

	const unsigned int ntight = fLeptonSelection->GetNAnalysisTightLeptons();
	const unsigned int nfailing = fLeptonSelection->GetNAnalysisNoTightLeptons();
	std::vector<double> p; // index ordered in tight-noTight
	std::vector<double> f;
	// 1. Tight 
	for(unsigned int k = 0; k < ntight; ++k)
	{
		const unsigned int i = fLeptonSelection->GetTightIndex(k);
		const LeptonTypes ileptontype= fLeptonSelection->GetTightLeptonType(k);
		const char * name = 0;
		if( ileptontype == MUON )
		{
			name = "Muon";
		}
		else
		{
			name = "Elec";
		}
		TLorentzVector lvec = this->GetTLorentzVector(name,i);
		const double pt  = lvec.Pt();
		const double eta = lvec.Eta();
		p.push_back(fPO->GetWeight(ileptontype,pt,eta));
		f.push_back(fFO->GetWeight(ileptontype,pt,eta));
	}
	// 2. NoTight (or failing)
	for(unsigned int k = 0; k < nfailing; ++k)
	{
		const unsigned int i = fLeptonSelection->GetNoTightIndex(k);
		const LeptonTypes ileptontype= fLeptonSelection->GetNoTightLeptonType(k);
		const char * name = 0;
		if( ileptontype == MUON )
		{
			name = "Muon";
		}
		else
		{
			name = "Elec";
		}
		TLorentzVector lvec = this->GetTLorentzVector(name,i);
		const double pt  = lvec.Pt();
		const double eta = lvec.Eta();
		p.push_back( fPO->GetWeight(ileptontype,pt,eta) );
		f.push_back( fFO->GetWeight(ileptontype,pt,eta) );
	}
	// Weight calculation:
	// 1. 2 Prompt estimated passing + 1 Fake estimated fail
	double PpPpFf = 1.0;
	for( unsigned int k = 0; k < ntight; ++k)
	{
		PpPpFf *= (1.0/(p[k]-f[k]))*p[k]*(1.0-f[k]);
	}
	for(unsigned int k = 0; k < nfailing; ++k)
	{
		PpPpFf *= (1.0/(p[k]-f[k]))*p[k]*f[k];
	}
	
	// 2. 1 Prompt estimated passing + 1 Prompt estimated fail + 1 Fake estimated passing
	//    Note the indetermination on which one of the two passing is prompt and which
	//    one is fake
	double PpPfFp = 0.0;  
	for( unsigned int k = 0; k < nfailing; ++k)
	{
		// 1 prompt estimated failing
		const double pfail = (1.0/(p[k]-f[k]))*p[k]*f[k];
		// the remaining tights are a prompt estimated and a fake estimated
		// FIXME: HARDCODED for 3-lepton case
		// 0-Prompt 1-Fake  term
		PpPfFp = pfail*(1.0/(p[0]-f[0]))*p[0]*(1.0-f[0])*(1.0/(p[1]-f[1]))*f[1]*(1.0-p[1]);
		// 0-Fake 1-Prompt  term
		PpPfFp += pfail*(1.0/(p[1]-f[1]))*p[1]*(1.0-f[1])*(1.0/(p[0]-f[0]))*f[0]*(1.0-p[0]);
	}

/*std::cout << " ===================== " << std::endl;
std::cout << "Prompt rates: ";
for(unsigned int k = 0; k < p.size(); ++k)
{
std::cout << "[" << k << "]=" << p[k] << " ";
}
std::cout << std::endl << "Fake rates: ";
for(unsigned int k = 0; k < f.size(); ++k)
{
std::cout << "[" << k << "]=" << f[k] << " ";
}
std::cout << std::endl << "Prompt-Tight, Prompt-Tight, Fake-Fail:" << PpPpFf <<
" || Prompt-Tight, Prompt-Fail, Fake-Tight:" << PpPfFp << " || TOTAL weight: " <<
(PpPpFf+PpPfFp) << std::endl;*/

	return PpPpFf+PpPfFp;
}
