#!/usr/bin/env python
"""
:mod:`cosmethicshub_mod` -- Centralizing cosmethic related stuff
.. module:: cosmethicshub_mod
   :platform: Unix
   :synopsis: module which centralize auxiliary datamembers related
              with the cosmethics (latex names, colors,...) to be used by
              python scripts. 
.. moduleauthor:: Jordi Duarte Campderros <duarte@ifca.unican.es>
"""
#Pseudo ROOT colors
kBlack=1
kRed=632
kGreen=416
kAzure=860
kCyan=432
kOrange=800

OTHERBKG = [ "WW", "WJets_Madgraph", "TW_DR", "TbarW_DR" ]

LEGENDSDICT = { "WW": "WW", "WZTo3LNu": "WZ#rightarrow3l#nu", "WJets_Madgraph": "W+Jets",
		"ZZ": "ZZ",
		"TTbar_2L2Nu_Powheg": "t#bar{t}", "TTbar_Madgraph": "t#bar{t} inclusive",
		"ZJets": "Z+Jets", "ZJets_Madgraph": "Z+Jets (MG)",
		"Data": "Data", "Fakes": "Data-driven bkg",
		"TW_DR": "tW", "TbarW_DR": "#bar{t}W",
		"TW": "tW", "TbarW": "#bar{t}W",
		"DDM_ZJets": "DDM Z+Jets",
		"DDM_TTbar": "DDM t#bar{t}",
		"PhotonVJets_Madgraph": "V#gamma",
		"VGamma": "V#gamma",
		"WHToWW2L120": "WH, M_{H}=120",
		"WHToWW2L130": "WH, M_{H}=130", 
		"wzttH120ToWW": "WH, M_{H}=120",
		"wzttH125ToWW": "WH, M_{H}=125",
		"wzttH130ToWW": "WH, M_{H}=130",
		}

PAVECOORD = {'fHNRecoLeptons': 'UPRIGHT', 'fHNSelectedLeptons': 'UPRIGHT',
		'fHMET': 'UPRIGHT', 'fHNJets': 'UPRIGHT', 
		'fHMETAfterZCand': 'UPRIGHT', 'fHZInvMass': 'UPLEFT', 
		'fHD0Lepton': 'UPRIGHT', 'fHZInvMassAfterWCand': 'UPLEFT',
		'fHIsoLepton': 'UPRIGHT', 
		'fHNPrimaryVerticesAfter3Leptons': 'UPRIGHT', 
		'fHNSelectedPVLeptons': 'UPRIGHT', 'fHNSelectedIsoGoodLeptons': 'UPRIGHT',
		'fHEtaLepton1': 'UPLEFT', 'fHEtaLepton2': 'UPLEFT', 'fHEtaLepton3': 'UPLEFT',
		'fHZInvMassAfterZCand': 'UPLEFT', 'fHTransversMass': 'UPRIGHT', 
		'fHTransversMassAfterWCand': 'UPLEFT',
		'fHNPrimaryVertices': 'UPRIGHT', 'fHNSelectedIsoLeptons': 'UPRIGHT',
		'fHPtLepton3': 'UPRIGHT', 'fHPtLepton2': 'UPRIGHT', 'fHPtLepton1': 'UPRIGHT',
		'fHdRl1Wcand': 'UPLEFT', 'fHEventsPerCut': 'UPRIGHT', 'fHdRl2Wcand': 'UPLEFT',
		'fHEventsPerCut3Lepton': 'UPRIGHT', 'fHLeptonCharge': 'UPLEFT', 
		'fHMETAfterWCand': 'UPRIGHT', 'fHProcess': 'UPRIGHT',
		'fHFlavour': 'UPRIGHT',
		'fHHInvMass': 'UPRIGHT', 'fHHInvMassAfterOppSign': 'UPLEFT', \
			'fHHInvMassAfterZVeto': 'UPRIGHT',
		'fHMinDeltaRLp1Lp2': 'UPRIGHT', 'fHMaxDeltaRLp1Lp2': 'UPLEFT',
		'fHMinDeltaPhiLp1Lp2': 'UPRIGHT', 'fHMaxDeltaPhiLp1Lp2':'UPLEFT',
		'fHTrileptonMass': 'UPRIGHT', 'fHTrileptonMassAfterWCand': 'UPRIGHT',
		'fHHT': 'UPRIGHT', 'fHHTAfterWCand':'UPRIGHT',
		'fHLeadingJetET': 'UPRIGHT',
		'fHDeltaPhiWMET': 'UPRIGHT'
		}

COLORSDICT = { "WW" : kRed+4, "WZTo3LNu": kOrange-2, "WJets_Madgraph": kAzure+3,
		"TTbar_2L2Nu_Powheg": kOrange+5, "TTbar_Madgraph": kOrange+5,
		"ZZ": kRed+3, "ZJets": kCyan-2, "ZJets_Madgraph": kCyan-2,
		"Data": kBlack, "Fakes": kAzure-7, 
		"TW_DR": kGreen-2, "TbarW_DR": kGreen+4,
		"TW": kGreen-2, "TbarW": kGreen+4,
		"DDM_ZJets": kOrange-3,
		"DDM_TTbar": kOrange+5,
		"PhotonVJets_Madgraph": kGreen-5,
		"VGamma": kGreen-5,
		"WHToWW2L120": kRed-4,
		"WHToWW2L130": kRed-3,
		"wzttH120ToWW": kRed-4,
		"wzttH125ToWW": kRed-2,
		"wzttH130ToWW": kRed-3
		}

UNITDICT = { "MET": "(GeV/c)", "PT": "(GeV/c)", "ETA": "", "PHI": "",
		"ZINVMASS": "(GeV/c^{2})", "TRANSVERSMASS": "(GeV/c^{2})",
		"D0": "(cm)",
		"HINVMASS": "(GeV/c^{2})",
		"TRI": "(GeV/c^{2})", # TRILEPTONMASS
		"HT" : "(GeV/c)",
		"LEADINGJETET": "(GeV/c)",
		"DELTAPHIWMET": "",
		"MAXDELTAPHILP1LP2": "", "MINDELTAPHILP1LP2": "",
		"MAXDELTARLP1LP2":"", "MINDELTARLP1LP2":""
		}

VARDICT = { "MET": "E_{t}^{miss}", "PT": "p_{t}", "ETA": "#eta", "PHI": "#phi",
		"ZINVMASS": "M_{ll}", "TRANSVERSMASS": "m_{T}",
		"D0": "d_{0}", "CHARGE": "#Sigma q",
		"HINVMASS": "M_{l^{+}l^{-}}", 
		"MINDELTARLP1LP2" : "#DeltaR(l^{+},l^{-}) smallest",
		"MAXDELTARLP1LP2" : "#DeltaR(l^{+},l^{-}) largest",
		"MINDELTAPHILP1LP2" : "#Delta#Phi(l^{+},l^{-}) smallest",
		"MAXDELTAPHILP1LP2" : "#Delta#Phi(l^{+},l^{-}) largest",
		"TRI": "M_{lll}", # TRILEPTONMASS
		"HT": "H_{T}",
		"LEADINGJETET": "E_{T} lead. jet",
		"DELTAPHIWMET": "#Delta#phi(l_{W},MET)"
		}
