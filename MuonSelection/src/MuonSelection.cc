///////////////////////////////////////////////////////////////////////
//
//    FILE: MuonSelection.h
//   CLASS: MuonSelection
// AUTHORS: I. Gonzalez Caballero
//    DATE: 09/05/2011
//
///////////////////////////////////////////////////////////////////////

#include "MuonSelection.h"

//ROOT includes 
#include <TLorentzVector.h>

using namespace std;

//------------------------------------------------------------------------------
// Constructor and destructor
//------------------------------------------------------------------------------
MuonSelection::MuonSelection(TreeManager * selector,
			     bool fillhistos ):
  fSelector(selector),
  fIEvent(-1),
  fIGoodVertex(0),
  //   - Pt, eta and DeltaPT/PT of muons
  fCutMinMuPt(10),
  fCutMaxMuEta(2.4),
  //   - Pt resolution general cut
  fCutMaxDeltaPtMuOverPtMu(0.1),
  //   - Muon isolation in the different regions
  fCutMaxPTIsolationR1(0.13),
  fCutMaxPTIsolationR2(0.09),
  fCutMaxPTIsolationR3(0.06),
  fCutMaxPTIsolationR4(0.05),
  //   - IP and DeltaZ of track associated with muon w.r.t PV
  fUseBiasedIP(true), //Only for Latino Trees
  fCutMaxMuIP2DInTrackR1(0.02), //PT >= 20
  fCutMaxMuIP2DInTrackR2(0.01), //PT <  20
  fCutMaxDeltaZMu(0.1),
  //   - Isolation: (PTtraks + ETcalo)/PTmuon
  fCutMaxIsoMu(0.15),
  //   - Chi2 of global track associated to muon
  fCutMaxNormChi2GTrk(10),
  //   - N hits of pixel, inner and SAdefines los muones aislados de la siguiente forma: track associated to muon
  fCutMinNValidHitsInTrk(10),
  fCutMinNValidHitsSATrk(0),
  fCutMinNValidPixelHitsInTrk(0),
  //   - Number of track matches to the muon chamber
  fCutMinNumOfMatches(1) //, FIXME: PROV
  //   - True if histograms should be filled
  // FIXME: PROVfFillHistos(fillhistos) 
{
  // Initialize containers
  fSelectedBasicMuons = new vector<unsigned int>;
  fSelectedPVMuons = new vector<unsigned int>;
  fSelectedIsoMuons = new vector<unsigned int>;
  fSelectedIsoGoodMuons = new vector<unsigned int>;

  /* FIXME: PROV
  // Initialize histograms
  if (fFillHistos) {
    //   - Pt muons
    fHMuonSelectionPT = fSelector->CreateH1F("fHMuonSelectionPT","P_{T}^{#mu}", 100, 0, 100);
    //   - Eta Muons
    fHMuonSelectionEta = fSelector->CreateH1F("fHMuonSelectionEta","#eta^{#mu}", 100, -5, 5);
    //   - Pt resolution
    fHMuonSelectionDeltaPTOverPT = fSelector->CreateH1F("fHMuonSelectionDeltaPTOverPT","#frac{#Delta P_{T}^{#mu}}{P_{T}^{#mu}}", 100, 0, 1);
  }*/
  
}

MuonSelection::~MuonSelection(){
  delete fSelectedBasicMuons;
  delete fSelectedPVMuons;
  delete fSelectedIsoMuons;
  delete fSelectedIsoGoodMuons;
}





//------------------------------------------------------------------------------
// Get Methods
//------------------------------------------------------------------------------
std::vector<unsigned int>* MuonSelection::GetBasicMuons() {
  if (InNewEvent())
    SelectMuons();
  return fSelectedBasicMuons;
}

std::vector<unsigned int>* MuonSelection::GetMuonsCloseToPV() {
  if (InNewEvent())
    SelectMuons();
  return fSelectedPVMuons;
}
std::vector<unsigned int>* MuonSelection::GetIsolatedMuons() {
  if (InNewEvent())
    SelectMuons();
  return fSelectedIsoMuons;
}
std::vector<unsigned int>* MuonSelection::GetIsolatedGoodMuons() {
  if (InNewEvent())
    SelectMuons();
  return fSelectedIsoGoodMuons;
}




