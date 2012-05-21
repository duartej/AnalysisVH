#!/usr/bin/env python


##-- SOME GLOBALS
# BRANCHINGS RATIOS
W2e  =0.1075
W2mu =0.1057
W2tau=0.1125

Z2ee    = 0.03363
Z2mumu  = 0.03366
Z2tautau= 0.03367
Z2ll    = 0.033658

tau2e = 0.1782
tau2mu= 0.1739

# When using the madgraph WZ->3lnu, the cross-section of the sample is already 
# sigma_{WZTo3LNu sample}=sigma_WZ*BR(WZ->3lnu), so if
WZ23lnu = 3.0*(Z2ll)*(W2e+W2mu+W2tau)

WZ2eee = W2e*Z2ee+W2tau*tau2e*Z2ee+W2tau*tau2e*Z2tautau*tau2e**2.0+W2e*Z2tautau*tau2e**2.0
WZ2mmm = W2mu*Z2mumu+W2tau*tau2mu*Z2mumu+W2tau*tau2mu*Z2tautau*tau2mu**2.0+W2mu*Z2tautau*tau2mu**2.0
WZ2eem= W2mu*Z2ee+W2mu*Z2tautau*tau2e**2.0+\
   	2.0*W2e*Z2tautau*tau2e*tau2mu+2.0*W2tau*tau2e*Z2tautau*tau2e*tau2mu+\
   	W2tau*tau2mu*Z2ee+W2tau*tau2mu*Z2tautau*tau2e**2.0
WZ2mme= W2e*Z2mumu+W2e*Z2tautau*tau2mu**2.0+\
   	2.0*W2mu*Z2tautau*tau2mu*tau2e+2.0*W2tau*tau2mu*Z2tautau*tau2mu*tau2e+\
   	W2tau*tau2e*Z2mumu+W2tau*tau2e*Z2tautau*tau2mu**2.0
WZ2lnu_tau2eORmu = WZ2eee+WZ2mmm+WZ2eem+WZ2mme

WZ2tauNoMuNoE=WZ23lnu-WZ2lnu_tau2eORmu

#print "Branching ratios: "
#print "WZ --> 3lnu: %.5f" % WZ23lnu
#print "WZ --> eee : %.5f   WZ --> eem : %.5f" % (WZ2eee,WZ2eem)
#print "WZ --> mmm : %.5f   WZ --> mme : %.5f" % (WZ2mmm,WZ2mme)
#print "Total BR WZ->3lnu going to eee,eem,mme or mmm: %.5f" % (WZ2lnu_tau2eORmu)

# SYSTEMATICS For FAKES (in relative error)
#DDDMC = { "mmm": 1.2, "mme": 1.8, "eem": 1.2, "eee": 0.8, "leptonchannel": 5.0 }
DDMMC = { "mmm": 0.31, "mme": 0.46, "eem": 0.42, "eee": 0.14, "leptonchannel": 0.35 }
# -- FIXME: Extract the statistical SAMPLESTAT error for WZ and ZZ from the files
# In relative error
SYS = { "Lumi": 0.022, " TriggerEff": 0.015, "LeptonEff": 0.020, "MuonMomentumScale": 0.015,
		"ElecEnergyScale": 0.025, "METRes": 0.020, "DDMMC": DDMMC, "PILEUP": 0.020,
		"PDF": 0.014 , 
		"SAMPLESTAT": { "WZ": {"mmm": 0.008 , "mme": 0.008 , "eem":0.010 ,"eee": 0.0096, "leptonchannel": 0.004 },  
			"ZZ": { "mmm": 0.048, "mme": 0.078, "eem": 0.18 , "eee": 0.105 , "leptonchannel": 0.0367 } 
			}
	}


