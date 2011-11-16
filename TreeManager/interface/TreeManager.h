// -*- C++ -*-
//
// Package:    TreeManager
// Class:      TreeManager
// 
/**\class  TreeManager.h

 Description: This class manage the output format of the root files.
              This is an abstract class and the concrete classes
	      will content the data members available on a file, taking
	      care of the branches initialization, etc... The class is used 
	      to encapsulate the different types of Trees (MiniTrees, Latinos
	      and TESCO) taking the responsability just for this class to known how is
	      the content of a ROOT file. Some concrete type of this class is
	      going to be as datamember in the Analysis (client), the methods used 
	      by the client has to be defined here (as pure virtual methods). So,
	      in order to unify all the kind of Trees here has to be defined the
	      methods used to extract info from the file, and in the concrete class
	      the concrete implementation depending of the Tree kind.

 Implementation: Abstract class used by the Analysis client. Just the initialization
              of the tree and the definition of the methods which has to be implemented
	      in the concrete classes
*/
//
// Original Author: Jordi Duarte Campderros  
//         Created:  Sun Oct  30 12:20:31 CET 2011
// 
// jordi.duarte.campderros@cern.ch
//
//
#ifndef TREEMANAGER_H
#define TREEMANAGER_H

#include "TROOT.h"
#include "TChain.h"

class TreeManager
{
	public:
		TreeManager() : fChain(0) { }
		virtual ~TreeManager() { }; // WHY virtual??!!!

		void Init(TTree * tree);  // = 0;
		void GetEntry( const int & entry );
		
		template <class T>
		T Get(const char * branchname,const int & index = 0) const;

		template <class T>
		unsigned int GetSize(const char * branchname) const;
		// ==== ADD HERE YOUR NEW METHOD AND DON'T FORGET 
		//      TO INCLUDE ALSO IN TreeManager::getmethods 
		// ==================================================
		// Methods to extract concret values from a Tree
		
