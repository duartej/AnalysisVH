#!/usr/bin/env python
"""
Evaluating the cross section given the ouput of the analysis
"""
def adduperrors(xserror,xs,verbose=False):
	"""
	"""
	from math import sqrt

	xsabserror = { "sys": {}, "stat": {}, "lumi": {}, "total": {} }
	channels = xserror.values()[0].keys()
	
	# Note that the asymetric errors are taken as mean
	evalsyserr = lambda errtupleval,ch: ((errtupleval[0]+errtupleval[1])/2.0*xs[ch])**2.0
	evalrelsyserr = lambda errtupleval: ((errtupleval[0]+errtupleval[1])/2.0)**2.0

	# -- Initializing
	for channel in channels:
		for errortype in xsabserror.keys(): 
			xsabserror[errortype][channel] = 0.0
	# And filling
	for errorname,channeldict in xserror.iteritems():
		for channel, errval in channeldict.iteritems():
			if errorname == "STATS" or errorname == "Fakes":
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
	if verbose:
		print "\033[1;34mxscalc INFO\033[1;m ERRORS splitted by source:"
		message = "\033[1;34mxscalc INFO\033[1;m Cross-section absolute values\n"
		message2 = "\033[1;34mxscalc INFO\033[1;m Relative values\n"
		for errorname, channeldict in xserror.iteritems():
			mess = "+++ '%s' SOURCE" % errorname
			if errorname == "Fakes":
				mess += " (Embebbed in STATS)"
			mess += "\n  |"
			messabs = ""
			messrel = ""
			for channel, errval in channeldict.iteritems():
				messabs += " %s: %0.4e " % (channel,sqrt(evalsyserr(errval,channel)))
				messrel += " %s: %0.1f%s " % (channel,sqrt(evalrelsyserr(errval))*100.0,"%")
			message += mess+messabs+"\n"
			message2 += mess+messrel+"\n"
		print message
		print message2
		print "\n"
	
	return xsabserror

def xscalc(path,zoutrange,format,mcprod,lumi,sysinfolder,verbose):
	"""
	"""
	from functionspool_mod import getxserrorsrel
	import os

	# Check we have the expected folder structure
	if not os.path.isdir(os.path.join(path,"leptonchannel")):
		message =  "\033[31mxscalc ERROR\033[m The '%s' path has not the expected folder structure"
		message += " (leptonchannel SIGNALeee SIGNALeem SIGNALmme SIGNALmmm)"
		raise RuntimeError(message)
	
	xsWZ,xsWZrelerrors = getxserrorsrel(path,xstype="inclusive",mcprod=mcprod,lumi=lumi,sysinfolder=sysinfolder)
	xs,xsrelerrors = getxserrorsrel(path,xstype="exclusive",mcprod=mcprod,lumi=lumi,sysinfolder=sysinfolder)
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

	xsabserrors = adduperrors(xsrelerrors,xs,verbose)
	xsWZabserrors = adduperrors(xsWZrelerrors,xsWZ,verbose)
	if hasprint:
		print "Legend: value+-(stat)+-(sys_up,sys_down)+-(lumi)"
		print "Cross-section for WZ*BR(channel) || Cross-section for WZ "
	outmessage = ""
	xscalcdict = {}
	# Ordering the channel
	sortedchannels = sorted(xs.keys(),key=lambda x: (x[1],x[-1]))
	for channel in sortedchannels:
		csval = xs[channel]
		cserr = xsabserrors["stat"][channel]
		cssys = xsabserrors["sys"][channel]
		cssys_Lumi = xsabserrors["lumi"][channel]
		csval_WZ = xsWZ[channel] 
		cserr_WZ = xsWZabserrors["stat"][channel]
		cssys_WZ = xsWZabserrors["sys"][channel]
		cssys_Lumi_WZ = xsWZabserrors["lumi"][channel]
		if hasprint:
			mainstr= "%.4f+-%.4f+-%.4f+-%.4f"
			txtout = "| Channel %s : "+mainstr+" || "+mainstr
			print txtout % (channel,csval,cserr,cssys,cssys_Lumi,csval_WZ,cserr_WZ,cssys_WZ,cssys_Lumi_WZ)
			outmessage += channelline % (channel,csval,cserr,cssys,cssys_Lumi,csval_WZ,cserr_WZ,cssys_WZ,cssys_Lumi_WZ)
		xscalcdict[channel] = {'exclusive': (csval,cserr,cssys,cssys_Lumi),
				'inclusive': (csval_WZ,cserr_WZ,cssys_WZ,cssys_Lumi_WZ) }
	
	# tex or html output
	if hasprint:
		if format == "html":
			print "\n= HTLM OUTPUT "+"="*90
			tablestr= "|  | <latex size=\"scriptsize\">\\sigma_{WZ\\rightarrow3\\ell\\nu}\\;(pb^{-1})</latex> | <latex size=\"scriptsize\"> \\sigma_{WZ}\\;(pb_{-1}) </latex> |\n"
			print tablestr+outmessage[:-1]
			print "= HTML OUTPUT "+"="*90
		elif format == "tex":
			print "\n= TEX OUTPUT "+"="*90
			tablestr =  "\\begin{tabular}{l c c }\\hline\\hline\n"
			tablestr += " & $\\sigma_{WZ\\rightarrow3\\ell\\nu}\\;(pb^{-1})$ & $\\sigma_{WZ}$ \\\\ \\hline\\hline\n"
			print tablestr+outmessage+"\\hline\n\\end{tabular}"
			print "= TEX OUTPUT "+"="*90

	return xscalcdict

