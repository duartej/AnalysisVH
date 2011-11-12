
#include "TreeManager.h"

#include "TLeaf.h"

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
	return fChain->GetLeaf(branchname)->GetValue(index);
}
