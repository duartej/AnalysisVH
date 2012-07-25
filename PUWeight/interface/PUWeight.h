///////////////////////////////////////////////////////////////////////
//
//    FILE: PUWeight.h  $CVS_TAG: V6_0_0
//   CLASS: PUWeight
// AUTHORS: I. Gonzalez Caballero
//    DATE: 09/03/2011
//
// CONTENT: An utility class to weight the events according to the real
//          data pile up with respect to the generated pileup.
//          For more details see:
// https://twiki.cern.ch/twiki/bin/viewauth/CMS/PileupMCReweightingUtilities
//
// Adapted to be used without PROOF (J. Duarte Campderros)
///////////////////////////////////////////////////////////////////////

#ifndef PUWEIGHT_H
#define PUWEIGHT_H 1


#include "TH1F.h"
#include "TH3F.h"

enum EMCDistribution {
  Spring11,       /*Flat10+Tail*/ 

  Summer11,       /*PU_S4 averaging the number of interactions in each beam
		   * crossing
		   */
  Summer113D,     /*Using Flat10 + tail since this is the "true" input for all
		   * Summer11 samples.
		   */
  Summer11InTime, /*PU_S4 obtained by only looking at the in-time crossing*/
  
  Summer11ITSmear,/*PU_S4 obtained by only looking at the in-time crossing.
		   *This is the "spike+smear" distribution. RECOMENDED!
		   */
  Summer11True,   /*PU_S4 obtained by averaging the number of interactions in
		   * each beam crossing to estimate the true mean.
		   */
  
  Fall11,         /*Fall11*/

  Fall11True,     /*Fall11 True from TTbar 2l2Nu Powheg sample by Matt*/
  
  Summer12        /*Summer12 MC*/
};

class PUWeight {
 public:
  /**************************************
   * Main Constructor: Builds the weights for the pileup from the Data 
   * PU profile for a given luminosity, and the MC PU Profile according
   * to the distribution specified in mcdistr
   * 
   * + luminosity: Determines the data profile to use. Set it to -1 to
   * use the default value
   * 
   * + mcdistr: MC PU Profile with which MC was generated. See the declaration
   * of EMCDistribution for details. It defaults to Spring11
   * 
   * + year: An string identifying the processing. It currently defaults to
   * the only possible value (2011A)
   * 
   */
  PUWeight(float luminosity, 
	   EMCDistribution mcdistr = Spring11,
	   const char* year="2011A");


  /**************************************
   * Destructor
   */
  ~PUWeight() {delete fWeight; delete fMC; delete fData;}


  /**************************************
   * Methods
   */

  // Tells if this object has loaded correctly the histograms
  bool IsValid() const { return fWeight;}


  // Returns the weight for a given PU value
  float GetWeight(unsigned int pu) const {
    return (fWeight? fWeight->GetBinContent(pu+1):0);
  }

  // Returns the weights for a given PU value using 3D algorithm
  float GetWeight3D(unsigned int puminus, 
		    unsigned int pu0, 
		    unsigned int puplus) {
    if (!fWeight3D)
      CalculateWeight3D();
    return fWeight3D->GetBinContent(puminus+1, pu0+1, puplus+1);
  }

  // Returns the MC only weight for a given PU value
  float GetPUMC(unsigned int pu) const {
    return (fMC? fMC->GetBinContent(pu+1):0);
  }
  // Returns the Data only weight for a given PU value
  float GetPUData(unsigned int pu) const {
    return (fData? fData->GetBinContent(pu+1):0);
  }

  // Get the histogram with the weights
  TH1F* GetWeightsHisto() const {return (TH1F*)fWeight->Clone();}

  // Get the histogram with the weights in 3D
  TH3F* GetWeightsHisto3D() {
    if (!fWeight3D)
      CalculateWeight3D();
    return (TH3F*)fWeight3D->Clone();
  }

  // Get the histogram with the profile for Data
  TH1F* GetDataHisto() const {return (TH1F*)fData->Clone();}

  // Get the histogram with the profile for MC
  TH1F* GetMCHisto() const {return (TH1F*)fMC->Clone();}

  // Rescale the 3D weight to study systematic errors
  TH3F* RescaleWeights3D(float ScaleFactor = 1.0) {return CalculateWeight3D(ScaleFactor);}

 protected:
  // Build the PU ideal profile for MC
  TH1F* IdealMCHistogram(EMCDistribution);
  // Helper class filling 

  // Load the PU profile for MC
  TH1F* LoadMCHistogram(const char* mcfolder, const char* mcproccess);
  // Load the PU profile for Data
  TH1F* LoadDataHistogram(float luminosity, const char* year="2011A");

  // Divide the Data profile by the MC profile
  TH1F* CalculateWeight();

  // Build 3D histogram with weights
  TH3F* CalculateWeight3D(float ScaleFactor = 1.0);

 protected:
  TH1F* fData;   //PU profile for data
  TH1F* fMC;     //PU profile for MC
  TH1F* fWeight; //Histogram with the weight content
  TH3F* fWeight3D; //The 3D weights
  bool  fIs3D;     //True if 3D should be used  
};


#endif
