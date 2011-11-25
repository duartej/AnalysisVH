#include<sstream>
#include<stdlib.h>
#include<cstring>
#include<iostream>
#include<functional>
#include<algorithm>

#include "AnalysisVH.h"
#include "InputParameters.h"
#include "CutManager.h"
#include "PUWeight.h"

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
AnalysisVH::AnalysisVH(TreeManager * data, std::map<LeptonTypes,InputParameters*> ipmap, 
		CutManager * selectioncuts, const unsigned int & finalstate ) :
	//CMSAnalysisSelector(data),
	AnalysisBase(data,ipmap,selectioncuts,finalstate )
	//_nLeptons(3), //FIXME: argumento de entrada ? --> No dependera de la SignatureFS
	//fIsData(false),
	//fIsWH(false),
	//fLuminosity(0),
	//fFS(999),
	////fLeptonName(0),
	//fNGenElectrons(0),
	//fNGenMuons(0),
	//fNGenLeptons(0),
	//fPUWeight(0),
	//fWasStored(false)
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

	// The Inputparameters have to be initialized before, just to complete it
	// introducing the set of datasets: 
	//  --> extract filenames: datafilenames_[index]
	std::string filenames( "datafilenames_" );
	// -- Checking is there
	if( fInputParameters == 0 )
	{
		std::cerr << "\033[1;31mAnalysisVH::AnalysisVH ERROR:\033[1;m The 'InputParameters' argument "
			<< "cannot be passed as NULL pointer, initialize first!! Exiting... " << std::endl;
		exit(-1);
	}

	if( fInputParameters->TheNamedString( filenames+"0" ) == 0 )
	{
		std::cerr << "\033[1;31mAnalysisVH::AnalysisVH ERROR:\033[1;m The 'InputParameters' argument "
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
}

AnalysisVH::~AnalysisVH()
{
	// Checking if already was stored the results,
	// otherwise do it
//	if( ! fWasStored )
//	{
//		this->SaveOutput();
//	}
//
//	/*if( fData != 0)
//	{
//		delete fData; //WARNING: Somebody deleting-- > YES!!
//	}*/
//	if( fInputParameters != 0)
//	{
//		delete fInputParameters; 
//	}
//	if( fInput != 0 )
//	{
//		delete fInput;  // new TList deleted
//	}

}

