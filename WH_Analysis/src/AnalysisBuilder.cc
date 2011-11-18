
#include "AnalysisBuilder.h"
#include "AnalysisVH.h"
//#include "AnalysisWHmmm.h"
//#include "AnalysisWHeee.h"

//#include "TreeManagerMiniTrees.h"
#include "MuonSelection.h"
#include "ElecSelection.h"
#include "SignatureFS.h"

#include<iostream>
#include<stdlib.h>

// Or template --> Clase lista para construir diferentes analysis AnalysisVH <-- AnalysisWH (3leptones), <-- Analysis...
// De momento es mas un builder del cut manager
AnalysisVH * AnalysisBuilder::Build( treeTypes thetype, const char * finalstateStr, std::map<LeptonTypes,InputParameters *> ipmap )
{

	CutManager * selectioncuts = 0;

	// Signature of the analysis:
	unsigned int finalstate = SignatureFS::GetFSID(finalstateStr);
	
	TreeManager * data = new TreeManager();
	if( finalstate == SignatureFS::_iFSmmm )
	{
		// The selector
		selectioncuts = new MuonSelection(data);
	}
	else if( finalstate == SignatureFS::_iFSeee )
	{
		selectioncuts = new ElecSelection(data);
	}
	else if( finalstate == SignatureFS::_iFSeem ||
			finalstate == SignatureFS::_iFSmme )
	{
		selectioncuts = new LeptonMixingSelection(data);
	}
	else
	{
		std::cerr << "AnalysisBuilder::Build: '" << finalstateStr << "'"
			<< " Not implemented yet. Exiting..."
			<< std::endl;
		exit(-1);
	}

	return new AnalysisVH( data, ipmap, selectioncuts, finalstate);
}

