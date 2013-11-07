#!/usr/bin/env python
"""
Evaluating W-Z/W+Z ratio given the ouput of the analysis
	:nminusup param: number of signal events for W^-Z when a systematic is variated up. 
	                The keys of the dict are the systematic type.
	:nminusup type: dict(str:float)
	:nminusdown param: number of signal events for W^-Z when a systematic is variated down. 
	                The keys of the dict are the systematic type.
	:nminusdown type: dict(str:float)
"""

# Number of W+Z and W-Z generated within the mass window Mz+-20
# JUST FOR 7 TeV!!!
NGEN = { 'PLUS': 514379.0, 'MINUS': 272812.0 }
NGENPUWEIGHTED = { 'PLUS': 551992.9, 'MINUS': 293104.9 }

def getobsyields(channelfolder,signalname,charge):
	""".. function:: getobsyields(channelfolder,signalname,charge) -> Nobs,errs,NsysUp,NsysDown,Acc,AccSysUp,AccSysDown

	Evaluate the number of estimated signal events inside a standard folder
	structure. It is calculated also the systematic propagation of the backgrounds
	Also is going to estimate the acceptance systematics in relative

	:channelfolder param: path where to do calculation. It should contain the 'cluster_*'
	                    folders inside
	:channelfolder type: str
	:signalname param: name of the signal
	:signalname type: str
	:charge param: the W charge production, valid values are PLUS|MINUS
	:charge type: str

	:return: a 6-tuple with the number of signal events, its statistical error, and
	        a couple of dictionaries with the number of signal events variated with
		the different systematic sources considered. Also it returns the acceptance
		and the systematic variations
	:rtype: tuple(float,float,dict(str:float),dict(str:float),float,dict(str:float),dict(str:float))
	"""
	import os
	import glob
	from math import sqrt
	from functionspool_mod import processedsample

	import sys
	sys.path.insert(0,os.path.abspath(channelfolder.split("/")[0]))
	import systematics_mod
	sys.path = sys.path[1:]
	
	# Extract the name of the signal
	try:
		actsignalname = os.path.basename(glob.glob(channelfolder+"/cluster_"+signalname+"*")[0]).replace("cluster_","")
	except IndexError:
		message = "\033[1;31mgetobsyields ERROR\033[1;m Some unexpected error"
		raise RuntimeError(message)
	# Get the channel
	channel = channelfolder.split("/")[-1][-3:]
	evalsys = True
	if channel not in [ 'eee', 'eem', 'mme', 'mmm' ]:
		evalsys = False

	# Name of the observed data
	dataname = "Data"
	dataps = processedsample(channelfolder+"/cluster_"+dataname+"/Results/"+dataname+".root")
	Nobs,Nerr = dataps.getvalue("MET")
	Ndata = Nobs

	# ACCEPTANCE CALCULATION 
	signalps = processedsample(channelfolder+"/cluster_"+actsignalname+"/Results/"+actsignalname+".root")
	Npass,NpassErr = signalps.getrealvalue("MET")
	#Ngen = NGEN[charge]
	Ngen = NGENPUWEIGHTED[charge]
	acc = Npass/Ngen
	# ACCEPTANCE SYSTEMATICS 
	accsys = eval("systematics_mod.SYS"+actsignalname)
	accSysUp  ={}
	accSysDown={}
	if evalsys:
		for systype,chdict in accsys.iteritems():
			Npassup = Npass*(1.0+chdict[channel])
			accSysUp[systype] = Npassup/Ngen
			Npassdown = Npass*(1.0-chdict[channel])
			accSysDown[systype] = Npassdown/Ngen
	
	# Systematic propagation for Background samples 
	samplesname = filter(lambda x: x != actsignalname and x != dataname, \
			map(lambda x: os.path.basename(x).replace("cluster_",""),glob.glob(channelfolder+"/cluster_"+"*")) )
	
	syssamples = {}
	for sample in samplesname:
		try:
			syssamples[sample] = eval("systematics_mod.SYS"+sample)
		except AttributeError:
			# Assuming ZZ: Just a patch because ZZ is split
			syssamples[sample] = eval("systematics_mod.SYSZZ")
	# plus Fakes, modifying dict to the standard notation
	syssamples["Fakes"] = { "DDMMC": systematics_mod.DDMMC, "Fakes": systematics_mod.SYSFakes }
	
	# Building the full list of systematics types
	allsysset = set()
	dummy = map(lambda x: [allsysset.add(k) for k in x.keys()], syssamples.values())
	allsys = list(allsysset)
	
	Nerr2 = Nerr**2.0
	TotalBkgSysUp = dict([(systype,0.0) for systype in allsys])
	TotalBkgSysDown = dict([(systype,0.0) for systype in allsys])
	# Creating the instances of process
	for sample in samplesname:
		rootfilename = channelfolder+"/cluster_"+sample+"/Results/"+sample+".root"
		evts = processedsample(rootfilename)
		val,err = evts.getvalue("MET")
		Nobs -= val
		Nerr2 += err**2.
		if not evalsys:
			del evts
			continue
		#for systype in syssamples[sample].keys():
		for systype in allsys:
			try:
				sys = syssamples[sample][systype][channel] 
			except KeyError:
				sys = 0.0
			# Total Background with all the background samples variated
			# to the same direction (UP/DOWN) for the same systematic
			TotalBkgSysUp[systype] += val*(1+sys)
			TotalBkgSysDown[systype] += val*(1-sys)
			# Adding "Fakes" to stat error---> FIXME???
			if sample == "Fakes" and systype == "Fakes":
				Nerr2 += (val*sys)**2.0
		del evts
	del dataps
	# Get in Number of observed events with the bkg variated with systematics
	NsysUp   = dict( map(lambda (systype,nTotbkg): (systype,Ndata-nTotbkg),TotalBkgSysUp.iteritems()) )
	NsysDown = dict( map(lambda (systype,nTotbkg): (systype,Ndata-nTotbkg),TotalBkgSysDown.iteritems()) )

	return Nobs,sqrt(Nerr2),NsysUp,NsysDown,acc,accSysUp,accSysDown


