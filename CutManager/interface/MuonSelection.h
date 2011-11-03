
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
		virtual bool PassTopologicalCuts(const unsigned int & i, 
				const double & pt, const double & eta) const;
		virtual bool PassIsoCuts(const double & i,
				const double & pt, const double & eta) const;
		virtual bool PassIdCuts(const unsigned int & i, 
				const double & ptResolution) const; 
		virtual bool PassQualityCuts(const unsigned int & i);
		virtual bool PassUndefCuts(const unsigned int & i,
				const int & cutindex );

		//-- Selection
		//---------------------------------------------
		// Select basic muons: 
		// - with pt > MinPt and fabs(eta) < eta 
		// - not standalone onl
		unsigned int SelectBasicLeptons();
		// Select close to PV muons: 
		// - Depends on kMaxMuIP2DInTrack and kMaxDeltaZMu Undef cuts
		unsigned int SelectLeptonsCloseToPV();
		// Select Iso Leptons: 
		// - Depends on MaxPTIsolationR# dependent of the region
		unsigned int SelectIsoLeptons();
		// Select Good Identified Leptons: 
		// - Depends on 
		unsigned int SelectGoodIdLeptons();

	ClassDef(MuonSelection,0);
};
#endif
