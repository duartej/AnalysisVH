#ifndef ANALYSISWZ_H
#define ANALYSISWZ_H 1

// Blabla description

#include<vector>
#include<map>
#include<cstring>

//#include "CMSAnalysisSelector.h"
#include "AnalysisBase.h"
#include "TreeManager.h"
#include "CutLevels.h"
#include "LeptonTypes.h"
#include "SignatureFS.h"

#include "TString.h"
#include "TLorentzVector.h"


class InputParameters;
class PUWeight;
class TTree;
class CutManager;
class TH1D;


class AnalysisWZ : public AnalysisBase
{
	public: 
		//! Constructor
		AnalysisWZ( TreeManager * data, std::map<LeptonTypes,InputParameters*> ipmap, 
				CutManager * selectorcuts, const unsigned int & finalstate ) :
			AnalysisBase(data,ipmap,selectorcuts,finalstate ) { } 
		//! Destructor
		virtual ~AnalysisWZ() { }

	protected:
		virtual void InsideLoop();

	private:
		AnalysisWZ();
		ClassDef(AnalysisWZ,0);

};

#endif

