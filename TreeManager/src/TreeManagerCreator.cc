// -*- C++ -*-
//
// Package:    TreeManagerCreator
// Class:      TreeManagerCreator
// 
//
// Original Author: Jordi Duarte Campderros  
//         Created:  Sun Oct  30 14:20:31 CET 2011
// 
// jordi.duarte.campderros@cern.ch
//
#include "TreeManagerCreator.h"
#include "TreeManager.h"

//ROOT includes
#include "TFile.h"
#include "TTree.h"
#include "TDirectory.h"
#include "TKey.h"
#include "TROOT.h"
#include "TClass.h"
#include "TMath.h"
#include "TString.h"
#include "TLeafObject.h"
#include "TChain.h"
#include "TBranchElement.h"
#include "TBranchObject.h"
#include "TVirtualStreamerInfo.h"
#include "TStreamerElement.h"

//C++ includes
#include<stdlib.h>
#include<iostream>
#include<sstream>
#include<string>



using namespace std;


TreeManagerCreator::TreeManagerCreator(const char * filename):
	_file(0),_tree(0),
	_classnamestr("TreeManager"),
	_classname(0)
{
	// Instanciating the filename 
	_file = new TFile(filename);

	if(_file == 0 || _file->IsZombie())
	{
		std::cerr << "TreeManagerCreator::TreeManagerCreator ERROR: File '"
			<< filename << "' not initialize! Exiting..." 
			<< std::endl;
		exit(-1);
	}
	// Extract tree
	this->FindTree(_file);
	//
	if(_tree == 0)
	{
		std::cerr << "TreeManagerCreator::TreeManagerCreator ERROR: "
			<< "Can't instanciate this class. File structure not "
			<< "implemented [MiniTrees] [TESCO]" 
			<< std::endl;
		exit(-1);
	}
	
	_classname = _classnamestr.c_str();
}

TreeManagerCreator::~TreeManagerCreator()
{
	if(_file != 0)
	{
		_file->Close();
		delete _file;
		_file = 0;
	}
}


// Recursive search for a known structure
void TreeManagerCreator::FindTree(TDirectory *indir)
{
	// Current directory (or file)
	TDirectory * aDir = gDirectory;
	aDir->cd();
	// Loop on all entries of the subdirectory
	TKey * key = 0;
	TIter nextkey(indir->GetListOfKeys());
	while( (key = (TKey*)nextkey()) )
	{
		const char * classname = key->GetClassName();
		TClass * cl = gROOT->GetClass(classname);
		if( !cl )
		{
			continue;
		}
		
		if( cl->InheritsFrom(TDirectory::Class()) )
		{
			// Entering the directory
			indir->cd(key->GetName());
			TDirectory * subdir = gDirectory;
			aDir->cd();
			// Following the search
			FindTree(subdir);
			aDir->cd();
		}
		else if( cl->InheritsFrom(TTree::Class()) )
		{
			// Found a tree, here deciding what type
			// is: MiniTree or TESCO
			TTree * atree = (TTree*)indir->Get(key->GetName());
			if( (strcmp(atree->GetName(), "Tree") == 0)
					|| (strcmp(atree->GetName(), "Analysis") == 0) )
			{
				// Found the right tree, storing to the datamember
				_tree = atree;
				// And the type of tree
				if( strcmp(atree->GetName(), "Tree") == 0 ) 
				{
					_classnamestr += "MiniTrees";
					_treeType = MiniTrees;
				}
				else
				{
					_classnamestr += "TESCO";
					_treeType = TESCO;
				}

				aDir->cd();
				break;
			}
			aDir->cd();
		}
	}
}


