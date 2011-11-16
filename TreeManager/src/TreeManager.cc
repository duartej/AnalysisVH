
#include "TreeManager.h"

#include "TLeaf.h"
#include "TBranchElement.h"
#include "TBranch.h"

#include <iostream>
#include <stdlib.h>
#include <vector>


void TreeManager::Init(TTree * tree)
{
	if(tree == 0)
	{
		return;
	}

	fChain = tree;
}

void TreeManager::GetEntry(const int & entry)
{
	if( fChain != 0)
	{
		fChain->GetTree()->GetEntry(entry);
	}
}


template <class T>
T TreeManager::Get(const char * branchname, const int & index) const
{
	// Checking for existence
	if( fChain->GetBranch(branchname) == 0 )
	{
		std::cerr << "TreeManager::Get: ERROR The branchname '"
			<< branchname << "' it does not exists in the file." 
			<< " Check for mispelling and launch again" << std::endl;
		exit(-1);
	}
	// Assumming we deal just with Leafs plains or (STL) vectors 
	if( fChain->GetBranch(branchname)->IsA() == TBranchElement::Class() )
	{
		TBranchElement * branch = (TBranchElement*)(fChain->GetBranch(branchname));
		char * address = branch->GetObject();
		void * theobj  = address;
		std::vector<T> * thevector = static_cast<std::vector<T>* >(theobj);

		if( thevector == 0 )
		{
			std::cerr << "TreeManager::Get: ERROR Unexpected Error!" 
				<< " The '" << branchname << "' does not contain its"
				<< " address object or the object is not initialized" 
				<< std::endl;
			exit(-1);
		}
		if( thevector->size() < (unsigned int)index )
		{
			std::cerr << "TreeManager::Get: ERROR The index '"
				<< index << "' is greater than the size of the vector:" 
				<< thevector->size() << "' in the branch '" 
				<< branchname << "'" << std::endl;
			exit(-1);
		}

		return thevector->at(index);
	}
	else
	{
		return (T)(fChain->GetLeaf(branchname)->GetValue(0));
	}
}

// Declaration of the used types 
template int TreeManager::Get<int>(const char*,const int &) const;
template float TreeManager::Get<float>(const char*,const int &) const;
template bool TreeManager::Get<bool>(const char*,const int &) const;

// Extract the size of the vector
template <class T>
unsigned int TreeManager::GetSize(const char * branchname) const
{
	// Checking for existence
	if( fChain->GetBranch(branchname) == 0 )
	{
		std::cerr << "TreeManager::GetSize: ERROR The branchname '"
			<< branchname << "' it does not exists in the file." 
			<< " Check for mispelling and launch again" << std::endl;
		exit(-1);
	}
	// Assumming we deal just with Leafs plains or (STL) vectors 
	if( fChain->GetBranch(branchname)->IsA() == TBranchElement::Class() )
	{
		TBranchElement * branch = (TBranchElement*)(fChain->GetBranch(branchname));
		char * address = branch->GetObject();
		void * theobj  = address;
		std::vector<T> * thevector = static_cast<std::vector<T>* >(theobj);

		if( thevector == 0 )
		{
			std::cerr << "TreeManager::Get: ERROR Unexpected Error!" 
				<< " The '" << branchname << "' does not contain its"
				<< " address object or the object is not initialized" 
				<< std::endl;
			exit(-1);
		}
		return thevector->size();
	}
	else
	{
		return 0;
	}
}
// Declaration of the used types 
template unsigned int TreeManager::GetSize<int>(const char*) const;
template unsigned int TreeManager::GetSize<float>(const char*) const;
template unsigned int TreeManager::GetSize<bool>(const char*) const;