//------------------------------------------------------------------------------
// Other helper methonds
//------------------------------------------------------------------------------
//
//---------------------------------------------
// Is Good Muon:
// - Returns true if the muon with index iMuon is considered a good muon
//---------------------------------------------
bool MuonSelection::IsGoodMuon(unsigned int iMuon) const {

  Bool_t pass = false;
  double ptResolution = fSelector->GetMuondeltaPt()->at(iMuon)/fSelector->GetMuonPt()->at(iMuon);
    
  //Fill Histograms
  /*FIXME: PROV 
    if (fFillHistos) {
    fHMuonSelectionDeltaPTOverPT->Fill(ptResolution);
  }*/

  //Lepton ID
  if( 
     ( 
      ( fSelector->IsGlobalMuon()->at(iMuon) == true && 
	fSelector->GetMuonNValidHitsSATrk()->at(iMuon) > fCutMinNValidHitsSATrk &&
	fSelector->GetMuonNormChi2GTrk()->at(iMuon) < fCutMaxNormChi2GTrk && 
	fSelector->GetMuonNumOfMatches()->at(iMuon) > fCutMinNumOfMatches 
      ) ||
      ( fSelector->IsAllTrackerMuons()->at(iMuon) && 
	fSelector->IsTMLastStationTight()->at(iMuon) 
      ) 
     ) && 
     fSelector->GetMuonNValidPixelHitsInTrk()->at(iMuon) > fCutMinNValidPixelHitsInTrk && 
//#ifdef MINITREES
     fSelector->GetMuonNValidHitsInTrk()->at(iMuon) > fCutMinNValidHitsInTrk &&
//#endif
//#ifdef LATINOTREES
//     fSelector->GetMuonInnerTrackFound()->at(iMuon) > fCutMinNValidHitsInTrk &&           
//#endif
     fabs(ptResolution) < fCutMaxDeltaPtMuOverPtMu  
    )
    pass = true;

  //pass &= (fSelector->GetMuonIsAllTrackerMuons()->at(iMuon));
  //pass &= (fSelector->GetMuonIsGMPTMuons()      ->at(iMuon));

  //pass &= (fSelector->GetMuonNormChi2GTrk()       ->at(iMuon) < fCutMaxNormChi2GTrk);
  //pass &= (fSelector->GetMuonNValidHitsInTrk()     ->at(iMuon) > fCutMinNValidHitsInTrk);
  //pass &= (fSelector->GetMuonNValidHitsSATrk()     ->at(iMuon) > fCutMinNValidHitsSATrk);
  //pass &= (fSelector->GetMuonNValidPixelHitsInTrk()->at(iMuon) > fCutMinNValidPixelHitsInTrk);
  //pass &= (fSelector->GetMuonNumOfMatches()        ->at(iMuon) > fCutMinNumOfMatches);
  //pass &= (fSelector->GetMuondeltaPt()->at(iMuon)/fSelector->GetMuonPt()->at(iMuon)) < 0.1;

  return pass;
}





//------------------------------------------------------------------------------
// Dump
//------------------------------------------------------------------------------

void MuonSelection::Dump(ostream& os) const {
  os << "===============================" << endl;
  os << "|| Muon Selection parameters ||" << endl;
  os << "===============================" << endl;
  //   - Pt, eta and DeltaPT/PT of muons
  os << "+ CutMinMuPt................ = " << fCutMinMuPt << "\t[Min PT of muons]" << endl;
  os << "+ CutMaxMuEta............... = " << fCutMaxMuEta << "\t[Max eta of muons]" << endl;
  os << "+ CutMaxDeltaPtMuOverPtMu... = " << fCutMaxDeltaPtMuOverPtMu << "\t[Max DeltaPT/PT of muons]" << endl;
  //   - IP and DeltaZ of track associated with muon w.r.t PV
  os << "+ CutMaxMuIP2DInTrackR1..... = " << fCutMaxMuIP2DInTrackR1 << "\t[Max. IP of track associated with muon w.r.t PV in R1]" << endl;
  os << "+ CutMaxMuIP2DInTrackR2..... = " << fCutMaxMuIP2DInTrackR2 << "\t[Max. IP of track associated with muon w.r.t PV in R2]" << endl;
  os << "+ CutMaxDeltaZMu............ = " << fCutMaxDeltaZMu << "\t[Max DeltaZ of track associated with muon w.r.t PV]" << endl;
  //   - Isolation: (PTtraks + ETcalo)/PTmuon
  os << "+ CutMaxIsoMu............... = " << fCutMaxIsoMu << "\t[Max. Isolation: PTtraks + ETcalo/PTmuon]" << endl;
  //   - Chi2 of global track associated to muon
  os << "+ CutMaxNormChi2GTrk........ = " << fCutMaxNormChi2GTrk << "\t[Max. Chi2 of global track associated to muon]" << endl;
  //   - N hits of pixel, inner and SA track associated to muon
  os << "+ CutMinNValidHitsInTrk..... = " << fCutMinNValidHitsInTrk << "\t[Min number of valid hits in inner track associated to muon]" << endl;
  os << "+ CutMinNValidHitsSATrk..... = " << fCutMinNValidHitsSATrk << "\t[Min number of valid hits in SA track associated to muon]" << endl;
  os << "+ CutMinNValidPixelHitsInTrk = " << fCutMinNValidPixelHitsInTrk << "\t[Min number of valid pixel hits of track associated to muon]" << endl;
  //   - Number of track matches to the muon chamber
  os << "+ CutMinNumOfMatches........ = " << fCutMinNumOfMatches << "\t[Min Number of track matches to the muon chamber]" << endl;
  os << "================================" << endl;

}