		// ---- Generation
		// ------- Status 3 particles
		/*virtual std::vector<int>* GetGenElecSt3PID() const = 0;
		virtual std::vector<int>* GetGenMuonSt3PID() const = 0;
		virtual std::vector<int>* GetGenTauSt3PID()  const = 0;

		// ---------- Electrons
		// ------ Mother PID
		virtual std::vector<int>* GetGenElecMPID()     const  = 0;
		// ------ Status Mother
		virtual std::vector<int>* GetGenElecMSt()      const  = 0;
		// ------ Energy and momentum
		virtual std::vector<float>* GetGenElecEnergy() const = 0;
		virtual std::vector<float>* GetGenElecPx()     const = 0;
		virtual std::vector<float>* GetGenElecPy()     const = 0;
		virtual std::vector<float>* GetGenElecPz()     const = 0;
		// ---------- Muons
		// ------ Mother PID
		virtual std::vector<int>* GetGenMuonMPID()     const  = 0;
		// ------ Status Mother
		virtual std::vector<int>* GetGenMuonMSt()      const  = 0;
		// ------ Energy and momentum
		virtual std::vector<float>* GetGenMuonEnergy() const = 0;
		virtual std::vector<float>* GetGenMuonPx()     const = 0;
		virtual std::vector<float>* GetGenMuonPy()     const = 0;
		virtual std::vector<float>* GetGenMuonPz()     const = 0;

		// ---- End Generation

		// ---- Reco Electron
		// ------- kinematic
		virtual std::vector<float>* GetElecPt() const = 0;
		virtual std::vector<float>* GetElecPx() const = 0;
		virtual std::vector<float>* GetElecPy() const = 0;
		virtual std::vector<float>* GetElecPz() const = 0;
		virtual std::vector<float>* GetElecEnergy() const = 0;
		
		virtual std::vector<int>* GetElecCharge() const = 0;
		
		// ------- Position
		virtual std::vector<float>* GetElecvx() const = 0;
		virtual std::vector<float>* GetElecvy() const = 0;
		virtual std::vector<float>* GetElecvz() const = 0;

		// ------- Quality
		virtual std::vector<float>* GetElecSCEta() const = 0;
		virtual std::vector<float>* GetElecdeltaPhiIn() const = 0;
		virtual std::vector<float>* GetElecdeltaEtaIn() const = 0;
		virtual std::vector<float>* GetElecHtoE() const = 0;
		virtual std::vector<float>* GetElecfBrem() const = 0;
		virtual std::vector<float>* GetEleceSuperClusterOverP() const = 0;
		virtual std::vector<float>* GetElecsigmaIetaIeta() const = 0;
		
		// ------- Isolation
		virtual std::vector<float>* GetEleceleSmurfPF() const = 0;
		virtual std::vector<bool>* GetElecpassesNewConversion() const = 0;
		virtual std::vector<int>* GetElecnHits() const = 0;
		
		// ------- IP, PV stuff
		virtual std::vector<float>* GetElecIP2DInTrack() const = 0;
		virtual std::vector<float>* GetElecIP2DBiasedPV() const = 0;
		virtual std::vector<float>* GetElecIP2DUnBiasedPV() const = 0;
		virtual std::vector<float>* GetElecdzPVBiasedPV() const = 0;
		virtual std::vector<float>* GetElecdzPVUnBiasedPV() const = 0;

		// ---- Reco Muon
		// ------- kinematic
		virtual std::vector<float>* GetMuonPt() const = 0;
		virtual std::vector<float>* GetMuondeltaPt() const = 0;
		virtual std::vector<float>* GetMuonPx() const = 0;
		virtual std::vector<float>* GetMuonPy() const = 0;
		virtual std::vector<float>* GetMuonPz() const = 0;
		virtual std::vector<float>* GetMuonEnergy() const = 0;

		virtual std::vector<int>* GetMuonCharge() const = 0;
		
		// ------- Isolation
		virtual std::vector<float>* GetMuonSumIsoTrack() const = 0;
		virtual std::vector<float>* GetMuonSumIsoCalo() const = 0;
		virtual std::vector<float>* GetMuonmuSmurfPF() const = 0;
		
		// ------- Position
		virtual std::vector<float>* GetMuonvx() const = 0;
		virtual std::vector<float>* GetMuonvy() const = 0;
		virtual std::vector<float>* GetMuonvz() const = 0;
		
		// ------- IP, PV stuff
		virtual std::vector<float>* GetMuonIP2DInTrack() const = 0;
		virtual std::vector<float>* GetMuonIP2DBiasedPV() const = 0;
		virtual std::vector<float>* GetMuonIP2DUnBiasedPV() const = 0;
		virtual std::vector<float>* GetMuondzPVBiasedPV() const = 0;
		virtual std::vector<float>* GetMuondzPVUnBiasedPV() const = 0;
		
		// ------- Identification
		virtual std::vector<bool>* IsGlobalMuon() const = 0;
		virtual std::vector<bool>* IsAllTrackerMuons() const = 0;
		virtual std::vector<bool>* IsAllStandAloneMuons() const = 0;
		virtual std::vector<bool>* IsTMLastStationTight() const = 0;
		
		// ------- Quality
		virtual std::vector<int>* GetMuonNValidHitsSATrk() const = 0;
		virtual std::vector<int>* GetMuonNumOfMatches() const = 0;
		virtual std::vector<int>* GetMuonNValidPixelHitsInTrk() const = 0;
		//virtual std::vector<int>* GetMuonNValidHitsInTrk() const = 0;
		virtual std::vector<int>* GetMuonInnerTrackFound() const = 0;
		virtual std::vector<float>* GetMuonNormChi2GTrk() const = 0;
		// --- End Reco Muon
		
		
		//-- Jet
		virtual std::vector<float>* GetJetAKPFNoPUPx() const = 0;
		virtual std::vector<float>* GetJetAKPFNoPUPy() const = 0;
		virtual std::vector<float>* GetJetAKPFNoPUPz() const = 0;
		virtual std::vector<float>* GetJetAKPFNoPUEnergy() const = 0;*/
		/*virtual std::vector<float>* GetJetAKPF2PATPx() const = 0;
		virtual std::vector<float>* GetJetAKPF2PATPy() const = 0;
		virtual std::vector<float>* GetJetAKPF2PATPz() const = 0;
		virtual std::vector<float>* GetJetAKPF2PATEnergy() const = 0; MINITREES*/

