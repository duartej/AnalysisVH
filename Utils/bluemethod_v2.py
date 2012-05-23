#!/usr/bin/env python
"""
Combine several unbiassed estimation of an observable by using the
BLUE tecnique (Best Linear Unbiased Estimate)        
"""
# -- Correspondence between index tuples,list and channel
IDCHANNEL = { 0: "eee", 1: "eem", 2: "mme", 3:"mmm" }
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


def geterrorarray(xs):
	"""
	"""
	from array import array
	# ========= ERROR MATRIX INITIALIZATION =====================
	# -- Separate the error array into three contributions: sys,lumi,stat
	#    plus the total of them
	arrayE = dict([(itype,array('d',[0.0 for k in IDCHANNELMATRIX.keys()])) \
			for itype in ["sys","lumi","stat","total"]])
	# Special dictionary of arrays-matrix to separate each systematic error
	arrayEsys = dict([(itype,array('d',[0.0 for k in IDCHANNELMATRIX.keys()])) \
			                        for itype in SYS.keys()])

	evalsyserr = lambda sysname,lchannel,rchannel: (SYS[sysname][lchannel]*xs[lchannel])*(SYS[sysname][rchannel]*xs[rchannel])
	# -- CORRELATIONS.
	#    We say two systematics are fully correlated between two channels when the variation of the error in one channel 
	#    is the same as the variation of the error of the other channel
	for id,(lval,rval) in IDCHANNELMATRIX.iteritems():
		# - 1. Lepton eff. - Asumming fully correlation between channels 
		arrayEsys["LeptonEff"][id] = evalsyserr("LeptonEff",lval,rval)
		# - 2. Trigger eff.: fully correlated when Z->ll channels, i.e., eee with eee and eem (and viceversa)
		#                    and mmm with mmm and mme. 
		if lval[:-1] == rval[:-1]:
			arrayEsys["TriggerEff"][id] = evalsyserr("TriggerEff",lval,rval)
		# - 3. Electron Energy Scale: assuming fully correlation between electronic channels (contains at least an electron)
		if lval.find('e') != -1 and rval.find('e') != -1:
			arrayEsys["ElecEnergyScale"][id] = evalsyserr("ElecEnergyScale",lval,rval)
		# - 4. Muon Momentum Scale: assuming fully correlation between muonic channels (contains at least an muon)
		if lval.find('m') != -1 and rval.find('m') != -1:
			arrayEsys["MuonMomentumScale"][id] = evalsyserr("MuonMomentumScale",lval,rval)
		# - 5. Pile-up reweighting technique: assuming fully correlation between all channels
		arrayEsys["PILEUP"][id] = evalsyserr("PILEUP",lval,rval)
		# - 6. PDF sytematic effect: assuming fully correlation between all channels
		arrayEsys["PDF"][id] = evalsyserr("PDF",lval,rval)
		# - 7. MET resolution effect: assuming fully correlation between all channels
		arrayEsys["METres"][id] = evalsyserr("METres",lval,rval)
		# - 8. Fakeable object method: assuming not correlation between channels
		if lval == rval:
			arrayEsys["DDMMC"][id] = evalsyserr("DDMMC",lval,rval)
		# - 9. Luminosity effect: assuming fully corration between all channels
		arrayEsys["Lumi"][id] = evalsyserr("Lumi",lval,rval)
		# - 10.Statistics, not correlated
		if lval == rval:
			arrayEsys["Stat"][id] = evalsyserr("Stat",lval,rval)







def bluemethod_V2(workingpath,zoutrange,whatuse):
	"""
	"""
	from xscalc import xscalc 
	from array import array
	from math import sqrt
	from functionspool import getxserrorsrel
	import ROOT
	
	nchannels = len(IDCHANNEL.keys())
	getxserrorsrel(workingpath)
	# Need the xs 
	#xsdict = xscalc(workingpath,zoutrange,None)
	#xsdictredux = dict(map(lambda (channel,xsd): (channel,xsd[whatuse]),filter(lambda (channel,xsd): channel != "leptonchannel",xsdict.iteritems())))
	#xs = dict(map(lambda (channel,xsd): (channel,xsd[0]),xsdictredux.iteritems()))
	## ========== INITIALIZATIONS ================================
	## Arrays to be used to fill the matrices: arrayName  
	## --- U: link between observable and measure 
	#arrayU = array('d',[1.0 for i in xrange(nchannels)])
	## --- E: measure covariance 
	#arrayE = geterrorarray(xs)
	## --- w: weights ==> xs_mean = Sum_{i} w_i*xs_i
	#arrayW = array('d', [0.0 for i in xrange(nchannels)])
	## --- xs: cross-section measurements
	#arrayXS= array('d', [0.0 for i in xrange(nchannels)])








