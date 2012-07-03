#!/usr/bin/env python
"""
Script to extract the systematics associated to the yields (normalized to
a given luminosity) or acceptance. The procedure is as following: 
	1. Evaluate the yields using the central value (usual repository) and sendall, plotall, ...
	2. Evaluate the yields using some systematic effect incorporated in the code (using some
	   systematic specific branch in the repository) and sendall, plotall, ...
	3. Launch this script per channels
"""

from functionspool_mod import processedsample

def getsystwofiles(file1,file2,verbose):
	"""
	"""
	import sys

	if file1 == file2:
		message = "\033[1;31mgetsystematics ERROR\033[1;m Cannot evaluate differences"\
				" between the same files arguments '%s'. See usage." % (file1)
		sys.exit(message)
	if verbose:
		print "\033[1;34mgetsystematics INFO\033[1;m Extracting systematics using files %s and %s" % (file1,file2)
		sys.stdout.flush()
	
	evt = processedsample(file1,showall=verbose)-processedsample(file2,showall=verbose)

	return evt
	

if __name__ == '__main__':
	import sys
	import os
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
	
	usage  ="usage: getsystematics filename1 filename2 [options]"
	usage +="\nExtracts the yield differences in percentage between the two 'fHEventsPerCut'\n"
	usage +="histograms of 'filename1' and 'filename2'. Also it is possible to launch the\n"
	usage +="script without arguments but with the mandatory options '-p' and '-s'; then it\n"
	usage +="will be evaluate each channel inside the '-p' introduced folder for the sample\n"
	usage +="introduced with '-s' option"
	parser = OptionParser(usage=usage)
	parser.set_defaults(samplename="Fakes",verbose=False)
	parser.add_option( '-p', '--parentfolder', action='store', dest='parentfolder', help='Paths'\
			' where to find the standard folder structure. It must be two paths, comma'\
			' separated, one for the central yields, the other for the sytematics yields.'\
			' Incompatible option with the arguments \'filename1\' and \'filename2\'')
	parser.add_option( '-s', '--sample', dest='samplename',help='Sample to evaluate the systematics.'\
			' This option, apart fo the stricly sample name (as it going to be found in the'\
			' channel folders "cluster_samplename"), can take the '\
			' specific values "yields" and "acceptance". The "yields" keyword is going to'\
			' extract the differences between all the samples but the Monte Carlo signal'\
			' (WZ or WH)'\
			' and return the relative differences for the signal, i.e.'\
			' N_S = N_Data-N_bkg <-- This is goiing to be deprecated .'\
			'The "acceptance" keyword is going to return the'\
			' relative difference for the yields of the Monte Carlo signal (WZ or WH)'\
			' which can be used for the A*eff cross-section term.'\
			' Incompatible option with the arguments \'filename1\' and \'filename2\'')
	parser.add_option( '-n', '--namesignal', dest='namesignal', help='Name of the signal')
	parser.add_option( '-v', '--verbose', action='store_true', dest='verbose',help='Show all the'\
			' differences by cut')


	( opt, args ) = parser.parse_args()

	allchannels = False
	if len(args) == 0:
		allchannels = True
		
	if len(args) != 0 and len(args) != 2:
		message = "\033[1;31mgetsystematics ERROR\033[1;m Missing mandatory arguments files, see usage."
		sys.exit(message)

	# Using two concrete files and exit
	if not allchannels:
		file1 = args[0]
		file2 = args[1]
		diffsample = getsystwofiles(file1,file2,opt.verbose)
		print diffsample
		sys.exit(0)
	
	# Using all the channels within a parent directory
	if not opt.parentfolder:
		message = "\033[1;31mgetsystematics ERROR\033[1;m Missing mandatory option '-p' when"\
				" you are using the 'per channel' modality"
		sys.exit(message)
	if not opt.samplename:
		message = "\033[1;31mgetsystematics ERROR\033[1;m Missing mandatory option '-s' when"\
				" you are using the 'per channel' modality"
		sys.exit(message)


	# available filenames
	path1 = opt.parentfolder.split(",")[0]
	try:
		path2 = opt.parentfolder.split(",")[1]
	except IndexError:
		message = "\033[1;31mgetsystematics ERROR\033[1;m Option '-i' needs two"\
				" paths, one for the central yields, the other for the"\
				" systematics, comma separated"
		sys.exit(message)

	# extract the channels directories
	thechannelfolders1 = filter(lambda x: os.path.isdir(x), glob.glob(os.path.join(path1,"*")))
	thechannelfolders2 = filter(lambda x: os.path.isdir(x), glob.glob(os.path.join(path2,"*")))
	
	# Case we want to get the acceptance systematic
	if opt.samplename.lower() == "acceptance":
		if not opt.namesignal:
			message = "\033[1;31mgetsystematics ERROR\033[1;m Missing mandatory option '-n' when"\
					" using the '-s acceptance'"
			sys.exit(message)
		# Extract the name of the signal
		try:
			signalname = os.path.basename(glob.glob(thechannelfolders1[0]+"/cluster_"+opt.namesignal+"*")[0]).replace("cluster_","")
		except IndexError:
			message = "\033[1;31mgetsystematics ERROR\033[1;m Missing mandatory option '-s' when"\
				" you are using the 'per channel' modality"
			sys.exit(message)
		print "\033[1;34mgetsystematics INFO\033[1;m Systematics for acceptance*efficiency term. You can get the values below directly as %"
		for folder1 in sorted(thechannelfolders1):
			channel = os.path.basename(folder1)
			print "\033[1;34mgetsystematics INFO\033[1;m Channel %s" % channel
			folder2 = filter(lambda x: os.path.basename(folder1) == os.path.basename(x), thechannelfolders2)[0]
			file1 = os.path.join(folder1,"cluster_"+signalname+"/Results/"+signalname+".root")
			file2 = os.path.join(folder2,"cluster_"+signalname+"/Results/"+signalname+".root") 
			diffsample = getsystwofiles(file1,file2,opt.verbose)
			cutlist = [ "MET" ]
			if opt.verbose:
				cutlist = diffsample.getcutlist()
			for cut in cutlist:
				print "\033[1;34mgetsystematics INFO\033[1;m ---- MC %s Yield relative diff. at %s cut: %.3f%s" % \
						(signalname,cut,diffsample.getsysrelative(cut)*100.0,"%")

	elif opt.samplename.lower() == "yields":
		if not opt.namesignal:
			message = "\033[1;31mgetsystematics ERROR\033[1;m Missing mandatory option '-n' when"\
					" using the '-s acceptance'"
			sys.exit(message)
		# Extract the name of the signal
		try:
			signalname = os.path.basename(glob.glob(thechannelfolders1[0]+"/cluster_"+opt.namesignal+"*")[0]).replace("cluster_","")
		except IndexError:
			message = "\033[1;31mgetsystematics ERROR\033[1;m Missing mandatory option '-s' when"\
				" you are using the 'per channel' modality"
			sys.exit(message)
		try:
			dataname = os.path.basename(glob.glob(thechannelfolders1[0]+"/cluster_Data*")[0]).replace("cluster_","")
		except IndexError:
			message = "\033[1;31mgetsystematics ERROR\033[1;m Not found Data folder! Check the folder structure..."
			raise RuntimeError(message)
		# Build all the files but signal: dN_S = sqrt(dN_D**2+dN_bkg**2), where dN_bkg**2 = sum_{i=all bkg} dN_i**2
		samplesname = filter(lambda x: x != signalname, \
				map(lambda x: os.path.basename(x).replace("cluster_",""),glob.glob(thechannelfolders1[0]+"/cluster_"+"*")) )
	
		print "\033[1;34mgetsystematics INFO\033[1;m Systematics for WZ signal yields (N_S=N_D-N_BKG) term. You can get the values below directly as %"
		for folder1 in sorted(thechannelfolders1):
			channel = os.path.basename(folder1)
			print "\033[1;34mgetsystematics INFO\033[1;m Channel %s" % channel
			folder2 = filter(lambda x: os.path.basename(folder1) == os.path.basename(x), thechannelfolders2)[0]
			dN_S2 = 0.0
			N_S = 0.0
			for sample in samplesname:
				file1 = os.path.join(folder1,"cluster_"+sample+"/Results/"+sample+".root")
				file2 = os.path.join(folder2,"cluster_"+sample+"/Results/"+sample+".root") 
				diffsample = getsystwofiles(file1,file2,opt.verbose)
				cut = diffsample.getcutlist()[-1]
				dN_S2 += diffsample.getvalue(cut)[0]**2.
				# Extracting the signal
				sp = processedsample(file1)
				value = sp.getvalue(cut)[0]
				del sp
				if sample == dataname:
					N_S += value
				else:
					N_S -= value
			dN_S = sqrt(dN_S2)
			print "\033[1;34mgetsystematics INFO\033[1;m ---- Total WZ yield relative difference at %s cut: %.3f%s" % (cut,dN_S/N_S*100.0,"%")
	# Introduced a regular sample name
	else:
		# first check: the sample exists
		knownsamples = map(lambda x: os.path.basename(x).replace("cluster_",""),glob.glob(thechannelfolders1[0]+"/cluster_"+"*")) 
		if not opt.samplename in knownsamples:
			message = "\033[1;31mgetsystematics ERROR\033[1;m Sample not found '%s' in the channel folders"
			sys.exit(message)
		print "\033[1;34mgetsystematics INFO\033[1;m Systematics for the '%s' sample. You "\
				"can get the values below directly as %s" % (opt.samplename,"%")
		for folder1 in sorted(thechannelfolders1):
			channel = os.path.basename(folder1)
			print "\033[1;34mgetsystematics INFO\033[1;m Channel %s" % channel
			folder2 = filter(lambda x: os.path.basename(folder1) == os.path.basename(x), thechannelfolders2)[0]
			file1 = os.path.join(folder1,"cluster_"+opt.samplename+"/Results/"+opt.samplename+".root")
			file2 = os.path.join(folder2,"cluster_"+opt.samplename+"/Results/"+opt.samplename+".root") 
			diffsample = getsystwofiles(file1,file2,opt.verbose)
			cutlist = [ "MET" ]
			if opt.verbose:
				cutlist = diffsample.getcutlist()
			for cut in cutlist:
				print "\033[1;34mgetsystematics INFO\033[1;m ---- MC %s Yield relative diff. at %s cut: %.3f%s" % \
						(opt.samplename,cut,diffsample.getsysrelative(cut)*100.0,"%")




	
