#!/usr/bin/env python
"""
:mod:`functionspool` -- Centralizing utilities
.. module:: functionspool
   :platform: Unix
   :synopsis: module which centralize auxiliary functions used by other
                     python scripts. 
.. moduleauthor:: Jordi Duarte Campderros <duarte@ifca.unican.es>

   + gettablecontent
   + extractyields
"""


def gettablecontent(workingpath,channel,signal="WZ"):
	""".. function:: gettablecontent(workingpath,channel) --> lines
	Extract the content of the standard tex table which should exist
	in every processed folder (after plotall)

	:param workingpath: the parent path, just contain the channels folders
	:type workinpath: string
	:param channel: the nam of the channel 
	:type channel: string

	:return: a list containing all the lines of the table file
	:rtype: list(str)
	"""
	import os

	tablefile = "table_"+channel+".tex"
	if channel != "leptonchannel":
		tablefile = tablefile.replace(channel,signal+channel)
	f = open(os.path.join(workingpath,tablefile))
	lines = f.readlines()
	f.close()

	return lines


def extractyields(asciirawtable,titlerow):
	""".. function:: extractyields(asciirawtable,titlerow) --> (n,nerrorstat)
	
	From a the lines of the table in .tex format (the reduced one of the standard output), 
	the yields of the titlerow' row will be extracted

	:param asciirawtable: lines in the .tex reduced table output from the standard analysis chain
	:type asciirawtable: str
	:param titlerow: the name of the sample or of the column/row you want to extract the yields
	:type titlerow: str

	:return: the yields and the statistical error
	:rtype: tuple(float,float)
	"""
	line=None
	try:
		line = filter(lambda x: x.find(titlerow) != -1,asciirawtable)[0]
	except IndexError:
		raise RuntimeError("\033[31;1mextractyields ERROR\033[m Not found the sample '%s'" % titlerow)
	N = float(line.split("&")[-1].split("$\\pm$")[0].strip())
	Nerrstat = float(line.split("&")[-1].split("$\\pm$")[1].replace("\\","").strip())

	return (N,Nerrstat)


def getpassevts(rootfile,cutlevel=-1):
	""".. function:: getpassevts(rootfile,cutlevel) --> evts
	
	Given a cutlevel and the rootfile, extract the number of passed events at that cut level
	"""
	import ROOT

	f = ROOT.TFile(rootfile)
	if f.IsZombie():
		message="\033[31;1mgetpassevts ERROR\033[m File '%s' not found." % rootfile
		raise OSError(message)
	histo = f.Get("fHEventsPerCut")
	if cutlevel == -1:
		cutlevel = histo.GetNbinsX()
	else:
		if cutlevel > histo.GetNbinsX():
			message="\033[31;1mgetpassevts ERROR\033[m Cutlevel '%i' greater than maximum (%i)." % (cutlevel,histo.GetNbinsX())
			raise RuntimeError(message)

	Npass = float(histo.GetBinContent(cutlevel))
	f.Close()
	f.Delete()

	return Npass



