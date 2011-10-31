#ifndef ANALYSISVH_H
#define ANALYSISVH_H 1

// Blabla description

#include<vector>
#include<cstring>

#include "CMSAnalysisSelector.h"
#include "TreeManager.h"

class InputParameters;
class TTree;
class CutManager;


class AnalysisVH : public CMSAnalysisSelector 
{
	public: 
		// State prepare analisis
		AnalysisVH( TreeManager * data, InputParameters * ip, 
				CutManager * selectorcuts, TTree * tree );
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
		bool HLT();

	private:
		AnalysisVH();

		// Cut selections
		CutManager * _selectioncuts;

		// Variables describing dataset
		// -----------------------------------------------------------------------
		TString fDataName; // Dataset Name
		bool    fIsData;   // True if it should be treated as data
		bool    fIsWH;     // True if it should be treated as signal (WH)
		
		// Data files to analyse
		std::vector<std::string> _datafiles;

		// Luminosity: FIXME: Possibly to the mother since has to be used for each
		//----------------------------------------------------------------------------
		double fLuminosity;

		// FIXME: NECESARIO???
		TTree * _tree;

	public:
		ClassDef(AnalysisVH,0);

};

#endif

