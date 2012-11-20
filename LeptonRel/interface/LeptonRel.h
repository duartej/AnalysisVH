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

class LeptonRel
{
	public:
		// Default constructor
		LeptonRel(): _p4(0), _index(-1), _q(-999),_leptontype(-999),_name("") { ; }
		//! Constructors
		LeptonRel(const unsigned int & index);
		LeptonRel(const TLorentzVector & p, const unsigned int & index);
		LeptonRel(const TLorentzVector & p, const unsigned int & index, const int & charge, const int & leptontype);
		virtual ~LeptonRel();

		bool operator ==(const LeptonRel & l1) const;
		
		//! Accessors
		inline const TLorentzVector getP4() const { return *(this->_p4); }
		inline const unsigned int index() const { return this->_index; }
		inline const int charge() const { return this->_q; }
		inline const std::string leptonname() const { return this->_name; }
		inline const int leptontype() const { return this->_leptontype; }

		//! Setters
		void setP4(const TLorentzVector & p4);
		inline void setcharge(const int & q) { this->_q = q; }
		void setleptontype(const int & leptontype);

	private:
		TLorentzVector * _p4;
		//! Index related with the tree being extracted
		unsigned int _index;
		int _q;
		int _leptontype;
		std::string _name;
	
	ClassDef(LeptonRel,0);
};

#endif
