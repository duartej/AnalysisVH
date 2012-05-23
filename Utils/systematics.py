#!/usr/bin/env python
""" 
:mod:`sytematics`
.. module:: systematics
   :platform: Unix
   :synopsis: module containing the raw systematics
.. moduleauthor:: Jordi Duarte Campderros <duarte@ifca.unican.es>
"""
# == SYSTEMATICS for WZ analysis ===========================================================
# Relative error with respect the
# 1. Yields of 
#    a. the fake rate sample (data-driven bkg)
DDMMC = { "mmm": 0.31, "mme": 0.46, "eem": 0.42, "eee": 0.14, "leptonchannel": 0.35 }
#    b. the ZZ MC sample (note the stat has to be extracted from the table/plots ...
SYSZZ = {
	'ElecEnergyScale': {'eee': 0.025, 'eem': 0.025, 'mmm': 0.025, 'mme': 0.025},
	'TriggerEff': {'eee': 0.015, 'eem': 0.015, 'mmm': 0.015, 'mme': 0.015},
	'MuonMomentumScale': {'eee': 0.015, 'eem': 0.015, 'mmm': 0.015, 'mme': 0.015},
	'PILEUP': {'eee': 0.023, 'eem': 0.023, 'mmm': 0.023, 'mme': 0.023}, 
	'METres': {'eee': 0.02, 'eem': 0.02, 'mmm': 0.02, 'mme': 0.02}, 
	'LeptonEff': {'eee': 0.02, 'eem': 0.02, 'mmm': 0.02, 'mme': 0.02},
	'Lumi': {'eee': 0.022, 'eem': 0.022, 'mmm': 0.022, 'mme': 0.022},
	'PDF': {'eee': 0.015, 'eem': 0.015, 'mmm': 0.015, 'mme': 0.015},
	'Stat': {'eee': 0.0, 'eem': 0.0, 'mmm': 0.0, 'mme': 0.0}
	}
#    b. the WZ MC sample (note the stat has to be extracted from the table/plots ...
SYSWZ = {
	'ElecEnergyScale': {'eee': 0.025, 'eem': 0.025, 'mmm': 0.025, 'mme': 0.025},
	'TriggerEff': {'eee': 0.015, 'eem': 0.015, 'mmm': 0.015, 'mme': 0.015},
	'MuonMomentumScale': {'eee': 0.015, 'eem': 0.015, 'mmm': 0.015, 'mme': 0.015},
	'PILEUP': {'eee': 0.02, 'eem': 0.02, 'mmm': 0.02, 'mme': 0.02}, 
	'METres': {'eee': 0.02, 'eem': 0.02, 'mmm': 0.02, 'mme': 0.02}, 
	'LeptonEff': {'eee': 0.02, 'eem': 0.02, 'mmm': 0.02, 'mme': 0.02},
	'Lumi': {'eee': 0.022, 'eem': 0.022, 'mmm': 0.022, 'mme': 0.022},
	'PDF': {'eee': 0.014, 'eem': 0.014, 'mmm': 0.014, 'mme': 0.014},
	'Stat': {'eee': 0.0, 'eem': 0.0, 'mmm': 0.0, 'mme': 0.0}
	}
# == and Statistical for signal ======================================================
STAT = { 'eee' : 0.0, 'eem': 0.0, 'mme' : 0.0, 'mmm': 0.0 }

