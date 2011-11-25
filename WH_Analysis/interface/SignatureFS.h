// -*- C++ -*-
//
// Package:    WH_Analysis
// Class:      SignatureFS
// 
/**\class 

 Description: Final states (by the moment 3 leptons)

 Implementation: Just statics functions to extract the info
*/
//
// Original Author: Jordi Duarte Campderros  
//         Created:  Sun Oct  30 18:45:11 CET 2011
// 
// jordi.duarte.campderros@cern.ch
//
//
#ifndef SIGNATURETYPES_H
#define SIGNATURETYPES_H 

class SignatureFS
{
	public:
		//! 3 leptons final state
		// CODE: Nelectrons*1000+Nmuons*100+Ntaus*10
		enum EFS
		{
			_iFSeee, // = 3000,   // 3 electrons
			_iFSmmm, // = 300,    // 3 muons
			_iFSttt, // = 30,     // 3 taus
			_iFSeem, // = 2100,   // 2 electrons 1 muon
			_iFSeet, // = 2010,   // 2 electrons 1 tau
			_iFSmme, // = 1200,   // 1 electron  2 muons
			_iFSmmt, // = 210,    // 2 muons     1 tau
			_iFStte, // = 1020,   // 1 electron  2 taus
			_iFSttm, // = 120,    // 1 muon      2 taus
			_iFSemt, // = 1110,   // 1 electron  1 muon 1 tau
			//_iFSlll,              // 3 leptons (whatever they are)
			_iFSunknown, // = 0,  // Something went wrong
			_iFStotal // = 11    // Remember to change everytime you add/remove on
		};
		
		//! Array of latex strings describing final states
		static const char * kFinalStates[];
		//! Return the EFS enum final state giving the content of leptons
		static unsigned int GetFSID(const unsigned int & nel, 
				const unsigned int & nmu, const unsigned int & ntau);
		//! Return the EFS enum final state giving a char* describing it
		static unsigned int GetFSID( const char * finalstate );

		SignatureFS() { }
		~SignatureFS() { }
};

#endif


