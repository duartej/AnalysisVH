// -*- C++ -*-
//
// Package:    WManager
// Class:      WManager
// 
/**\class  WManager

 Class to deal with general weights pt and eta dependent. This is the abstract class
 containing the general methods shared by all the weights. The weights are stored
 in TH2F histograms saved in root files, the root files are part of the package and
 can be found in the 'data' directory.
*/
//
// @author Original Author: Jordi Duarte Campderros  
//         Created:  Mon Feb  20 14:57:31 CET 2012
// 
// @author jordi.duarte.campderros@cern.ch
//
//
#ifndef WMANAGER_H
#define WMANAGER_H

#include<map>
#include<iostream>

#include "LeptonTypes.h"

class TFile;
class TH2F;

class WManager
{
	public:
		//! Enums defining the weight known weights
		enum 
		{
			EFF,      // Total efficiencies
			SF,       // Total scale factors
			FR, 	  // Fake Rate
			PR,       // Prompt rate
			_n        // Not to use (internal)
		};

		//! Constructor
		WManager(const unsigned int & weighttype, const bool & isSystematics = false); 

		//! Destructor
		virtual ~WManager();


		//! Get the weight 
		const double GetWeight(const LeptonTypes & lt, const double & pt, 
				const double & eta); 

		//! Get the name of the weight type introduced as an  enum
		const char * GetWTStr(const unsigned int & wt) const;
	
	private:
		//! Set the root file where to find the TH2F with the weights
		void setweightfile(const LeptonTypes & lt, const char * filename);
		
		//! get the complete path to the weight maps
		std::string getfile(const unsigned int & lepton, const bool & isSystematics);

		//! Weight type
		unsigned int _wtype;
		//! Weight map
		std::map<LeptonTypes,TH2F*> _weights;
};

#endif
