#!/usr/bin/env python
"""
Evaluating the cross section given the ouput of the analysis
"""
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


def adduperrors(xserror,xs):
	"""
	"""
	from math import sqrt

	xsabserror = { "sys": {}, "stat": {}, "lumi": {}, "total": {} }
	channels = xserror.values()[0].keys()
	
	# Note that the asymetric errors are taken as mean
	evalsyserr = lambda errtupleval,ch: ((errtupleval[0]*+errtupleval[1])/2.0*xs[ch])**2.0


	# -- Initializing
	for channel in channels:
		for errortype in xsabserror.keys(): 
			xsabserror[errortype][channel] = 0.0
	# And filling
	for errorname,channeldict in xserror.iteritems():
		for channel, errval in channeldict.iteritems():
			if errorname == "STATS":
				xsabserror["stat"][channel] += evalsyserr(errval,channel)
			elif errorname == "Lumi":
				xsabserror["lumi"][channel] += evalsyserr(errval,channel)
			else:
				xsabserror["sys"][channel] += evalsyserr(errval,channel)
	
	# Evaluating total
	for errorname,channeldict in filter(lambda (x,y): x!= "total",xsabserror.iteritems()):
		for channel,errval in channeldict.iteritems():
			xsabserror["total"][channel] += errval
	# And sqrting
	for errorname,channeldict in xsabserror.iteritems():
		for channel,errval in channeldict.iteritems():
			xsabserror[errorname][channel] = sqrt(errval)
	
	return xsabserror

def xscalc(path,zoutrange,format):
	"""
	"""
	from functionspool_mod import gettablecontent,getxserrorsrel,BR
	import os,sys
	import ROOT

	# Check we have the expected folder structure
	if not os.path.isdir(os.path.join(path,"leptonchannel")):
		message =  "\033[31mxscalc ERROR\033[m The '%s' path has not the expected folder structure"
		message += " (leptonchannel SIGNALeee SIGNALeem SIGNALmme SIGNALmmm)"
		raise RuntimeError(message)
	
	# -- Cross-section 
	xs = lambda Nsig,eff,L: float(Nsig)/(float(eff)*float(L))
	Lumi=4922.0 # pb^-1
	
	channelBRdict = { "mmm": BR.WZ2mmm, "eee": BR.WZ2eee, "eem" : BR.WZ2eem, "mme": BR.WZ2mme, \
			"leptonchannel" : BR.WZ2lnu_tau2eORmu}

	xsWZ,xsWZrelerrors = getxserrorsrel(path,xstype="inclusive")
	xs,xsrelerrors = getxserrorsrel(path,xstype="exclusive")
	# -- 
	hasprint=True
	if not format:
		hasprint=False
	elif format == "html":
		mainstr    = "%.3f&plusmn;%.3f(stat)&plusmn;%.3f(sys)&plusmn;%.3f(lumi)"
		channelline= "| %s | "+mainstr+" | "+mainstr+" |\n"
	elif format == "tex":
		mainstr = "$%.3f\\pm%.3f(\\text{stat})\\pm%.3f(\\text{sys})\\pm%.5f(\\text{lumi})$"
		channelline = " %s & "+mainstr+" & "+mainstr+"\\\\\n "

	xsabserrors = adduperrors(xsrelerrors,xs)
	xsWZabserrors = adduperrors(xsWZrelerrors,xsWZ)
	if hasprint:
		print "Legend: value+-(stat)+-(sys_up,sys_down)+-(lumi)"
		print "Cross-section for WZ*BR(channel) || Cross-section for WZ "
	outmessage = ""
	xscalcdict = {}
	for channel,csval in xs.iteritems():
		cserr = xsabserrors["stat"][channel]
		cssys = xsabserrors["sys"][channel]
		cssys_Lumi = xsabserrors["lumi"][channel]
		csval_WZ = xsWZ[channel] 
		cserr_WZ = xsWZabserrors["stat"][channel]
		cssys_WZ = xsWZabserrors["sys"][channel]
		cssys_Lumi_WZ = xsWZabserrors["lumi"][channel]
		if hasprint:
			mainstr= "%.3f+-%.3f+-%.3f+-%.3f"
			txtout = "| Channel %s : "+mainstr+" || "+mainstr
			print txtout % (channel,csval,cserr,cssys,cssys_Lumi,csval_WZ,cserr_WZ,cssys_WZ,cssys_Lumi_WZ)
			outmessage += channelline % (channel,csval,cserr,cssys,cssys_Lumi,csval_WZ,cserr_WZ,cssys_WZ,cssys_Lumi_WZ)
		xscalcdict[channel] = {'exclusive': (csval,cserr,cssys,cssys_Lumi),
				'inclusive': (csval_WZ,cserr_WZ,cssys_WZ,cssys_Lumi_WZ) }
	
	# tex or html output
	if hasprint:
		if format == "html":
			print "\n= HTLM OUTPUT "+"="*90
			tablestr= "|  | <latex size=\"scriptsize\">\\sigma_{WZ}\\Gamma(channel)</latex> | <latex size=\"scriptsize\"> \\sigma_{WZ} </latex> |\n"
			print tablestr+outmessage[:-1]
			print "= HTML OUTPUT "+"="*90
		elif format == "tex":
			print "\n= TEX OUTPUT "+"="*90
			tablestr =  "\\begin{tabular}{l c c }\\hline\\hline\n"
			tablestr += " & $\\sigma_{WZ}\\Gamma(channel)$ & $\\sigma_{WZ}$ \\\\ \\hline\\hline\n"
			print tablestr+outmessage+"\\hline\n\\end{tabular}"
			print "= TEX OUTPUT "+"="*90

	return xscalcdict

if __name__ == '__main__':
	from optparse import OptionParser,OptionGroup
	import os
	
	#Opciones de entrada
	parser = OptionParser()
	parser.set_defaults(workingpath=os.getcwd(),zoutrange=False,format="tex")
	parser.add_option( '-w', '--workingdir', action='store', type='string', dest='workingpath',\
			help="Working directory. It must exist the usual folder structure")
	parser.add_option( '-z', '--ZrangeasinMC', action='store_true', dest='zoutrange',\
			help="Per default, the number of generated events to calculate the efficiency"\
			" is calculated using the phase space of M_Z in [60,120]. Activating this option,"\
			" the script is going to use the phase space used in the WZ Monte Carlo sample"\
			" creation.")
	parser.add_option( '-f', '--format', action='store', type='string', dest='format',\
			help="Output format, it could be 'tex' or 'html'. Per default: 'tex'")

	(opt,args) = parser.parse_args()

	if opt.format != "html" and opt.format != "tex":
		message = "\033[31;1mxscalc ERROR\033[m Format %s not recognized. See help for further info." % opt.format
		raise RuntimeError(message)

	print "\033[34mxscalc INFO\033[m Evaluating the cross-section at '%s'" % opt.workingpath
	xscalc(opt.workingpath,opt.zoutrange,opt.format)
