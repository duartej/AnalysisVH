#!/usr/bin/env python

"""
Python script to send jobs to cluster
Exemple of usage:
python sendcluster.py -a submit -c ../analisiswh_mmm.ip -d WH120 -j 1 --pkgdir=../ --basedir=../../
python sendcluster.py -a harvest --wd=cluster_WHToWW2L120
"""

from clustermanager_mod import clustermanager


if __name__ == '__main__':
	import sys
	import os
	import glob
	from optparse import OptionParser,OptionGroup

	#Comprobando la version (minimo 2.4)
	vX,vY,vZ,_t,_t1 = sys.version_info
	if vX > 2 and vY < 4:
		message = 'sendcluster: I need python version >= 2.4'
		sys.exit( message )
	elif vX < 2:
		message = 'sendcluster: I need python version >= 2.4'
		sys.exit( message )
	#Opciones de entrada
	usage="usage: sendcluster <\033[39msubmit\033[m|\033[39mharvest\033[m|"+\
			"\033[39mresubmit\033[m|\033[39mresubmit.stalled\033[m|"+\
			"\033[39mdelete\033[m> [options]"
	parser = OptionParser(usage=usage)
	parser.set_defaults(shouldCompile=False,jobsNumber=0,fakeasdata=False)
	
	group = OptionGroup(parser, "\033[39msubmit\033[m options","")
	group.add_option( '-a', '--analysis', action='store', type='string', dest='antype', help="Analysis to be processed WZ|WH" )
	group.add_option( '-f', '--finalstate', action='store', type='string', dest='finalstate', help="Final state signature: mmm eee mme eem")
	group.add_option( '-F', '--fakeable', action='store', dest='fakeable', metavar="N,T", help="Fakeable mode, so N,T (where N=Leptons and T=Tight leptons."+\
			" Used with the '-k' option")
	group.add_option( '-k', '--fakeasdata', action='store_true', dest='fakeasdata', help="Fakeable mode ALLOWING whatever datasample (not only"+\
			" the so called Fakes) to be created the N,T sample. So it must use with the '-F' option.")
	group.add_option( '-d', '--dataname',  action='store', type='string', dest='dataname', help='Name of the data (see runanalysis -h). Also'
			' not using this option, the script will use all the datafiles *_datanames.dn found in the working directory to launch process')
	group.add_option( '-j', '--jobs',  action='store', type='int',    dest='jobsNumber', help='Number of jobs. Not using this option, the script'
			' will trying to found how many jobs are needed to create a 10min job')
	group.add_option( '-c', '--cfg' ,  action='store', type='string', dest='config', metavar="LEPTON:cfgfile[,..]",\
			help='name of the lepton and config file (absolute path), \':\' separated' )
	group.add_option( '-p', '--precompile',action='store_true', dest='shouldCompile', help='Set if exist a previous job to do compilation' \
			' (launching datamanager executable) \033[31mTO BE DEPRECATED\033[m' )
	parser.add_option_group(group)

	groupharvest = OptionGroup(parser,"\033[39mharvest\033[m, \033[39mresubmit\033[m, "+\
			"\033[39mresubmit.stalled\033[m and \033[39mdelete\033[m options","")
	groupharvest.add_option( '-w', '--wd', action='store', type='string', dest='workingdir', help="Working directory used with the '-a harvest' option")
	parser.add_option_group(groupharvest)
	
	( opt, args ) = parser.parse_args()

	if len(args) == 0 :
		message = "\033[31msendcluster: ERROR\033[m Note that is mandatory an action: 'submit' 'harvest'"
		message += "\nSee usage: "+parser.usage
		sys.exit(message)
	#Mandatory configuration file
	configabspath = ''
	
	if args[0] == 'submit':
		leptoncfgmap = {}
		if not opt.config:
			message = "\033[31sendcluster: ERROR\033[m the '-c' option is mandatory"
			sys.exit( message )
		else:
			#Extract type of lepton
			leptoncfglist = opt.config.split(',')
			# create dict with lepton: cfg
			for i in leptoncfglist:
				# FIXME: Check the error (try: except ValueError)
				lepton = i.split(':')[0]
				config = i.split(':')[1]
				#Checking is a file and can be find it
				if not os.path.exists(config):
					message = "\033[34msendcluster: ERROR\033[m Not found '"+config+"'"
					sys.exit( message )
				configabspath = os.path.abspath(config)
				leptoncfgmap[lepton] = configabspath
		# Dataname mandatory:
		if not opt.dataname:
			# Obtaining all the datanames from files in the current directory
			message = "\033[34msendcluster: INFO\033[m obtaining the datanames"\
					" files from the current directory"
			print message
			datanameslist = [ x.replace("WHToWW2L","WH").replace("_datanames.dn","") \
					for x in glob.glob("*.dn") ]
		else:
			# Checking if is a list
			dnislist = opt.dataname.split(',')
			if len(dnislist) > 1:
				datanameslist = dnislist
			else:
				datanameslist = [ opt.dataname ]

		# Also final state:
		if not opt.finalstate:
			message = "\033[31;1msendcluster: ERROR\033[0m the '-f' option is mandatory"
			sys.exit( message )
		# Also analysis type:
		if not opt.antype:
			message = "\033[31;1msendcluster: ERROR\033[0m the '-a' option is mandatory"
			sys.exit( message )
		# Also fakeable or not:
		if not opt.fakeable:
			opt.fakeable=False
		# Checking the correct use of -k option 
		if opt.fakeasdata:
			if not opt.fakeable:
				message = "\033[31;1msendcluster: ERROR\033[0m It is mandatory the '-F' option"+\
						" with the '-k' option"
		# Instantiate and submit
		manager = []
		for dataname in datanameslist:
			print "\033[1;34m[sendcluster INFO]\033[1;m:: Dataname: %s" % dataname
			# Checks and some changes
			fakeable = opt.fakeable
			if not opt.fakeable:
				if dataname.find("Fakes") == 0:
					message = "\033[31msendcluster: ERROR\033[m It is mandatory the '-F' option"+\
							" with the 'Fakes' dataname"
					sys.exit( message )			
			else:
				if dataname.find("Fakes") != 0 and not opt.fakeasdata:
					# not sending to the instance
					#print "\033[1;33msendcluster WARNING:\033[1;m The job is going to be send as"+\
					#		" regular not in FAKE mode. Use the '-k' option if you want to"+\
					#		" force the FAKE mode in this sample"
					fakeable = False
			manager.append( clustermanager(dataname=dataname,cfgfilemap=leptoncfgmap,\
					njobs=opt.jobsNumber,\
					finalstate=opt.finalstate, \
					analysistype=opt.antype,fakeable=fakeable) )
	elif args[0] == 'harvest':
		if opt.workingdir is None:
			message = "\033[31msendcluster: ERROR\033[m the '--cw' option is mandatory"
			sys.exit( message )
		
		if not os.path.exists(opt.workingdir):
			message = "\033[31msendcluster: ERROR\033[m the working path '"+opt.workingdir \
					+"' does not exists"
			sys.exit( message )
		
		manager = clustermanager(opt.workingdir)
	
	elif args[0] == 'delete' or args[0] == 'resubmit' or args[0] == 'resubmit.stalled':
		if opt.workingdir is None:
			message = "\033[31msendcluster: ERROR\033[m the '--cw' option is mandatory"
			sys.exit( message )
		
		if not os.path.exists(opt.workingdir):
			message = "\033[31msendcluster: ERROR\033[m the working path '"+opt.workingdir \
					+"' does not exists"
			sys.exit( message )
		
		manager = clustermanager(opt.workingdir)
	else:
		message = "\033[31msendcluster: ERROR\033[mNot recognized the action '"+args[0]+"'"
		sys.exit( message )
	
	action=args[0]
	try:
		manager.process(action)
	except AttributeError:
		map(lambda x: x.process(action), manager)
	#try:
	#	manager.process(action)
	#except:
	#	# Be sure we have persistency
	#	manager.store()
	#	raise
	


