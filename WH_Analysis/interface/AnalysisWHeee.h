
// -*- C++ -*-
//
// Package:    WH_Analysis
// Class:      AnalysisWHeee
// 
/**\class  interface/AnalysisWHeee.h src/AnalysisWHeee.h

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
#ifndef ANALYSISWHEEE_H
#define ANALYSISWHEEE_H

#include "AnalysisVH.h"

class AnalysisWHeee : public AnalysisVH
{
	public:
		AnalysisWHeee( TreeManager * data, InputParameters *ip,
				CutManager * selectorcuts, TTree * tree ) : 
			AnalysisVH(data,ip,selectorcuts,tree) { }

		~AnalysisWHeee() { }

	protected:
		virtual void InsideLoop();
		//virtual void Initialise();
		virtual void FillGenPlots(ECutLevel cut, double puw); //FIXME. de momento 

	private:
		AnalysisWHeee();
		
	
		ClassDef(AnalysisWHeee,0);
};


#endif
