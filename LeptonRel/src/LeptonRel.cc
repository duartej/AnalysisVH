#include "LeptonRel.h"

LeptonRel::LeptonRel(const unsigned int & index) 
	: _p4(0), _index(index), _q(-999), _leptontype(UNDEFINED),_name("")
{
}

LeptonRel::LeptonRel(const TLorentzVector & p, const unsigned int & index) 
	: _p4(new TLorentzVector(p)), _index(index), _q(-999), _leptontype(UNDEFINED),_name("")
{
}

LeptonRel::LeptonRel(const TLorentzVector & p, const unsigned int & index, const int & charge, const LeptonTypes & leptontype)
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

LeptonRel::LeptonRel(const LeptonRel & lp)
{
	if( lp._p4 != 0 )
	{
		this->_p4 = new TLorentzVector(*(lp._p4));
	}
	else
	{
		this->_p4 = 0;
	}
	this->_index = lp._index;
	this->_q = lp._q;
	this->_leptontype = lp._leptontype;
	this->_name = lp._name;
}

LeptonRel::~LeptonRel()
{
	if( this->_p4 != 0 )
	{
		delete this->_p4;
		this->_p4 = 0;
	}
}

LeptonRel & LeptonRel::operator=(const LeptonRel & l1)
{
	if( this == &l1 )
	{
		return *this;
	}

	if( this->_p4 != 0 )
	{
		delete this->_p4;
		this->_p4 = 0;
	}
	
	this->_p4 = new TLorentzVector(*(l1._p4));
	this->_index = l1._index;
	this->_q = l1._q;
	this->_leptontype = l1._leptontype;
	this->_name = l1._name;

	return *this;
}


bool LeptonRel::operator ==(const LeptonRel & l1) const
{
	// Just the index and the leptontype is enough to check if we 
	// have the same object
	if( (this->_index == l1.index()) &&  (this->_leptontype == l1.leptontype()) )
	{
		return true;
	}
	
	return false;
}

void LeptonRel::setScale(const double & scale)
{
	if( this->_p4 != 0)
	{
		this->_p4->SetPxPyPzE(scale*_p4->Px(),scale*_p4->Py(),scale*_p4->Pz(),scale*_p4->E());
	}
}

void LeptonRel::setP4(const TLorentzVector & p4)
{
	if( this->_p4 != 0 )
	{
		delete this->_p4;
		this->_p4 = 0;
	}

	this->_p4 = new TLorentzVector(p4);
}

void LeptonRel::setleptontype(const LeptonTypes & leptontype)
{
	this->_leptontype = leptontype;
	
	if( this->_leptontype == MUON )
	{
		_name = "Muon";
	}
	else if( this->_leptontype == ELECTRON )
	{
		_name = "Elec";
	}
}

