#ifndef CUTLEVELS_H
#define CUTLEVELS_H 1

#include "TString.h"


// Class for WH Analysis
struct WHCuts
{
	// Enum holding the index of every cut
	enum {
		_iAllEvents,          //  0
		_iIsWH,               //  1 For WH samples get the number of WH events
		_iHLT,                //  2
		_iHas2Leptons,        //  3
		_iHas2PVLeptons,      //  4
		_iHas2IsoLeptons,     //  5
		_iHas2IsoGoodLeptons, //  6
		_iHasExactly3Leptons, //  8 Exactly 3 leptons (Pt(H->WW->mumu)> 20,10 Pt(W->mu) > 20)
		_iOppositeCharge,     //  9 At least a pair of opposite charge muons
		//_iJetVeto,            // 10 Reject events with a jet
		_iDeltaR,             // 13 Check on DeltaR between closest muons
		_iZVeto,              // 11 A pair of (same flavour) leptons with inv mass compatible with Z
		_iWCandidate,         // 12 A W lepton candidate (just dR bremsstrahlung cut)
		_iMET,                // 14 Missing ET cut
		_iNCuts               //  N
	};

	static TString kCutNames[_iNCuts+1];
};


// Struct for WZ analysis
struct WZCuts
{
	// Enum holding the index of every cut
	enum 
	{
		_iAllEvents,          //  0
		_iIsWZ,               //  1 For WZ samples get the number of WZ events
		_iHLT,                //  2
		//_iGoodVertex,         //  3 : DEPRECATED --> In Latinos tree always true
		_iHas2Leptons,        //  4
		_iHas2PVLeptons,      //  5
		_iHas2IsoLeptons,     //  6
		_iHas2IsoGoodLeptons, //  7
		_iHasExactly3Leptons, //  8 Exactly 3 leptons
		_iOppositeCharge,     //  9 At least a pair of opposite sign
		_iHasZCandidate,      // 10 Has a Z candidate
		_iHasZOverlapping,    // 11 Has another Z candidate (ZZ)
		_iHasWCandidate,      // 11 Has a W candidate
		//_iJetVeto,            // 12 Reject events with a jet: DEPRECATED
		_iMET,                // 15 Missing ET cut FIXME: PROVISIONAL
		_iNCuts               //  N
	};

	static TString kCutNames[_iNCuts+1];
};


#endif