def getnsignalsys(asciirawtable,Nsig,channel):
	"""..function::getnsignalsys(asciirawtable,Nsig,channel) --> (Nsigup,Nsigdown)	
	
	Systematics related with the number of signal. Returns the Nsig+sysup,Nsig-sysdown
	"""
	from math import sqrt
	## Systematics affecting Number of signal= N_data- N_bkg.expected

	# ---- ZZ systematics -----------------------------------
	zsys2 = 0.0
	for sysname,val in SYS.iteritems():
		if sysname == "Lumi":
			continue
		elif sysname == "SAMPLESTAT": # --> ?? I think it is already considered with
			# the statistical error (when do N_data-N_bkg and err(N_data-N_bkg)
			#zsys2 += val["ZZ"][channel]**2.0
			continue
		elif sysname == "DDMMC":
			continue
		else:
			zsys2 += val**2.0
	zsys = sqrt(zsys2)

	lineZZ = filter(lambda x: x.find("ZZ") != -1,asciirawtable)[0]
	NZZ = lineZZ.split("&")[-1].split("$\\pm$")[0].strip()
	dNZZup   = float(NZZ)*zsys
	dNZZdown = -1.0*float(NZZ)*zsys
	# --- Fakeable Object sample (ttbar + ZJets) systematics ---
	frsys2 = DDMMC[channel]
	lineFR = filter(lambda x: x.lower().find("fakes") != -1 or
			x.lower().find("data-driven") != -1, asciirawtable)[0]
	Nfr = lineFR.split("&")[-1].split("$\\pm$")[0].strip()
	dNfrup   = float(Nfr)
	dNfrdown = -float(Nfr)

	## --> Aplying to N_bkg.expected
	Nsigup  = float(Nsig)-dNZZup-dNfrup
	Nsigdown= float(Nsig)-dNZZdown-dNfrdown
	#-------------------------------------------------------
	return (Nsigup,Nsigdown)
	#return (abs(Nsigup-float(Nsig))/float(Nsig),abs(Nsigdown-float(Nsig))/float(Nsig))


def getwzsys(Npass,Ngen,channel):
	"""..function::getwzsys(asciirawtable,Npass,Ngen,channel) --> (effWZ_up,effWZ_down)	
	
	Systematics related with the efficiency of the WZ Monte Carlo. Returns the eff+sysup,eff-sysdown
	"""
	from math import sqrt
	## Systematics affecting eff_WZ

	# ---- ZZ systematics -----------------------------------
	wzsys2 = 0.0
	for sysname,val in SYS.iteritems():
		if sysname == "Lumi":
			continue
		elif sysname == "SAMPLESTAT":
			wzsys2 += val["WZ"][channel]**2.0
		elif sysname == "DDMMC":
			continue
		else:
			wzsys2 += val**2.0
	wzsys = sqrt(wzsys2)

	Npass_up   = float(Npass)*(1.0+wzsys)
	Npass_down = float(Npass)*(1.0-wzsys)

	eff_up  = float(Npass_up)/float(Ngen)
	eff_down= float(Npass_down)/float(Ngen)

	return (eff_up,eff_down)




