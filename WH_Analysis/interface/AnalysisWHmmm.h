
// -*- C++ -*-
//
// Package:    WH_Analysis
// Class:      AnalysisWHmmm
// 
/**\class  interface/AnalysisWHmmm.h src/AnalysisWHmmm.h

 Description: WH analysis for 3 muons.

 Implementation: This class is an concrete instance of the AnalysisVH
                 class which contain most of the methods, the only method
		 have to be implemented here is InsideLoop (and possibly
		 the Initialise in order to put the right labels to the
		 histograms)
*/
//
// Original Author: Jordi Duarte Campderros  
//         Created:  Sun Nov  2 14:21:31 CET 2011
// 
// jordi.duarte.campderros@cern.ch
//
//
#ifndef ANALYSISWHMMM_H
#define ANALYSISWHMMM_H

#include "AnalysisVH.h"

class AnalysisWHmmm : public AnalysisVH
{
	public:
		AnalysisWHmmm( TreeManager * data, InputParameters *ip,
				CutManager * selectorcuts, TTree * tree ) : 
			AnalysisVH(data,ip,selectorcuts,tree) { }

		~AnalysisWHmmm() { }

	protected:
		virtual void InsideLoop();
		//virtual void Initialise();
		virtual void FillHistoPerCut(const ECutLevel & cut,const double & puw, 
				const unsigned int & fs);  //FIXME de momento
		virtual void FillGenPlots(ECutLevel cut, double puw); //FIXME. de momento 

	private:
		AnalysisWHmmm();
		
	
		ClassDef(AnalysisWHmmm,0);
};


#endif
