
#include "CutLevels.h"

// Initializing the static for WH
TString WHCuts::kCutNames[] = 
{ 
	"AllEvents",
	"IsWH",
	"HLT",
	"Has2Leptons",
	"Has2PVLeptons",
	"Has2IsoLeptons",
	"Has2IsoGoodLeptons",
	"Exactly3Leptons",
	"OppositeCharge",
	//"JetVeto",
	"DeltaR",
	"ZVeto",
	"WCandidate",
	"MET",
	"NCuts"
};

// Initializing the static for WZ
TString WZCuts::kCutNames[] = 
{ 
	"AllEvents",
	"IsWH",
	"HLT",
	"Has2Leptons",
	"Has2PVLeptons",
	"Has2IsoLeptons",
	"Has2IsoGoodLeptons",
	"Exactly3Leptons",
	"OppositeCharge",
	"HasZCandidate",
	"HasZOverlapping",
	"HasWCandidate",
	//"JetVeto",
	"MET",
	"NCuts"
};
