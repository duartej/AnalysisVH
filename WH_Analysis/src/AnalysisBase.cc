#include<sstream>
#include<stdlib.h>
#include<cstring>
#include<iostream>
#include<functional>
#include<algorithm>
#include<cassert>

#include "AnalysisBase.h"
#include "LeptonRel.h"
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
const unsigned int kTauPID = 15; //Found with TDatabasePDG::Instance()->GetParticle("tau-")->PdgCode()a

//Momentum/energy scale SYSTEMATICS
const float MUONPTSYS = 0.01;
const float ELECPTBARRELSYS = 0.02;
const float ELECPTENDCAPSYS = 0.05;
const float METMUONPTSYS = 0.015;
const float METELECPTSYS = 0.025;
const float METJETSPTSYS = 0.05;
const float PUVARSYS = 0.05;


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
	fFOZJetsRegion(0),
	fPO(0),
	fSF(0),
	fTRL(0),
	fTRT(0),
	_cuttree(0),
	_cutweight(1),
	_cutvalue(-1),
	_eventnumber(-1),
	_runnumber(-1),
	_evtlisttree(0),
	_wcharge(0),
	_jetname(""),
	_issysrun(false),
	_namesys(""),
	_typesys(-1),
	_modesys(0),
	_mptsys(MUONPTSYS),
	_eptbarrelsys(ELECPTBARRELSYS),
	_epteesys(ELECPTENDCAPSYS),
	_metsysmode(0),
	_metmuonpt(METMUONPTSYS),
	_metelecpt(METELECPTSYS),
	_metjetspt(METJETSPTSYS),
	_pusys(1.0),
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
	const char * muonidchar = 0;  //FIXME: patch while do not merge the input parameters
	for(std::map<LeptonTypes,InputParameters*>::iterator ipI = ipmap.begin();
			ipI != ipmap.end(); ++ipI)
	{
		// FIXME: patch while do not merge the input parameters
		//        in only one
		if( ipI->first == MUON )
		{
			muonidchar = ipI->second->TheNamedString("MuonID");
		}

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
	// FIXME: Provisional patch to avoid the segmentation if do not intialize muonidchar
	if( muonidchar == 0 )
	{
		std::cout << "\033[1;33mAnalysisBase WARNING\033[1;m The inputsheet for muons" <<
			" doesn't contain the 'MuonID' parameter, so forced to be 'VBTF'" <<
			std::endl;
		muonidchar = "VBTF";
	}
	// FIXME END 
	
	// Extract the run period (FIXME: Check if RunPeriod exist, otherwise it
	// will crash with a logic error exception
	fRunPeriod = fInputParameters->TheNamedString("RunPeriod");

	// Set jet name
	_jetname = "JetAKPFNoPU";
	if( fRunPeriod.find("2012") != std::string::npos )
	{
		_jetname = "JetAKCHS";
	}
	
	// If 2011, check the ID for muons
	std::string muonid("");
	if( fRunPeriod.find("2011") != std::string::npos )  //FIXME see above: Bug when introducing -c ELECTRON
	//  only, -- > muonidchar = 0 so initializing a string with the NULL char gives you a segmentation
	{
		muonid=muonidchar;		
		// Transforming to lower case
		std::transform(muonid.begin(),muonid.end(),muonid.begin(), ::tolower);
	}

	// Check if we have to deal with systematics and which one: FIXME: Do that in a function
	const char * sys = fInputParameters->TheNamedString("Systematic");
	std::map<int,int> systypemode;
	// Initialize
	// Systematic related with the scale factors: using sf+-sigma 
	systypemode[AnalysisBase::LEPTONSYS] = 0;
	// Systematic related with the fake rate matrices errors, using fr+-sigma
	systypemode[AnalysisBase::FRSYS] = 0;
	// Systematic related with the muon momentum scale 
	systypemode[AnalysisBase::MMSSYS] = 0;
	// Systematic related with the electron energy scale
	systypemode[AnalysisBase::EESSYS] = 0;
	// Systematic related with the energy scale and resolution for MET
	systypemode[AnalysisBase::METSYS] = 0;
	// Systematic related with the Pile up estimation
	systypemode[AnalysisBase::PUSYS] = 0;
	if( sys )
	{
		_issysrun = true;
		const std::string systr(sys);
		// Parsing content  TYPESYS:UP|DOWN
		const size_t pos_colon = systr.find(":");
		if( pos_colon == std::string::npos )
		{
			std::cerr << "\033[1;31mAnalysisBase::AnalysisBase ERROR:\033[1;m Parsing 'Systematic'"
				<< " from InputParameters with wrong format: '" << systr << "' The argument must"
				<< " have exactly the format: SYSTEMATICTYPE:VAR where VAR should be either UP"
				<< " or DOWN" << std::endl;
			exit(-1);
		}
		_namesys = systr.substr(0,pos_colon);
		// Checking it is defined
		if( _namesys == "LEPTONSYS" )
		{
			_typesys = AnalysisBase::LEPTONSYS;
		}
		else if( _namesys == "FRSYS" )
		{
			_typesys = AnalysisBase::FRSYS;
		}
		else if( _namesys == "MMSSYS" )
		{
			_typesys = AnalysisBase::MMSSYS;
		}
		else if( _namesys == "EESSYS" )
		{
			_typesys = AnalysisBase::EESSYS;
		}
		else if( _namesys == "METSYS" )
		{
			_typesys = AnalysisBase::METSYS;
		}
		else if( _namesys == "PUSYS" )
		{
			_typesys = AnalysisBase::PUSYS;
		}
		else
		{
			std::cerr << "\033[1;31mAnalysisBase::AnalysisBase ERROR:\033[1;m Parsing 'Systematic'"
				<< " from InputParameters with wrong systematic type: '" << systr << "' "
				<< " Valid arguments are 'LEPTONSYS' 'FRSYS' MMSSYS' 'EESSYS' 'METSYS' 'PUSYS'"
				<< std::endl;
			exit(-1);
		}
		// Checking the key is in there
		if( systypemode.find(_typesys) == systypemode.end() )
		{
			std::cerr << "\033[1;31mAnalysisBase::AnalysisBase ERROR:\033[1;m Parsing 'Systematic'"
				<< " from InputParameters with systematic type: '" << _namesys << "' This type"
				<< " is not implemented yet. Exiting..." << std::endl;
			exit(-1);
		}
		
		const std::string mode = systr.substr(pos_colon+1);
		// Set-up all the systematics environment (return false if fails something)
		if( ! this->initializeSys(mode) )
		{
			std::cerr << "\033[1;31mAnalysisBase::AnalysisBase ERROR:\033[1;m Parsing 'Systematic'"
				<< " from InputParameters with wrong format: '" << systr << "' The argument must"
				<< " have exactly the format: SYSTEMATICTYPE:VAR where VAR should be either UP"
				<< " or DOWN" << std::endl;
			exit(-1);
		}
		systypemode[_typesys] = _modesys;
	}
	// End parsing systematics

	// Initialize the scale factors and trigger
	fSF  = new WManager( WManager::SF, fRunPeriod, muonid, systypemode[AnalysisBase::LEPTONSYS] );
	fTRL = new WManager( WManager::TR_LEADING, fRunPeriod, muonid, systypemode[AnalysisBase::LEPTONSYS] );
	fTRT = new WManager( WManager::TR_TRAILING, fRunPeriod, muonid, systypemode[AnalysisBase::LEPTONSYS] );

	// Are in fake sample mode?
	if( fLeptonSelection->IsInFakeableMode() ) 
	{
		fFO = new WManager( WManager::FR, fRunPeriod, muonid, systypemode[AnalysisBase::FRSYS] );
		fFOZJetsRegion = new WManager( WManager::FR, fRunPeriod, muonid, systypemode[AnalysisBase::FRSYS], true );
		fPO = new WManager( WManager::PR, fRunPeriod, muonid, systypemode[AnalysisBase::FRSYS] ); 
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
	
	if( fFOZJetsRegion != 0 )
	{
		delete fFOZJetsRegion;
		fFOZJetsRegion = 0;
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

	if( fTRL != 0 )
	{
		delete fTRL;
		fTRL = 0;
	}
	
	if( fTRT != 0 )
	{
		delete fTRT;
		fTRT = 0;
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

// Systematics management
bool AnalysisBase::initializeSys(const std::string & variation)
{
	if( variation == "UP" )
	{
		_modesys = WManager::UP;
		if( _typesys == AnalysisBase::EESSYS )
		{
			// Muons do not affect by Electron Energy scale
			_mptsys = 1.0;
			_eptbarrelsys = (1.0+_eptbarrelsys);
			_epteesys = (1.0+_epteesys);
			// And activate the scales
			fLeptonSelection->SetPtSystematicFactor(_mptsys,_eptbarrelsys,_epteesys);
		}
		else if( _typesys == AnalysisBase::MMSSYS )
		{
			_mptsys = (1.0+_mptsys);
			// Electrons do not affect by Muon Momentum scale
			_eptbarrelsys = 1.0;
			_epteesys = 1.0;
			// And activate the scales
			fLeptonSelection->SetPtSystematicFactor(_mptsys,_eptbarrelsys,_epteesys);
		}
		else if( _typesys == AnalysisBase::METSYS)
		{
			_metsysmode = 1;
			_metmuonpt = 1.0+_metmuonpt;
			_metelecpt = 1.0+_metelecpt;
			_metjetspt = 1.0+_metjetspt;
		}
		else if( _typesys == AnalysisBase::PUSYS )
		{
			_pusys = 1.0+PUVARSYS;
		}
	}
	else if( variation == "DOWN" )
	{
		_modesys = WManager::DOWN;
		if( _typesys == AnalysisBase::EESSYS )
		{
			// Muons do not affect by Electron Energy scale
			_mptsys = 1.0;
			_eptbarrelsys = (1.0-_eptbarrelsys);
			_epteesys = (1.0-_epteesys);
			// And activate the scales
			fLeptonSelection->SetPtSystematicFactor(_mptsys,_eptbarrelsys,_epteesys);
		}
		if( _typesys == AnalysisBase::MMSSYS )
		{
			_mptsys = (1.0-_mptsys);
			// Electrons do not affect by Muon Momentum scale
			_eptbarrelsys = 1.0;
			_epteesys = 1.0;
			// And activate the scales
			fLeptonSelection->SetPtSystematicFactor(_mptsys,_eptbarrelsys,_epteesys);
		}
		else if( _typesys == AnalysisBase::METSYS)
		{
			_metsysmode = -1;
			_metmuonpt = 1.0-_metmuonpt;
			_metelecpt = 1.0-_metelecpt;
			_metjetspt = 1.0-_metjetspt;
		}
		else if( _typesys == AnalysisBase::PUSYS )
		{
			_pusys = 1.0-PUVARSYS;
		}
	}
	else
	{
		return false;
	}

	return true;

}
	
const double AnalysisBase::GetMET(const std::vector<LeptonRel> * const theLeptons) const
{
	const double met = (double)fData->Get<float>("T_METPFTypeI_ET");
	if( _metsysmode == 0)
	{
		return met;
	}

	// Else we're evaluating the energy and resolution systematic, due
	// to the lack of all variables we'll do a correction in an approximate
	// MET because there are not available collection in the Tree. So, the
	// approximate MET is evaluated as follows (vectorial sum),
	// approxMET = -\sum jets (not containing a lepton) - \sum electrons -\sum muons
	// The MET is recalculated using a new pt variated for jets,elec and muons getting
	// smearMET = -\sum jets*scaledpt -\sum elec*scaledpt -\sum muon*scaledpt
	// After that the resolution variation is evaluated,
	// resolMET = |approxMET-smearMET|/approxMET
	// And it is used to correct the real calculated MET as follows
	// correctMET = MET*(1.0+/-resolMET)
	// Note that when evaluate 1sigma up(down), the scales for the jets,muons,elecs pt are used
	// with 1sigma up(down) 
	TLorentzVector aproxMET;
	TLorentzVector smearMET;
	for(unsigned int k = 0; k < fData->GetSize<float>(std::string("T_"+_jetname+"_Energy").c_str()); ++k) 
	{
		TLorentzVector Jet = this->GetTLorentzVector(_jetname.c_str(),k);
		// Extract jets < 30 and eta > 5
		if( Jet.Pt() <= 30 || fabs(Jet.Eta()) >= 5 )
		{
			Jet.SetPz(0.0);
			Jet.SetE(0.0);
			aproxMET -= Jet;
	
			Jet.SetPx(Jet.Px()*_metjetspt);
			Jet.SetPy(Jet.Py()*_metjetspt);
			smearMET -= Jet;
			continue;
		}
		// Otherwise, jets > 30 and eta < 5
		// Leptons not inside the Jets
		bool leptoninsideJet = false;
		for(unsigned int j = 0; j < theLeptons->size(); ++j)
		{
			if( fabs(Jet.DeltaR((*theLeptons)[j].getP4())) <= 0.3 )
			{
				leptoninsideJet = true;
				break;
			}
		}
		if( leptoninsideJet )
		{
			continue;
		}
		Jet.SetPz(0.0);
		Jet.SetE(0.0);
		aproxMET -= Jet;

		Jet.SetPx(Jet.Px()*_metjetspt);
		Jet.SetPy(Jet.Py()*_metjetspt);
		smearMET -= Jet;
	}
	
	// Aproximate MET,
	//------------------------------------------------------------------
	// Electrons contributing
	for(unsigned int i=0; i < fData->GetSize<float>("T_Elec_Px"); ++i)
	{
		TLorentzVector elec( fData->Get<float>("T_Elec_Px",i),
			fData->Get<float>("T_Elec_Py",i),0.0,0.0);
		aproxMET -= elec;
		
		elec.SetPx(elec.Px()*_metelecpt);
		elec.SetPy(elec.Py()*_metelecpt);
		smearMET -= elec;

	}
	// Muons contributing
	for(unsigned int i=0; i < fData->GetSize<float>("T_Muon_Px"); ++i)
	{
		TLorentzVector muon( fData->Get<float>("T_Muon_Px",i),
			fData->Get<float>("T_Muon_Py",i),0.0,0.0) ;
		aproxMET -= muon;
		
		muon.SetPx(muon.Px()*_metmuonpt);
		muon.SetPy(muon.Py()*_metmuonpt);
		smearMET -= muon;
	}
	
	const double resolMET = fabs(aproxMET.Pt()-smearMET.Pt())/aproxMET.Pt();
	const double correctMET = met*(1.0+_metsysmode*resolMET);

	//std::cout << "PFMET=" << met << " correcMET=" << correctMET 
	//	<< " myMET without unclustered E=" << aproxMET.Pt() << std::endl;
	return correctMET;
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
	// FIXED BUG: Using BeginsWith method in order to avoid the MC Fakes
	else if(fDataName.Contains("Data") || fDataName.BeginsWith("Fakes") )
	{
		fIsData = true;
	}
	// Square of the Sum of the weight when dealing with MC or Fakes (weighted) or Fakes (weighted)
	if( ! fIsData || fDataName.BeginsWith("Fakes") )
	{
		TH1::SetDefaultSumw2();
	}

	// if data, not needed 
	if( fIsData )
	{
		delete fSF; 
		fSF = 0;
		delete fTRL;
		fTRL = 0;
		delete fTRL;
		fTRT = 0;
	}

	
	// Luminosity
	//--------------------------------
	ip->TheNamedDouble("Luminosity", fLuminosity);
	// Taking the variation +-5% in the lumi when systematic for PU
	if( _issysrun != 0 )
	{
		fLuminosity *= _pusys;
	}

	
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
		// Switch to this one, after some checks seems to match better
		MCdist = Fall11;
	}
	else if( fRunPeriod == "2012A" || fRunPeriod == "2012B" || fRunPeriod == "2012" )
	{
		MCdist = Summer12_53X; 
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

// TO BE DEPRECATED---> The inclusion of systematics has to be done in other way
// Overloaded function for take into account momentum systematic
/*const TLorentzVector AnalysisBase::GetTLorentzVector( const char * name, const int & index, const int & leptontype) const
{
	 TLorentzVector aux = this->GetTLorentzVector(name,index);
	// Aplying the correction factor
	float systematic = 0.0;
	switch(leptontype)
	{
		case ELECTRON:
			if( fabs(aux.Eta()) < 1.479 )
			{
				systematic = _eptbarrelsys;
			}
			else
			{
				systematic = _epteesys;
			}
			break;
		case MUON:
			systematic = _mptsys;
	}
	//TLorentzVector *auxpr = new TLorentzVector;
	aux.SetPtEtaPhiE(aux.Pt()*systematic,aux.Eta(),aux.Phi(),aux.E());

	return aux;
}*/

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
		std::cout << " + Trigger Efficiencies used: |" << std::endl;
		std::cout << "                              + MUON |- Leading leg : ";
		if( fFS == SignatureFS::_iFSmmm ||
				fFS == SignatureFS::_iFSeem || SignatureFS::_iFSmme )
		{
			std::cout << fTRL->GetFilename(MUON) << std::endl;
			std::cout << "                                     |- Trailing leg: " << fTRL->GetFilename(MUON) << std::endl;
		}
		
		std::cout << "                              + ELEC |- Leading leg : ";
		if( fFS == SignatureFS::_iFSeee ||
				fFS == SignatureFS::_iFSeem || SignatureFS::_iFSmme )
		{
			std::cout << fTRL->GetFilename(ELECTRON) << std::endl;
			std::cout << "                                     |- Trailing leg: " << fTRL->GetFilename(ELECTRON) << std::endl;
		}
	}
	if( fLeptonSelection->IsInFakeableMode() ) 
	{
		std::cout << " + FAKEABLE MODE ENABLED: | " << std::endl;
		if( fFS == SignatureFS::_iFSmmm ||
				fFS == SignatureFS::_iFSeem || SignatureFS::_iFSmme )
		{
			std::cout << "                          + [MUON] ZJets Region: " << fFOZJetsRegion->GetFilename(MUON) << std::endl;
			std::cout << "                          + [MUON] ttbar Region: " << fFO->GetFilename(MUON) << std::endl;

		}
		
		if( fFS == SignatureFS::_iFSeee ||
				fFS == SignatureFS::_iFSeem || SignatureFS::_iFSmme )
		{
			std::cout << "                          + [ELEC] ZJets Region: " << fFOZJetsRegion->GetFilename(ELECTRON) << std::endl;
			std::cout << "                          + [ELEC] ttbar Region: " << fFO->GetFilename(ELECTRON) << std::endl;
		}
	}
	if( _issysrun )
	{
		std::cout << " + SYSTEMATIC MODE ACTIVATED for '" << _namesys << "': ";
		if( _modesys == WManager::UP )
		{
			std::cout << "UP";
		}
		else
		{
			std::cout << "DOWN";
		}
		std::cout << std::endl;
	}
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
	//return true;
	//Only data by the moment
	if( ! fIsData )
	{
		return true;
	}

	bool passtrigger = false;
	if( fFS == SignatureFS::_iFSmmm || fFS == SignatureFS::_iFSmme )
	{
		passtrigger = fData->Get<bool>("T_passTriggerDoubleMu");
	}
	else if( fFS == SignatureFS::_iFSeee || fFS == SignatureFS::_iFSeem )
	{
		passtrigger = fData->Get<bool>("T_passTriggerDoubleEl");
	}

	return passtrigger;
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

void AnalysisBase::StoresEvtInf(const TLorentzVector & zcand1, const TLorentzVector & zcand2,
		const TLorentzVector & wcand, const double & transversmass, const TLorentzVector & METV)
{
        _evtinfo.run = fData->Get<int>("T_Event_RunNumber");
        _evtinfo.lumi = fData->Get<int>("T_Event_LuminosityBlock");
        _evtinfo.evt  = fData->Get<int>("T_Event_EventNumber");
        _evtinfo.channel = fFS;

        _evtinfo.zmass = (zcand1+zcand2).M();
        _evtinfo.zlep1pt = zcand1.Pt();
        _evtinfo.zlep1eta = zcand1.Eta();
        _evtinfo.zlep1phi = zcand1.Phi();

        _evtinfo.zlep2pt = zcand2.Pt();
        _evtinfo.zlep2eta = zcand2.Eta();
        _evtinfo.zlep2phi = zcand2.Phi();

        _evtinfo.wmt = transversmass;
        _evtinfo.wleppt = wcand.Pt();
        _evtinfo.wlepeta = wcand.Eta();
        _evtinfo.wlepphi = wcand.Phi();

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

// Trigger weight
double AnalysisBase::GetTriggerWeight(const std::vector<LeptonRel> * const theLeptons) const
{
	// The weight is split in:
	// * 3mu, 3e channels:
	//	p(HLT_MuL_MuT) = 1 -[ (1-eL1)(1-eL2)(1-eL3)+eL1(1-eT2)(1-eT3)+
	//                                    +eL2(1-eT1)(1-eT3)+eL3(1-eT1)(1-eT2)]
	// * 2mu1e, 2e1mu channels
	//	p(HLT_MuL_MuT) = 1- [ (1-eL1)(1-eL2)+eL1(1-eT2)+eL2(1-eT1)]

	// Find out trigger channel and select the relevant leptons (mu or e). 	
	const int nMuons = SignatureFS::GetNMuons( fFS );
	const int nElecs = SignatureFS::GetNElecs( fFS );
	LeptonTypes mainLepton = UNDEFINED;
	bool ismixedchannel = false;
	if( nMuons >= 2 )
	{
		mainLepton = MUON;
		if( nMuons < 3 )
		{
			ismixedchannel = true;
		}

	}
	else if( nElecs >= 2 )
	{
		mainLepton = ELECTRON;
		if( nElecs < 3 )
		{
			ismixedchannel = true;
		}
	}
	else
	{
		std::cerr << "\033[1;31mAnalysisBase::GetTriggerWeight ERROR:\033[1;m" 
			<<" Invalid final state '" << fFS << "'. This is an unexpected error"
			<< " and should be reported as bug. Exiting... " << std::endl;
		exit(-1);
	}
	
	// pair of leading leg eff: eff,(1-eff)
	std::vector<std::pair<double,double> > effL;
	// pair of trailing leg eff: eff,(1-eff)
	std::vector<std::pair<double,double> > effT;	
	for(std::vector<LeptonRel>::const_iterator it = theLeptons->begin(); it != theLeptons->end();
			++it)
	{
		const double pt  = it->getP4().Pt();
		const double eta = it->getP4().Eta();
		const LeptonTypes ilt = it->leptontype();
		//If is mixed channel, just take care of the trigger leptons
		if( ismixedchannel && ilt != mainLepton )
		{
			continue;
		}
		const double eL = fTRL->GetWeight(ilt,pt,eta);
		const double eT = fTRT->GetWeight(ilt,pt,eta);
		effL.push_back(std::pair<double,double>(eL,(1.0-eL)));
		effT.push_back(std::pair<double,double>(eT,(1.0-eT)));
	}

	// leading fail term: (1-eL1)(1-eL2)....
	double lFterm = 1.0;
	// mixed term: eLi(1-eLj)...
	double mterm = 0.0;
	assert( effL.size() == effT.size() && "Bug found! The number of leading and"
			" trailing efficiencies must be equal" );
	const unsigned int nleptons = effL.size();  
	for(unsigned int i = 0; i < nleptons; ++i)
	{
		// for the fail leading term
		lFterm *= effL[i].second;
		// leading term in the mixed term
		double auxmterm = effL[i].first;
		for(unsigned k = 0; k < nleptons; ++k)
		{
			if( k == i )
			{
				continue;
			}
			auxmterm *= effT[k].second;
		}
		mterm += auxmterm;
	}

	const double weight = 1.0-(lFterm+mterm);

	return weight;
}

// Fake rate estimations
//===============================================================

// Simplistic calculation using full approximations p\sim 1, f->0
double AnalysisBase::GetPPFWeightApprx(const bool & zjetsregion)
{
	// As we are using the approximation PromptRate=1, then
	// PPF (3,2) = fF0->GetWeight
	// PFF (3,1) = (fFO->GetWeight)^2
	// FFF (3,0) = (fFO->GetWeight)^3
	// Where (N,T) are actually the number of Total leptons and PROMPT leptons. 
	// This equivalence between tight-prompt can be done because of the approximations
	// used. So, each no-tight lepton is weighted in order to get its probability to be
	// fake.
	
	// Take the proper FR Matrix depending the region of interestt
	WManager * properFR = fFO;
	if( zjetsregion )
	{
		properFR = fFOZJetsRegion;
	}

	double puw = 1.0;
	for(std::vector<LeptonRel>::iterator it = fLeptonSelection->GetNoTightLeptons()->begin(); 
			it != fLeptonSelection->GetNoTightLeptons()->end(); ++it)
	{
		const double pt  = it->getP4().Pt();
		const double eta = it->getP4().Eta();
		const LeptonTypes ileptontype = it->leptontype();
		puw *= properFR->GetWeight(ileptontype,pt,eta);
	}

	return puw;
}

// PPF estimation (full calculation)
double AnalysisBase::GetPPFWeight(const bool & zjetsregion)
{
	// Take the proper FR Matrix depending the region of interest
	WManager * properFR = fFO;
	if( zjetsregion )
	{
		properFR = fFOZJetsRegion;
	}

	// Weighting rules:
	//  PROMPT ESTIMATED:     p(1-f)  for each passing 
	//                        pf      for each failing
	//  FAKE ESTIMATED:       f(1-p)  for each passing
	//                        pf      for each failing
	//  common factor: 1/(p-f)
	std::vector<double> p; // index ordered in tight-noTight
	std::vector<double> f;
	// 1. Tight 
	for(std::vector<LeptonRel>::iterator it = fLeptonSelection->GetTightLeptons()->begin(); 
			it != fLeptonSelection->GetTightLeptons()->end(); ++it)
	{
		const double pt  = it->getP4().Pt();
		const double eta = it->getP4().Eta();
		const LeptonTypes ileptontype = it->leptontype();
		p.push_back(fPO->GetWeight(ileptontype,pt,eta));
		f.push_back(properFR->GetWeight(ileptontype,pt,eta));
	}
	// 2. NoTight (or failing)
	for(std::vector<LeptonRel>::iterator it = fLeptonSelection->GetNoTightLeptons()->begin(); 
			it != fLeptonSelection->GetNoTightLeptons()->end(); ++it)
	{
		const double pt  = it->getP4().Pt();
		const double eta = it->getP4().Eta();
		const LeptonTypes ileptontype = it->leptontype();
		p.push_back( fPO->GetWeight(ileptontype,pt,eta) );
		f.push_back( properFR->GetWeight(ileptontype,pt,eta) );
	}
	
	const unsigned int ntight = fLeptonSelection->GetNAnalysisTightLeptons();
	const unsigned int nfailing = fLeptonSelection->GetNAnalysisNoTightLeptons();
	// Weights applied to each lepton as map-> index: weight
	// Estimated as prompt pass (tight) and fail (no tight)
	std::map<int,double> wPromptPass; 
	std::map<int,double> wPromptFail; 
	// Estimated as fake pass (tight) and fail (no tight)
	std::map<unsigned int,double> wFakePass; 
	std::map<unsigned int,double> wFakeFail; 
	for(unsigned int k = 0; k < ntight; ++k)
	{
		wPromptPass[k] = (1.0/(p[k]-f[k]))*p[k]*(1.0-f[k]);
		wFakePass[k] = (1.0/(p[k]-f[k]))*f[k]*(1.0-p[k]);
	}

	for(unsigned int k = ntight; k < ntight+nfailing; ++k)
	{
		wPromptFail[k] = (1.0/(p[k]-f[k]))*p[k]*f[k];
		wFakeFail[k] = (1.0/(p[k]-f[k]))*f[k]*p[k];
	}

	// The contributions to the PPF estimation: given the observed (tight or no-tight)
	// ordered as 0,1,2,... (see p and f vectors), the three different contributions are
	// calculated assuming the observed i-essim leptons is estimated as P(prompt) or F(fake)
	double PPF = 0;  
	double PFP = 0;
	double FPP = 0;
	switch(ntight)
	{
		case 0: //Nt0
			PPF = wPromptFail[0]*wPromptFail[1]*wFakeFail[2];
			PFP = wPromptFail[0]*wFakeFail[1]*wPromptFail[2];
			FPP = wFakeFail[0]*wPromptFail[1]*wPromptFail[0];
			break;
		case 1: //Nt1
			PPF = wPromptPass[0]*wPromptFail[1]*wFakeFail[2];
			PFP = wPromptPass[0]*wFakeFail[1]*wPromptFail[2];
			FPP = wFakePass[0]*wPromptFail[1]*wPromptFail[2];
			break;
		case 2: //Nt2
			PPF = wPromptPass[0]*wPromptPass[1]*wFakeFail[2];
			PFP = wPromptPass[0]*wFakePass[1]*wPromptFail[2];
			FPP = wFakePass[0]*wPromptPass[1]*wPromptFail[2];
			break;
		case 3: //Nt3
			PPF = wPromptPass[0]*wPromptPass[1]*wFakePass[2];
			PFP = wPromptPass[0]*wFakePass[1]*wPromptPass[2];
			FPP = wFakePass[0]*wPromptPass[1]*wPromptPass[2];
			break;
		default:
			std::cerr << "\033[1;31mAnalysisBase::AnalysisBase::GetPPFWeight ERROR:\033[1;m" 
				<<" The 'Nt=" << ntight << "' contribution is not coded."
				<< " Exiting... " << std::endl;
			exit(-1);
	}
	
	return PPF+PFP+FPP;
}

// PFF estimation (full calculation)
double AnalysisBase::GetPFFWeight(const bool & zjetsregion)
{
	// Take the proper FR Matrix depending the region of interest
	WManager * properFR = fFO;
	if( zjetsregion )
	{
		properFR = fFOZJetsRegion;
	}

	// Weighting rules:
	//  PROMPT ESTIMATED:     p(1-f)  for each passing 
	//                        pf      for each failing
	//  FAKE ESTIMATED:       f(1-p)  for each passing
	//                        pf      for each failing
	//  common factor: 1/(p-f)
	std::vector<double> p; // index ordered in tight-noTight
	std::vector<double> f;
	// 1. Tight 
	for(std::vector<LeptonRel>::iterator it = fLeptonSelection->GetTightLeptons()->begin(); 
			it != fLeptonSelection->GetTightLeptons()->end(); ++it)
	{
		const double pt  = it->getP4().Pt();
		const double eta = it->getP4().Eta();
		const LeptonTypes ileptontype = it->leptontype();
		p.push_back(fPO->GetWeight(ileptontype,pt,eta));
		f.push_back(properFR->GetWeight(ileptontype,pt,eta));
	}
	// 2. NoTight (or failing)
	for(std::vector<LeptonRel>::iterator it = fLeptonSelection->GetNoTightLeptons()->begin(); 
			it != fLeptonSelection->GetNoTightLeptons()->end(); ++it)
	{
		const double pt  = it->getP4().Pt();
		const double eta = it->getP4().Eta();
		const LeptonTypes ileptontype = it->leptontype();
		p.push_back( fPO->GetWeight(ileptontype,pt,eta) );
		f.push_back( properFR->GetWeight(ileptontype,pt,eta) );
	}
	
	const unsigned int ntight = fLeptonSelection->GetNAnalysisTightLeptons();
	const unsigned int nfailing = fLeptonSelection->GetNAnalysisNoTightLeptons();
	
	// Weights applied to each lepton as map-> index: weight
	// Estimated as prompt pass (tight) and fail (no tight)
	std::map<int,double> wPromptPass; 
	std::map<int,double> wPromptFail; 
	// Estimated as fake pass (tight) and fail (no tight)
	std::map<unsigned int,double> wFakePass; 
	std::map<unsigned int,double> wFakeFail; 
	for(unsigned int k = 0; k < ntight; ++k)
	{
		wPromptPass[k] = (1.0/(p[k]-f[k]))*p[k]*(1.0-f[k]);
		wFakePass[k] = (1.0/(p[k]-f[k]))*f[k]*(1.0-p[k]);
	}

	for(unsigned int k = ntight; k < ntight+nfailing; ++k)
	{
		wPromptFail[k] = (1.0/(p[k]-f[k]))*p[k]*f[k];
		wFakeFail[k] = (1.0/(p[k]-f[k]))*f[k]*p[k];
	}

	// The contributions to the PPF estimation: given the observed (tight or no-tight)
	// ordered as 0,1,2,... (see p and f vectors), the three different contributions are
	// calculated assuming the observed i-essim leptons is estimated as P(prompt) or F(fake)
	double PFF = 0;  
	double FPF = 0;
	double FFP = 0;
	switch(ntight)
	{
		case 0: //Nt0
			PFF = wPromptFail[0]*wFakeFail[1]*wFakeFail[2];
			FPF = wFakeFail[0]*wPromptFail[1]*wFakeFail[2];
			FFP = wFakeFail[0]*wFakeFail[1]*wPromptFail[0];
			break;
		case 1: //Nt1
			PFF = wPromptPass[0]*wFakeFail[1]*wFakeFail[2];
			FPF = wFakePass[0]*wPromptFail[1]*wFakeFail[2];
			FFP = wFakePass[0]*wFakeFail[1]*wPromptFail[2];
			break;
		case 2: //Nt2
			PFF = wPromptPass[0]*wFakePass[1]*wFakeFail[2];
			FPF = wFakePass[0]*wPromptPass[1]*wFakeFail[2];
			FFP = wFakePass[0]*wFakePass[1]*wPromptFail[2];
			break;
		case 3: //Nt3
			PFF = wPromptPass[0]*wFakePass[1]*wFakePass[2];
			FPF = wFakePass[0]*wPromptPass[1]*wFakePass[2];
			FFP = wFakePass[0]*wFakePass[1]*wPromptPass[2];
			break;
		default:
			std::cerr << "\033[1;31mAnalysisBase::AnalysisBase::GetPPFWeight ERROR:\033[1;m" 
				<<" The 'Nt=" << ntight << "' contribution is not coded."
				<< " Exiting... " << std::endl;
			exit(-1);
	}
	
	return PFF+FPF+FFP;
}

// FFF estimation (full calculation)
double AnalysisBase::GetFFFWeight(const bool & zjetsregion)
{
	// Take the proper FR Matrix depending the region of interest
	WManager * properFR = fFO;
	if( zjetsregion )
	{
		properFR = fFOZJetsRegion;
	}

	// Weighting rules:
	//  PROMPT ESTIMATED:     p(1-f)  for each passing 
	//                        pf      for each failing
	//  FAKE ESTIMATED:       f(1-p)  for each passing
	//                        pf      for each failing
	//  common factor: 1/(p-f)
	std::vector<double> p; // index ordered in tight-noTight
	std::vector<double> f;
	// 1. Tight 
	for(std::vector<LeptonRel>::iterator it = fLeptonSelection->GetTightLeptons()->begin(); 
			it != fLeptonSelection->GetTightLeptons()->end(); ++it)
	{
		const double pt  = it->getP4().Pt();
		const double eta = it->getP4().Eta();
		const LeptonTypes ileptontype = it->leptontype();
		p.push_back(fPO->GetWeight(ileptontype,pt,eta));
		f.push_back(properFR->GetWeight(ileptontype,pt,eta));
	}
	// 2. NoTight (or failing)
	for(std::vector<LeptonRel>::iterator it = fLeptonSelection->GetNoTightLeptons()->begin(); 
			it != fLeptonSelection->GetNoTightLeptons()->end(); ++it)
	{
		const double pt  = it->getP4().Pt();
		const double eta = it->getP4().Eta();
		const LeptonTypes ileptontype = it->leptontype();
		p.push_back( fPO->GetWeight(ileptontype,pt,eta) );
		f.push_back( properFR->GetWeight(ileptontype,pt,eta) );
	}
	
	const unsigned int ntight = fLeptonSelection->GetNAnalysisTightLeptons();
	const unsigned int nfailing = fLeptonSelection->GetNAnalysisNoTightLeptons();
	
	// Weights applied to each lepton as map-> index: weight
	// Estimated as prompt pass (tight) and fail (no tight)
	std::map<int,double> wPromptPass; 
	std::map<int,double> wPromptFail; 
	// Estimated as fake pass (tight) and fail (no tight)
	std::map<unsigned int,double> wFakePass; 
	std::map<unsigned int,double> wFakeFail; 
	for(unsigned int k = 0; k < ntight; ++k)
	{
		wPromptPass[k] = (1.0/(p[k]-f[k]))*p[k]*(1.0-f[k]);
		wFakePass[k] = (1.0/(p[k]-f[k]))*f[k]*(1.0-p[k]);
	}

	for(unsigned int k = ntight; k < ntight+nfailing; ++k)
	{
		wPromptFail[k] = (1.0/(p[k]-f[k]))*p[k]*f[k];
		wFakeFail[k] = (1.0/(p[k]-f[k]))*f[k]*p[k];
	}

	// The contributions to the PPF estimation: given the observed (tight or no-tight)
	// ordered as 0,1,2,... (see p and f vectors), the three different contributions are
	// calculated assuming the observed i-essim leptons is estimated as P(prompt) or F(fake)
	double FFF = 0;  
	switch(ntight)
	{
		case 0: //Nt0
			FFF = wFakeFail[0]*wFakeFail[1]*wFakeFail[2];
			break;
		case 1: //Nt1
			FFF = wFakePass[0]*wFakeFail[1]*wFakeFail[2];
			break;
		case 2: //Nt2
			FFF = wFakePass[0]*wFakePass[1]*wFakeFail[2];
			break;
		case 3: //Nt3
			FFF = wFakePass[0]*wFakePass[1]*wFakePass[2];
			break;
		default:
			std::cerr << "\033[1;31mAnalysisBase::AnalysisBase::GetPPFWeight ERROR:\033[1;m" 
				<<" The 'Nt=" << ntight << "' contribution is not coded."
				<< " Exiting... " << std::endl;
			exit(-1);
	}
	
	return FFF;
}

// PPP estimation (full calculation)
double AnalysisBase::GetPPPWeight(const bool & zjetsregion)
{
	// Take the proper FR Matrix depending the region of interest
	WManager * properFR = fFO;
	if( zjetsregion )
	{
		properFR = fFOZJetsRegion;
	}

	// Weighting rules:
	//  PROMPT ESTIMATED:     p(1-f)  for each passing 
	//                        pf      for each failing
	//  FAKE ESTIMATED:       f(1-p)  for each passing
	//                        pf      for each failing
	//  common factor: 1/(p-f)
	std::vector<double> p; // index ordered in tight-noTight
	std::vector<double> f;
	// 1. Tight 
	for(std::vector<LeptonRel>::iterator it = fLeptonSelection->GetTightLeptons()->begin(); 
			it != fLeptonSelection->GetTightLeptons()->end(); ++it)
	{
		const double pt  = it->getP4().Pt();
		const double eta = it->getP4().Eta();
		const LeptonTypes ileptontype = it->leptontype();
		p.push_back(fPO->GetWeight(ileptontype,pt,eta));
		f.push_back(properFR->GetWeight(ileptontype,pt,eta));
	}
	// 2. NoTight (or failing)
	for(std::vector<LeptonRel>::iterator it = fLeptonSelection->GetNoTightLeptons()->begin(); 
			it != fLeptonSelection->GetNoTightLeptons()->end(); ++it)
	{
		const double pt  = it->getP4().Pt();
		const double eta = it->getP4().Eta();
		const LeptonTypes ileptontype = it->leptontype();
		p.push_back( fPO->GetWeight(ileptontype,pt,eta) );
		f.push_back( properFR->GetWeight(ileptontype,pt,eta) );
	}
	
	const unsigned int ntight = fLeptonSelection->GetNAnalysisTightLeptons();
	const unsigned int nfailing = fLeptonSelection->GetNAnalysisNoTightLeptons();
	// Weights applied to each lepton as map-> index: weight
	// Estimated as prompt pass (tight) and fail (no tight)
	std::map<int,double> wPromptPass; 
	std::map<int,double> wPromptFail; 
	// Estimated as fake pass (tight) and fail (no tight)
	std::map<unsigned int,double> wFakePass; 
	std::map<unsigned int,double> wFakeFail; 
	for(unsigned int k = 0; k < ntight; ++k)
	{
		wPromptPass[k] = (1.0/(p[k]-f[k]))*p[k]*(1.0-f[k]);
		wFakePass[k] = (1.0/(p[k]-f[k]))*f[k]*(1.0-p[k]);
	}

	for(unsigned int k = ntight; k < ntight+nfailing; ++k)
	{
		wPromptFail[k] = (1.0/(p[k]-f[k]))*p[k]*f[k];
		wFakeFail[k] = (1.0/(p[k]-f[k]))*f[k]*p[k];
	}

	// The contributions to the PPF estimation: given the observed (tight or no-tight)
	// ordered as 0,1,2,... (see p and f vectors), the three different contributions are
	// calculated assuming the observed i-essim leptons is estimated as P(prompt) or F(fake)
	double PPP = 0;  
	switch(ntight)
	{
		case 0: //Nt0
			PPP = wPromptFail[0]*wPromptFail[1]*wPromptFail[2];
			break;
		case 1: //Nt1
			PPP = wPromptPass[0]*wPromptFail[1]*wPromptFail[2];
			break;
		case 2: //Nt2
			PPP = wPromptPass[0]*wPromptPass[1]*wPromptFail[2];
			break;
		case 3: //Nt3
			PPP = wPromptPass[0]*wPromptPass[1]*wPromptPass[2];
			break;
		default:
			std::cerr << "\033[1;31mAnalysisBase::AnalysisBase::GetPPFWeight ERROR:\033[1;m" 
				<<" The 'Nt=" << ntight << "' contribution is not coded."
				<< " Exiting... " << std::endl;
			exit(-1);
	}
	
	return PPP;
}
