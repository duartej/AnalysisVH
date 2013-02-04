#!/usr/bin/env python
"""
Combine several unbiassed estimation of an observable by using the
BLUE tecnique (Best Linear Unbiased Estimate)        
"""
# -- Correspondence between index tuples,list and channel
# --- VERY IMPORTANT NOT TO CHANGE THE ORDER!
IDCHANNEL = { 0: "eee", 1: "eem", 2: "mme", 3:"mmm" }
CHANNELID = dict(map(lambda (x,y): (y,x), IDCHANNEL.iteritems()))
# -- Correspondence between matrices
IDCHANNELMATRIX = {}
idmat = 0
# Assuming return of .keys is ordered
for id in IDCHANNEL.keys():
	for idother in IDCHANNEL.keys():
		IDCHANNELMATRIX[idmat] = (IDCHANNEL[id],IDCHANNEL[idother])
		idmat += 1


def checkenv():
	"""
	"""
	import os, sys
	# realpath() with make your script run, even if you symlink it :)
	#cmd_folder = os.path.realpath(os.path.abspath(os.path.split(inspect.getfile( inspect.currentframe() ))[0]))
	#if cmd_folder not in sys.path:
	#	sys.path.insert(0, cmd_folder)

	# Trying to extract the env variables to define 
	# the path of the General package
	if os.getenv("VHSYS"):
		basedir = os.path.abspath(os.getenv("VHSYS"))
		pythondir = os.path.join(basedir,"Utils")
		if not os.path.exists( os.path.join(basedir,"CutManager") ):
				message = "\033[31;2msetenv: ERROR\033[0m Not initialized environment. Do 'source setup'"
				raise RuntimeError(message )


def getprobability(S,dof):
	""".. function:: checkchisquared(S,dof) -> p
	Knowing that S follows a chi square distribution (with dof degrees of freem), 
	return the probability that S is fulfill the hypothesis
	"""
	from ROOT import Math

	return Math.chisquared_cdf(S,dof)



def geterrorarray(xs,xserrors):
	""".. function:: geterrorarray(xs,xserrors) --> (E,Esys)
	Calculate the arrays (to fill later the matrices) of absolute errors and returns
	two objects, a dictionary with errors separated into sys, stat and lumi (plus the total of them)
	and another dictionary where all the errors are separated by its sources; this is done
	just to keep separatly this information if it is needed at the future. 
	Also, the arrays are calculated taking into account the correlations

	:param xs: dictionary of cross-section values splitted in channels
	:type xs: dict(str:float)
	:param xserrors: dictionary of cross-section relative errors splitted in sources of error 
	                 an in channels. See help in functionspool_mod.getxserrorsrel
	:type xserrors: dict(str:dict(str:float))

	:return: absolute errors, just splitted in stat,sys,lumi and total in the first element
	         of the tuple, and splitted by source of error for the second one
	:rtype: (dict(str:array(float)),dict(srt:array(float)))
	"""
	from array import array
	# ========= ERROR MATRIX INITIALIZATION =====================
	# -- Separate the error array into three contributions: sys,lumi,stat
	#    plus the total of them
	arrayE = dict([(itype,array('d',[0.0 for k in IDCHANNELMATRIX.keys()])) \
			for itype in ["sys","lumi","stat","total"]])
	# Special dictionary of arrays-matrix to separate each systematic error
	arrayEsys = dict([(itype,array('d',[0.0 for k in IDCHANNELMATRIX.keys()])) \
			                        for itype in xserrors.keys()])
	
	# Note that the asymetric errors are taken as mean
	evalsyserr = lambda sysname,lchannel,rchannel: ((xserrors[sysname][lchannel][0]*xs[lchannel])*(xserrors[sysname][rchannel][0]*xs[rchannel])+\
			(xserrors[sysname][lchannel][1]*xs[lchannel])*(xserrors[sysname][rchannel][1]*xs[rchannel]))/2.0

	# -- CORRELATIONS: DECIDING WHAT IS CORRELATED
	#    We say two systematics are fully correlated between two channels when the variation of the error in one channel 
	#    is the same as the variation of the error of the other channel
	for id,(lval,rval) in IDCHANNELMATRIX.iteritems():
		# - 1. Lepton eff. - Asumming fully correlation between channels 
		arrayEsys["LEPTON"][id] = evalsyserr("LEPTON",lval,rval)
		# - 2. Trigger eff.: fully correlated when Z->ll channels, i.e., eee with eee and eem (and viceversa)
		#                    and mmm with mmm and mme. 
		#if lval[:-1] == rval[:-1]:
		#	arrayEsys["TriggerEff"][id] = evalsyserr("TriggerEff",lval,rval)
		# - 3. Electron Energy Scale: assuming fully correlation between electronic channels (contains at least an electron)
		if lval.find('e') != -1 and rval.find('e') != -1:
			arrayEsys["EES"][id] = evalsyserr("EES",lval,rval)
		# - 4. Muon Momentum Scale: assuming fully correlation between muonic channels (contains at least an muon)
		if lval.find('m') != -1 and rval.find('m') != -1:
			arrayEsys["MMS"][id] = evalsyserr("MMS",lval,rval)
		# - 5. Pile-up reweighting technique: assuming fully correlation between all channels
		arrayEsys["PU"][id] = evalsyserr("PU",lval,rval)
		# - 6. PDF sytematic effect: assuming fully correlation between all channels
		arrayEsys["PDF"][id] = evalsyserr("PDF",lval,rval)
		# - 7. MET resolution effect: assuming fully correlation between all channels
		arrayEsys["MET"][id] = evalsyserr("MET",lval,rval)
		# - 8. Fakeable object method: assuming not correlation between channels
		if lval == rval:
			arrayEsys["DDMMC"][id] = evalsyserr("DDMMC",lval,rval)
		# - 9. Luminosity effect: assuming fully corration between all channels
		arrayEsys["Lumi"][id] = evalsyserr("Lumi",lval,rval)
		# - 10.Statistics, not correlated
		if lval == rval:
			# -- Signal
			arrayEsys["STATS"][id] = evalsyserr("STATS",lval,rval)
			# -- WZ
			arrayEsys["Stat"][id] = evalsyserr("Stat",lval,rval)
	
	# Per contributions array
	for errorname,arrayvalues in arrayEsys.iteritems():
		whereitgoes = ""
		if errorname == "STATS":
			whereitgoes="stat"
		elif errorname == "Lumi":
			whereitgoes="lumi"
		else:
			whereitgoes="sys"
		for id in xrange(len(arrayvalues)):
			arrayE[whereitgoes][id] += arrayvalues[id]

	# Finally filling the total
	for id in xrange(len(arrayvalues)):
		for kinderr in filter(lambda x: x != "total",arrayE.keys()):
			arrayE["total"][id] += arrayE[kinderr][id]

	return arrayE,arrayEsys


