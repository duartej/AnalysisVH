#ifndef ANALYSISVH_H
#define ANALYSISVH_H 1

// Blabla description

#include<vector>
#include<cstring>

#include "TTree.h"

class CMSAnalysisSelector;


class AnalysisVH : public CMSAnalysisSelector 
{
	public: 
		// State prepare analisis
		AnalysisVH( CMSAnalysisSelector * consel, InputParameters * ip, Tree * tree = 0 ) : 
			_cmsselector(consel) { }
		// State runanalisis
		//AnalysisVH( CMSAnalysisSelector * consel) : _cmsselector(conselr) { } 
		~AnalysisVH();

	protected:
		CMSAnalysisSelector * _cmsselector; 
		
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
};

