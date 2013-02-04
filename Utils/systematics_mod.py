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
          applied to the yields of the WZ sample. The errors are relative to the WZ yields
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

print """\033[1;31m**WARNING**WARNING**WARNING**WARNING**WARNING**WARNING**WARNING**WARNING**\033[1;m
  THIS FILE IS DEPRECATED. You should use the 'systematic_mod' module you built in
  the main working directory. See 'sendall -h' in particular '-s' and '-S' options
  and the 'collectall -h' help
\033[1;31m**WARNING**WARNING**WARNING**WARNING**WARNING**WARNING**WARNING**WARNING**\033[1;m"""


# == SYSTEMATICS for WZ analysis ===========================================================
# Relative error with respect the
# 1. Yields of 
#    a. the fake rate sample (data-driven bkg)
DDMMC = { "mmm": 0.31, "mme": 0.46, "eem": 0.42, "eee": 0.14, "leptonchannel": 0.35 }
#    b. the ZZ MC sample (note the stat has to be extracted from the table/plots ...
SYSZZ = {
	'EES': {'eee': 0.009, 'eem': 0.007, 'mmm': 0.00, 'mme': 0.00},
	#'TriggerEff': {'eee': 0.015, 'eem': 0.015, 'mmm': 0.015, 'mme': 0.015},
	'MMS': {'eee': 0.00, 'eem': 0.00, 'mmm': 0.002, 'mme': 0.010},
	'PU': {'eee': 0.03, 'eem': 0.003, 'mmm': 0.006, 'mme': 0.027}, 
	'MET': {'eee': 0.15, 'eem': 0.06, 'mmm': 0.147, 'mme': 0.04}, 
	'LEPTON': {'eee': 0.013, 'eem': 0.022, 'mmm': 0.003, 'mme': 0.005},
	'Lumi': {'eee': 0.022, 'eem': 0.022, 'mmm': 0.022, 'mme': 0.022},
	'PDF': {'eee': 0.015, 'eem': 0.015, 'mmm': 0.015, 'mme': 0.015},
	'Stat': {'eee': 0.0, 'eem': 0.0, 'mmm': 0.0, 'mme': 0.0}
	}
#    b. the WZ MC sample (note the stat has to be extracted from the table/plots ...
SYSWZ = {
	'EES': {'eee': 0.009, 'eem': 0.008, 'mmm': 0.00, 'mme': 0.00},
	#'TriggerEff': {'eee': 0.015, 'eem': 0.015, 'mmm': 0.015, 'mme': 0.015},
	'MMS': {'eee': 0.0, 'eem': 0.00, 'mmm': 0.002, 'mme': 0.001},
	'PU': {'eee': 0.005, 'eem': 0.004, 'mmm': 0.003, 'mme': 0.003}, 
	'MET': {'eee': 0.026, 'eem': 0.024, 'mmm': 0.023, 'mme': 0.029}, 
	'LEPTON': {'eee': 0.019, 'eem': 0.016, 'mmm': 0.003, 'mme': 0.005},
	'Lumi': {'eee': 0.022, 'eem': 0.022, 'mmm': 0.022, 'mme': 0.022},
	'PDF': {'eee': 0.014, 'eem': 0.014, 'mmm': 0.014, 'mme': 0.014},
	'Stat': {'eee': 0.0, 'eem': 0.0, 'mmm': 0.0, 'mme': 0.0}
	}
# == and Statistical for signal ======================================================
STAT = { 'eee' : 0.0, 'eem': 0.0, 'mme' : 0.0, 'mmm': 0.0 }

