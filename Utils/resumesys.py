#!/usr/bin/env python
"""
Harvest the systematics presents below a SYSTEMATICS folder 
and build a systematics python file with all the results in the
right format ready to be used for the python framework scripts 
of xs and derivates
"""

def presentsys(syspath, common="SYS_"):
	""" .. presentsys(parentfolder) ->  { namesys: (abspath_up,abspath_down)), ... }

	Given a folder, seek for all the folders presents with the 
	name structure *+common and returns the name before common plus
	it absolute path. 
	"""
	import glob
	import os

	systypes = list(set(map(lambda x: x.split("SYS_")[0].split("/")[-1],\
			glob.glob(syspath+"/*"+common+"*"))))

	sysoutput = {}
	# Check the expected folders and build the ouput
	for syst in systypes:
		fupdown = []
		for var in ["UP", "DOWN"]:
			folder = os.path.join(syspath,syst+common+var)
			if not os.path.isdir(folder):
				message= "\033[31mpresentsys ERROR\033[m The folder '%s'" \
						" is not present"% folder
				raise RuntimeError(message)
			fupdown.append(folder)
		sysoutput[syst] = tuple(fupdown)

	return sysoutput

def nt3call(folder,signal):
	""".. nt3call(folder) 

	Call the nt3subtract script within folder
	"""
	from subprocess import Popen,PIPE
	import glob
	import os

	cwd = os.getcwd()
	os.chdir(folder)

	# -- Check if already done
	for chf in filter(lambda x: os.path.isdir(x),glob.glob(folder+"/*")):
		ispresentdir = ( len(filter(lambda x: os.path.basename(x) == "cluster_Fakes_Nt3",glob.glob(chf+"/*"))) != 0 )
		if not ispresentdir:
			return
	
	command = [ 'nt3subtract', '-s', signal ]
	p = Popen(command,stdout=PIPE,stderr=PIPE).communicate()
	print p[0]

	if len(p[1]) != 0:
		message = "\033[1;31mnt3call CAPTURED ERROR\033[1;m from nt3subtract:\n"
		message += p[1]
		raise RuntimeError(message)

	os.chdir(cwd)


def getdifferences(samplesnames,baselinefolder,comparedfolder,sysdict = None):
	""".. getdifferences(samplesnames,baselinefolder,comparedfolder) -> (dict(s
	"""
	from functionspool_mod import processedsample
	import os

	# Get some previous info
	# --- who is the analysis folder
	analysisdir = filter(lambda x: type(x) == list, [baselinefolder]+[comparedfolder])[0]
	# --- extract channels folders
	channelsfolders = map(lambda x: x.split("/")[-1],analysisdir)
	# --- build the channels path for the str. directory
	if type(baselinefolder) == list:
		baselinetop,channeldir = os.path.split(analysisdir[0])
		comparedtop = comparedfolder
	else:
		baselinetop = baselinefolder
		comparedtop,channeldir = os.path.split(analysisdir[0])
	
	# --- get signal name-
	signal = ''.join(ch for ch in channelsfolders[0] if ch != "m" and ch != "e")

	# Do it per channel
	sysdict = {}
	for channeldir in channelsfolders:
		print ".",
		for dataname in samplesnames:
			filebase = baselinetop+"/"+channeldir+"/"+"cluster_"+dataname+\
					"/Results/"+dataname+".root"

			filecomp = comparedtop+"/"+channeldir+"/"+"cluster_"+dataname+\
					"/Results/"+dataname+".root"
			diffsample = processedsample(filebase,showall=True)-\
					processedsample(filecomp,showall=True)
			channel = channeldir.replace(signal,"")
			lastcut = diffsample.getcutlist()[-1]
			sysrel = diffsample.getsysrelative(lastcut)
			try:
				sysdict["SYS"+dataname][channel] = sysrel
			except KeyError:
				sysdict["SYS"+dataname] = { channel: sysrel }
	print ""

	return sysdict