def getxserrorsrel(workingpath):
	"""
	"""
	from systematics import SYSZZ,SYSWZ,DDMMC,STAT
	import os
	from math import sqrt

	xserrors = { "total": {}, "stat": {}, "lumi": {}, "sys": {}}

	Ngen = 801792.0 
	print "\033[33;1mgetxserrorsrel WARNING\033[m HARDCODED number of WZ->3lnu events generated within the"\
			" Z range mass [60,120]: %d" % Ngen
	# Number of generated events (if not extracted yet)
	if Ngen == 0:
		f = open(signaldn)
		lines=f.readlines()
		f.close()
		Ngen = float(filter(lambda x: x.find("NEvents") != -1,lines)[0].replace("NEvents:",""))
	
	# Lumi
	Lumi = 4922.0  # pb-1
	# XS calculation
	xs = lambda N,eff,lumi: float(N)/(float(eff)*float(lumi))


	for channel in [ "eee", "eem", "mme", "mmm" ]:
		# Previuosly ...
		pathchannel = "WZ"+channel
		signalroot=os.path.join(os.path.join(workingpath,pathchannel),
				"cluster_WZTo3LNu/Results/WZTo3LNu.root")
		signaldn = os.path.join(os.path.join(workingpath,pathchannel),
				"WZTo3LNu_datanames.dn")
		# Z in [60,120]--- 
		Npass = getpassevts(signalroot)

		# ================= 1. Extract yields
		pathchannel = "WZ"+channel
		asciirawtable = gettablecontent(os.path.join(workingpath,pathchannel),channel)
		# -- Signal:
		try:
			Nsig,Nsigerr = extractyields(asciirawtable,"Data-TotBkg")
		except RuntimeError:
			# TO BE DEPRECATED WHEN ALL FILES ARE UPDATED
			Nsig,Nsigerr = extractyields(asciirawtable,"Nobs-Nbkg")
		# updating the stat part
		STAT[channel] = Nsigerr/Nsig
		# -- ZZ: 
		Nzz,Nzzerr = extractyields(asciirawtable,"ZZ")
		# updating the stat part
		SYSZZ["Stat"][channel] = Nzzerr/Nzz
		# -- Fakes:
		try:
			Nf,Nferr = extractyields(asciirawtable,"data-driven")
		except RuntimeError:
			# TO BE DEPRECATED WHEN ALL FILES ARE UPDATED
			Nf,Nferr = extractyields(asciirawtable,"Fakes")
		# -- WZ: 
		Nwz,Nwzerr=extractyields(asciirawtable,"WZ")
		# updating the stat part
		SYSWZ["Stat"][channel] = Nwzerr/Nwz
		
		# -- Relative to yields:
		# 1.2. ZZ sytematics
		zsys2 = 0.0
		for sysname,valdict in SYSZZ.iteritems():
			# Note it is going to be considered separately
			# the statistics are considered with the Nsigerr
			if sysname == "Stat":
				continue
			elif sysname == "Lumi":
				continue
			zsys2 += valdict[channel]**2.0
		zsys = sqrt(zsys2)
		dNzz = Nzz*zsys
		# 1.3. Fakeable object systematics
		frsys = DDMMC[channel]
		dNf = Nf*frsys

		# 1.4. evaluate the up and down variation on the signal
		Nsigdown = Nsig-(dNzz+dNf)
		Nsigup   = Nsig+(dNzz+dNf)

		# ======================== 2. Relative to WZ efficiency (on xs = N/(eff*L))
		wzsys2 = 0.0
		for sysname,valdict in SYSWZ.iteritems():
			# Note it is going to be considered separately
			# Also, note we are considering the statistical error here
			# because it affects to the efficiency of the signal
			if sysname == "Lumi":
				continue
			wzsys2 += valdict[channel]**2.0
		wzsys = sqrt(wzsys2)
		# 2.1. Move down and up the number of passing events for WZ
		Npassup = Npass*(1.0+wzsys)
		Npassdown = Npass*(1.0-wzsys)
		# 2.1. Applying to the efficiency
		effup  = Npassup/Ngen
		effdown= Npassdown/Ngen
		# ======================== 3. Rebuild the systematic table where now 
		#                          the errors are relatives to the cross-section
		# 3.1 Calculate the actual xs
		eff = Npass/Ngen
		xsmean = xs(Nsig,eff,Lumi)
		# 3.2 Statistics: using signal stat error
		xserrors["stat"][channel] = (abs(xs(Nsig+Nsigerr,eff,Lumi)-xsmean)/xsmean,
				abs(xs(Nsig-Nsigerr,eff,Lumi)-xsmean)/xsmean)
		# 3.3 Systematics: using Npassup,Npassdown,effup,effdown
		syscomb = []
		for n in [ Nsigup, Nsigdown ]:
			for e in [ effup, effdown ]:
				syscomb.append( (xs(n,e,Lumi)-xsmean)/xsmean )
		sysmin = abs(min(syscomb))
		sysmax = max(syscomb)
		xserrors["sys"][channel] = (sysmax,sysmin)
		# 3.4 Lumi
		lumierr= SYSWZ["Lumi"][channel]
		lumiup = Lumi*(1+lumierr)
		lumidown = Lumi*(1-lumierr)
		xserrors["lumi"][channel] = (abs(xs(Nsig,eff,lumiup)-xsmean)/xsmean,
				abs(xs(Nsig,eff,lumidown)-xsmean)/xsmean)

		xserrors["total"][channel]= 0.0
		
	for channel in [ "eee", "eem", "mme", "mmm" ]: 
		errmin2 = 0.0
		errmax2 = 0.0
		for name,valdict in filter(lambda (x,y): x != "total", xserrors.iteritems()):
			errmax2 += valdict[channel][0]**2.0
			errmin2 += valdict[channel][1]**2.0
		xserrors["total"][channel] = (sqrt(errmax2),sqrt(errmin2))

	print xserrors
	return xserrors




		
	