		/*//-- Met
		virtual const float GetMETPFET() const = 0;
		
		//-- Vertex
		virtual std::vector<float>* GetVertexz() const = 0;
		
		// General
		virtual int GetEventEventNumber() const = 0;
		virtual int GetEventprocessID()   const = 0;
		virtual int GetEventnPU()         const = 0;
		// ==== END
		// ==================================================*/
  		
		// Reflexion: to extract methods needed to be implemented
		// in the concrete Manager... 
		/*inline static std::vector<std::string> getmethods()
		{
			std::vector<std::string> methods;
	
			// Generation
			methods.push_back("virtual std::vector<int>* GetGenElecSt3PID() const");
			methods.push_back("virtual std::vector<int>* GetGenMuonSt3PID() const");
			methods.push_back("virtual std::vector<int>* GetGenTauSt3PID() const");
			
			methods.push_back("virtual std::vector<int>* GetGenElecMPID()     const");
			methods.push_back("virtual std::vector<int>* GetGenElecMSt()      const");
			methods.push_back("virtual std::vector<float>* GetGenElecEnergy() const");
			methods.push_back("virtual std::vector<float>* GetGenElecPx()     const");
			methods.push_back("virtual std::vector<float>* GetGenElecPy()     const");
			methods.push_back("virtual std::vector<float>* GetGenElecPz()     const");
			methods.push_back("virtual std::vector<int>* GetGenMuonMPID()     const");
			methods.push_back("virtual std::vector<int>* GetGenMuonMSt()      const");
			methods.push_back("virtual std::vector<float>* GetGenMuonEnergy() const");
			methods.push_back("virtual std::vector<float>* GetGenMuonPx()     const");
			methods.push_back("virtual std::vector<float>* GetGenMuonPy()     const");
			methods.push_back("virtual std::vector<float>* GetGenMuonPz()     const");
			
			// Reco Electrons
			// kinematic
			methods.push_back("virtual std::vector<float>* GetElecPt() const");
			methods.push_back("virtual std::vector<float>* GetElecPx() const");
			methods.push_back("virtual std::vector<float>* GetElecPy() const");
			methods.push_back("virtual std::vector<float>* GetElecPz() const");
			methods.push_back("virtual std::vector<float>* GetElecEnergy() const");
			
			methods.push_back("virtual std::vector<int>* GetElecCharge() const");
			
			// Position
			methods.push_back("virtual std::vector<float>* GetElecvx() const");
			methods.push_back("virtual std::vector<float>* GetElecvy() const");
			methods.push_back("virtual std::vector<float>* GetElecvz() const");
	
			// Quality
			methods.push_back("virtual std::vector<float>* GetElecSCEta() const");
			methods.push_back("virtual std::vector<float>* GetElecdeltaPhiIn() const");
			methods.push_back("virtual std::vector<float>* GetElecdeltaEtaIn() const");
			methods.push_back("virtual std::vector<float>* GetElecHtoE() const");
			methods.push_back("virtual std::vector<float>* GetElecfBrem() const");
			methods.push_back("virtual std::vector<float>* GetEleceSuperClusterOverP() const");
			methods.push_back("virtual std::vector<float>* GetElecsigmaIetaIeta() const");

			// Isolation
			methods.push_back("virtual std::vector<float>* GetEleceleSmurfPF() const");
			methods.push_back("virtual std::vector<bool>* GetElecpassesNewConversion() const");
			methods.push_back("virtual std::vector<int>* GetElecnHits() const");
			
			// IP, PV stuff
			methods.push_back("virtual std::vector<float>* GetElecIP2DInTrack() const");
			methods.push_back("virtual std::vector<float>* GetElecIP2DBiasedPV() const");
			methods.push_back("virtual std::vector<float>* GetElecIP2DUnBiasedPV() const");
			methods.push_back("virtual std::vector<float>* GetElecdzPVBiasedPV() const");
			methods.push_back("virtual std::vector<float>* GetElecdzPVUnBiasedPV() const");


			// Reco Muons
			// kinematic
			methods.push_back("virtual std::vector<float>* GetMuonPt() const");
			methods.push_back("virtual std::vector<float>* GetMuondeltaPt() const");
			methods.push_back("virtual std::vector<float>* GetMuonPx() const");
			methods.push_back("virtual std::vector<float>* GetMuonPy() const");
			methods.push_back("virtual std::vector<float>* GetMuonPz() const");
			methods.push_back("virtual std::vector<float>* GetMuonEnergy() const");
			methods.push_back("virtual std::vector<int>* GetMuonCharge() const");
			
			// Isolation
			methods.push_back("virtual std::vector<float>* GetMuonSumIsoTrack() const");
			methods.push_back("virtual std::vector<float>* GetMuonSumIsoCalo() const");
			methods.push_back("virtual std::vector<float>* GetMuonmuSmurfPF() const");
			
			// Position
			methods.push_back("virtual std::vector<float>* GetMuonvx() const");
			methods.push_back("virtual std::vector<float>* GetMuonvy() const");
			methods.push_back("virtual std::vector<float>* GetMuonvz() const");
			
			// IP, PV stuff
			methods.push_back("virtual std::vector<float>* GetMuonIP2DInTrack() const");
			methods.push_back("virtual std::vector<float>* GetMuonIP2DBiasedPV() const");
			methods.push_back("virtual std::vector<float>* GetMuonIP2DUnBiasedPV() const");
			methods.push_back("virtual std::vector<float>* GetMuondzPVBiasedPV() const");
			methods.push_back("virtual std::vector<float>* GetMuondzPVUnBiasedPV() const");
			
			// Identification 
			methods.push_back("virtual std::vector<bool>* IsGlobalMuon() const");
			methods.push_back("virtual std::vector<bool>* IsAllTrackerMuons() const");
			methods.push_back("virtual std::vector<bool>* IsAllStandAloneMuons() const");
			methods.push_back("virtual std::vector<bool>* IsTMLastStationTight() const");
			
			// Quality
			methods.push_back("virtual std::vector<int>* GetMuonNValidHitsSATrk() const");
			methods.push_back("virtual std::vector<int>* GetMuonNumOfMatches() const");
			methods.push_back("virtual std::vector<int>* GetMuonNValidPixelHitsInTrk() const");
			//methods.push_back("virtual std::vector<int>* GetMuonNValidHitsInTrk() const");
			methods.push_back("virtual std::vector<int>* GetMuonInnerTrackFound() const");
			methods.push_back("virtual std::vector<float>* GetMuonNormChi2GTrk() const");
		
			//-- Jet
			methods.push_back("virtual std::vector<float>* GetJetAKPFNoPUPx() const");
			methods.push_back("virtual std::vector<float>* GetJetAKPFNoPUPy() const");
			methods.push_back("virtual std::vector<float>* GetJetAKPFNoPUPz() const");
			methods.push_back("virtual std::vector<float>* GetJetAKPFNoPUEnergy() const");*/
			/*methods.push_back("virtual std::vector<float>* GetJetAKPF2PATPx() const");
			methods.push_back("virtual std::vector<float>* GetJetAKPF2PATPy() const");
			methods.push_back("virtual std::vector<float>* GetJetAKPF2PATPz() const");
			methods.push_back("virtual std::vector<float>* GetJetAKPF2PATEnergy() const"); MINITREES*/

			/*//-- Met
			methods.push_back("virtual const float GetMETPFET() const");
			
			//-- Vertex
			methods.push_back("virtual std::vector<float>* GetVertexz() const");
			
			// General
			methods.push_back("virtual int GetEventEventNumber() const");
			methods.push_back("virtual int GetEventprocessID() const");
			methods.push_back("virtual int GetEventnPU() const");
			
			return methods;
		}*/

	private:
		TTree * fChain;

	
	ClassDef(TreeManager,0);
};


#endif
