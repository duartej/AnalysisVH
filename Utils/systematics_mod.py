#!/usr/bin/env python
""" 
:mod:`sytematics`
.. module:: systematics
   :platform: Unix
   :synopsis: module containing the raw systematics
.. moduleauthor:: Jordi Duarte Campderros <duarte@ifca.unican.es>

.. data:: DDMMC The relative error systematic due to the fakeable object method and to be 
          applied to the yields of the Tight-Tight-noTight sample. The errors are relative
	  to the yields.
   :type: dict(str:float)

.. data:: SYSZZ The relative error systematics separated by systematic sources and to be 
          applied to the yields of the ZZ sample. The errors are relative to the ZZ yields
	  and they are going to affect the Number of signal
   :type: dict(dict(str:float))

.. data:: SYSWZ The relative error systematics separated by systematic sources and to be 
          applied to the yields of the ZZ sample. The errors are relative to the WZ yields
	  and they are going to affect the efficiency (on cross-section calculation)
   :type: dict(dict(str:float))

.. data:: STAT The relative error statistic of the signal, i.e. number of data observed minus
          number of background estimated (so the statistic errors of the data observed and 
	  background expected are incorporated here). The errors are relative to the yields.
	  WARNING: the dictionary is just initialized here and must be initialized and filled
	  using the functionspool_mod.getxserrosrel function, so this data cannot be used before
	  the call of the functionspool_mod.getxserrosrel
   :type: dict(str:float)
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

