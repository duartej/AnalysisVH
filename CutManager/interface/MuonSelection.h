
///////////////////////////////////////////////////////////////////////
//
//    FILE: MuonSelection.h
//   CLASS: MuonSelection
// AUTHORS: I. Gonzalez Caballero
//    DATE: 09/05/2011
//
// CONTENT: This class for PAF standarizes the selection of good muons 
//          for analysis
//
// Changelog: Incorporates some getters functions to the 
//            CMSAnalysisSelector class in order to avoid the creation
//            of every MuonSelector class for every diferent kind of
//            tree. Now, the responsable to call the current selector
//            to use is the client, not the MuonSelector class.
//                                       J. Duarte Campderros Oct, 2011
///////////////////////////////////////////////////////////////////////
// -*- C++ -*-
//
// Package:    MuonSelection
// Class:      MuonSelection
// 
/**\class  MuonSelection.h src/MuonSelection.cc

 Description: Concrete class to select muons

 Implementation: 
*/
//
// Modifications Author: Jordi Duarte Campderros  
//         Created:  Sun Oct  30 12:20:31 CET 2011
// 
// jordi.duarte.campderros@cern.ch
//
//
#ifndef MUONSELECTION_H
#define MUONSELECTION_H

#include "TROOT.h"
#include "CutManager.h"


class MuonSelection : public CutManager
{
	public:

		MuonSelection( TreeManager * data, const int & nLeptons = 2 ) : CutManager(data) { }
		virtual ~MuonSelection() { }
		
		//-- Cut definitions
		virtual bool PassTriggerCuts();
		virtual bool PassEventCuts();
		virtual std::vector<int> * PassTopologicalCuts();
		virtual std::vector<int> * PassIsoCuts();
		virtual std::vector<int> * PassIdCuts();
		virtual std::vector<int> * PassQualityCuts();
		virtual std::vector<int> * PassUndefCuts( const int & cutindex );

	ClassDef(MuonSelection,0);
};
#endif
