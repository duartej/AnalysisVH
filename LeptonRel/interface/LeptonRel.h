// -*- C++ -*-
//
// Package:    LeptonRel
// Class:      LeptonRel
// 
/**\class  LeptonRel

 Description: Class to encapsulate the minimum info about the two leptons
 used in analysis (muon, electron), keeping the relation indices with respect
 the original ntuple

 Implementation:
*/
//
// @author Original Author: Jordi Duarte Campderros  
//         Created:  Tue Nov  20 18:46:31 CET 2012
// 
// @author jordi.duarte.campderros@cern.ch
//
//
#ifndef LEPTONREL_H
#define LEPTONREL_H

#include<cstring>
#include "TLorentzVector.h"
#include "LeptonTypes.h"

class LeptonRel
{
	public:
		//! Constructors
		LeptonRel(): _p4(0), _index(-1), _q(-999),_leptontype(UNDEFINED),_name("") { ; }
		LeptonRel(const unsigned int & index);  // Probably to be deprecated
		LeptonRel(const TLorentzVector & p, const unsigned int & index);
		LeptonRel(const TLorentzVector & p, const unsigned int & index, const int & charge, const LeptonTypes & leptontype);
		LeptonRel(const LeptonRel & lr); // copy constructor
		virtual ~LeptonRel();

		//Equally operator
		LeptonRel & operator=(const LeptonRel & l1);

		//! boolean operator
		bool operator ==(const LeptonRel & l1) const;
		inline bool operator !=(const LeptonRel & l1) const { return ! this->operator==(l1); }
		
		//! Accessors
		inline const TLorentzVector & getP4() const { return *(this->_p4); }
		inline const unsigned int index() const { return this->_index; }
		inline const int charge() const { return this->_q; }
		inline const std::string leptonname() const { return this->_name; }
		inline const LeptonTypes leptontype() const { return this->_leptontype; }

		//! Setters
		void setP4(const TLorentzVector & p4);
		inline void setcharge(const int & q) { this->_q = q; }
		void setleptontype(const LeptonTypes & leptontype);
		//! Modify the 4-Momentum scale
		void setScale(const double & scale);

	private:
		// Data members
		TLorentzVector * _p4;
		//! Index related with the tree being extracted
		unsigned int _index;
		int _q;
		LeptonTypes _leptontype;
		std::string _name;
	
	ClassDef(LeptonRel,0);
};

#endif