def xscalc(path,zoutrange):
	"""
	"""
	import os,sys
	import ROOT

	# -- Cross-section 
	xs = lambda Nsig,eff,L: float(Nsig)/(float(eff)*float(L))
	Lumi=4922.0 # pb^-1
	
	channelBRdict = { "mmm": WZ2mmm, "eee": WZ2eee, "eem" : WZ2eem, "mme": WZ2mme, \
			"leptonchannel" : WZ2lnu_tau2eORmu}
	# -- 
	signalroot="cluster_WZTo3LNu/Results/WZTo3LNu.root"
	signaldn = "WZTo3LNu_datanames.dn"
	# Z in [60,120]--- HARDCODED WARNING
	Ngen = 801792.0 
	if opt.zoutrange:
		# Using the phase space of the WZ Monte Carlo sample
		Ngen=0
	pwd=path
	# Check we have the excepted structure
	if not os.path.isdir(os.path.join(path,"leptonchannel")):
		message =  "\033[31mxscalc ERROR\033[m The '%s' path has not the expected folder structure"
		message += " (leptonchannel SIGNALeee SIGNALeem SIGNALmme SIGNALmmm)"
		sys.exit(message)
	print "Channel name: value+-(stat)+-(sys_up,sys_down)+-(lumi) --- Same as WZ->3lnu"
	for channel in [ "mmm", "mme", "eem", "eee" , "leptonchannel"]:
		if channel != "leptonchannel":
			workingpath=os.path.join(pwd,"WZ"+channel)
		else:
			workingpath=os.path.join(pwd,channel)
		os.chdir(workingpath)
		# Number of generated events (if not extracted yet)
		if Ngen == 0:
			f = open(signaldn)
			lines=f.readlines()
			f.close()
			Ngen = float(filter(lambda x: x.find("NEvents") != -1,lines)[0].replace("NEvents:",""))
		
		f = ROOT.TFile(os.path.join(workingpath,signalroot))
		histo = f.Get("fHEventsPerCut")
		Npass = float(histo.GetBinContent(histo.GetNbinsX()))
		f.Close()
		f.Delete()
		# --- efficiency
		eff_WZ = Npass/(Ngen/WZ23lnu)
		# -- Nsignal extraction
		tablefile = "table_"+channel+".tex"
		if channel != "leptonchannel":
			tablefile = tablefile.replace(channel,"WZ"+channel)
		f = open(os.path.join(workingpath,tablefile))
		lines = f.readlines()
		f.close()
		try:
			lineSignal = filter(lambda x: x.find("Data-TotBkg") != -1,lines)[0]
		except IndexError:
			# Just for compatibility: TO BE DEPRECATED When all the samples
			# has updated
			lineSignal = filter(lambda x: x.find("Nobs-Nbkg") != -1,lines)[0]
		Nsig = lineSignal.split("&")[-1].split("$\\pm$")[0].strip()
		NsigErr = lineSignal.split("&")[-1].split("$\\pm$")[1].replace("\\","").strip()
		
		# Number of signal up-down due to the systematics applyed to the N-signal (ZZ, FR)
		NsysUp,NsysDown = getnsignalsys(lines,Nsig,channel)
		N_sys_list = [ NsysUp, NsysDown ]
		# Efficiency related systematic (WZ-Monte Carlo)
		eff_WZUp,eff_WZDown = getwzsys(Npass,(Ngen/WZ23lnu),channel)
		eff_WZ_sys_list = [ eff_WZUp, eff_WZDown ]
		# -- Extracting the systematics: maximum and minimum between the Nsig-sys and the 
		#    eff-sys
		cs_sys = []
		for Nsys in N_sys_list:
			for eff_WZ_sys in eff_WZ_sys_list:
				cs_sys.append( xs(Nsys,eff_WZ_sys,Lumi) )
		# -- Cross-section
		cs_WZ = xs(Nsig,eff_WZ,Lumi)
		cserr_WZ = xs(NsigErr,eff_WZ,Lumi)
		cs_WZ_Up = max(cs_sys)
		cs_WZ_Down = min(cs_sys)
		cs_sys_WZ_Up = abs(cs_WZ_Up-cs_WZ)
		cs_sys_WZ_Down = abs(cs_WZ-cs_WZ_Down)
		cs_sys_WZ_Lumi= abs(xs(Nsig,eff_WZ,Lumi*(1.0+SYS["Lumi"]))-cs_WZ)
		cs = cs_WZ*channelBRdict[channel]
		cserr = cserr_WZ*channelBRdict[channel]
		cssys_Up = cs_sys_WZ_Up*channelBRdict[channel]
		cssys_Down = cs_sys_WZ_Down*channelBRdict[channel]
		cs_sys_Lumi = cs_sys_WZ_Lumi*channelBRdict[channel]
		mainstr = "%.5f&plusmn;%.5f(_stat_)<sub>+%.5f</sub><sup>-%.5f</sup>(_sys_)&plusmn;%.5f(_lumi_)"
		message = "| Channel %s | "+mainstr+" | "+mainstr+" |"
		print message %	(channel,cs,cserr,cssys_Up,cssys_Down,cs_sys_Lumi,cs_WZ,cserr_WZ,cs_sys_WZ_Up,cs_sys_WZ_Down,cs_sys_WZ_Lumi)
	
		os.chdir(pwd)

if __name__ == '__main__':
	from optparse import OptionParser,OptionGroup
	import os
	
	#Opciones de entrada
	parser = OptionParser()
	parser.set_defaults(workingpath=os.getcwd(),zoutrange=False)
	parser.add_option( '-w', '--workingdir', action='store', type='string', dest='workingpath',\
			help="Working directory. It must exist the usual folder structure")
	parser.add_option( '-z', '--ZrangeasinMC', action='store_true', dest='zoutrange',\
			help="Per default, the number of generated events to calculate the efficiency"\
			" is calculated using the phase space of M_Z in [60,120]. Activating this option,"\
			" the script is going to use the phase space used in the WZ Monte Carlo sample"\
			" creation.")

	(opt,args) = parser.parse_args()

	print "\033[34mxscalc INFO\033[m Evaluating the cross-section at '%s'" % opt.workingpath
	xscalc(opt.workingpath,opt.zoutrange)