//void AnalysisVH::SaveOutput( const char * outputname )
//{
//	// Create the output file and fill it
//	// Putting the outputfile name per default if not defined
//	std::string outputfile;
//
//	if( outputname == 0 )
//	{
//		// Extract the name of the file and get the last 
//		std::string filename( fInputParameters->TheNamedString("datafilenames_0") );
//		size_t barlastpos = filename.rfind("/")+1;
//		if( barlastpos == filename.npos )
//		{
//			// all the string is valid
//			barlastpos = 0;
//		}
//		// Extracting the .root suffix
//		const size_t rootpos = filename.find(".root");
//		const size_t length  = rootpos-barlastpos;
//		std::string almostfinalname = filename.substr(barlastpos,length);
//		// And extract the Tree_
//		size_t prefix = almostfinalname.rfind("Tree_")+5;
//		if( prefix == almostfinalname.npos )
//		{
//			prefix = 0;
//		}
//		std::string finalname = almostfinalname.substr(prefix);;
//
//		outputfile = "Results/"+std::string(fInputParameters->TheNamedString("MyAnalysis"))+"_"
//			+finalname+".root";
//	}
//	else
//	{
//		outputfile = outputname;
//	}
//
//	std::cout << ">> Saving results to " << outputfile << " ..." << std::endl;
//	TString outputfileTS = TString(outputfile);
//	if(gSystem->FindFile(".", outputfileTS)) 
//	{
//		std::cout << "WARNING: File " << outputfile << " already exits!" << std::endl;
//		TString outputFileBak = outputfile + ".bak";
//		std::cout << "         Moving it to " << outputFileBak << std::endl;
//		gSystem->CopyFile(outputfile.c_str(), outputFileBak.Data(), kTRUE);
//		gSystem->Unlink(outputfile.c_str());
//	}
//	TFile histoAnalysis(outputfile.c_str(), "NEW");
//	if (histoAnalysis.IsOpen()) 
//	{
//		TList* li = 0;
//		TList* lo = 0;
//		li = this->GetInputList();
//		lo = this->GetOutputList();
//		li->Write();
//		lo->Write();
//		histoAnalysis.Close();
//	}
//
//	this->fWasStored = true;
//}
//
//void AnalysisVH::InitialiseParameters()
//{
//	InputParameters * ip = this->GetInputParameters();
//
//	// FIXME: I need a method to checked that the cut
//	// is really in the configuration file
//	//Cuts ---> Reorganized!! See CutManager src
//	
//	// Variables describing dataset...
//	//--------------------------------
//	fDataName = ip->TheNamedString("DataName");
//	fIsWH = false;
//	fIsData = false;
//	if(fDataName.Contains("WH"))
//	{
//		fIsWH = true;
//	}
//	else if(fDataName.Contains("Data"))
//	{
//		fIsData = true;
//	}
//	
//	// Luminosity
//	//--------------------------------
//	ip->TheNamedDouble("Luminosity", fLuminosity);
//	
//#ifdef DEBUGANALYSIS
//	std::cout << "DEBUG: IsWH   = " << fIsWH << std::endl;
//	std::cout << "DEBUG: IsData = " << fIsData << std::endl;
//#endif
//	std::cout << *fLeptonSelection << std::endl;
//}
//
//void AnalysisVH::Initialise()
//{
//	// PU Weight
//	//----------------------------------------------------------------------------
//	fPUWeight = new PUWeight(fLuminosity, Summer11InTime); //EMCDistribution enum
//
//	// Selection cuts
//  	//----------------------------------------------------------------------------
//  
//	// Histograms
//	//----------------------------------------------------------------------------
//	// Process ID
//	_histos[fHProcess] = CreateH1D("fHProcess", "Proccess ID", _iNProcesses, 0, _iNProcesses);
//	for(unsigned int i = 0; i < _iNProcesses; i++)
//	{
//		_histos[fHProcess]->GetXaxis()->SetBinLabel(i+1,kProcesses[i]);
//	}
//	
//	// Final state from generation (incl. taus)
//	_histos[fHGenFinalState] = CreateH1D("fHGenFinalState", "Final State (incl. #tau)", 
//			SignatureFS::_iFStotal, 0, SignatureFS::_iFStotal);
//	for(unsigned int i = 0; i < SignatureFS::_iFStotal; i++)
//	{
//		_histos[fHGenFinalState]->GetXaxis()->SetBinLabel(i+1,SignatureFS::kFinalStates[i]);
//	}
//	_histos[fHGenFinalState]->Sumw2();
//
//	// Final state from generation (no taus)
//	_histos[fHGenFinalStateNoTaus] = CreateH1D("fHGenFinalStateNoTaus", "Final State (no #tau)", 
//			SignatureFS::_iFStotal, 0, SignatureFS::_iFStotal);
//	for(unsigned int i = 0; i < SignatureFS::_iFStotal; i++)
//	{
//		_histos[fHGenFinalStateNoTaus]->GetXaxis()->SetBinLabel(i+1,SignatureFS::kFinalStates[i]);
//	}
//	_histos[fHGenFinalStateNoTaus]->Sumw2();
//
//	// Generated electrons coming from a W
//	_histos[fHNGenWElectrons] = CreateH1D("fHNGenWElectrons", "N Gen e from W", 5, -0.5, 4.5);
//	// Generated muons coming from a W
//	_histos[fHNGenWMuons] = CreateH1D("fHNGenWMuonss", "N Gen #mu from W", 5, -0.5, 4.5);
//	// Generated leptons coming from a W
//	_histos[fHNGenWLeptons] = CreateH1D("fHNGenWLeptons", "N Gen leptons from W", 5, -0.5, 4.5);
//	
//	// PT and Eta of most energetic gen muon from W or tau
//	for(unsigned int i = 0; i < _nLeptons; i++) 
//	{
//		for (unsigned int j = 0; j < _iNCuts; j++) 
//		{
//			TString ptname  = Form("fHGenPtLepton%i_%i", i+1,j);
//			TString pttitle = Form("P_{T} #mu_{gen}^{%i} from W (#tau)", i+1);
//			TString etaname  = Form("fHGenEtaLepton%i_%i", i+1,j);
//			TString etatitle = Form("#eta #mu_{gen}^{%i} from W (#tau)", i+1);
//			fHGenPtLepton[i][j]  = CreateH1D(ptname,  pttitle, 150, 0, 150);
//			fHGenEtaLepton[i][j] = CreateH1D(etaname, etatitle, 100, -5, 5);
//		}
//	}
//
//
//	// Events passing every cut
//	_histos[fHEventsPerCut] = CreateH1D("fHEventsPerCut", "Events passing each cut", 
//			_iNCuts, 0, _iNCuts);
//	for (unsigned int i = 0; i < _iNCuts; i++)
//	{
//		_histos[fHEventsPerCut]->GetXaxis()->SetBinLabel(i+1,kCutNames[i]);
//	}
//  
//	// Events passing every cut that are 3 mu from gen
//	_histos[fHEventsPerCut3Lepton] = CreateH1D("fHEventsPerCut3Lepton", "Events passing each cut that are 3 mu from gen", 
//			_iNCuts, 0, _iNCuts);
//	for(unsigned int i = 0; i < _iNCuts; i++)
//	{
//		_histos[fHEventsPerCut3Lepton]->GetXaxis()->SetBinLabel(i+1,kCutNames[i]);
//	}
//
//	// Reconstructed muons in the event
//	_histos[fHNRecoLeptons] = CreateH1D("fHNRecoLeptons", "Reconstructed #mu", 
//			10, -0.5, 9.5);
//	
//	// Muons passing the basic selection
//	_histos[fHNSelectedLeptons] = CreateH1D("fHNSelectedLeptons", "Selected #mu", 
//			       10, -0.5, 9.5);
//
//	// Selected Muons close to the PV
//	_histos[fHNSelectedPVLeptons] = CreateH1D("fHNSelectedPVLeptons", 
//			"Selected #mu close to PV", 10, -0.5, 9.5);
//	// Selected Isolated Muons
//	_histos[fHNSelectedIsoLeptons] = CreateH1D("fHNSelectedIsoLeptons", 
//			"Selected Isolated #mu", 10, -0.5, 9.5);
//	// Selected Isolated Good Muons
//	_histos[fHNSelectedIsoGoodLeptons] = CreateH1D("fHNSelectedIsoGoodLeptons", 
//			"Selected good Isolated #mu", 10, -0.5, 9.5);
//	
//	// Pt and eta of first/second/third good isolated muon
//	for(unsigned int i = 0; i < _nLeptons; i++) 
//	{
//		TString ptname  = Form("fHPtLepton%i", i+1);
//		TString pttitle = Form("P_{T}^{#mu_{%i}}", i+1);
//		TString etaname  = Form("fHEtaLepton%i", i+1);
//		TString etatitle = Form("#eta^{#mu_{%i}}", i+1);
//		TString drname  = Form("fHDeltaRGenRecoLepton%i", i+1);
//		TString drtitle = Form("#Delta R for #mu %i", i+1);
//		fHPtLepton[i]  = CreateH1D(ptname,  pttitle, 150, 0, 150);
//		fHEtaLepton[i] = CreateH1D(etaname, etatitle, 100, -5, 5);
//		fHDeltaRGenRecoLepton[i] = CreateH1D(drname, drtitle, 150, 0, 5);
//	}
//	
//	//Smallest DeltaR between 2 opp. sign leptons
//	_histos[fHMinDeltaRLp1Lp2] = CreateH1D("fHMinDeltaRLp1Lp2", "Smallest #Delta R_{#mu#mu}",
//			125, 0, 5);
//	//Smallest DeltaR between 2 opp. sign leptons
//	_histos[fHMaxDeltaRLp1Lp2] = CreateH1D("fHMaxDeltaRLp1Lp2", "Largest #Delta R_{#mu#mu}",
//			125, 0, 5);
//	
//	//Smallest DeltaPhi between 2 opp. sign leptons
//	_histos[fHMinDeltaPhiLp1Lp2] = CreateH1D("fHMinDeltaPhiLp1Lp2", "Smallest #Delta #phi_{#mu#mu}",
//			120, 0, TMath::Pi());
//	//Largest DeltaPhi between 2 opp. sign leptons
//	_histos[fHMaxDeltaPhiLp1Lp2] = CreateH1D("fHMaxDeltaPhiLp1Lp2", "Largest #Delta #phi_{#mu#mu}",
//			120, 0, TMath::Pi());
//	
//	// Selected Isolated Good Muons
//	_histos[fHLeptonCharge] = CreateH1D("fHLeptonCharge", "#Sum q_{#mu}", 7, -3.5, 3.5);
//		
//	// Invariant mass of leptons supposedly from H  
//	_histos[fHHInvMass] = CreateH1D("fHHInvMass", "M^{inv.}_{#mu#mu}", 150, 0, 150);
//	
//	// Invariant mass of leptons in/out of Z peak
//	_histos[fHZInvMass] = CreateH1D("fHZInvMass", "M^{inv.}_{#mu#mu}",150, 0, 150);
//	
//	// Missing ET after inv mass cut
//	_histos[fHMET] = CreateH1D("fHMET", "MET",160, 0, 160);
//	
//}
//
///*const TLorentzVector AnalysisVH::GetTLorentzVector( const int & index) const
//{
//	std::string name;
//	// FIXME: WARNING, Not implemented
//std::cout << "HOLA CARACOLA";
//	if( fLeptonSelection->GetLeptonType(index) == MUON )
//	{
//		name = "Muon";
//	}
//	else
//	{
//		name = "Elec";
//	}
//
//	const std::string px("T_"+name+"_Px");
//	const std::string py("T_"+name+"_Py");
//	const std::string pz("T_"+name+"_Pz");
//	const std::string energy("T_"+name+"_Energy");
//std::cout << "ADIOS CARACOLA" << std::endl;
//
//	return TLorentzVector( fData->Get<float>(px.c_str(),index),
//			fData->Get<float>(py.c_str(),index),
//			fData->Get<float>(pz.c_str(),index),
//			fData->Get<float>(energy.c_str(),index)
//			);
//}*/
//
//// Overloaded function to extract 4-moments of other than leptons
//const TLorentzVector AnalysisVH::GetTLorentzVector( const char * name, const int & index) const
//{
//	std::string namestr = name;
//	const std::string px("T_"+namestr+"_Px");
//	const std::string py("T_"+namestr+"_Py");
//	const std::string pz("T_"+namestr+"_Pz");
//	const std::string energy("T_"+namestr+"_Energy");
//
//	return TLorentzVector( fData->Get<float>(px.c_str(),index),
//			fData->Get<float>(py.c_str(),index),
//			fData->Get<float>(pz.c_str(),index),
//			fData->Get<float>(energy.c_str(),index)
//			);
//}


