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
#include<cstring>
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
			EFF,        // Total efficiencies
			SF,         // Total scale factors
			FR, 	    // Fake Rate
			PR,         // Prompt rate
			TR_LEADING, // Trigger efficiencies: Leading leg
			TR_TRAILING,// Trigger efficiencies: Trailing leg
			_n          // Not to use (internal)
		};
		//! Enums defining the systematic variation of weights
		enum
		{
			UP    = 1,  
			DOWN  = 2
		};

		//! Constructor
		WManager(const unsigned int & weighttype, const std::string & runperiod,
				const std::string & muonid,
				const int & systematic = 0);
		
		//! Copnstructor including systematics
		WManager(const unsigned int & weighttype, const std::string & runperiod,
				const std::string & muonid,
				const int & sytematics,
				const bool & isZJetsRegionFRMatrix); 

		//! Destructor
		virtual ~WManager();


		//! Get the weight 
		const double GetWeight(const LeptonTypes & lt, const double & pt, const double & eta); 

		//! Get the name of the weight type introduced as an  enum
		const char * GetWTStr(const unsigned int & wt) const;

		//! Get the file name used 
		const std::string GetFilename(const LeptonTypes & lt); 

	
	private:
		//! Initialize the class, decoupling both constructors
		void init(const int & sytematics,const bool & isZJetsRegionFRMatrix); 

		//! Set the root file where to find the TH2F with the weights
		void setweightfile(const LeptonTypes & lt, const char * filename, const int & systematic);
		
		//! get the complete path to the weight maps
		std::string getfile(const unsigned int & lepton, const bool & isZJetsRegionFRMatrix = false);

		//! get the internal name of the weight type (internal use)
		const std::string getstrtype( const unsigned int & wt) const;

		//! Run period
		std::string _runperiod;

		//! The muonID (used when 2011 run period)
		std::string _muonid;

		//! Weight type
		unsigned int _wtype;
		//! Weight map
		std::map<LeptonTypes,TH2F*> _weights;

		//! Informational file names
		std::map<LeptonTypes,std::string> _filesnames;
};

#endif
