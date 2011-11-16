#include "SignatureFS.h"
#include <stdlib.h>
#include <iostream>

const char * SignatureFS::kFinalStates[SignatureFS::_iFStotal] = 
{
	"e e e",
	"#mu #mu #mu",
	"#tau #tau #tau",
	"e e #mu",
	"e e #tau",
	"#mu #mu e",
	"#mu #mu #tau",
	"#tau #tau e",
	"#tau #tau #mu",
	"e #mu #tau",
	"Unknown"
};

unsigned int SignatureFS::GetFSID(const char * fs)
{
	unsigned int finalstate;

	if( strcmp(fs,"eee") == 0 )
	{
		finalstate = (int)_iFSeee;
	}
	else if( strcmp(fs,"mmm") == 0 )
	{
		finalstate = (int)_iFSmmm;
	}
	else
	{
		std::cerr << "SignatureFS::SignatureFS ERROR Not Implemented"
			<< " final state: '" << fs << "'" 
			<< std::endl;
		return 999;
	}

	return finalstate;
}

// Get Final state
unsigned int SignatureFS::GetFSID(const unsigned int & nel, 
		const unsigned int & nmu, const unsigned int & ntau)
{
	unsigned int fs = _iFSunknown;
        if (nel == 3)
        {
		fs = _iFSeee;
	}
        else if (nel == 2) 
        {
		if (nmu == 1)
		{
			fs = _iFSeem;
		}
		else if (ntau == 1)
		{
			fs = _iFSeet;
		}
	}
	else if (nel == 1) 
	{
		if (nmu == 2)
		{
			fs = _iFSmme;
		}
		else if(ntau == 2)
		{
			fs = _iFStte;
		}
		else if (nmu == 1 && ntau == 1)
		{
			fs = _iFSemt;
		}
	}
	else if(nel == 0) 
	{
		if (nmu == 3)
		{
			fs = _iFSmmm;
		}
		else if (ntau == 3)
		{
			fs = _iFSttt;
		}
		else if (nmu == 2 && ntau == 1)
		{
			fs = _iFSmmt;
		}
		else if (nmu == 1 && ntau == 2)
		{
			fs = _iFSttm;
		}
	}
        
        return fs;
}
