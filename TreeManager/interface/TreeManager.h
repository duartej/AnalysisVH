// -*- C++ -*-
//
// Package:    TreeManager
// Class:      TreeManager
// 
/**\class  TreeManager

 Description: This class manage the output format of the root files.
              This is an abstract class and the concrete classes
	      will content the data members available on a file, taking
	      care of the branches initialization, etc... The class is used 
	      to encapsulate the different types of Trees (MiniTrees, Latinos
	      and TESCO) taking the responsability just for this class to known how is
	      the content of a ROOT file. Some concrete type of this class is
	      going to be as datamember in the Analysis (client), the methods used 
	      by the client has to be defined here (as pure virtual methods). So,
	      in order to unify all the kind of Trees here has to be defined the
	      methods used to extract info from the file, and in the concrete class
	      the concrete implementation depending of the Tree kind.

 Implementation: Abstract class used by the Analysis client. Just the initialization
              of the tree and the definition of the methods which has to be implemented
	      in the concrete classes
*/
//
// Original Author: Jordi Duarte Campderros  
//         Created:  Sun Oct  30 12:20:31 CET 2011
// 
// jordi.duarte.campderros@cern.ch
//
//
#ifndef TREEMANAGER_H
#define TREEMANAGER_H

#include "TROOT.h"
#include "TChain.h"

class TreeManager
{
	public:
		TreeManager() : fChain(0) { }
		virtual ~TreeManager() { }; // WHY virtual??!!!

		void Init(TTree * tree);  // = 0;
		void GetEntry( const int & entry );
		//! Get the TChain global entry given a local (tree) entry
		inline const long GetGlobalEntryNumber(const long & entry) const { return fChain->GetChainEntryNumber(entry); }
		
		//! Extract the value for a given branchname at a given index
		template <class T>
		T Get(const char * branchname,const int & index = 0) const;
	
		//! Extract the number of elements contained in branchname
		template <class T>
		unsigned int GetSize(const char * branchname) const;

	private:
		TTree * fChain;

	
	ClassDef(TreeManager,0);
};


#endif
