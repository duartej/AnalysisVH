// -*- C++ -*-
//
// Package:    FOManager
// Class:      FOManager
// 
/**\class  FOManager

 Description: Class to deal with the fakeable objects sample
   and its application in a concrete analysis

 Implementation: Abstract class us
*/
//
// @author Original Author: Jordi Duarte Campderros  
//         Created:  Tue Jan  31 20:52:31 CET 2012
// 
// @author jordi.duarte.campderros@cern.ch
//
//
#ifndef FOMANAGER_H
#define FOMANAGER_H

#include<map>
#include<iostream>

#include "LeptonTypes.h"
#include "TH2F.h"

class TFile;

class FOManager
{
	public:
		//! Constructor
		FOManager(); 

		//! Destructor
		~FOManager();

		//! Setters
		void SetFR(const LeptonTypes & lt, const char * filename);
		void SetPR(const LeptonTypes & lt, const char * filename);

		//! Get the weight 
		const double GetWeight(const LeptonTypes & lt, const double & pt, 
				const double & eta); 

	private:
		std::map<LeptonTypes,TH2F*> _fakerate;
		std::map<LeptonTypes,TH2F*> _promptrate;
	//ClassDef(FOManager,0);
};

#endif
