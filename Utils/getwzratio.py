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
def getobsyields(channelfolder,signalname):
	""".. function:: getobsyields(channelfolder,signalname) -> Nobs,errs,NsysUp,NsysDown

	Evaluate the number of estimated signal events inside a standard folder
	structure. It is calculated also the systematic propagation of the backgrounds

	:channelfolder param: path where to do calculation. It should contain the 'cluster_*'
	                    folders inside
	:channelfolder type: str
	:signalname param: name of the signal
	:signalname type: str

	:return: a 4-tuple with the number of signal events, its statistical error, and
	        a couple of dictionaries with the number of signal events variated with
		the different systematic sources considered
	:rtype: tuple(float,float,dict(str:float),dict(str:float))
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
		message = "\033[1;31mgetobsyields ERROR\033[1;m Some unexpeted error"
		raise RuntimeError(message)
	# Name of the observed data
	dataname = "Data"
	dataps = processedsample(channelfolder+"/cluster_"+dataname+"/Results/"+dataname+".root")
	Nobs,Nerr = dataps.getvalue("MET")
	Ndata = Nobs
	
	samplesname = filter(lambda x: x != actsignalname and x != dataname, \
			map(lambda x: os.path.basename(x).replace("cluster_",""),glob.glob(channelfolder+"/cluster_"+"*")) )
	
	# Systematic propagation for Background samples
	channel = channelfolder.split("/")[-1][-3:]
	evalsys = True
	if channel not in [ 'eee', 'eem', 'mme', 'mmm' ]:
		evalsys = False
	syssamples = {}
	for sample in samplesname:
		try:
			syssamples[sample] = eval("systematics_mod.SYS"+sample)
		except AttributeError:
			# Asumo que es el ZZ
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

	return Nobs,sqrt(Nerr2),NsysUp,NsysDown


def getyieldsdict(channeldict,signalname):
	""".. function:: getyieldsdict(channeldict,signalname) -> chyielddict
	
	Wrapper function to getobsyields (see help(getobsyields). Probably
	to be deprecated
	"""
	import os
	channelyieldsdict = {}
	for channelfolder in sorted(channeldict):
		channel = os.path.basename(channelfolder)
		channelyieldsdict[channel] = getobsyields(channelfolder,signalname)

	return channelyieldsdict

def getsysdict(wzratio,nminusup,nminusdown,nplusup,nplusdown):
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
	sysdict = {}
	# Adding in quadrature the sys, the filtered systematic
	# are compensated by the ratio or used in the STAT dict
	syscompensated = [ "Lumi", "Fakes", "Stat" ]
	for systype in filter(lambda x: x not in syscompensated,nminusup.keys()):
		# up
		nsysup = nminusup[systype]/nplusup[systype]
		# down
		nsysdown = nminusdown[systype]/nplusdown[systype]
		# Relative difference with respect the wz ratio, take the maximum
		sysdict[systype] = max(abs(nsysup-wzratio)/wzratio,abs(wzratio-nsysdown)/wzratio)
	
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
	channelfoldersplus = filter(lambda x: os.path.isdir(x), glob.glob(os.path.join(wzpluspath,"*")))
	channelfoldersminus = filter(lambda x: os.path.isdir(x), glob.glob(os.path.join(wzminuspath,"*")))

	channelyields = {}
	# Evaluate the observed yields (per channel)
	channelyields["WPositive"] = getyieldsdict(channelfoldersplus,opt.signalname) 
	channelyields["WNegative"] = getyieldsdict(channelfoldersminus,opt.signalname) 

	# Evaluate the ratio per channel
	channellist = sorted(channelyields.values()[0].keys())

	print "+++ W-Z/W+Z +++"
	syswzratio ={}
	ratio = {}
	for channel in filter(lambda x: x != 'leptonchannel',channellist):
		wzplus,errplus,nsysplusup,nsysplusdown  = channelyields["WPositive"][channel]
		wzminus,errminus,nsysminusup,nsysminusdown = channelyields["WNegative"][channel]
		wzratio = wzminus/wzplus
		ratio[channel] = wzratio
		syswzratio[channel] = getsysdict(wzratio,nsysminusup,nsysminusdown,nsysplusup,nsysplusdown)
		totalsys = wzratio*sqrt(reduce(lambda x,y: x+y,map(lambda valsys: valsys**2.0,syswzratio[channel].values())))
		err = wzratio*sqrt((errminus/wzminus)**2.+(errplus/wzplus)**2.)
		err2 = sqrt((errminus-errplus)**2.0/(1-errplus**2.)**2.0)
		print "%s channel w-z/w+z=%.4f+-%.4f(stat)+-%.4f(sys)" % (channel,wzratio,err,totalsys)
		# Adding the STATS (relative error) key to the systematic dict in order to be stored
		syswzratio[channel]["STATS"] = err/wzratio
	# Store the full errors (systematics and statistical) to be used by the bluemethod script
	storeratioerrors(ratio,syswzratio)