def getyieldsdict(channeldict,signalname,charge):
	""".. function:: getyieldsdict(channeldict,signalname,charge) -> chyielddict
	
	Wrapper function to getobsyields (see help(getobsyields). Probably
	to be deprecated
	"""
	import os
	if charge != 'PLUS' and charge != 'MINUS':
                message = '\033[1;31getwzratio ERROR\033[1;m Invalid charge (%s)! ' % charge
		message += 'Valid values are PLUS|MINUS'
		raise RuntimeError(message)
	channelyieldsdict = {}
	for channelfolder in sorted(channeldict):
		channel = os.path.basename(channelfolder)
		channelyieldsdict[channel] = getobsyields(channelfolder,signalname,charge)

	return channelyieldsdict

def getsysdict(nminus,nminusup,nminusdown,nplus,nplusup,nplusdown,accstuff):
	""".. function:: getsysdict(wzratio,nminusup,nminusdown,nplusup,nplusdown) -> sysdict
	
	Build the systematics errors for the ratio

	:wzratio param: ratio value
	:wzratio type: float
	:nminusup param: number of signal events for W^-Z when a systematic is variated up. 
	                The keys of the dict are the systematic type.
	:nminusup type: dict(str:float)
	:nminusdown param: number of signal events for W^-Z when a systematic is variated down. 
	                The keys of the dict are the systematic type.
	:nminusdown type: dict(str:float)
	:nplusup param: number of signal events for W^+Z when a systematic is variated up. 
	                The keys of the dict are the systematic type.
	:nplusup type: dict(str:float)
	:nplusdown param: number of signal events for W^+Z when a systematic is variated down. 
	                The keys of the dict are the systematic type.
	:nplusdown type: dict(str:float)

	:return: The relative systematic errors split by systematic source
	:rtype: dict(str:float)
	"""
	#Acceptance
	accminus   = accstuff['minus']
	accminusup = accstuff['minusup']
	accminusdown=accstuff['minusdown']

	accplus   = accstuff['plus']
	accplusup = accstuff['plusup']
	accplusdown=accstuff['plusdown']

	sysdict = {}
	# Nominal ratio
	wzratio = (nminus/accminus)/(nplus/accplus)
	# Adding in quadrature the sys, the filtered systematic
	# are compensated by the ratio or used in the STAT dict
	syscompensated = [ "Lumi", "Fakes", "Stat" ]
	for systype in filter(lambda x: x not in syscompensated,nminusup.keys()):
		# up
		nsysup = nminusup[systype]/nplusup[systype]
		try:
			accsysup = accminusup[systype]/accplusup[systype]
		except KeyError:
			accsysup = accminus/accplus
		# down
		nsysdown = nminusdown[systype]/nplusdown[systype]
		try:
			accsysdown=accminusdown[systype]/accplusdown[systype]
		except KeyError:
			accsysdown = accminus/accplus
		#ratios
		rup = nsysup/accsysup
		rdown=nsysdown/accsysdown

		# Relative difference with respect the wz ratio, take the maximum
		sysdict[systype] = max(abs(rup-wzratio)/wzratio,abs(wzratio-rdown)/wzratio)
	
	# in order to build the systematic dict properly, filling with 0 the compensated sys
	for systype in syscompensated:
		sysdict[systype] = 0.0
	
	return sysdict