// Method to extract what datamember must be used for each getter
// This method has to be modified each time the ....
const char * TreeManagerCreator::getImplementationOfGetter(const std::string & decl)
{
	// Assuming the name after the Get word is describing the datamember
	int index = decl.find("Get");
	std::string datamember;
	if( index > 0 )
	{
		datamember = decl.substr(index+3,datamember.npos);
		const int lastind = datamember.find("(");
		datamember = datamember.substr(0,lastind);
		// Be aware with the event_eventNumber: FIXME: Chapuza
	}
	else
	{
		// Case without Get, just Is
		index = decl.find("Is");
		datamember = decl.substr(index,datamember.npos);
		const int lastind = datamember.find("(");
		// Adding muon
		datamember = std::string("Muon")+datamember.substr(0,lastind);
	}


	std::string realleaf("");
	if( _treeType == MiniTrees || _treeType == Latinos )
	{
		// Extract object type
		std::map<std::string,std::string>::iterator leafname = _leafmap.find(datamember) ;
		if( leafname == _leafmap.end() )
		{
			std::cout << "TreeManagerCreator: WARNING: Not found the correspondent leaf "
				<< "to the method '" << decl << "' " << std::endl;
			std::cout << "Using '" << datamember << "' as key to find the leaf. "
				<< "The method is going to be empty!!\n" << std::endl;
		}
		else
		{
			realleaf = leafname->second;
		}
	}
	else if( _treeType == TESCO )
	{
		std::cout << "TreeManagerCreator:: 'TESCO' trees not implemented yet. "
			<< "Exiting..." << std::endl;
		exit(0);
	}
	else
	{
		std::cout << "TreeManagerCreator:: '" << _treeType << "' trees not implemented yet. "
			<< "Exiting..." << std::endl;
		exit(0);
	}

	// Filling
	std::string impl;
	if( realleaf != "" )
	{
		impl = "return this->"+realleaf+";";
	}
	else
	{
		impl = "return 0; /* Not implemented */ ";
	}

	return impl.c_str();
}


