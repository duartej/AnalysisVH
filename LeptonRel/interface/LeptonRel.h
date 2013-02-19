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
		// loose categories
		enum 
		{
			LOOSE = 100,  // Before check if pass or fail the tight cuts
			TIGHT,
			FAIL,
			_undefined
		};
		//! Constructors
		LeptonRel(): _p4(0), _index(-1), _q(-999),_leptontype(UNDEFINED),_name("") { ; }
		LeptonRel(const unsigned int & index);  // Probably to be deprecated
		LeptonRel(const TLorentzVector & p, const unsigned int & index);
		LeptonRel(const TLorentzVector & p, const unsigned int & index, 
				const int & charge, const LeptonTypes & leptontype, 
				const unsigned int & category);
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
		inline const unsigned int category() const { return this->_category; }

		//! Setters
		void setP4(const TLorentzVector & p4);
		inline void setcharge(const int & q) { this->_q = q; }
		void setleptontype(const LeptonTypes & leptontype);
		inline void setcategory(const unsigned int & category) { this->_category = category; }
		//! Modify the 4-Momentum scale
		void setScale(const double & scale);

	private:
		// Data members
		TLorentzVector * _p4;
		//! Loose category: loose, tight and fail
		unsigned int _category;
		//! Index related with the tree being extracted
		unsigned int _index;
		int _q;
		LeptonTypes _leptontype;
		std::string _name;
	
	ClassDef(LeptonRel,0);
};

#endif
