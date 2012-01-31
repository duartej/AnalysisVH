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
				const double & eta) const; 

	private:
		std::map<LeptonTypes,TH2F*> _fakerate;
		std::map<LeptonTypes,TH2F*> _promptrate;


	//ClassDef(FOManager,0);
};

//! Print method
/*inline std::ostream & operator<<(std::ostream & out, const FOManager & cutmanager)
{
	out << "|========== FOManager Print ============|" << std::endl;
	out << "| Operational mode:" << cutmanager._samplemode << std::endl;
	if( cutmanager._samplemode == FOManager::FAKEABLESAMPLE )
	{
		out << "| ++ Sample: ";
		for(unsigned int j = 0; j < cutmanager._nTights; ++j)
		{
			out << "Tight ";
		}
		for(unsigned int j = 0; j < cutmanager._nFails; ++j)
		{
			out << "No-Tight ";
		}
		out << std::endl;
	}
	out << "|============ Selection Cuts: " << std::endl;
	if( cutmanager._cuts != 0 )
	{
		for(std::map<std::string,double>::iterator it = cutmanager._cuts->begin();
				it != cutmanager._cuts->end(); ++it)
		{
			out << " + " << it->first << ": " << it->second << std::endl;
		}
	}
	out << "|============ Selection Cuts ============| " << std::endl;

	return out;
}*/



#endif
