#!/usr/bin/env python
"""
Evaluating W+/W+Z ratio given the ouput of the analysis
"""
def getobsyields(channelfolder,signalname):
	"""
	"""
	import os
	import glob
	from functionspool_mod import extractyields

	
	channel = os.path.basename(channelfolder)
	tablename = channelfolder+"/table_"+channel+".tex"
	ftab = open(tablename)
	asciirawtable = ftab.readlines()
	ftab.close()
	Nsig,Nsigerr = extractyields(asciirawtable,"Data-TotBkg")

	return Nsig,Nsigerr


def getobsyieldsprecise(channelfolder,signalname):
	"""
	"""
	import os
	import glob
	from math import sqrt
	from functionspool_mod import processedsample

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
	
	samplesname = filter(lambda x: x != actsignalname and x != dataname, \
			map(lambda x: os.path.basename(x).replace("cluster_",""),glob.glob(channelfolder+"/cluster_"+"*")) )

	Nerr2 = Nerr**2.0
	# Creatinga the instances of proccess
	for sample in samplesname:
		rootfilename = channelfolder+"/cluster_"+sample+"/Results/"+sample+".root"
		evts = processedsample(rootfilename)
		val,err = evts.getvalue("MET")
		Nobs -= val
		Nerr2 += err**2.
		del evts
	del dataps
	
	return Nobs,sqrt(Nerr2)


def getyieldsdict(channeldict,signalname,isprecise):
	"""
	"""
	import os
	channelyieldsdict = {}
	for channelfolder in sorted(channeldict):
		channel = os.path.basename(channelfolder)
		if isprecise:
			print "\033[1;34mgetyieldsdict INFO\033[m Extracting info from %s" % channelfolder
			channelyieldsdict[channel] = getobsyieldsprecise(channelfolder,signalname)
		else:
			channelyieldsdict[channel] = getobsyields(channelfolder,signalname)

	return channelyieldsdict


if __name__ == '__main__':
	import os,sys
	import glob
	from math import sqrt

	from optparse import OptionParser
        #Comprobando la version (minimo 2.4)
        vX,vY,vZ,_t,_t1 = sys.version_info
        if vX > 2 and vY < 4:
                message = '\033[1;31getsystematics ERROR\033[1;m I need python version >= 2.4'
                sys.exit( message )
        elif vX < 2:
                message = '\033[1;31getsystematics ERROR\033[1;m I need python version >= 2.4'
                sys.exit( message )
        
        parser = OptionParser()
        parser.set_defaults(signalname="WZ")
        parser.add_option( '-p', '--parentfolder', action='store', dest='parentfolder', help='Paths'\
                        ' where to find the standard folder structure. It must be two paths, comma'\
                        ' separated, one for the positive and the other for the negative')
	parser.add_option( '-s', '--signalname', dest='signalname',help='Name of the signal [Default: WZ]')
	parser.add_option( '-r', '--recalculate', action='store_true', dest='recalculate',\
			help='Activate the a more precise function to extract yields. Otherwise it uses'\
			' the output of the table*.tex.')

        ( opt, args ) = parser.parse_args()

	if not opt.parentfolder:
		wzpluspath = os.path.join(os.getcwd(),"WPositive")
		wzminuspath = os.path.join(os.getcwd(),"WNegative")
	else:
		pl = opt.parentfolder.split(",")
		wzpluspath = pl[0]
		wzminuspath = pt[1]

	
	# Extract the channel folders
	channelfoldersplus = filter(lambda x: os.path.isdir(x), glob.glob(os.path.join(wzpluspath,"*")))
	channelfoldersminus = filter(lambda x: os.path.isdir(x), glob.glob(os.path.join(wzminuspath,"*")))

	channelyields = {}
	# Evaluate the observed yields (per channel)
	channelyields["WPositive"] = getyieldsdict(channelfoldersplus,opt.signalname,opt.recalculate) 
	channelyields["WNegative"] = getyieldsdict(channelfoldersminus,opt.signalname,opt.recalculate) 

	# Evaluate the ratio per channel
	channellist = sorted(channelyields.values()[0].keys())

	print "+++ W-Z/W+Z +++"
	for channel in channellist:
		wzplus,errplus = channelyields["WPositive"][channel]
		wzminus,errminus = channelyields["WNegative"][channel]
		wzratio = wzminus/wzplus
		err = sqrt((errminus/wzplus)**2.+(wzminus/wzplus**2.)**2.*errplus**2.)
		print "%s channel w-z/w+z=%.3f+-%.3f" % (channel,wzratio,err)





	

	