//---------------------------------------------------------------------
// InsideLoop
//---------------------------------------------------------------------
void AnalysisVH::InsideLoop()
{
#ifdef DEBUGANALYSIS
	std::cout << "========================================================" << std::endl;
	std::cout << "New event: " << fData->Get<int>("T_Event_EventNumber") << std::endl;
#endif
	// Get PU Weight
	//----------------------------------------------------------------------
	double puw(1);
	if(!fIsData)
	{
		puw = fPUWeight->GetWeight(fData->Get<int>("T_Event_nPU"));
	}

	// Generation studies
	//----------------------------------------------------------------------
	unsigned int fsTaus = SignatureFS::_iFSunknown;
	unsigned int fsNTau = SignatureFS::_iFSunknown;
	fNGenElectrons = 0; //Number of generated electrons from W or tau
	fNGenMuons = 0;     //Number of generated muons from W or tau
	fNGenLeptons = 0;   //Number of generated leptons from W or tau
	if(fIsWH) 
	{
		// + Classify by leptonic final state (taus undecayed)
		unsigned int nelecsfromW = fData->GetSize<int>("T_Gen_ElecSt3_PID");
		unsigned int nmusfromW = fData->GetSize<int>("T_Gen_MuonSt3_PID");
		unsigned int ntausfromW = fData->GetSize<int>("T_Gen_TauSt3_PID");
		
		// Later it will be usefull knowing in what lepton decay the W->tau
		std::vector<TLorentzVector> elecsfromtaus;
		std::vector<TLorentzVector> muonsfromtaus;
		for(unsigned int i = 0; i < ntausfromW; ++i)
		{
			if( abs(fData->Get<int>("T_Gen_TauSt3_LepDec_PID",i)) == 11 )
			{
				elecsfromtaus.push_back(
                                     TLorentzVector( fData->Get<float>("T_Gen_TauSt3_LepDec_Px",i),
					     fData->Get<float>("T_Gen_TauSt3_LepDec_Px",i),
					     fData->Get<float>("T_Gen_TauSt3_LepDec_Px",i),
					     fData->Get<float>("T_Gen_TauSt3_LepDec_Px",i) ) );
#ifdef DEBUGANALYSIS
				std::cout << "DEBUG: Elec from tau (from W) "
					<< " E: " << fData->Get<float>("T_Gen_TauSt3_LepDec_Energy",i)
					<< " Px: " << fData->Get<float>("T_Gen_TauSt3_LepDec_Px",i)
					<< std::endl;
#endif
			}
			else if( abs(fData->Get<int>("T_Gen_TauSt3_LepDec_PID",i)) == 13 )
			{
				muonsfromtaus.push_back(
                                     TLorentzVector( fData->Get<float>("T_Gen_TauSt3_LepDec_Px",i),
					     fData->Get<float>("T_Gen_TauSt3_LepDec_Px",i),
					     fData->Get<float>("T_Gen_TauSt3_LepDec_Px",i),
					     fData->Get<float>("T_Gen_TauSt3_LepDec_Px",i) ) );
#ifdef DEBUGANALYSIS
				std::cout << "DEBUG: Muon from tau (from W) "
					<< " E: " << fData->Get<float>("T_Gen_TauSt3_LepDec_Energy",i)
					<< " Px: " << fData->Get<float>("T_Gen_TauSt3_LepDec_Px",i)
					<< std::endl;
#endif
			}
		}


		unsigned int ngenfromW;
		if( fFS  == SignatureFS::_iFSmmm )
		{
			ngenfromW = nmusfromW;
		}
		else if( fFS == SignatureFS::_iFSeee ) 
		{
			ngenfromW = nelecsfromW;
		}
		else if( fFS == SignatureFS::_iFSmme || fFS == SignatureFS::_iFSeem )
		{
			ngenfromW = nelecsfromW+nmusfromW;
		}
		
		_histos[fHNGenWElectrons]->Fill(nelecsfromW,puw);
		_histos[fHNGenWMuons]->Fill(nmusfromW,puw);
		_histos[fHNGenWLeptons]->Fill(ngenfromW,puw);
		fsTaus = SignatureFS::GetFSID(nelecsfromW, nmusfromW, ntausfromW);
		_histos[fHGenFinalState]->Fill(fsTaus, puw);
		
#ifdef DEBUGANALYSIS
		std::cout << "DEBUG: W->e/mu/tau " << nelecsfromW 
			<< "/" << nmusfromW << "/" << ntausfromW << std::endl;
		std::cout << "DEBUG: fsTaus --> " << fsTaus << std::endl;
#endif
		int igen[3] = {-1, -1, -1};
		std::string iname[3] = {"", "", ""}; 
		// + Classify by leptonic final state (no taus)
		for(unsigned int i = 0; i < fData->GetSize<int>("T_Gen_Elec_MPID"); i++) 
		{
#ifdef DEBUGANALYSIS
			std::cout << "DEBUG: Elec [" << i << "/" << fData->GetSize<int>("T_Gen_Elec_MPID") << "]" 
				<< " Status: " << fData->Get<int>("T_Gen_Elec_MSt",i)
				<< " Mother: " << fData->Get<int>("T_Gen_Elec_MPID",i)
				<< " E: " << fData->Get<float>("T_Gen_Elec_Energy",i)
				<< " Px: " << fData->Get<float>("T_Gen_Elec_Px",i)
				<< std::endl;
#endif
			//I have seen that electrons from taus have status 2 and are not replicated
			// also I have to be sure the mother tau has decayed from the W
			unsigned int mpidel  = TMath::Abs(fData->Get<int>("T_Gen_Elec_MPID",i));
			if( mpidel == kWPID || ( mpidel == kTauPID && elecsfromtaus.size() != 0) ) 
			{
				// Checking that the electron really decay from the W->tau (st3)
				if( mpidel == kTauPID )
				{
					TLorentzVector elec( fData->Get<float>("T_Gen_Elec_Px",i),
							fData->Get<float>("T_Gen_Elec_Py",i),
							fData->Get<float>("T_Gen_Elec_Pz",i),
							fData->Get<float>("T_Gen_Elec_Energy",i) );
					int neq = std::count_if(elecsfromtaus.begin(), 	elecsfromtaus.end(), 
							std::bind1st(std::equal_to<TLorentzVector>(),elec) );
					if( neq == 0 )
					{
						continue;
					}
				}
				if( fLeptonType == ELECTRON 
	        			|| fLeptonType == MIX2MU1ELE || fLeptonType == MIX2ELE1MU )
				{
					igen[fNGenLeptons] = i;
					iname[fNGenLeptons] = "Elec";
					++fNGenLeptons;
				}
				++fNGenElectrons;
			}
		}
		
		for (unsigned int i = 0; i < fData->GetSize<int>("T_Gen_Muon_MPID"); ++i) 
		{
#ifdef DEBUGANALYSIS
			std::cout << "DEBUG: Muon [" << i+1 << "/" << fData->GetSize<int>("T_Gen_Muon_MPID") << "]" 
				<< " Status: " << fData->Get<int>("T_Gen_Muon_MSt",i)
				<< " Mother: " << fData->Get<int>("T_Gen_Muon_MPID",i)
				<< " E: " << fData->Get<float>("T_Gen_Muon_Energy",i)
				<< " Px: " << fData->Get<float>("T_Gen_Muon_Px",i)
				<< std::endl;
#endif
			//I have seen that muons from taus have status 2 and are not replicated
			// also I have to be sure the mother tau has decayed from the W
			unsigned int mpid  = TMath::Abs(fData->Get<int>("T_Gen_Muon_MPID",i));
			if ( mpid == kWPID || (mpid == kTauPID && muonsfromtaus.size() != 0) ) 
			{
				// Checking that the muon really decay from the W->tau (st3)
				if( mpid == kTauPID )
				{
					TLorentzVector muon( fData->Get<float>("T_Gen_Muon_Px",i),
							fData->Get<float>("T_Gen_Muon_Py",i),
							fData->Get<float>("T_Gen_Muon_Pz",i),
							fData->Get<float>("T_Gen_Muon_Energy",i) );
					int neq = std::count_if(elecsfromtaus.begin(), 	elecsfromtaus.end(), 
							std::bind1st(std::equal_to<TLorentzVector>(),muon) );
					if( neq == 0 )
					{
						continue;
					}
				}
				if( fLeptonType == MUON 
	        			|| fLeptonType == MIX2MU1ELE || fLeptonType == MIX2ELE1MU )
				{
					igen[fNGenLeptons] = i;
					iname[fNGenLeptons] = "Muon";
					++fNGenLeptons;
				}
				++fNGenMuons;
			}
		}
		fsNTau = SignatureFS::GetFSID(fNGenElectrons, fNGenMuons, 3-fNGenMuons-fNGenElectrons);
		_histos[fHGenFinalStateNoTaus]->Fill(fsNTau, puw);
		
#ifdef DEBUGANALYSIS
		std::cout << "DEBUG: W->e/mu/tau " <<  fNGenElectrons
			<< "/" << fNGenMuons << "/" << (3-fNGenMuons-fNGenElectrons) << std::endl;
		std::cout << "DEBUG: fsNTau --> " << fsNTau << std::endl;
#endif
		// Initialize generation vector
		fGenLepton.clear();
		fGenLeptonName.clear();
		//   Sort by energy (? or by pt?)
		if(fNGenLeptons == 3)  // FIXME:: PORQUE SOLO 3?? --> cAMBIAR POR nLeptons
		{
			std::map<double,std::pair<TLorentzVector,std::string> > vOrder;
			std::vector<TLorentzVector> * vGenMuon = new std::vector<TLorentzVector>;
			for(unsigned int i = 0; i < fNGenLeptons; i++) 
			{
				// Extract the name
				std::string genname = std::string("Gen_"+iname[i]);
				vGenMuon->push_back( this->GetTLorentzVector( genname.c_str(),igen[i]) ); 
				vOrder[vGenMuon->back().Pt()] = 
					std::pair<TLorentzVector,std::string>(vGenMuon->back(),iname[i]);
			}
			for(std::map<double,std::pair<TLorentzVector,std::string> >::reverse_iterator it = vOrder.rbegin(); 
					it != vOrder.rend(); ++it)
			{
				fGenLepton.push_back( it->second.first );
				fGenLeptonName.push_back( it->second.second );
			}
			delete vGenMuon;
			vGenMuon = 0;
			
#ifdef DEBUGANALYSIS
			for (unsigned int i = 0; i < fNGenLeptons; ++i) 
			{
				std::cout << "DEBUG: Lepton:" << fGenLeptonName[i] << " [" << i << "] <- [" << igen[i] 
					<< "] PT = " << fGenLepton[i].Pt() 
					<< " - ET = " << fGenLepton[i].Energy() 
					<< std::endl;
			}      
#endif
			FillGenPlots(_iAllEvents,puw);
		}
	}
	
	// All events
	//------------------------------------------------------------------
	FillHistoPerCut(_iAllEvents, puw, fsNTau);
  
	// Proccess ID
	//------------------------------------------------------------------
	int procn = _iOther;
	const int processID = fData->Get<int>("T_Event_processID");//GetEventprocessID();
	if(processID >= 0 && processID <= 4)  //ZJets
	{
		procn = _iVarious;
	}
	else if(processID == 22)  //ZZ
	{
		procn = _iZZ;
	}
	else if (processID == 23)  //WZ
	{
		procn = _iWZ;
	}
	else if (processID == 24)  //ZH
	{   
		procn = _iZH; 
	}
	else if (processID == 25)  //WW
	{
		procn = _iWW; 
	}
	else if (processID == 26)  //WH
	{
		procn = _iWH;
	}
	else if (processID >= 120 && processID <= 123 ) //ttH
	{
		procn = _ittH;
	}
	_histos[fHProcess]->Fill(procn);

	if(fIsWH && (procn != _iWH || fsNTau != fFS))
	{
		return;
	}
	
	FillHistoPerCut(_iIsWH, puw, fsNTau);

	// HLT: TBD...
	//------------------------------------------------------------------
	if( ! IspassHLT() )
	{
		return;
	}
	FillHistoPerCut(_iHLT, puw, fsNTau);

	// Vertex cut (Event stuff)-- OBSOLETE (implemented per default) SURE?
	//int iGoodVertex = GoodVertex();
	//int iGoodVertex = 0; // First one
	//fLeptonSelection->PassEventsCuts();
	//if( iGoodVertex < 0)
	//{
	//	return;
	//}
	FillHistoPerCut(_iGoodVertex, puw, fsNTau);
	

	// First looking at events with 2 leptons
	//const int kNMuons = 2; --> Defined in the preamble
	// Muon selection
	//------------------------------------------------------------------
	//
	//this->SetGoodVertexIndex(iGoodVertex);
	
	// Store the number of reconstructed leptons without any filter
	int nLeptonsbfCuts = 0;
	for(unsigned int i = 0; i < fLeptonName.size(); ++i)
	{
		std::string leptonpx = std::string("T_"+fLeptonName[i]+"_Px");
		nLeptonsbfCuts += fData->GetSize<float>(leptonpx.c_str());
	}
	_histos[fHNRecoLeptons]->Fill(nLeptonsbfCuts);
	
	// (1) Basic selection
	//--------------------
	unsigned int nSelectedMuons = fLeptonSelection->GetNBasicLeptons();
	_histos[fHNSelectedLeptons]->Fill(nSelectedMuons,puw);

	if(nSelectedMuons < kNMuons)
	{
		return;
	}
	
	FillHistoPerCut(_iHas2Leptons, puw, fsNTau);
	
	// (2) PV selection
	//--------------------
	unsigned int nSelectedPVMuons = fLeptonSelection->GetNLeptonsCloseToPV();
	_histos[fHNSelectedPVLeptons]->Fill(nSelectedPVMuons,puw); 
	
	if(nSelectedPVMuons < kNMuons)
	{
		return;
	}

	FillHistoPerCut(_iHas2PVLeptons, puw, fsNTau);


	// (3) Isolated muons
	//--------------------
	unsigned int nSelectedIsoMuons = fLeptonSelection->GetNIsoLeptons();
	_histos[fHNSelectedIsoLeptons]->Fill(nSelectedIsoMuons,puw);  
	
	if(nSelectedIsoMuons < kNMuons)
	{
		return;
	}
	
	FillHistoPerCut(_iHas2IsoLeptons, puw, fsNTau);
	
	// (4) Isolated good muons: Identification
	//--------------------
	unsigned int nSelectedIsoGoodMuons = fLeptonSelection->GetNGoodIdLeptons();
	_histos[fHNSelectedIsoGoodLeptons]->Fill(nSelectedIsoGoodMuons,puw);
	
	if(nSelectedIsoGoodMuons < kNMuons)
	{
		return;
	}
	
	FillHistoPerCut(_iHas2IsoGoodLeptons, puw, fsNTau);	
      
	// The leading lepton has to have PT > fCutMinMuPt (20 most probably)
	// Applying the pt-cuts
	//-------------------------------------------------------------------
	std::vector<double> * nLeptons= new std::vector<double>;
	// Describe the number of leptons we want to cut in order: 
	// --- nLeptons[0] = mmuons
	// --- nLeptons[1] = electrons 
	// Just needed for the mixing (and total?) case
	// FIXME: I don't like this patch but... 
	if( fFS == SignatureFS::_iFSmme )
	{
		nLeptons->push_back(2);
		nLeptons->push_back(1);
	}
	else if( fFS == SignatureFS::_iFSeem )
	{
		nLeptons->push_back(1);
		nLeptons->push_back(2);
	}

	if( ! fLeptonSelection->IsPass("PtMuonsCuts",nLeptons) )
	{
		return;
	}
	FillHistoPerCut(_iMuPTPattern, puw, fsNTau);
	delete nLeptons;
	nLeptons = 0;
	
	// Keep events with just 3 leptons and store momentum and charge
	//---------------------------------------------------------------------------
	if(nSelectedIsoGoodMuons != _nLeptons)
	{
		return;
	}
	// Indexs of good leptons
	std::vector<int> * theLeptons = fLeptonSelection->GetGoodLeptons(); 
	// + Fill histograms with Pt and Eta
	int k = 0;  // Note that k is the index of the vectors, not the TTree
	for(std::vector<int>::iterator it = theLeptons->begin(); it != theLeptons->end(); ++it) 
	{
		unsigned int i = *it;
		const char * name = "Elec";
		if( fLeptonSelection->GetLeptonType(k) == MUON )
		{
			name = "Muon";
		}
		const TLorentzVector vL = this->GetTLorentzVector(name,i);
		fHPtLepton[k]->Fill(vL.Pt(), puw);
		fHEtaLepton[k]->Fill(vL.Eta(), puw);
		if( fGenLepton.size() != 0 )
		{
			fHDeltaRGenRecoLepton[k]->Fill(vL.DeltaR(fGenLepton.at(k)), puw);
		}
		++k;
	}
	FillGenPlots(_iHasExactly3Leptons,puw);
	
	FillHistoPerCut(_iHasExactly3Leptons, puw, fsNTau);
	// + Store Momentum and charge for the muons : FIXME: This loop inside the
	// last one
	std::vector<TLorentzVector> lepton;
	std::vector<int> leptonCharge;
	k = 0;
	for(std::vector<int>::iterator it = theLeptons->begin(); it != theLeptons->end(); ++it) 
	{
		const unsigned int i = *it;
		const char * name = 0;
		LeptonTypes ileptontype = fLeptonSelection->GetLeptonType(k);
		std::string chargedm;
		if( ileptontype == MUON )
		{
			name = "Muon";
			chargedm = "T_Muon_Charge";
		}
		else
		{
			name = "Elec";
			chargedm = "T_Elec_Charge";
		}
		lepton.push_back( this->GetTLorentzVector(name,i) );
		// Change to: chargedm = "T_"+fLeptonSelector->GetLeptonTypeStr(i)+"charge"; 
		leptonCharge.push_back( fData->Get<int>(chargedm.c_str(),i) );
		++k;
	}
	
	// Discard extra electrons and extra muons
	// ------------------------------------------------------------------
	// 
	// Not yet implemented...
  
	// Discard 3 muons with the same charge
	// ------------------------------------------------------------------
	// 
	// + Add up charges. If the abs value of the total number is equal to N then 
	//   all have the same sign
	int charge = 0;
	for(std::vector<int>::iterator it = leptonCharge.begin(); it != leptonCharge.end(); ++it)
	{
		charge += *it;
	}
	// Fill muon charge before rejecting or accepting  
	_histos[fHLeptonCharge]->Fill(charge, puw);
	
	if( (unsigned int)TMath::Abs(charge) == theLeptons->size() )
	{
		return;
	}
	
	FillHistoPerCut(_iOppositeCharge, puw, fsNTau);
	
	// Find muons with opposite charges and calculate DeltaR, invMass...
	// Keep the pair with DeltaR minimum
	// ------------------------------------------------------------------
	// 
	// + Store the real vector index of the opposite charge muons in pairs
	//   (for the lepton and leptonCharge vectors: 0,...,nLeptons)
	std::vector<std::pair<int,int> > leptonPair;
	for(unsigned int kbegin = 0; kbegin < leptonCharge.size(); ++kbegin)
	{
		for(unsigned int kfromend = leptonCharge.size()-1; kfromend > kbegin; --kfromend) 
		{
			if( leptonCharge[kbegin]*leptonCharge[kfromend] < 0 )
			{
				leptonPair.push_back( std::pair<int,int>(kbegin,kfromend) );
			}
		}
	}
	// + Find Min/Max DeltaR and DeltaPhi
	// Using ordering from maps (from lower to higher)
	// Again uses the index of the lepton and leptonCharge vectors
	std::map<double,std::pair<int,int> > deltaRMuMu;
	std::map<double,std::pair<int,int> > deltaPhiMuMu;
	for(std::vector<std::pair<int,int> >::iterator it = leptonPair.begin(); 
			it != leptonPair.end(); ++it)
	{
		unsigned int i1 = it->first;
		unsigned int i2 = it->second;
		const double deltaR = lepton[i1].DeltaR(lepton[i2]);
		deltaRMuMu[deltaR] = *it;
		const double deltaPhi = TMath::Abs(lepton[i1].DeltaPhi(lepton[i2]));
		deltaPhiMuMu[deltaPhi] = *it;
	}
	
	double minDeltaRMuMu   = (deltaRMuMu.begin())->first;
	double maxDeltaRMuMu   = (deltaRMuMu.rbegin())->first;
	double minDeltaPhiMuMu = (deltaPhiMuMu.begin())->first;
	double maxDeltaPhiMuMu = (deltaPhiMuMu.rbegin())->first;
        
        // + Calculate inv mass of closest pair in R plane
	// Remember map<double,pair>  (second is the pair)
	const unsigned int i1 = ((deltaRMuMu.begin())->second).first;
	const unsigned int i2 = ((deltaRMuMu.begin())->second).second;
	const double invMassMuMuH = (lepton[i1] + lepton[i2]).M();

	// + Fill histograms
	//   - Smallest and Largest DeltaR between 2 opposite charged muons
	_histos[fHMinDeltaRLp1Lp2]->Fill(minDeltaRMuMu,puw);
	_histos[fHMaxDeltaRLp1Lp2]->Fill(maxDeltaRMuMu,puw);
	//   - Smallest and Largest Delta Phi between 2 opposite charged muons
	_histos[fHMinDeltaPhiLp1Lp2]->Fill(minDeltaPhiMuMu,puw);
	_histos[fHMaxDeltaPhiLp1Lp2]->Fill(maxDeltaPhiMuMu,puw);
	//   - Invariant mass of leptons supposedly from H
	_histos[fHHInvMass]->Fill(invMassMuMuH,puw);
  
	
	// Jet Veto:
	//------------------------------------------------------------------
	unsigned int nJets = 0;
	//for(unsigned int k = 0; k < fData->GetJetAKPFNoPUEnergy()->size(); ++k) 
	for(unsigned int k = 0; k < fData->GetSize<float>("T_JetAKPFNoPU_Energy"); ++k) 
	{
		TLorentzVector Jet = this->GetTLorentzVector("JetAKPFNoPU",k);
		//FIXME: Add the cuts to the config file
		if(Jet.Pt() > 30 && fabs(Jet.Eta()) < 5  && fabs(Jet.DeltaR(lepton[0])) > 0.3 
				&& fabs(Jet.DeltaR(lepton[1])) > 0.3 && 
				fabs(Jet.DeltaR(lepton[2])) > 0.3) 
		{
			nJets++;
		}
	}
	if(nJets > 0)
	{
		return;
	}
	FillHistoPerCut(_iJetVeto, puw, fsNTau);
  	
	// Cut in DeltaR
	std::vector<double> * auxVar= new std::vector<double>;
	auxVar->push_back(minDeltaRMuMu);
	//------------------------------------------------------------------
	if( ! fLeptonSelection->IsPass("DeltaRMuMuCut", auxVar) )
	{
		return;
	}
	
	FillHistoPerCut(_iDeltaR, puw, fsNTau);
  	
	// Check invariant mass of muons with opposite charge (outside Z)
	//------------------------------------------------------------------
	(*auxVar)[0] = invMassMuMuH;
	if( ! fLeptonSelection->IsPass("ZMassWindow", auxVar) )
	{
		return;
	}
	
	_histos[fHZInvMass]->Fill(invMassMuMuH,puw);
	FillHistoPerCut(_iZMuMuInvMass, puw, fsNTau);
	
	// MET
	//------------------------------------------------------------------
	const double met = fData->Get<float>("T_METPF_ET");
	(*auxVar)[0] = met;
	if( ! fLeptonSelection->IsPass("MinMET", auxVar) ) 
	{
		return;
	}
	delete auxVar;
	auxVar=0;
	
	_histos[fHMET]->Fill(met,puw);
	FillHistoPerCut(_iMET, puw, fsNTau);
}

