
#include "AnalysisBuilder.h"
#include "AnalysisVH.h"

#include "MiniTreesManager.h"

//FIXME:PROV@
#include<iostream>

// Or template ??
AnalysisVH * AnalysisBuilder::Build( const char * treeType, InputParameters *ip /*, const int & analysisType */ )
{

	AnalysisVH * an = 0;

	// FIXME: Por el momento lo dejo asi, si incluyo tipo 
	// De analisis se tendran que cambiar unas cuantas cosas
	// para instanciar correctamente...

	// Tree type --> to decide selector
	std::cout << "[" << treeType << "]" << std::endl;
	if( treeType /*== "MiniTrees"*/ )
	{
		MiniTreesManager * data = new MiniTreesManager;
		an = new AnalysisVH( data, ip );
	}

	return an;
}
