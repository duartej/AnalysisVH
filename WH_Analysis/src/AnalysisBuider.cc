
#include "AnalysisBuilder.h"
#include "AnalysisVH.h"

#include "TreeManagerMiniTrees.h"
#include "MuonSelection.h"

//FIXME:PROV@
#include<iostream>

// Or template ??
AnalysisVH * AnalysisBuilder::Build( treeTypes thetype, int leptonskind, InputParameters *ip, TTree * tree )
{

	AnalysisVH * an = 0;

	// FIXME: Por el momento lo dejo asi, si incluyo tipo 
	// De analisis se tendran que cambiar unas cuantas cosas
	// para instanciar correctamente...

	// Tree type --> to decide selector
	if( thetype == MiniTrees )
	{
		TreeManagerMiniTrees * data = new TreeManagerMiniTrees(tree);
		// The selector: si son Muones...
		MuonSelection * selectioncuts = new MuonSelection(data);

		an = new AnalysisVH( data, ip, selectioncuts, tree );
	}

	return an;
}

