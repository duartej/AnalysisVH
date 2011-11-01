#ifndef CUTLEVELS_H
#define CUTLEVELS_H 1

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
  _iMuPTPattern,        //  8 Ask for Pt(H->WW->mumu)> 20,10 Pt(W->mu) > 20
  _iHasExactly3Leptons, //  9 Exactly 3 leptons
  _iOppositeCharge,     // 10 At least a pair of opposite charge muons
  _iJetVeto,            // 11 Reject events with a jet
  _iDeltaR,             // 12 Check on DeltaR between closest muons
  _iZMuMuInvMass,       // 13 A pair of mu with inv mass compatible with Z
  _iMET,                // 14 Missing ET cut
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
                        "MuPtPattern(20,20,10)",
                        "Exactly3Leptons",
                        "OppositeCharge",
                        "JetVeto",
                        "DeltaR",
                        "ZMuMuInvMass",
                        "MET",
                        "NCuts"};
#endif