def storeratioerrors(ratiodict,errorsdict):
	""".. function:: storeratioerrors(ratiodict,errorsdict)

	Using the dict arguments which should contain the ratio and
	errors associated in the measured channels, stores a shelve
	file which is going to be used by the bluemethod script to
	combine the measurements. The ratio and errors store are
	converted to the standard input format needed by the bluemethod
	script

	:ratiodict param: w^-z/w^+z measurements split by channels
	:ratiodict type: dict(str:float)
	:errorsdict param: ratio stat and systematic errors
	:errorsdict type: dict(str: dict(str:float))
	"""
	import shelve
	from math import sqrt

	# -- Reorganizing the dict to be in the format needed by the bluemethod script
	#    { SYSTYPE: { CHANNEL: (valUP,valDOWN), ... } ,... }
	channelsequiv = dict(map(lambda x: (x[-3:],x),filter(lambda x: x != "leptonchannel", errorsdict.keys())))
	channels = channelsequiv.keys()
	systypesset = set()
	dummy = map(lambda x: [systypesset.add(k) for k in x.keys()], errorsdict.values())
	systypes = list(systypesset)

	# dict to store
	errwzratio = {}
	for sys in systypes:
		errwzratio[sys] = {}
		for ch in channels:
			val = errorsdict[channelsequiv[ch]][sys]
			errwzratio[sys][ch] = (val,val)
	# Using the 'EEM' and 'MMS' keys to evaluate the efficiency ratio e-/e+ systematic
	# Note we are using this sys because of the correlation matrix defined in the bluemethod
	# script
	syseffratio = { 'e': 0.03, 'm': 0.023 }
	p = lambda ch: reduce(lambda x,y: x*y,map(lambda x: syseffratio[x],ch))
	for ch in channels:
		newval = sqrt(errwzratio["EES"][ch][0]**2.+p(ch)**2)
		errwzratio["EES"][ch] = (newval,newval)
		newval2 = sqrt(errwzratio["MMS"][ch][0]**2.+p(ch)**2)
		errwzratio["MMS"][ch] = (newval2,newval2)
	
	# ratio
	wzratio = {}
	for ch in channels:
		wzratio[ch] = ratiodict[channelsequiv[ch]]
	
	d = shelve.open('.errorswz',writeback=True)
	d['errwzratio'] = errwzratio
	d['wzratio'] = wzratio
	d.close()