// Simple version of MakeSelector/MakeClass just to deal
// with MiniTrees and TESCO trees
int TreeManagerCreator::MakeSimpleSelector(const char* path)
{
	// Dumping to the output file
	bool ischain = _tree->InheritsFrom("TChain");
	TString treefile = _file->GetName();

	std::string totalname(path);
	totalname += "/";
	totalname += _classname;
	std::string srctotalname(totalname+".cc");
	totalname += ".h";

	FILE *fp = fopen(totalname.c_str(), "w");
	if (!fp) 
	{
		std::cerr << "ERROR: TreeManagerCreator::MakeSimpleSelector. Cannot open output " 
			<< "file '" << totalname << "'" << std::endl;
		return 3;
	}

	fprintf(fp,"//////////////////////////////////////////////////////////\n");
	fprintf(fp,"// This class has been automatically generated\n");
	fprintf(fp,"// by ROOT version %s\n",gROOT->GetVersion());
	fprintf(fp,"// using a modified version of MakeSelector\n");
	if (!ischain) 
	{
		fprintf(fp,"// from TTree %s/%s\n",_tree->GetName(),_tree->GetTitle());
		fprintf(fp,"// found on file: %s\n",treefile.Data());
	} 
	else 
	{
		fprintf(fp,"// from TChain %s/%s\n",_tree->GetName(),_tree->GetTitle());
	}
	fprintf(fp,"//////////////////////////////////////////////////////////\n");
	fprintf(fp,"\n");
	fprintf(fp,"#ifndef %s_h\n",_classname);
	fprintf(fp,"#define %s_h\n",_classname);
	fprintf(fp,"\n");
	fprintf(fp,"#include <TROOT.h>\n");
	fprintf(fp,"#include <TChain.h>\n");
	
	fprintf(fp,"#include \"TreeManager.h\"\n");
	fprintf(fp,"\n");
	fprintf(fp,"using namespace std;");
	fprintf(fp,"\n");
	fprintf(fp,"class %s : public TreeManager \n{\n",_classname);
	//fprintf(fp,"\tpublic :\n");
	fprintf(fp,"\tprivate :\n"); //--> FIXME
	fprintf(fp,"\n\t\t// Declaration of leaf types");

	// Obtaining all the leaves and ordering in a dict
	// { type of leaf: [ (name,branchname), ...], ... }
	std::vector<std::pair<TString,TString> >branchleafpair;
	std::vector<TString> mustInit;
	
	TObjArray * leaves = _tree->GetListOfLeaves();
	TIter next(leaves);
	TLeaf * theleafs;
	//std::queue<TString> lname;
	while( (theleafs=(TLeaf *)next()) )
	{
		// Extract dimension: FIXME Hay que hacerlo para todos
		// los ficheros de la TChain...
		int dim = 0;
		TLeaf * dimleaf = theleafs->GetLeafCounter(dim);

		std::string lnameanddim = theleafs->GetTitle();

		std::string namedim("");
		if(dimleaf != 0)
		{
			dim = dimleaf->GetMaximum();
			if(dim == 0)
			{
				dim =1;
			}
			namedim = std::string("  //[")+dimleaf->GetName()+std::string("]");
		}
		std::string dimension("");
		if( dimleaf != 0 )
		{
			std::ostringstream buffer;
			buffer << dim;
			dimension = std::string("[")+buffer.str()+std::string("]");
			lnameanddim.replace(lnameanddim.find("["),lnameanddim.find("]"),
					dimension);
		}

		// Pointers
		std::string star("");
		if( theleafs->GetBranch()->IsA() == TBranchElement::Class() )
		{
			star = "*";
			mustInit.push_back(theleafs->GetName());
		}

		fprintf(fp,"\n\t\t%-20s%-1s%s; %s",theleafs->GetTypeName(),
				star.c_str(),lnameanddim.c_str(),
				namedim.c_str());
		
		branchleafpair.push_back( std::pair<TString,TString>(
					theleafs->GetBranch()->GetName(),theleafs->GetName()) );
		// To be used when implementing the getters methods
		std::string leafname = theleafs->GetName();
		if( _treeType == MiniTrees )
		{
			// Asumming T_
			leafname.replace(0,1,"");
			unsigned int ind = leafname.find("_");
			unsigned int size= leafname.size();
			while( ind < size )
			{
				leafname.replace(ind,1,"");
				ind = leafname.find("_");
				size = leafname.size();
			}
		}
		_leafmap[leafname] = theleafs->GetName();

	}

	fprintf(fp,"\n\n\t\t // List of branches");
	for(unsigned int i = 0; i < branchleafpair.size(); ++i)
	{
		fprintf(fp,"\n\t\tTBranch             *b_%s;   //!", 
				branchleafpair.at(i).first.Data());
	}
	
	// generate class member functions prototypes
	fprintf(fp,"\n\n");
 	fprintf(fp,"\tpublic :\n");  // ---> FIXME
	fprintf(fp,"\t\t%s(TTree * /* tree */ =0) { }\n",_classname) ;
	fprintf(fp,"\t\tvirtual ~%s() { }\n",_classname);
	fprintf(fp,"\t\tvirtual void    Init(TTree *tree);\n");

	fprintf(fp,"\t\t// Specific getters methods\n");
	std::vector<std::string> methods = TreeManager::getmethods();
	for(unsigned int i = 0; i < methods.size(); ++i)
	{
		const std::string decl = methods.at(i);
		const char * impl = getImplementationOfGetter( decl );
		fprintf(fp, "\t\t%s {%s }\n",decl.c_str(),impl);
	}
	fprintf(fp,"\n\n");

	fprintf(fp,"\tClassDef(%s,0);\n",_classname);
	fprintf(fp,"};\n");
	fprintf(fp,"\n");
	fprintf(fp,"#endif\n");
	fprintf(fp,"\n");

	fclose(fp);  // Closing file and  Freeing memory

	// Source code
	fp = fopen(srctotalname.c_str(), "w");
	fprintf(fp,"#include \"%s\"\n",totalname.c_str());
	fprintf(fp,"\n");
	
	// generate code for class constructor
	// fprintf(fp,"#ifdef %s_cxx\n",_classname); -- Not needed anymor
	
	// generate code for class destructor()
	// generate code for class member function GetEntry()
	// generate code for class member function LoadTree()
	// generate code for class member function Init(), first pass = get branch pointer
	fprintf(fp,"void %s::Init(TTree *tree)\n",_classname);
	fprintf(fp,"{\n");
	fprintf(fp,"\t\t// The Init() function is called when the selector needs to initialize\n"
			"\t\t// a new tree or chain. Typically here the branch addresses and branch\n"
			"\t\t// pointers of the tree will be set.\n"
			"\t\t// It is normally not necessary to make changes to the generated\n"
			"\t\t// code, but the routine can be extended by the user if needed.\n"
			"\t\t// Init() will be called many times when running on PROOF\n"
			"\t\t// (once per file to be processed).\n\n");

	for(unsigned int i = 0; i < mustInit.size(); ++i)
	{
		fprintf(fp,"\t\t%s = 0;\n",mustInit.at(i).Data() );
	}
	fprintf(fp,"\n\t\t// Set branch addresses and branch pointers\n");
	fprintf(fp,"\t\tif (!tree) return;\n");
	fprintf(fp,"\t\tfChain = tree;\n");
	fprintf(fp,"\t\tfChain->SetMakeClass(1);\n");
	fprintf(fp,"\n");
	

	for(std::vector<std::pair<TString,TString> >::iterator it = branchleafpair.begin();
			it != branchleafpair.end(); ++it)
	{
		fprintf(fp,"\t\tfChain->SetBranchAddress(\"%s\", &%s, &b_%s);\n",
					it->first.Data(), it->second.Data(), it->first.Data());
	}
	
	fprintf(fp,"}\n");
	fprintf(fp,"\n");
	
	//fprintf(fp,"#endif // #ifdef %s_cxx\n",_classname); --> Not needed anymore
	
	fclose(fp);


	
	return 0;

}

