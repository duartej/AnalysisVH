
#include "TreeManager.h"

void TreeManager::GetEntry(const int & entry)
{
	if( fChain != 0)
	{
		fChain->GetTree()->GetEntry(entry);
	}
}