//------------------------------------------------------------------------------
// Selection methods
//------------------------------------------------------------------------------
//
//---------------------------------------------
// Perform the whole selection sequence
//---------------------------------------------
void MuonSelection::SelectMuons() {
  if (!InNewEvent())
    return;

  SelectBasicMuons();
  SelectMuonsCloseToPV(fIGoodVertex);
  SelectIsolatedMuons(fSelectedPVMuons);
  SelectIsolatedGoodMuons(fSelectedIsoMuons);

  fIEvent = fSelector->GetEventEventNumber();
}



//---------------------------------------------
// Select muons
// - Returns a vector with the index of the muons passing our cuts
// - Depends on fCutMinMuPt, fCutMaxMuEta and fCutMaxDeltaPtMuOverPtMu
//---------------------------------------------
unsigned int MuonSelection::SelectBasicMuons() {

  // Empty the selected muons vector
  fSelectedBasicMuons->clear();

  // Loop over muons
  for (unsigned int i=0; i<fSelector->GetMuonPx()->size(); i++) {

    //Build 4 vector for muon
    TLorentzVector Mu(fSelector->GetMuonPx()->at(i), fSelector->GetMuonPy()->at(i), fSelector->GetMuonPz()->at(i), 
		      fSelector->GetMuonEnergy()->at(i));

    //Fill Histograms
    /* FIXME:PROV
    if (fFillHistos) {
      fHMuonSelectionPT->Fill(Mu.Pt());
      fHMuonSelectionEta->Fill(Mu.Eta());
    }*/

    //[Cut in Eta and Pt]
    //-------------------
    if (fabs(Mu.Eta()) >= fCutMaxMuEta) continue;
    if (Mu.Pt()        <= fCutMinMuPt) continue;


    //[If the muon is standalone, and it is neither Tracker nor Global then get rid of it]
    //-------------------
    Bool_t muStaOnly = false;
    
    if( ( fSelector->IsAllStandAloneMuons()->at(i) && !fSelector->IsGlobalMuon()->at(i) )&&  
	( fSelector->IsAllStandAloneMuons()->at(i) && !fSelector->IsAllTrackerMuons()->at(i) ) )
      muStaOnly = true;
    
    if (muStaOnly) continue;


    // If we got here it means the muon is good
    fSelectedBasicMuons->push_back(i);


  }

  return fSelectedBasicMuons->size();
}