if __name__ == '__main__':
	from optparse import OptionParser,OptionGroup
	import os
	
	#Opciones de entrada
	parser = OptionParser()
	parser.set_defaults(workingpath=os.getcwd(),zoutrange=False,format="tex",mcprod="Summer12",\
			sysnotcalculated=False,verbose=False)
	parser.add_option( '-w', '--workingdir', action='store', type='string', dest='workingpath',\
			help="Working directory. It must exist the usual folder structure")
	parser.add_option( '-z', '--ZrangeasinMC', action='store_true', dest='zoutrange',\
			help="Per default, the number of generated events to calculate the efficiency"\
			" is calculated using the phase space of M_Z in [71,111]. Activating this option,"\
			" the script is going to use the phase space used in the WZ Monte Carlo sample"\
			" creation. See '-m' to decide what MC-production has to be used")
	parser.add_option( '-m', '--mcprod', action='store', type='string', dest='mcprod',\
			help="The MC production to be used as signal. This affects the number of"\
			" generated events inside the Z mass range [71,111]. Per default: 'Summer12'")
	parser.add_option( '-l', '--lumi', action='store', type='string', dest='lumi',\
			help="Luminosity to be used [in pb-1]. Per defaults it is used '4922.0' if -m Fall11"\
			" and '12103.3' if -m Summer12")
	parser.add_option( '-n', '--nsys', action='store_true', dest='sysnotcalculated',\
			help="Flag indicating that the systematics were NOT calculated (with 'resumesys' utility)."\
			" Activating this option you are avoiding the use of the module created by 'resumesys'"\
			" and instead you will use a dummy systematic module containing zeros.")
	parser.add_option( '-f', '--format', action='store', type='string', dest='format',\
			help="Output format, it could be 'tex' or 'html'. Per default: 'tex'")
	parser.add_option( '-v', '--verbpse', action='store_true', dest='verbose',\
			help="Activate verbosity, show the relative and absolute errors splitted by source'")

	(opt,args) = parser.parse_args()

	if opt.format != "html" and opt.format != "tex":
		message = "\033[31;1mxscalc ERROR\033[m Format %s not recognized. See help for further info." % opt.format
		raise RuntimeError(message)

	print "\033[34mxscalc INFO\033[m Evaluating the cross-section at '%s'" % opt.workingpath
	print "\033[34mxscalc INFO\033[m MCProduction to be used: '%s'" % opt.mcprod
	if not opt.lumi:
		if opt.mcprod == "Fall11":
			opt.lumi = "4922.0"
		elif opt.mcprod == "Summer12":
			opt.lumi = "12103.3"
		else:
			message = "\033[31;1mxscalc ERROR\033[m MC production not supported."\
					" Valid values are 'Fall11' 'Summer12'" % opt.format
			raise RuntimeError(message)
	print "\033[34mxscalc INFO\033[m Luminosity to be used: %s" % opt.lumi
	try:
		lumi = float(opt.lumi)
	except ValueError:
		message = "\033[31;1mxscalc ERROR\033[m Option '-l' only accepts numbers!" % opt.format
		raise RuntimeError(message)
	# Was the user calculated the systematics? I.e. is there a systematics_mod.py file within the
	# working directory?
	sysinfolder = not opt.sysnotcalculated

	xscalc(opt.workingpath,opt.zoutrange,opt.format,opt.mcprod,lumi,sysinfolder,opt.verbose)