def createsysfile(sysdict, foldertostore):
	""".. createsysfile(sysdict) 
	
	Build the file which is going to be used by any other script needing systematics 
	and stores in the SYSTEMATICS directory as a module
	"""
	import os
	from datetime import date


	today = date.today()
	fileout = os.path.join(foldertostore,"systematics_mod.py")

	lines =  "#!\\usr\\bin\\env python\n\n"
	lines += "# WARNING: module generated automatically with 'resumesys' utility\n"
	lines += "# CREATE DATE: "+today.strftime("%2d-%2m-%Y")+"\n"
	lines += "# DON'T MODIFY unless you know what you are doing!\n"
	lines += "# The proper way to modify this file is trough the 'resumesys' utility'\n"
	lines += "# To use this file as module, incorporate to your python script the\n"
	lines += "# following lines before call the import statement:\n"
	lines += "# import sys\n"
	lines += "# sys.path.append('absolute_path_where_it_found_this_file')\n\n"
	lines += """
\"\"\"
:mod:`sytematics`
.. module:: systematics
   :platform: Unix
   :synopsis: module containing the raw systematics
.. moduleauthor:: Jordi Duarte Campderros <duarte@ifca.unican.es>

.. data:: DDMMC The relative error systematic due to the fakeable object method and to be 
          applied to the yields of the Tight-Tight-noTight sample. The errors are relative
	  to the yields.
   :type: dict(str:float)

.. data:: SYSZZ The relative error systematics separated by systematic sources and to be 
          applied to the yields of the ZZ sample. The errors are relative to the ZZ yields
	  and they are going to affect the Number of signal
   :type: dict(dict(str:float))

.. data:: SYSWZ The relative error systematics separated by systematic sources and to be 
          applied to the yields of the WZ sample. The errors are relative to the WZ yields
	  and they are going to affect the efficiency (on cross-section calculation)
   :type: dict(dict(str:float))

.. data:: STAT The relative error statistic of the signal, i.e. number of data observed minus
          number of background estimated (so the statistic errors of the data observed and 
	  background expected are incorporated here). The errors are relative to the yields.
	  WARNING: the dictionary is just initialized here and must be initialized and filled
	  using the functionspool_mod.getxserrosrel function, so this data cannot be used before
	  the call of the functionspool_mod.getxserrosrel
   :type: dict(str:float)
\"\"\"
"""
	lines += "# == SYSTEMATICS for WZ analysis =============================================\n"
	lines += "# Relative error with respect the \n"
	lines += "# 1. Yields of\n"
	lines += "#    a. the fake rate sample (data-driven bkg). To be included in the STATS\n"
	lines += "SYSFakes = "+str(sysdict["SYSFakes"]["FR"])+"\n"
	order = [ 'b', 'c', 'd', 'e', 'f' ]
	ordername = zip(order,filter(lambda x: x != "SYSFakes",sysdict.keys()))
	for i,name in ordername:
		lines += "#    "+i+". the "+name.replace("SYS","")+" MC sample (note the stat has to be"\
				" extracted from the table/plots...\n"
		lines += name+" = {\n" #+str(sysdict[name])+"\n"
		for systype,channeld in sysdict[name].iteritems():
			lines += "\t'"+systype+"': "+str(channeld)+",\n"
		lines += "\t'Lumi': {'eee': 0.022, 'eem': 0.022, 'mmm': 0.022, 'mme': 0.022},\n"
		lines += "\t'PDF': {'eee': 0.014, 'eem': 0.014, 'mmm': 0.014, 'mme': 0.014},\n"
		lines += "\t'Stat': {'eee': 0.0, 'eem': 0.0, 'mmm': 0.0, 'mme': 0.0}\n"
		# to keep compatibility with old systematic_mod
		lines += "     }\n" 
	lines += "# == and Statistical for signal ======================================================\n"
	lines += "STAT = { 'eee' : 0.0, 'eem': 0.0, 'mme' : 0.0, 'mmm': 0.0 }\n\n"
	lines += "\n# Just for compatibility with old version\n"
	lines += "DDMMC = {'eee': 0.0, 'eem': 0.0, 'mmm': 0.0, 'mme': 0.0}\n" 
	lines += "SYSWZ = SYSWZTo3LNu\n"
	#lines += "DDMMC = SYSFakes\n"
	
	f = open(fileout,"w")
	f.writelines(lines)
	f.close()

	# remove the compiled python everytime we create a new one (if it is there)
	try:
		os.remove(fileout.replace(".py",".pyc"))
	except OSError:
		pass



