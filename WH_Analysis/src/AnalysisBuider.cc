
#include "AnalysisBuilder.h"
#include "AnalysisVH.h"

#include "TreeManagerMiniTrees.h"

//FIXME:PROV@
#include<iostream>

// Or template ??
AnalysisVH * AnalysisBuilder::Build( treeTypes thetype, InputParameters *ip /*, const int & analysisType */ )
{

	AnalysisVH * an = 0;

	// FIXME: Por el momento lo dejo asi, si incluyo tipo 
	// De analisis se tendran que cambiar unas cuantas cosas
	// para instanciar correctamente...

	// Tree type --> to decide selector
	std::cout << "[" << thetype << "]" << std::endl;
	if( thetype == MiniTrees )
	{
		TreeManagerMiniTrees * data = new TreeManagerMiniTrees;
		an = new AnalysisVH( data, ip );
	}

	return an;
}

