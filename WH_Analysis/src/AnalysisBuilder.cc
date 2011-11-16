
#include "AnalysisBuilder.h"
#include "AnalysisVH.h"
//#include "AnalysisWHmmm.h"
//#include "AnalysisWHeee.h"

//#include "TreeManagerMiniTrees.h"
#include "MuonSelection.h"
#include "ElecSelection.h"
#include "LeptonTypes.h"
#include "SignatureFS.h"

#include<iostream>
#include<stdlib.h>

// Or template ??--> Clase lista para construir diferentes analysis AnalysisVH <-- AnalysisWH (3leptones), <-- Analysis...
AnalysisVH * AnalysisBuilder::Build( treeTypes thetype, const char * finalstateStr, InputParameters *ip, TTree *tree )
{

	AnalysisVH * an = 0;

	// Signature of the analysis:
	unsigned int finalstate = SignatureFS::GetFSID(finalstateStr);
	// what leptons I need
	LeptonTypes lepton1;
	//LeptonTypes lepton2;
	//LeptonTypes lepton3;
	
	if( finalstate == SignatureFS::_iFSmmm )
	{
		lepton1 = MUON;
	}
	else if( finalstate == SignatureFS::_iFSeee )
	{
		lepton1 = ELECTRON;
	}
	else
	{
		std::cerr << "AnalysisBuilder::Build: '" << finalstateStr << "'"
			<< " Not implemented yet. Exiting..."
			<< std::endl;
		exit(-1);
	}


	// Tree type --> to decide selector: TO BE DEPRECATED
	//if( thetype == MiniTrees )
	//{
		//TreeManagerMiniTrees * data = new TreeManagerMiniTrees(tree);
	TreeManager * data = new TreeManager();
	if( lepton1 == MUON )
	{
		// The selector: si son Muones...
		MuonSelection * selectioncuts = new MuonSelection(data);
		an = new AnalysisVH( data, ip, selectioncuts, finalstate);
	}
	else if( lepton1 == ELECTRON )
	{
		ElecSelection * selectioncuts = new ElecSelection(data);
		an = new AnalysisVH( data, ip, selectioncuts, finalstate);
	}				
	//}

	return an;
}

