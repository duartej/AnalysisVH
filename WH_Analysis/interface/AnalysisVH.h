#ifndef ANALYSISVH_H
#define ANALYSISVH_H 1

// Blabla description

#include<vector>
#include<cstring>

#include "CMSAnalysisSelector.h"
#include "TreeManager.h"

class InputParameters;
class TTree;


class AnalysisVH : public CMSAnalysisSelector 
{
	public: 
		// State prepare analisis
		AnalysisVH( TreeManager * data, InputParameters * ip );
		// State runanalisis
		//AnalysisVH( CMSAnalysisSelector * consel) : _cmsselector(conselr) { } 
		virtual ~AnalysisVH();

	protected:
		TreeManager * _data; 
		
		virtual void InitialiseParameters();
		virtual void Initialise();
		virtual void InsideLoop();
		virtual void Summary();

		// Selection methods
		bool HLT() const;

	private:
		AnalysisVH();

		// Variables describing dataset
		// -----------------------------------------------------------------------
		std::vector<std::string> _datafiles;

		TTree * _tree;

	public:
		ClassDef(AnalysisVH,0);

};

#endif
