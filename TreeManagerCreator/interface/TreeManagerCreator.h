// -*- C++ -*-
//
// Package:    TreeManagerCreator
// Class:      TreeManagerCreator
// 
/**\class  interface/TreeManagerCreator.h src/TreeManagerCreator.cc

 Description: 

 Implementation: 

*/
//
// Original Author: Jordi Duarte Campderros  
//         Created:  Sun Oct  30 14:20:31 CET 2011
// 
// jordi.duarte.campderros@cern.ch
//
#ifndef TREEMANAGERCREATOR_h
#define TREEMANAGERCREATOR_h 1

#include<string>
#include<map>
#include<vector>

#include "TreeTypes.h"


class TFile;
class TTree;
class TDirectory;
class TLeaf;

class TreeManagerCreator 
{
	public:
		TreeManagerCreator(const char * filename);
		~TreeManagerCreator();
		
		int MakeSimpleSelector(const char * path = ".");
		inline std::string getSelectorFilename() { return _classnamestr+".h"; }
		inline treeTypes getTreeType() { return _treeType; }

	private:
		void FindTree(TDirectory * indir);
		const char * getImplementationOfGetter(const std::string & decl);

		TFile * _file;
		TTree * _tree;

		std::string _classnamestr;
		const char * _classname;
		treeTypes  _treeType;

		// name without _ --> leaf name
		std::map<std::string,std::string> _leafmap;

};


#endif
