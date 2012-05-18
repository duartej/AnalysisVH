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

# The BR w.r.t. the BR(WZ->3lnu)
#WZ2eee = TotalWZ2eee/WZ23lnu
#WZ2eem = TotalWZ2eem/WZ23lnu
#WZ2mme = TotalWZ2mme/WZ23lnu
#WZ2mmm = TotalWZ2mmm/WZ23lnu
#WZ2lnu_tau2eORmu = TotalWZ2lnu_tau2eORmu/WZ23lnu

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
SYS = { "Lumi": 2.2, " TriggerEff": 1.5, "LeptonEff": 2.0, "MuonMomentumScale": 1.5,
		"ElecEnergyScale": 2.5, "METRes": 2.0, "DDMMC": DDMMC, "PILEUP": 2.0,
		"PDF": 1.4 , "SAMPLESTAT": { "WZ": 0.5, "ZZ": 0.2 } }


def systematics(asciirawtable,Nsig,channel):
	"""
	"""
	from math import sqrt
	# ---- ZZ systematics -----------------------------------
	zsys2 = 0.0
	for sysname,val in SYS.iteritems():
		if sysname == "Lumi":
			continue
		elif sysname == "SAMPLESTAT":
			zsys2 += val["ZZ"]**2.0
		elif sysname == "DDMMC":
			zsys2 += val[channel]**2.0
		else:
			zsys2 += val**2.0
	zsys = sqrt(zsys2)
	# --> Affecting the Yields of the background 
	lineZZ = filter(lambda x: x.find("ZZ") != -1,asciirawtable)[0]
	NZZ = lineZZ.split("&")[-1].split("$\\pm$")[0].strip()
	NZZup  = float(NZZ)*(1.0+zsys)
	NZZdown= float(NZZ)*(1.0-zsys)
	Nsigup  = float(Nsig)-NZZup
	Nsigdown= float(Nsig)-NZZdown
	#-------------------------------------------------------
	print zsys
	print Nsig,Nsigup,Nsigdown





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

		systematics(lines,Nsig,channel)
		# -- Cross-section
		cs_WZ = xs(Nsig,eff_WZ,Lumi)
		cserr_WZ = xs(NsigErr,eff_WZ,Lumi)
		cssysDDMMC_WZ = xs(DDMMC[channel],eff_WZ,Lumi)
		#cssysDDDMC_WZ = xs(DDDMC[channel],eff_WZ,Lumi)
		cs = cs_WZ*channelBRdict[channel]
		cserr = cserr_WZ*channelBRdict[channel]
		cssysDDMMC = cssysDDMMC_WZ*channelBRdict[channel]
		#cssysDDDMC = cssysDDDMC_WZ*channelBRdict[channel]
		print "Channel %s: %.5f+-%.5f+-%.5f ||  "\
				"xs_WZ inclusive: %.5f+-%.5f+-%.5f" %\
				(channel,cs,cserr,cssysDDMMC,cs_WZ,cserr_WZ,cssysDDMMC_WZ)
	
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