def bluemethod(workingpath,zoutrange,whatuse,mcprod,verbose):
	""" ..function:: bluemethod(workingpath,zoutrange,whatuse) --> 
	"""
	from array import array
	from math import sqrt
	from functionspool_mod import getxserrorsrel,BR
	import ROOT
	
	nchannels = len(IDCHANNEL.keys())
	# Get cross-section and its relative-errors
	xs,xserrors = getxserrorsrel(workingpath,xstype=whatuse,mcprod=mcprod)
	### ========== INITIALIZATIONS =============================
	### Arrays to be used to fill the matrices: arrayName  
	### --- U: link between observable and measure 
	arrayU = array('d',[1.0 for i in xrange(nchannels)])
	U = ROOT.TMatrixD(nchannels,1)
	U.SetMatrixArray(arrayU)
	U_T = ROOT.TMatrixD(1,nchannels)
	U_T.SetMatrixArray(arrayU)
	### --- E: measure covariance 
	arrayEmain,arrayEsep = geterrorarray(xs,xserrors)
	E = ROOT.TMatrixD(nchannels,nchannels)
	E.SetMatrixArray(arrayEmain["total"])
	E_inv = ROOT.TMatrixD(nchannels,nchannels)
	E_inv.SetMatrixArray(arrayEmain["total"])
	E_inv.Invert()
	### --- w: weights ==> xs_mean = Sum_{i} w_i*xs_i
	arrayW = array('d', [0.0 for i in xrange(nchannels)])
	W = ROOT.TMatrixD(nchannels,1)
	W.SetMatrixArray(arrayW)
	W_T = ROOT.TMatrixD(1,nchannels)
	### --- xs: cross-section measurements
	arrayXS= array('d', [xs[IDCHANNEL[i]] for i in xrange(nchannels)])
	XS = ROOT.TMatrixD(nchannels,1)
	XS.SetMatrixArray(arrayXS)
	XS_T = ROOT.TMatrixD(1,nchannels)
	XS_T.Transpose(XS)
	# ===== END INITIALIZATION

	# Extract the weights (using the BLUE method)
	kinv = (U_T*E_inv*U)(0,0)
	k = 1.0/kinv
	W = E_inv*U
	W *= k
	W_T.Transpose(W)

	# Get the estimated cross-section
	xsmean = (XS_T*W)(0,0)
	S = 0.0
	# Estimating the validity of the estimator
	for i in xrange(nchannels):
		for j in xrange(nchannels):
			S += (xsmean-XS(i,0))*E_inv(i,j)*(xsmean-XS(j,0))
	# variance separated by error type
	E_types = {}
	E_types_inv = {}
	for errname,arrayerr in arrayEmain.iteritems():
		E_types[errname] = ROOT.TMatrixD(nchannels,nchannels)
		E_types[errname].SetMatrixArray(arrayerr)

	variances = {}
	for errname,Ematrix in E_types.iteritems():
		variances[errname] = (W_T*Ematrix*W)(0,0)
	errors = dict(map(lambda (key,x): (key,sqrt(x)), variances.iteritems()))
		
	
	if whatuse == "inclusive":
		print "\033[33;1mbluemethod WARNING\033[m Inclusive cross-section"\
				" calculation is not well-understood. See 'bluemethod -v'"

	if verbose:
		newline = "\033[32;2mbluemethod VERBOSE\033[m "
		message = newline+"WEIGHTS:: " 
		for i,channel in IDCHANNEL.iteritems():
			message += "%s:%.4f " % (channel,W(i,0))
		# S estimator
		message += "\n"
		message += newline+"S-estimator:: %.2f, i.e., prob. of %.0f%s that our combination"\
				" is consistent with our measures" % (S,getprobability(S,nchannels-1)*100,"%")
		if whatuse == "exclusive":
			message += "\n"
			BRprompt = ((BR.W2e+BR.W2m+BR.W2tau)/3.0*BR.Z2ll)
			xswz  = xsmean/BRprompt
			syswz = errors["sys"]/BRprompt
			statwz= errors["stat"]/BRprompt
			lumiwz= errors["lumi"]/BRprompt
			totalwz = sqrt(syswz**2.+statwz**2.+lumiwz**2.)
			message += newline+"Inclusive XS using just the exclusive "\
					"values: %.3f+-%.3f(stat)+-%.3f(sys)+-%.3f(lumi)  (total=%.3f)" % \
					(xswz,statwz,syswz,lumiwz,totalwz)
		# Matrices: E y E_sys
		print message
	
	return xsmean,errors


