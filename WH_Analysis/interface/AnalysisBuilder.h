
// Crea un analisis: MiniTrees y posiblemente incorpore
// los diferentes tipos de analisis 3leptones, 5leptones, etc...
// Cuando esto se implemente la clase AnalysisVH pasa a ser una 
// clase abstracta y las clases concretas son las que seran 
// instancias en este builder

#ifndef ANALYSISBUILDER_H
#define ANALYSISBUILDER_H


#include "TreeTypes.h"
#include "LeptonTypes.h"

#include<map>

//class AnalysisVH;
class AnalysisBase;
class InputParameters;

class AnalysisBuilder
{
	public:
		//static AnalysisVH * Build( treeTypes thetype, const char * finalstate, std::map<LeptonTypes,InputParameters *> ip);
		static AnalysisBase * Build( const char * analysistype, treeTypes thetype,
				const char * finalstate, std::map<LeptonTypes,InputParameters *> ip);
};

#endif

