#!/usr/bin/env python
"""
Combine several unbiassed estimation of an observable by using the
BLUE tecnique (Best Linear Unbiased Estimate)        
"""
#-- Global systematics: In relative error
# --- systematics for fake rate method
DDMMC = { "mmm": 0.31, "mme": 0.46, "eem": 0.42, "eee": 0.14, "leptonchannel": 0.35 }
# -- FIXME: Extract the statistical SAMPLESTAT error for WZ and ZZ from the files
SYS = { "Lumi": 0.022, " TriggerEff": 0.015, "LeptonEff": 0.020, "MuonMomentumScale": 0.015,
		"ElecEnergyScale": 0.025, "METres": 0.020, "DDMMC": DDMMC, "PILEUP": 0.020,
		"PDF": 0.014 
	}

# -- Correspondence between index tuples,list and channel
CORRLIST = { 0: "eee", 1: "eem", 2: "mme", 3:"mmm" }
# -- Corrsepondece between matrices
CORRMATRIX = {}
idmat = 0
# Assuming return of .keys is ordered
for id in CORRLIST.keys():
	for idother in CORRLIST.keys():
		CORRMATRIX[idmat] = (CORRLIST[id],CORRLIST[idother])
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


def extractyields(asciirawtable,titlerow):
	"""..function::extractyields(asciirawtable,titlerow) --> (n,nerrorstat)
	
	From a table in .tex format (the reduced one of the standard output), the yields of the
	'titlerow' row will be extracted

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
	Nerrstat = float(line.split("&")[-1].split("$\\pm$")[1].strip())

	return (N,Nerrstat)


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
	bluemethod(opt.workingpath,opt.zoutrange,opt.xstype)

