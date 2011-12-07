#!/usr/bin/env python

def getdatanamefiles(t):
	"""
	"""
	import glob
	import os
	# Found the list of cluster_ directories
	clusterdirs = glob.glob(os.path.join(t,"cluster_*"))
	if len(clusterdirs) == 0:
		message = "\033[1;31mjoinall ERROR\033[1;m Not found 'cluster_' subdirectories in '"+t
		raise message
	datanamefilem = {}
	for clusterd in clusterdirs:
		if not os.path.isdir(clusterd):
			message = "\033[1;31mjoinall ERROR\033[1;m Some unexpected error: '"+clusterd+\
					"' is not a directory"
			raise message
		dataname = clusterd.split("cluster_")[-1]
		filename = os.path.join(os.path.join(clusterd,"Results"),dataname+".root")
		if not os.path.isfile(filename):
			message = "\033[1;31mjoinall ERROR\033[1;m Some unexpected error: '"+filename+\
					"' do not exist"
			raise message
		datanamefilem[dataname] = filename

	return datanamefilem

	
def domerge(dataname,rootfilenames):
	"""
	"""
	import ROOT
	from subprocess import Popen,PIPE
	import os
	
	lastdir = os.getcwd()
	finaldir=os.path.join("cluster_"+dataname,"Results")
	try:
		os.makedirs(finaldir)
	except OSError:
		pass
	os.chdir(finaldir)

	finalfilename = dataname+".root"
	command = [ 'haddPlus', finalfilename ]
	for f in rootfilenames:
		command.append(f)
	p = Popen( command, stdout=PIPE,stderr=PIPE).communicate()
	
	os.chdir(lastdir)
	# FIXME: Check...
	

if __name__ == '__main__':
	import sys
	import os
	from optparse import OptionParser

	#Comprobando la version (minimo 2.4)
	vX,vY,vZ,_t,_t1 = sys.version_info
	if vX > 2 and vY < 4:
		message = 'merge3leptonfs: I need python version >= 2.4'
		sys.exit( message )
	elif vX < 2:
		message = 'merge3leptonfs: I need python version >= 2.4'
		sys.exit( message )
	#Opciones de entrada
	parser = OptionParser()
	parser.add_option('-d', '--dirs' ,  action='store', type='string', dest='dirlist', \
			help='directories where it is contained the \'cluster_name\' directories. It is assumed that it is followed'+\
			' the convention: "cluster_name/Results/name.root"')
	
	( opt, args ) = parser.parse_args()

	if not opt.dirlist:
		message = "\033[31;1mmerge3leptonfs ERROR\033[0m Mandatory action '-d'"
		sys.exit( message )

	# Parsing and checking
	if not os.getenv("VHSYS"):
		message = "\033[31;1mmerge3leptonfs ERROR\033[0m Need the environment variables of the AnalysisVH package"
		sys.exit( message )

	dirlist = opt.dirlist.split(",")
	targetdir = []
	for d in dirlist:
		completepath = os.path.join(os.getenv("PWD"),d)
		if not os.path.isdir(completepath):
			message = "\033[31;1mmerge3leptonfs ERROR\033[0m Not found the directory '"+completepath+"'"
			sys.exit( message )
		targetdir.append(completepath)
	
	fsdfiles = {}
	print "\033[1;34mmerge3leptonfs INFO\033[1;m Searching the datafiles to be merged"
	for t in targetdir:
		fsdfiles[t] = getdatanamefiles(t)

	# Extract the datanames
	datanamesPRE = []
	for fdatadict in fsdfiles.itervalues():
		datanamesPRE.append( fdatadict.keys() )

	# Checking everything is fine
	dntobecompared = datanamesPRE[0]
	for dlist in datanamesPRE:
		if dlist != dntobecompared:
			message = "\033[1;31mmerge3leptonfs ERROR\033[1;m Unexpected error: not found"+\
					" the same datanames in each directory"
			sys.exit( message )
	datanames = datanamesPRE[0]
	# Preparing for the merge
	try:
		os.mkdir("leptonchannel")
	except OSError:
		# file exist.. I don't have problem with that
		pass
	os.chdir("leptonchannel")
	dirsfs = fsdfiles.keys()
	mergefiles = {}
	for dn in datanames:
		mergefiles[dn] = []
		for tdir in dirsfs:
			mergefiles[dn].append( fsdfiles[tdir][dn] )

	# Finally merge
	print "\033[1;34mmerge3leptonfs INFO\033[1;m Merging all the final states:"
	for dn, filelist in mergefiles.iteritems():
		print " +"+dn
		domerge(dn,filelist)
	