//void AnalysisVH::Summary()
//{
//	std::cout << std::endl << "[ AnalisysVH::Sumary ]" << std::endl << std::endl;
//  
//	std::cout << "N. events by process ID:" << std::endl;
//	std::cout << "------------------------" << std::endl;
//	for(unsigned int i = 0; i < _iNProcesses; i++)
//	{
//		std::cout << kProcesses[i] << ":\t" << _histos[fHProcess]->GetBinContent(i+1) << " events ("
//			<< 100.0*_histos[fHProcess]->GetBinContent(i+1)/_histos[fHProcess]->GetEntries() << "%)" << std::endl;
//	}
//	std::cout << std::endl << std::endl;
//	std::cout << "N. events selected at each stage:" << std::endl;
//	std::cout << "---------------------------------" << std::endl;
//	for(unsigned int i = 0; i < _iNCuts; i++)
//	{
//		std::cout << _histos[fHEventsPerCut]->GetBinContent(i+1) << " ["
//			<< 100.0*_histos[fHEventsPerCut]->GetBinContent(i+1)/_histos[fHEventsPerCut]->GetBinContent(1)
//			<< "%] selected events (" << kCutNames[i] << ")" << std::endl;
//	}
//	std::cout << std::endl << std::endl;
//}
//
//
//
//
////---------------------------------------------------------------------
//// Other helper methods
////---------------------------------------------------------------------
////
//
//bool IspassHLT()
//{
//	//FIXME
//	return  true;
//	/*// Only data by the moment
//	if( ! fIsData )
//	{
//		return true;
//	}
//
//	bool passtrigger = false;
//	if( fFS == SignatureFS::_iFSmmm )
//	{
//		passtrigger = fData->Get<bool>("T_passTriggerDoubleMu");
//	}
//	else if( fFS == SignatureFS::_iFSeee )
//	{
//		passtrigger = fData->Get<bool>("T_passTriggerDoubleEl");
//	}
//	else if( fFS == SignatureFS::_iFSmme || fFS == SignatureFS::_iFSeem )
//	{
//		passtrigger = fData->Get<bool>("T_passTriggerElMu");
//	}
//	else if( fFS == SignatureFS::_iFSlll )
//	{
//		// Note that here we are going to mix DoubleElectron samples with 
//		// DoubleMuon Samples: so to avoid double counting:
//		// -- 
//		passtrigger = fData->Get<bool>("T_passTriggerElMu");
//		if( ! passtrigger )
//		{
//			passtrigger = pass->Get<bool>("T_passTriggerMu");
//		}
//		if( ! passtrigger )
//		{
//			passtrigger = pass->Get<bool>("T_passTriggerEl");
//		}
//	}
//
//	return passtrigger;*/
//}
//
//void AnalysisVH::FillHistoPerCut(const ECutLevel & cut,const double & puw, const unsigned int & fs) 
//{
//	_histos[fHEventsPerCut]->Fill(cut, puw);
//	if(fs == fFS)
//	{
//		_histos[fHEventsPerCut3Lepton]->Fill(cut, puw);
//	}
//}
//
//void AnalysisVH::FillGenPlots(ECutLevel cut, double puw) 
//{
//	if (fIsWH && fNGenLeptons == _nLeptons) 
//	{
//		for(unsigned int i = 0; i < fNGenLeptons; i++) 
//		{
//			fHGenPtLepton[i][cut]->Fill(fGenLepton[i].Pt(), puw);
//			fHGenEtaLepton[i][cut]->Fill(fGenLepton[i].Eta(), puw);
//		}
//	}
//}