if __name__ == '__main__':
	from optparse import OptionParser,OptionGroup
	import os
	
	#Opciones de entrada
	parser = OptionParser()
	parser.set_defaults(workingpath=os.getcwd(),zoutrange=False,xstype="exclusive",mcprod="Summer12",verbose=False)
	parser.add_option( '-w', '--workingdir', action='store', type='string', dest='workingpath',\
			help="Working directory. It must exist the usual folder structure")
	parser.add_option( '-z', '--ZrangeasinMC', action='store_true', dest='zoutrange',\
			help="Per default, the number of generated events to calculate the efficiency"\
			" is calculated using the phase space of M_Z in [60,120]. Activating this option,"\
			" the script is going to use the phase space used in the WZ Monte Carlo sample"\
			" creation.")
	parser.add_option( '-x', '--xs', action='store', dest='xstype',\
			help="The type of cross-section to be combined. Valid keywords: 'WZinclusive' 'WZexclusive'."\
			" [Default 'exclusive']")
	parser.add_option( '-m', '--mcprod', action='store', type='string', dest='mcprod',\
			help="The MC production to be used as signal. This affects the number of"\
			" generated events inside the Z mass range [71,111]. Per default: 'Summer12'")
	parser.add_option( '-v', '--verbose', action='store_true', dest='verbose',\
			help="Activing the flag more information is printed [Default: False]")
	
	(opt,args) = parser.parse_args()

	if opt.xstype != "exclusive" and opt.xstype != "inclusive":
		message= "\033[31mbluemethod ERROR\033[m Not valid argument for the '-x' option. Valid arguments:"\
				" 'WZexclusive' 'WZinclusive'"
		raise RuntimeError(message)

	print "\033[34mbluemethod INFO\033[m Combining the %s cross-section at '%s'" % (opt.xstype,opt.workingpath)
	print "\033[34mbluemethod INFO\033[m MC production to be used: %s" % opt.mcprod
	xsmean,xserrors = bluemethod(opt.workingpath,opt.zoutrange,opt.xstype,opt.mcprod,opt.verbose)

	output  = "++++ Cross section combined using BLUE method ++++\n"
	output += "xs=%.4f" % xsmean
	for errname,val in filter(lambda (x,y): x!="total",xserrors.iteritems()):
		output+= "+-%.4f(%s)" % (val,errname)
	output += "  (total:%.4f)" % xserrors["total"]

	print output

