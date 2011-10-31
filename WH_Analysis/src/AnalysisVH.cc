#include<sstream>
#include<stdlib.h>
#include<cstring>
#include<iostream>

#include "AnalysisVH.h"
#include "InputParameters.h"

#include "TTree.h"

// Prepare analysis Constructor
AnalysisVH::AnalysisVH(TreeManager * data, InputParameters * ip, TTree * tree ) :
	CMSAnalysisSelector(tree)//,
//	fPUWeight(0),
//	fMuonSelector(0),
//	fHistos(0),	
//	fIsWH(false)
{

	// The Inputparameters have to be initialized before, just to complete it
	// introducing the set of datasets
	// Extract filenames: datafilenames_[index]
	std::string filenames( "datafilenames_" );
	// -- Checking is there
	if( ip == 0 )
	{
		std::cerr << "AnalysisVH::AnalysisVH ERROR: The 'InputParameters' argument "
			<< "cannot be passed as NULL pointer, initialize first!! Exiting... " << std::endl;
		exit(-1);
	}

	if( ip->TheNamedString( filenames+"0" ) == 0 )
	{
		std::cerr << "AnalysisVH::AnalysisVH ERROR: The 'InputParameters' argument "
			<< "must contain a 'datafilenames_0' value!! Exiting..." << std::endl;
		exit(-1);
	}
	std::stringstream istr;
	int id = 0;
	istr << id;
	const char * filename  = 0;
	while( (filename = ip->TheNamedString(filenames+istr.str())) )
	{
		_datafiles.push_back( filename );
		id++;
		istr << id;
	}
	fInputParameters = ip;

	// Including to the list of fInputs the InputParameter
	TList * inputlist = new TList;
	inputlist->Add(fInputParameters);
	this->SetInputList(inputlist);

	
	/*
	for(unsigned int i = 0; i < _datafiles.size(); ++i)
	{
		std::cout << _datafiles[i] << std::endl;
	}*/
}


void AnalysisVH::Initialise()
{
}

void AnalysisVH::InitialiseParameters()
{
}

void AnalysisVH::InsideLoop()
{
}

void AnalysisVH::Summary()
{
}

AnalysisVH::~AnalysisVH()
{
	if( _data != 0)
	{
		delete _data;
	}
	if( fInputParameters != 0)
	{
		delete fInputParameters;
	}
	if( fInput != 0 )
	{
		delete fInput;
	}
}