if __name__ == '__main__':
	import os,sys
	import glob
	from math import sqrt
	import shutil

	from optparse import OptionParser
        #Comprobando la version (minimo 2.4)
        vX,vY,vZ,_t,_t1 = sys.version_info
        if vX > 2 and vY < 4:
                message = '\033[1;31getwzratio ERROR\033[1;m I need python version >= 2.4'
                sys.exit( message )
        elif vX < 2:
                message = '\033[1;31getwzratio ERROR\033[1;m I need python version >= 2.4'
                sys.exit( message )
        
        parser = OptionParser()
        parser.set_defaults(signalname="WZ")
        parser.add_option( '-p', '--parentfolder', action='store', metavar="PLUS_PATH,MINUS_PATH",\
			dest='parentfolder', help='Paths'\
                        ' where to find the standard folder structure. It must be two paths, comma'\
                        ' separated, one for the positive and the other for the negative')
	parser.add_option( '-s', '--signalname', dest='signalname',help='Name of the signal [Default: WZ]')

        ( opt, args ) = parser.parse_args()

	if not opt.parentfolder:
		wzpluspath = os.path.join(os.getcwd(),"WPositive")
		wzminuspath = os.path.join(os.getcwd(),"WNegative")
	else:
		pl = opt.parentfolder.split(",")
		wzpluspath = pl[0]
		wzminuspath = pl[1]

	
	# Extract the channel folders
	channelfoldersplus = filter(lambda x: os.path.isdir(x) and x.find("SYSTEMATICS") == -1,\
			glob.glob(os.path.join(wzpluspath,"*")))
	channelfoldersminus = filter(lambda x: os.path.isdir(x) and x.find("SYSTEMATICS") == -1,\
			glob.glob(os.path.join(wzminuspath,"*")))

	channelyields = {}
	# Evaluate the observed yields (per channel)
	channelyields["WPositive"] = getyieldsdict(channelfoldersplus,opt.signalname,'PLUS') 
	channelyields["WNegative"] = getyieldsdict(channelfoldersminus,opt.signalname,'MINUS') 

	# Evaluate the ratio per channel
	channellist = sorted(channelyields.values()[0].keys())

	print "+++ W-Z/W+Z +++"
	syswzratio ={}
	ratio = {}
	# FIXME: Included this hardcoded values in some other way (a file maybe) 
	#acceptance= { 'WZeee': 0.944692387, 'WZeem': 0.947401383, 
	#		'WZmme': 0.936422384, 'WZmmm': 0.932961416 }
	# Acceptance correction factor for the WZ->3lnu MC sample: 
	# The madgraph WZ->3lnu sample is obtaining a ratio 0.5304 in the mass window defined
	# but the calculation done at NLO with MCFM is getting a 0.5631 at the same window,
	# so the acceptance calculated with the Madgraph sample is biased due this difference
	# We appply a correction factor to  the acceptance
	#ratiomadgraph=0.5303715742672231
	#ratiomcfm    =0.5631639013256079
	#fcorrA = ratiomcfm/ratiomadgraph
	#print '\033[1;33getwzratio WARNING\033[1;m Using hardcoded for Fall11 acceptances!!'\
	#		' You know what you are doing...'
	# END FIXME
	for channel in filter(lambda x: x != 'leptonchannel',channellist):
		wzplus,errplus,nsysplusup,nsysplusdown,Pacc,PaccSysUp,PaccSysDown  = channelyields["WPositive"][channel]
		wzminus,errminus,nsysminusup,nsysminusdown,Macc,MaccSysUp,MaccSysDown = channelyields["WNegative"][channel]
		wzratio = (wzminus/Macc)/(wzplus/Pacc)
		ratio[channel] = wzratio
		accstuff = { 'minus': Macc, 'minusup': MaccSysUp, 'minusdown': MaccSysDown,\
				'plus': Pacc, 'plusup': PaccSysUp, 'plusdown': PaccSysDown }
		syswzratio[channel] = getsysdict(wzminus,nsysminusup,nsysminusdown,wzplus,nsysplusup,nsysplusdown,accstuff)
		totalsys = wzratio*sqrt(reduce(lambda x,y: x+y,map(lambda valsys: valsys**2.0,syswzratio[channel].values())))
		# Missing acceptance systematic
		err = wzratio*sqrt((errminus/wzminus)**2.+(errplus/wzplus)**2.)
		err2 = sqrt((errminus-errplus)**2.0/(1-errplus**2.)**2.0)
		wzinvratio = 1.0/wzratio
		errinv = err/wzratio**2.
		totalsysinv = totalsys/wzratio**2.
		print "%s channel w-z/w+z=%.4f+-%.4f(stat)+-%.4f(sys)" % (channel,wzratio,err,totalsys)
		print "%s channel w+z/w-z=%.4f+-%.4f(stat)+-%.4f(sys)" % (channel,wzinvratio,errinv,totalsysinv)
		# Adding the STATS (relative error) key to the systematic dict in order to be stored
		syswzratio[channel]["STATS"] = err/wzratio
	# Store the full errors (systematics and statistical) to be used by the bluemethod script
	storeratioerrors(ratio,syswzratio)


