
// Crea un analisis: MiniTrees y posiblemente incorpore
// los diferentes tipos de analisis 3leptones, 5leptones, etc...
// Cuando esto se implemente la clase AnalysisVH pasa a ser una 
// clase abstracta y las clases concretas son las que seran 
// instancias en este builder

#ifndef ANALYSISBUILDER_H
#define ANALYSISBUILDER_H

class AnalysisVH;
class InputParameters;

class AnalysisBuilder
{
	public:
		static AnalysisVH * Build( const char * treeType, InputParameters *ip/*, const int & analisisType */ );

};

#endif
