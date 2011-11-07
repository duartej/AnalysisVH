#!/usr/bin/env python

"""
Python script to send jobs to cluster
PRIMERA VERSION: NO SEPARA LOS DATASETS EN MULTIPLES JOBS: opcion -j no activa
"""


from optparse import OptionParser

if __name__ == '__main__':
	import sys
	import os

	#Comprobando la version (minimo 2.4)
	vX,vY,vZ,_t,_t1 = sys.version_info
	if vX > 2 and vY < 4:
		message = 'sendcluster: I need python version >= 2.4'
		sys.exit( message )
	elif vX < 2:
		message = 'sendcluster: I need python version >= 2.4'
		sys.exit( message )

	#Opciones de entrada
	parser = OptionParser()
	parser.set_defaults(shouldCompile=False,jobsNumber=1)
	parser.add_option( '-d', '--dataname',  action='store', type='string', dest='dataname', help='Name of the data (see runanalysis)' )
	parser.add_option( '-j', '--jobs',  action='store', type='int',    dest='jobsNumber', help='Number of jobs' )
	parser.add_option( '-c', '--cfg' ,  action='store', type='string', dest='config', help='name of the config file (absolute path)' )
	parser.add_option( '-p', '--compile',action='store_true', dest='shouldCompile', help='Set if exist a previous job to do compilation' \
			' (launching datamanager executable)' )
	
	( opt, args ) = parser.parse_args()
	#Archivo de configuracion obligatori:
	if opt.config is None:
		message = "\nsendcluster: ERROR the '-c' option is mandatory.\n"
		sys.exit( message )
	else:
		#Checking is a file and can be find it
		pass
	#Dataname obligatorio:
	if opt.dataname is None:
		message = "\nsendcluster: ERROR the '-d' option is mandatory.\n"
		sys.exit( message )
	# Not implemented yet
	if opt.jobsNumber != 1:
		message = "WARNING: Not implemented yet splitting jobs. Forcing '-j 1'"
		opt.jobsNumber = 1

	#Creacion del directorio de trabajo
	launchDir = os.getenv( 'PWD' )
	executedir=launchDir+"/cluster_"+dataname
	try:
		os.mkdir( executedir )
	except OSError:
		# FIXME
		message = "PROVISIONAL: Posiblemente el directorio ya existe"
	# Moving to that dir
	os.chdir( executedir )
	#Archivo de configuracion

	jobsid = []
	# Send the previous job to compile and update the code
	if( shouldCompile ):
		# create job to compile
		jobnames = createbash('datamanagercreator',opt.dataname,opt.config)
		# sending the jobs
		for job in jobnames:
			jobsid.append( sendjob(job) )

	for i in xrange(opt.jobsNumber):
		# Creating the bash script to send the job
		jobnames = createbash('runanalysis',opt.dataname,opt.config)
		# sending the jobs
		for job in jobnames:
			jobsid.append( sendjob(job) )


	print ""
	print "The jobs have been sended: "
	for i in jobsid:
		print i,
	print ""
	print "You can observe the status using 'qstat -u "+os.getenv("USER")
	print "Good Luck!




