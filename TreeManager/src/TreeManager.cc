
#include "TreeManager.h"

#include "TLeaf.h"
#include "TBranchElement.h"
#include "TBranch.h"

#include <iostream>
#include <stdlib.h>
#include <vector>

void TreeManager::GetEntry(const int & entry)
{
	if( fChain != 0)
	{
		fChain->GetTree()->GetEntry(entry);
	}
}

/*template class<type>
type TreeManager::GetEntry(const char * branchname, const int & index)*/
double TreeManager::Get(const char * branchname, const int & index) const
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
		std::vector<float> * thevector = static_cast<std::vector<float>* >(theobj);

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
		return fChain->GetLeaf(branchname)->GetValue(0);
	}
}
