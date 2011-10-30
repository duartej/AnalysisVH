///////////////////////////////////////////////////////////////////////
//
//    FILE: MuonSelection.h
//   CLASS: MuonSelection
// AUTHORS: I. Gonzalez Caballero
//    DATE: 09/05/2011
//
// CONTENT: This class for PAF standarizes the selection of good muons 
//          for analysis
//
// Changelog: Incorporates some getters functions to the 
//            CMSAnalysisSelector class in order to avoid the creation
//            of every MuonSelector class for every diferent kind of
//            tree. Now, the responsable to call the current selector
//            to use is the client, not the MuonSelector class.
//                                       J. Duarte Campderros Oct, 2011
///////////////////////////////////////////////////////////////////////
// -*- C++ -*-
//
// Package:    MuonSelection
// Class:      MuonSelection
// 
/**\class  MuonSelection.h src/MuonSelection.cc

 Description: 

 Implementation: 
*/
//
// Modifications Author: Jordi Duarte Campderros  
//         Created:  Sun Oct  30 12:20:31 CET 2011
// 
// jordi.duarte.campderros@cern.ch
//
//

#ifndef MUONSELECTION_H
#define MUONSELECTION_H 1



#include "TreeManager.h"

// STL includes
#include <vector>
#include <iostream>

// ROOT includes
#include "TH1F.h"

using namespace std;

class MuonSelection {
 public:

  //Constructors and destructor
  //---------------------------
  MuonSelection(TreeManager * treemanager, 
		bool fillhistos = false);
  virtual ~MuonSelection();

  //Set Good Vertex Index
  //---------------------
  void SetGoodVertexIndex(unsigned int igoodvertex) {fIGoodVertex=igoodvertex;}


  //Get Methods
  //-----------
  std::vector<unsigned int>* GetBasicMuons();
  std::vector<unsigned int>* GetMuonsCloseToPV();
  std::vector<unsigned int>* GetIsolatedMuons();
  std::vector<unsigned int>* GetIsolatedGoodMuons();
  unsigned int GetNBasicMuons()       {return GetBasicMuons()->size();}
  unsigned int GetNMuonsCloseToPV()   {return GetMuonsCloseToPV()->size();}
  unsigned int GetNIsolatedMuons()    {return GetIsolatedMuons()->size();}
  unsigned int GetNIsolatedGoodMuons(){return GetIsolatedGoodMuons()->size();}

  unsigned int GetGoodVertexIndex() const {return fIGoodVertex;}
  Int_t GetCurrentEvent() const {return fIEvent;}


  //Variables used for cuts from Input Parameters
  //---------------------------------------------
  //   - Pt, eta and DeltaPT/PT of muons
  void SetMinMuPt(double cutMinMuPt){fCutMinMuPt=cutMinMuPt;}
  void SetMaxMuEta(double cutMaxMuEta){fCutMaxMuEta=cutMaxMuEta;}
  void SetMaxDeltaPtMuOverPtMu(double cutMaxDeltaPtMuOverPtMu){fCutMaxDeltaPtMuOverPtMu=cutMaxDeltaPtMuOverPtMu;}
  //   - Pt isolation by region in the eta/pt plane. There are 4 regions:
  //
  // PT ^
  //   /|\ | 
  //    |  |
  //    |R1|R2
  // 20-+--+---
  //    |R3|R4
  //    +--+---> eta
  //       |
  //      1.479 
  void SetMaxPTIsolationR1(double cutMaxPTIsolationR1) {fCutMaxPTIsolationR1 = cutMaxPTIsolationR1;}
  void SetMaxPTIsolationR2(double cutMaxPTIsolationR2) {fCutMaxPTIsolationR2 = cutMaxPTIsolationR2;}
  void SetMaxPTIsolationR3(double cutMaxPTIsolationR3) {fCutMaxPTIsolationR3 = cutMaxPTIsolationR3;}
  void SetMaxPTIsolationR4(double cutMaxPTIsolationR4) {fCutMaxPTIsolationR4 = cutMaxPTIsolationR4;}
  //   - IP and DeltaZ of track associated with muon w.r.t PV
  void UseBiasedIP() {fUseBiasedIP = true;}    //Only for Latino Trees
  void UseUnBiasedIP() {fUseBiasedIP = false;} //Only for Latino Trees
  void SetMaxMuIP2DInTrackR1(double cutMaxMuIP2DInTrack){fCutMaxMuIP2DInTrackR1=cutMaxMuIP2DInTrack;}
  void SetMaxMuIP2DInTrackR2(double cutMaxMuIP2DInTrack){fCutMaxMuIP2DInTrackR2=cutMaxMuIP2DInTrack;}
  void SetMaxDeltaZMu(double cutMaxDeltaZMu){fCutMaxDeltaZMu=cutMaxDeltaZMu;}
  //   - Isolation: (PTtraks + ETcalo)/PTmuon
  void SetMaxIsoMu(double cutMaxIsoMu){fCutMaxIsoMu=cutMaxIsoMu;}
  //   - Chi2 of global track associated to muon
  void SetMaxNormChi2GTrk(double cutMaxNormChi2GTrk){fCutMaxNormChi2GTrk=cutMaxNormChi2GTrk;}
  //   - N hits of pixel, inner and SA track associated to muon
  void SetMinNValidHitsInTrk(int cutMinNValidHitsInTrk){fCutMinNValidHitsInTrk=cutMinNValidHitsInTrk;}
  void SetMinNValidHitsSATrk(int cutMinNValidHitsSATrk){fCutMinNValidHitsSATrk=cutMinNValidHitsSATrk;}
  void SetMinNValidPixelHitsInTrk(int cutMinNValidPixelHitsInTrk){fCutMinNValidPixelHitsInTrk=cutMinNValidPixelHitsInTrk;}
  //   - Number of track matches to the muon chamber
  void SetMinNumOfMatches(int cutMinNumOfMatches){fCutMinNumOfMatches=cutMinNumOfMatches;}


