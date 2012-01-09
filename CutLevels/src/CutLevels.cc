
#include "CutLevels.h"

// Initializing the static for WH
TString WHCuts::kCutNames[] = 
{ 
	"AllEvents",
	"IsWH",
	"HLT",
	//"GoodVertex",
	"Has2Leptons",
	"Has2PVLeptons",
	"Has2IsoLeptons",
	"Has2IsoGoodLeptons",
	"MuPtPattern(20,20,10)",
	"Exactly3Leptons",
	"NotSameSign",
	"OppositeCharge",
	//"JetVeto",
	"ZMuMuInvMass",
	"WCandidate",
	"DeltaR",
	"MET",
	"NCuts"
};

// Initializing the static for WZ
TString WZCuts::kCutNames[] = 
{ 
	"AllEvents",
	"IsWH",
	"HLT",
	//"GoodVertex",
	"Has2Leptons",
	"Has2PVLeptons",
	"Has2IsoLeptons",
	"Has2IsoGoodLeptons",
	"AtLeast3Leptons",
	"OppositeCharge",
	"HasZCandidate",
	"HasZOverlapping",
	"HasWCandidate",
	//"JetVeto",
	"MET",
	"NCuts"
};
