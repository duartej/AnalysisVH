#include "LeptonRel.h"

LeptonRel::LeptonRel(const unsigned int & index) 
	: _p4(0), _index(index), _q(-999), _leptontype(-999),_name("")
{
}

LeptonRel::LeptonRel(const TLorentzVector & p, const unsigned int & index) 
	: _p4(new TLorentzVector(p)), _index(index), _q(-999), _leptontype(-999),_name("")
{
}

LeptonRel::LeptonRel(const TLorentzVector & p, const unsigned int & index, const int & charge, const int & leptontype)
	: _p4(new TLorentzVector(p)), _index(index), _q(charge), _leptontype(leptontype)
{
	if( _leptontype == MUON )
	{
		_name = "Muon";
	}
	else if( _leptontype == ELECTRON )
	{
		_name = "Elec";
	}
}


LeptonRel::~LeptonRel()
{
	if( this->_p4 )
	{
		delete this->_p4;
		this->_p4 = 0;
	}
}

bool LeptonRel::operator ==(const LeptonRel & l1) const
{
	if( (this->_index == l1.index()) && (*(this->_p4) == l1.getP4()) 
			&& ( this->_q == l1.charge()) &&  (this->_leptontype == l1.leptontype()) )
	{
		return true;
	}
	
	return false;
}

LeptonRel::setP4(const TLorentzVector & p4)
{
	if( this->_p4 )
	{
		delete this->_p4;
		this->_p4 = 0;
	}

	this->_p4 = new TLorentzVector(p4);
}

LeptonRel::setleptontype(const int & leptontype)
{
	_leptontype = leptontype;
	
	if( _leptontype == MUON )
	{
		_name = "Muon";
	}
	else if( _leptontype == ELECTRON )
	{
		_name = "Elec";
	}
}

