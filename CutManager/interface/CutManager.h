// -*- C++ -*-
//
// Package:    CutManager
// Class:      CutManager
// 
/**\class  CutManager.h src/CutManager.h

 Description: Abstract class in charge of apply cuts

 Implementation: Abstract class us
*/
//
// Original Author: Jordi Duarte Campderros  
//         Created:  Sun Oct  30 12:20:31 CET 2011
// 
// jordi.duarte.campderros@cern.ch
//
//
#ifndef CUTMANAGER_H
#define CUTMANAGER_H

#include<vector>
#include<map>
#include<iostream>

#include"TreeManager.h"

enum cuttype 
{
	_triggerCuts,
	_eventCuts,
	_ptCuts,
	_etaCuts,
	_IsoCuts,
	_IdCuts,
	_qualityCuts,
};


class CutManager
{
	public:
		//! Enum class to encode the specifics cuts (Undef): this has to be growing
		//! every time it is incorporated a new one cut: FIXME: Quizas dejar estos enums para las concretas??
		enum 
		{ 
			kMaxDeltaRMuMu, kMaxMuIP2DInTrack, kMaxDeltaZMu,
			kMaxZMass, kMinZMass 
		};

		//! Constructor
		CutManager(TreeManager * data, const int & nLeptons = 2); 
		//! Destructor
		virtual ~CutManager();
		
		//-- Cut definitions to be implemented in the 
		//   concrete classes
		//! Return if pass the trigger related cuts: FIXME
		virtual bool PassTriggerCuts()     = 0;
		//! Return if pass some Events related cuts: FIXME
		virtual bool PassEventCuts()       = 0;
		//! Return if the i-lepton passed the cuts
		virtual bool PassTopologicalCuts(const unsigned int & i,
				const double & pt, const double & eta) const = 0;
		virtual bool PassIsoCuts(const unsigned int & i)         = 0;
		virtual bool PassIdCuts(const unsigned int  & i)         = 0;
		virtual bool PassQualityCuts(const unsigned int & i)     = 0;
		virtual bool PassUndefCuts( const unsigned int & i, const int & cutindex ) = 0;
		
		//! Selection stuff
		//! Number of leptons which pass the basic selection. FIXME: Description
		inline unsigned int GetNBasicLeptons() {  return ( _selectedbasicLeptons ) ? SelectBasicLeptons() :  0 ; }
		//! Number of leptons closest to PV
		inline unsigned int GetNLeptonsCloseToPV() {  return ( _closeToPVLeptons ) ? SelectLeptonsCloseToPV() :  0 ; }

		//! Basic selection: usually consist in some loose kinematical cuts 
		//! and some loose id cuts
		virtual unsigned int SelectBasicLeptons() = 0; 
		//! Basic selection: usually consist in some loose kinematical cuts 
		//! and some loose id cuts
		virtual unsigned int SelectBasicLeptons() = 0; 
		//! Select leptons close to the Primary Vertex
		virtual unsigned int SelectMuonsCloseToPV() = 0;

		//-- Setters
		//! Set the number of leptons considered in the analysis client
		inline virtual void SetNLeptons( const unsigned int & nLeptons ) { _nLeptons = nLeptons; }
		//! Set the trigger cuts to be applied sequentially in the event
		virtual void SetTriggerCuts( const std::vector<double> & cuts);
		//! Set the event cuts to be applied sequentially in the event
		virtual void SetEventCuts( const std::vector<double> & cuts);
		//! Set the cuts to be applied to the i-lepton 
		virtual void SetPtMinCuts( const std::vector<double> & cuts);
		virtual void SetABSEtaMaxCuts( const std::vector<double> & cuts);
		virtual void SetIsoCuts( const std::vector<double> & cuts);
		virtual void SetIdCuts( const std::vector<double> & cuts);
		virtual void SetQualityCuts( const std::vector<double> & cuts);
		//! This function has to implemented with an enumerate type which defines
		//! the cutindex 
		virtual void SetUndefCuts( const std::vector<double> & cuts, const int & cutindex );


		friend std::ostream & operator<<(std::ostream & out, const CutManager & cm );
		//friend class TreeManager; // Accessing to the data members of TreeManager

	protected:
		// Exits if the cuts are not initialized
		void checkercutinit(const cuttype & cutclass) const;

		void setcut( const cuttype & cutclass, const std::vector<double> & cuts );
		//! Container of the data
		TreeManager * _data;

		//! Cut information: cuts are applied (when proceed) in the lepton 
		//!                  pt decreasing order; it means that the first
		//!                  element of the cut vector will apply the cut
		//!                  to the highest pt lepton
		std::map<cuttype, std::vector<double> *> * _cuts;
		//! Cut information for the undefined and specific cuts
		std::map<int, std::vector<double> *> * _undefcuts;

		//! Number of leptons to be considered in the analysis
		unsigned int _nLeptons;
		
		//! Selection datamembers
		//! Vector of index of leptons which pass the basic selection
		std::vector<int> * _selectedbasicLeptons;
		//! Vector of index of leptons closest to PV
		std::vector<int> * _closeToPVLeptons;
		//! Vector of leptons indices which have been pass all the cuts
		std::vector<int> * _idxLeptons;

	ClassDef(CutManager,0);
};

//! Print method
inline std::ostream & operator<<(std::ostream & out, const CutManager & cutmanager)
{
	out << "==== Selection Cuts: " << std::endl;
	if( cutmanager._cuts != 0 )
	{
		for(std::map<cuttype,std::vector<double> *>::iterator it = cutmanager._cuts->begin();
				it != cutmanager._cuts->end(); ++it)
		{
			out << "  Type-" << it->first << ": [ ";
			if( it->second != 0)
			{
				for(unsigned int i = 0; i < it->second->size(); ++i)
				{
					out << (*it->second)[i] << " ";
				}
			}
			out << "]" << std::endl;;
		}
	}
	
	if( cutmanager._undefcuts != 0 )
	{
		for(std::map<int,std::vector<double> *>::iterator it = cutmanager._undefcuts->begin();
				it != cutmanager._undefcuts->end(); ++it)
		{
			out << " Undef Type-" << it->first << ": [ ";
			if( it->second != 0)
			{
				for(unsigned int i = 0; i < it->second->size(); ++i)
				{
					out << (*it->second)[i] << " ";
				}
			}
			out << "]" << std::endl;
		}
	}
		

	return out;
}



#endif