//---------------------------------------------
// Select muons close to PV
// - Returns the number of muons close to PV
// - Depends on fCutMaxMuIP2DInTrack and fCutMaxDeltaZMu
//---------------------------------------------
unsigned int MuonSelection::SelectMuonsCloseToPV(unsigned int iGoodVertex) {
  //Empty the vector of indices
  fSelectedPVMuons->clear();

  //Loop over selected muons
  for (unsigned int mu = 0; mu < fSelectedBasicMuons->size(); mu++) {

    unsigned int i = fSelectedBasicMuons->at(mu);

    //Build 4 vector for muon
    double ptMu = TLorentzVector(fSelector->GetMuonPx()->at(i), fSelector->GetMuonPy()->at(i), fSelector->GetMuonPz()->at(i), 
				 fSelector->GetMuonEnergy()->at(i)).Pt();

    //[Require muons to be close to PV]
    //-------------------
//#ifdef MINITREES
    // Next two lines for pure minitrees
    double deltaZMu = fSelector->GetMuonvz()->at(i) - fSelector->GetVertexz()->at(iGoodVertex);
    double IPMu     = fSelector->GetMuonIP2DInTrack()->at(i);
//#endif
//#ifdef LATINOTREES
//    // Next two lines for latinos
//    double deltaZMu = 0;
//    double IPMu = 0;
//    // + Lara
//    if (fUseBiased) {
//      deltaZMu = fSelector->GetMuondzPVBiasedPV()->at(i);
//      IPMu     = fSelector->GetMuonIP2DBiasedPV()->at(i);
//    }
//    // + Jonatan
//    else {
//      deltaZMu = fSelector->GetMuondzPVUnBiasedPV()->at(i);
//      IPMu     = fSelector->GetMuonIP2DUnBiasedPV()->at(i);
//    }
//#endif
    // Apply cut on PV depending on region
    // + R1: PT >= 20
    // + R2: PT <  20
    if (ptMu >= 20 && fabs(IPMu) > fCutMaxMuIP2DInTrackR1) 
      continue;
    else if (ptMu < 20  && fabs(IPMu) > fCutMaxMuIP2DInTrackR2) 
      continue;

    if (fabs(deltaZMu) > fCutMaxDeltaZMu) continue;


    // If we got here it means the muon is good
    fSelectedPVMuons->push_back(i);
  }

  return fSelectedPVMuons->size();
}

//---------------------------------------------
// Select isolated muons
// - Returns the number of selected isolated muons
// - Depends on fCutMaxIsoMu
//---------------------------------------------
unsigned int MuonSelection::SelectIsolatedMuons(vector<unsigned int>* muonsInd) {
  //Empty the vector of indices
  fSelectedIsoMuons->clear();

  //Loop over selected muons
  for (unsigned int mu = 0; mu < muonsInd->size(); mu++) {

    unsigned int i = muonsInd->at(mu);

    //Build 4 vector for muon
    TLorentzVector Mu(fSelector->GetMuonPx()->at(i), fSelector->GetMuonPy()->at(i), fSelector->GetMuonPz()->at(i), 
		      fSelector->GetMuonEnergy()->at(i));

    //[Require muons to be isolated]
    //-------------------
    //The eta/pt plane is divided in 4 regions and the cut on isolation is different in each region
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
//#ifdef MINITREES
    double isolation = (fSelector->GetMuonSumIsoTrack()->at(i) + fSelector->GetMuonSumIsoCalo()->at(i)) / Mu.Pt();
//#endif
//#ifdef LATINOTREES
//    double isolation =(fSelector->T_Muon_muSmurfPF->at(i) )/ Mu.Pt();
//#endif

    Bool_t isolatedMuon = 
      ( (fabs(Mu.Eta()) < 1.479  && Mu.Pt() > 20  && isolation < fCutMaxPTIsolationR1) || //R1: Low Eta  / High PT
	(fabs(Mu.Eta()) >= 1.479 && Mu.Pt() > 20  && isolation < fCutMaxPTIsolationR2) || //R2: High Eta / High PT
	(fabs(Mu.Eta()) < 1.479  && Mu.Pt() <= 20 && isolation < fCutMaxPTIsolationR3) || //R3: Low Eta  / Low PT
	(fabs(Mu.Eta()) >= 1.479 && Mu.Pt() <= 20 && isolation < fCutMaxPTIsolationR4)    //R4: High Eta / Low PT
	); 
    
    if ( !isolatedMuon ) continue;


    // If we got here it means the muon is good
    fSelectedIsoMuons->push_back(i);
  }

  return fSelectedIsoMuons->size();
}
//---------------------------------------------
// Select isolated good muons
// - Returns the number of selected isolated good muons
// - No dependencies: FIX CUTS!!!
//---------------------------------------------
unsigned int MuonSelection::SelectIsolatedGoodMuons(vector<unsigned int>* muonsInd) {
  //Empty the vector of indices
  fSelectedIsoGoodMuons->clear();

  //Loop over selected muons
  for (unsigned int mu = 0; mu < muonsInd->size(); mu++) {

    unsigned int i = muonsInd->at(mu);

    if (!IsGoodMuon(i)) continue;


    // If we got here it means the muon is good
    fSelectedIsoGoodMuons->push_back(i);
  }

  return fSelectedIsoGoodMuons->size();
}
