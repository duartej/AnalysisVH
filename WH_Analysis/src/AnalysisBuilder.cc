
#include "AnalysisBuilder.h"
#include "AnalysisBase.h"
#include "AnalysisVH.h"
#include "AnalysisWZ.h"
//#include "AnalysisWHmmm.h"
//#include "AnalysisWHeee.h"

//#include "TreeManagerMiniTrees.h"
#include "MuonSelection.h"
#include "ElecSelection.h"
#include "SignatureFS.h"
#include "WPElecID.h"

#include<iostream>
#include<stdlib.h>
#include<string.h>


// FIXME: TO BE DEPRECATED treeTypes
// Or template --> Clase lista para construir diferentes analysis AnalysisVH <-- AnalysisWH (3leptones), <-- Analysis...
// De momento es mas un builder del cut manager
//AnalysisVH * AnalysisBuilder::Build( treeTypes thetype, const char * finalstateStr, std::map<LeptonTypes,InputParameters *> ipmap )
AnalysisBase * AnalysisBuilder::Build( const char * analysistype, treeTypes thetype, 
		const char * finalstateStr, std::map<LeptonTypes,InputParameters *> ipmap )
{

	CutManager * selectioncuts = 0;

	// Signature of the analysis:
	unsigned int finalstate = SignatureFS::GetFSID(finalstateStr);
	
	// Check the working point for electrons to use
	int WPlowPt = -1;
	int WPhighPt = -1;
	if( strcmp(analysistype,"WH") == 0 || strcmp(analysistype,"wh") == 0 )
	{
		WPlowPt = WPElecID::WP_70;
		WPhighPt= WPElecID::WP_80;
	}
	else if( strcmp(analysistype,"WZ") == 0 || strcmp(analysistype,"wz") == 0 )
	{
		WPlowPt = WPElecID::WP_80;//WP_95;
		WPhighPt= WPElecID::WP_80;//WP_95; 
		// This has to be changed in the analysis code, when you
		// are in the W stage you should to tight the cut (WP80)
	}
	else
	{
		std::cerr << "AnalysisBuilder::Build: '" << analysistype << "'"
			<< " Not implemented yet. Exiting..."
			<< std::endl;
		exit(-1);
	}
	
	TreeManager * data = new TreeManager();
	if( finalstate == SignatureFS::_iFSmmm )
	{
		// The selector
		selectioncuts = new MuonSelection(data);
	}
	else if( finalstate == SignatureFS::_iFSeee )
	{
		selectioncuts = new ElecSelection(data,WPlowPt,WPhighPt);
	}
	else if( finalstate == SignatureFS::_iFSeem ||
			finalstate == SignatureFS::_iFSmme )
	{
		selectioncuts = new LeptonMixingSelection(data,WPlowPt,WPhighPt);
	}
	else
	{
		std::cerr << "AnalysisBuilder::Build: '" << finalstateStr << "'"
			<< " Not implemented yet. Exiting..."
			<< std::endl;
		exit(-1);
	}

	// Analysis type
	AnalysisBase * analysis = 0;
	if( strcmp(analysistype,"WH") == 0 || strcmp(analysistype,"wh") == 0 )
	{
		analysis = new AnalysisVH(data,ipmap,selectioncuts,finalstate);
	}
	else if( strcmp(analysistype,"WZ") == 0 || strcmp(analysistype,"wz") == 0 )
	{
		analysis = new AnalysisWZ(data,ipmap,selectioncuts,finalstate);
	}

	return analysis;
	//return new AnalysisVH( data, ipmap, selectioncuts, finalstate);
}

