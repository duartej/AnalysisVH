#ifndef CUTLEVELSWZ_H
#define CUTLEVELSWZ_H 1

#include "TString.h"

// Enum holding the index of every cut
enum ECutLevel {
  _iAllEvents,          //  0
  _iIsWH,               //  1 For WH samples get the number of WH events
  _iHLT,                //  2
  _iGoodVertex,         //  3
  _iHas2Leptons,        //  4
  _iHas2PVLeptons,      //  5
  _iHas2IsoLeptons,     //  6
  _iHas2IsoGoodLeptons, //  7
  _iHasAtLeast3Leptons, //  8 At least 3 leptons
  _iOppositeCharge,     //  9 At least a pair of opposite sign
  _iHasZCandidate,      // 10 Has a Z candidate
  _iHasWCandidate,      // 11 Has a W candidate
  _iJetVeto,            // 12 Reject events with a jet
  _iMET,                // 15 Missing ET cut
  _iNCuts               //  N
};

const TString kCutNames[] = { "AllEvents",
                        "IsWH",
                        "HLT",
                        "GoodVertex",
                        "Has2Leptons",
                        "Has2PVLeptons",
                        "Has2IsoLeptons",
                        "Has2IsoGoodLeptons",
                        "AtLeast3Leptons",
			"OppositeCharge",
                        "HasZCandidate",
                        "HasWCandidate",
                        "JetVeto",
                        "MET",
                        "NCuts"};
#endif
