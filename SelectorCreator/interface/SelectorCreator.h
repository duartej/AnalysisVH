///////////////////////////////////////////////////////////////////////
//
//    FILE: .h
//   CLASS: SelectorCreator
// AUTHORS: J. Duarte Campderros
//    DATE: October, 2011
//
// CONTENT: Implements the functionality needed to run an Analysis
//          using the PROOF Analysis Framework (PAF). This file can
//          
//
///////////////////////////////////////////////////////////////////////
#ifndef SelectorCreator_h
#define SelectorCreator_h 1

#include<string>
#include<map>
#include<vector>

#include "TString.h"


class TFile;
class TTree;
class TDirectory;
class TLeaf;

class SelectorCreator 
{
	public:
		SelectorCreator(const char * filename);
		~SelectorCreator();
		
		int MakeSimpleSelector(const char * path = ".");
		inline std::string getSelectorFilename() { return _classnamestr+".h"; }
		inline std::string getTreeType() { return _treeType; }

	private:
		void FindTree(TDirectory * indir);
		const char * getImplementationOfGetter(const std::string & decl);

		TFile * _file;
		TTree * _tree;

		std::string _classnamestr;
		const char * _classname;
		const char * _treeType;

		// name without _ --> leaf name
		std::map<std::string,std::string> _leafmap;

};


#endif