def bluemethod(workingpath,zoutrange,whatuse):
	"""
	"""
	from xscalc import xscalc 
	from array import array
	from math import sqrt
	import ROOT
	# Set-up systematics
	corrsys = [ "TriggerEff", "LeptonEff", "PILEUP", "PDF" ]
	muoncorrsys = corrsys+["MuonMomentumScale"]
	eleccorrsys = corrsys+["ElecMomentumScale"]

	howchannels = len(CORRLIST)
	
	# Extract cross-section and errors
	xsdict = xscalc(workingpath,zoutrange,None)
	# -- Using the inclusive or exclusive?
	#whatuse = "WZinclusive"
	xs = dict(map(lambda (channel,xsd): (channel,xsd[whatuse]),filter(lambda (channel,xsd): channel != "leptonchannel",xsdict.iteritems())))
	# xs = { 'channel': (value,stat,sys_up,sys_down,lumi), ... }
	
	# Getting the indexs of the elements of the diagonal
	iddiagonal = filter( lambda x: (x % (len(CORRLIST)+1)) == 0, CORRMATRIX.keys())
	
	weights = dict(map(lambda channel: (channel,0.0),xs.keys()))
	# -- Loop for minimisation
	# NOTA: si he d'utilitzar la seccio eficaz nova, llavors haig de convertir els valors trobats dels errors de valors 
	#       absoluts a valors relatius (dividint per la xs corresponent) i tornant a convertir-los a valors absoluts fent
	#       servir la nova xs  ----> 
	combined_meas_old = 1
	combined_meas_this= 200
	while( abs(combined_meas_this-combined_meas_old) > 0.01 ):
		print "****************** NEW Iteration **********************"
		# Initialize the content of the errors
		arraysys = {}
		for err in [ "stat", "sys", "lumi"]:
			arraysys[err] = array('d',[ 0.0 for i in xrange(len(CORRMATRIX)) ])
		for id,(lval,rval) in CORRMATRIX.iteritems():
			# Systematics correlated??
			arraysys["sys"][id] = xs[lval][2]*xs[rval][2]
			# Not correlated
			if id in iddiagonal:
				arraysys["stat"][id] = xs[lval][1]*xs[rval][1]
				arraysys["lumi"][id] = xs[lval][4]*xs[rval][4]
		matrixerr = {}
		matrixerr_inv = {}
		for err in [ "stat", "sys", "lumi" ]: 
			matrixerr[err] = ROOT.TMatrixD(howchannels,howchannels)
			matrixerr[err].SetMatrixArray(arraysys[err])
			matrixerr_inv[err] = ROOT.TMatrixD(howchannels,howchannels)
			matrixerr_inv[err].SetMatrixArray(arraysys[err])
		matrixerr["total"] = matrixerr["stat"]+matrixerr["sys"]+matrixerr["lumi"]
		matrixerr_inv["total"] = matrixerr_inv["stat"]+matrixerr_inv["sys"]+matrixerr_inv["lumi"]
		matrixerr_inv["total"].Invert()
		# Up to here-- all the matrices built
		U = array('d',[ 1.0 for i in xrange(howchannels) ] )
		Unit = ROOT.TMatrixD(len(U),1)
		Unit.SetMatrixArray(U)
		Unit_T = ROOT.TMatrixD(1,len(U))
		Unit_T.SetMatrixArray(U)
		# Build the weights
		# -- Note R is a number
		R = Unit_T*matrixerr_inv["total"]*Unit
		IR = 1.0/R(0,0)
		weightMatrix = matrixerr_inv["total"]*Unit
		weightMatrix *= IR
		# And the variance, i.e. sigma**2
		weightMatrix_T = ROOT.TMatrixD(weightMatrix.GetNcols(),weightMatrix.GetNrows())
		weightMatrix_T.Transpose(weightMatrix)
		variance = weightMatrix_T*matrixerr["total"]*weightMatrix
		# -- Final value
		xscombined = 0.0
		for id,channel in CORRLIST.iteritems():
			xscombined += weightMatrix(id,0)*xs[channel][0]

		# Extract the total error (separated by stat, sys and lumi)
		#MXSec = ROOT.TMatrixD(howchannels,1)
		denom = 1.0/variance(0,0)
		# Total errors
		total = { "sys": 0.0, "stat": 0.0, "lumi":0.0 }
		for i in xrange(howchannels):
			for m in xrange(howchannels):
				for n in xrange(howchannels):
					for j in xrange(howchannels):
						for errtype in total.iterkeys():
							total[errtype] += matrixerr_inv["total"](i,m)*matrixerr[errtype](m,n)*matrixerr_inv["total"](n,j)

		errors = dict( map(lambda (errtype,val): (errtype,sqrt(val)/denom), total.iteritems()) )
		weightmessage = "+++ Weights:  "
		for id,channel in CORRLIST.iteritems():
			weightmessage += ("%s: %.4f  " % (channel,weightMatrix(id,0)))
		print weightmessage
		print "+++ xs_{%s}=%.4f+-%.4f(stat)+-%.4f(sys)+-%.4f(lumi)   total_err=%.4f" % (whatuse,xscombined,errors["stat"],errors["sys"],errors["lumi"],sqrt(variance(0,0)))
		combined_meas_old = combined_meas_this
		combined_meas_this= xscombined
		#print combined_meas_old,combined_meas_this
		


if __name__ == '__main__':
	from optparse import OptionParser,OptionGroup
	import os
	
	#Opciones de entrada
	parser = OptionParser()
	parser.set_defaults(workingpath=os.getcwd(),zoutrange=False,xstype="WZexclusive")
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
	
	(opt,args) = parser.parse_args()

	if opt.xstype != "WZexclusive" and opt.xstype != "WZinclusive":
		message= "\033[31mbluemethod ERROR\033[m Not valid argument for the '-x' option. Valid arguments:"\
				" 'WZexclusive' 'WZinclusive'"
		raise RuntimeError(message)

	print "\033[34mbluemethod INFO\033[m Combining the %s cross-section at '%s'" % (opt.xstype,opt.workingpath)
	bluemethod_V2(opt.workingpath,opt.zoutrange,opt.xstype)