if __name__ == '__main__':
	from optparse import OptionParser,OptionGroup
	import glob
	import os
	from functionspool_mod import getsamplenames
	from math import sqrt
	
	#Opciones de entrada
	usage  ="usage: resumesys sysfolder"
	usage +="\nThe sysfolder must contain one or more folders named TYPESYS_VAR,\n"
	usage +="where TYPE could be 'LEPTON' 'FR' 'MS' 'MET' or 'PU'. For each\n"
	usage +="systematic it must be two folders where VAR is sustituted by\n"
	usage +="UP and DOWN (1sigma and -1sigma variation)."
	usage +="\nThe ouput is [....]"

	parser = OptionParser(usage=usage)
	
	(opt,args) = parser.parse_args()

	if len(args) < 1:
		message= "\033[31mresumesys ERROR\033[m It is needed the systematic path"
		raise RuntimeError(message)

	try:
		syspathrel = glob.glob(args[0])[0]
	except IndexError:
		message= "\033[31mresumesys ERROR\033[m Invalid given path '%s'"% args[0]
		raise RuntimeError(message)

	syspath = os.path.abspath(syspathrel)
	# Check the directory
	if not os.path.isdir(syspath):
		message= "\033[31mresumesys ERROR\033[m Invalid given path '%s'"% args[0]
		raise RuntimeError(message)

	# Extract the systematics presents
	sysdefined = presentsys(syspath)

	# Extract the nominal folders
	# It must be the parent directory of the systematics folder
	anabsnominalfolder = os.path.abspath(os.path.join(syspath,".."))
	# FIXME: Do it with a regular expression, up to now, everytime you have
	# more directories than the channel ones, you have a problem
	nominalfolders = filter(lambda x: os.path.isdir(x) and x != syspath,\
			glob.glob(anabsnominalfolder+"/*"))
	if len(nominalfolders) < 4:
		message= "\033[31mresumesys ERROR\033[m Invalid folder structure. not found the"\
				" nominal analysis channels folder"
		raise RuntimeError(message)
	# -- delete the leptonchannel
	nominalfolders = filter(lambda x: x != anabsnominalfolder+'/leptonchannel',nominalfolders)
	# -- and any other thing not following the right notation: SIGNALlll, l=e,m, SIGNAL= WZ, WH..
	# -- just by checking the presence of cluster_ directories
	nominalfolders = filter(lambda x: len(glob.glob(x+"/cluster_*")) != 0, nominalfolders)
	# -- And get the name of the signal
	signal = os.path.split(nominalfolders[0])[1][:2]
	# -- need to do the fakes-nt3 for the FRSYS
	if sysdefined.has_key("FR"):
		for d in sysdefined["FR"]:
			nt3call(d,signal)

	syscommondict = {}
	# For each systematics evaluate the difference with respect the nominal
	for sysname,sysfoldertuple in sysdefined.iteritems():
		print "\033[34mresumesys INFO\033[m Evaluating '%s' systematic" % (sysname)
		fup   = filter(lambda x: x.find("_UP") != -1,sysfoldertuple)[0]
		fdown = filter(lambda x: x.find("_DOWN") != -1,sysfoldertuple)[0]
		# -- Extract the datasamples names present in the systematic folders
		datasamples = getsamplenames(fup)
		print "\033[34mresumesys INFO\033[m ---+ Datasamples afected:",
		for name in datasamples:
			print name,
			# Create the dictionaries ... EXPLICALO XXX 
			sysdictname = "SYS"+name
			if not syscommondict.has_key(sysdictname):
				syscommondict[sysdictname] = { sysname: {}}
			elif not syscommondict[sysdictname].has_key(sysname):
				syscommondict[sysdictname][sysname] = {}

		print ""
		# -- Do the calculation: (Nominal-Sys)/Nominal per channel
		print "\033[34mresumesys INFO\033[m ---+ Extracting DOWN variation",
		sysdict_d = getdifferences(datasamples,nominalfolders,fdown)
		print "\033[34mresumesys INFO\033[m ---+ Extracting UP variation",
		sysdict_u = getdifferences(datasamples,fup,nominalfolders)
		# -- Get the mean of the variations
		for sample,channeldict_u in sysdict_u.iteritems():
			channeldict_d = sysdict_d[sample]
			for ch,value_u in channeldict_u.iteritems():
				value_d = channeldict_d[ch]
				syscommondict[sample][sysname][ch] = sqrt((value_u)**2.0+(value_d)**2.0)/2.0
	
	# -- File creation: systematics_mod.py
	createsysfile(syscommondict,anabsnominalfolder)
	print "\033[32mresumesys INFO\033[m Created the file 'systematics_mod' in %s" % anabsnominalfolder


	 




