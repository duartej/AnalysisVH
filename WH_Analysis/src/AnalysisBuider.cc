
#include "AnalysisBuilder.h"
#include "AnalysisVH.h"

#include "CMSAnalisisSelectorMiniTrees.h"

AnalysisVH * AnalysisBuilder::Build( const char * treeType/*, const int & analysisType */ )
{

	AnalysisVH * an = 0;

	// FIXME: Por el momento lo dejo asi, si incluyo tipo 
	// De analisis se tendran que cambiar unas cuantas cosas
	// para instanciar correctamente...

	// Tree type --> to decide selector
	if( treeType == "MiniTrees" )
	{
		an = new AnalysisVH( new CMSAnalysisSelectorMiniTrees );
	}

	return an;
}