  //Activates the selection mechanism
  //------------------
  void SelectMuons();

  //Dump cut values
  //---------------
  void Dump(std::ostream& os = std::cout) const;

 protected:
  //Protected selection methods
  //-----------------
  unsigned int SelectBasicMuons();
  unsigned int SelectMuonsCloseToPV(unsigned int iGoodVertex);
  unsigned int SelectIsolatedMuons(vector<unsigned int>* muonsInd);
  unsigned int SelectIsolatedGoodMuons(vector<unsigned int>* muonsInd);

  //Other helper methods
  //--------------------
  bool InNewEvent() const {return (fSelector->GetEventEventNumber()!=fIEvent);}
  bool IsGoodMuon(unsigned int iMuon) const;

 protected:
  // The Tree Manager
  //----------------------------------------
  TreeManager* fSelector;

  // The last event processed
  //-------------------------
  Int_t fIEvent;

  // Index of good vertex
  //-------------------------
  unsigned int fIGoodVertex;

  // Variables used to hold indices to the leptons
  //----------------------------------------------
  std::vector<unsigned int> *fSelectedBasicMuons;   //Muons passing basic cuts
  std::vector<unsigned int> *fSelectedPVMuons;      //Muons close to PV
  std::vector<unsigned int> *fSelectedIsoMuons;     //Isolated muons
  std::vector<unsigned int> *fSelectedIsoGoodMuons; //Isolated good muons


  // Variables used for cuts from Input Parameters
  //----------------------------------------------------------------------------
  //   - Pt, eta and DeltaPT/PT of muons
  double fCutMinMuPt;
  double fCutMaxMuEta;
  //   - Pt resolution general cut
  double fCutMaxDeltaPtMuOverPtMu;
  //   - Muon isolation by region in the eta/pt plane. There are 4 regions:
  //
  // PT ^
  //   /|\ |
  //    |  |
  //    |R1|R2
  // 20-+--+---
  //    |R3|R4
  //    +--+---> eta
  //       |
  //      1.479 
  double fCutMaxPTIsolationR1;
  double fCutMaxPTIsolationR2;
  double fCutMaxPTIsolationR3;
  double fCutMaxPTIsolationR4;
  //   - IP and DeltaZ of track associated with muon w.r.t PV
  bool   fUseBiasedIP; //Only for Latino Trees
  double fCutMaxMuIP2DInTrackR1; //PT >= 20
  double fCutMaxMuIP2DInTrackR2; //PT <  20
  double fCutMaxDeltaZMu;
  //   - Isolation: (PTtraks + ETcalo)/PTmuon
  double fCutMaxIsoMu;
  //   - Chi2 of global track associated to muon
  double fCutMaxNormChi2GTrk;
  //   - N hits of pixel, inner and SA track associated to muon
  int    fCutMinNValidHitsInTrk;
  int    fCutMinNValidHitsSATrk;
  int    fCutMinNValidPixelHitsInTrk;
  //   - Number of track matches to the muon chamber
  int    fCutMinNumOfMatches;


  // FIXME: Como soluciono esto??? PROV
  // Histograms
  //----------------------------------------------------------------------------
  /*//   - True if histograms should be filled
  bool  fFillHistos;
  //   - Pt muons
  TH1F* fHMuonSelectionPT;
  //   - Eta Muons
  TH1F* fHMuonSelectionEta;
  //   - Pt resolution
  TH1F* fHMuonSelectionDeltaPTOverPT;*/


  ClassDef(MuonSelection,0);

};

#endif
