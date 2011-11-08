#!/usr/bin/env python

"""
Python script to send jobs to cluster
PRIMERA VERSION: NO SEPARA LOS DATASETS EN MULTIPLES JOBS: opcion -j no activa
"""

class clustermanager(object):
	"""
	Class to manage the sending and harvesting
	of jobs in the cluster
	"""
	precompile = False
	def __init__(self,status,**keywords):
		"""
		Initializes
		"""
		import os
		import sys
		
		validkeys = [ 'dataname', 'cfgfile', 'njobs', 'precompile' ]
		for key,value in keywords.iteritems():
			if key not in validkeys:
				message = "\nclustermanager: ERROR Not a valid argument '"+key+\
						"' to instanciate the class\n"
				sys.exit(message)
			
			if key == 'dataname':
				self.dataname = value # FIXME: Put the correct name
			elif key == 'cfgfile':
				#Checking is a file and can be find it
				if not os.path.exists(value):
					message = "\nclustermanager: ERROR Not found '"+value+"'.\n"
					sys.exit( message )
				self.cfgfile = os.path.abspath(value)
			elif key == 'njobs':
				self.njobs = value
			elif key == 'precompile':
				self.precompile = True
		
		# Extract the total number of events and split 
		self.jobidevt = self.getevents(value)
		self.status = status
		if self.status == "submit":
			self.submit()


	def submit(self):
		"""Submit the jobs
		"""
		import os
		import sys
		# Checking we have the datamembers initialized: FIXME
		#Creacion del directorio de trabajo
		launchDir = os.getenv( 'PWD' )
		executedir=os.path.join(launchDir,"cluster_"+self.dataname)
		try:
			os.mkdir( executedir )
		except OSError:
			# FIXME
			message  = "\nclustermanager: ERROR I cannot create the directory '"+executedir+"'"
			message += "\nPossibly the job is currently in use. Remove the directory if you "
			message += "want to send it again.\n"
			# FIXME: Comprobar si hay jobs activos---> usa status method
			sys.exit(message)
		# Moving to that dir
		os.chdir( executedir )
		# Extract the number of events per job
		jobsid = []
		for i,evttuple in self.jobidevt:
			# Splitting the config file in the number of jobs
			iconfig = self.createconfig(i,evttuple)
			# Creating the bash script to send the job
			if self.precompile:
				# Send the previous job to compile and update the code
				# create job to compile
				jobnames = createbash('datamanagercreator',self.dataname,self.cfgfile)
			else:
				jobnames = createbash('runanalysis',self.dataname,self.cfgfile)
			# sending the jobs
			for job in jobnames:
				jobsid.append( sendjob(job) )
		
		os.chdir(launchDir)
		print ""
		print "The jobs have been sended: "
		for i in jobsid:
			print i,
		print ""
		print "You can observe the status using 'qstat -u "+os.getenv("USER")+"'"
		print "Good Luck!"
		print ""
		
	def getevents(self,jobsNumber):
		"""Extract the number of events of a dataname
		and split them in the jobsNumber
		WARNING: As we are using possibly the 2.4
		we don't have ROOT so I used a wrapper in C++
		to extract it
		Returns a list of tuples of [(jobid,(Evt0,EvtN)),...]
		"""
		from subprocess import Popen,PIPE

		# Extract the files
		f = open(self.cfgfile)
		lines = f.readlines()
		datafiles = []
		for l in lines:
			if ".root" in l:
				datafiles.append( l )
		# Binary to extract the entries
		p = Popen( [ 'ls' ],stdout=PIPE,stderr=PIPE ).communicate()
		print p

		return []



def sendjob(bashscript):
	"""
	Send to the cluster the bash script input
	Return the job id 
	"""


def createconfig(cfgfile,jobNumber,evtTuple):
	"""
	From a reference cfgfile, constructs a new config file
	with the tuple of events to be analysed.
	Returns the new of the config (str)
	"""
	pass


def createbash(executable,dataname,cfgfile,jobsNumber):
	"""
	Create the bash scripts to be sended to the cluster
	Return a list of bash script names (str)
	"""
	import os
	import stat

	lines  = "#!\\bin\\bash\n"
	lines += "\n# Script created automatically by sendcluster.py utility\n"
	lines += "\n\n"+executable+" "+dataname+" -c "+cfgfile+"\n"

	for i in xrange(jobsNumber):
		filename = dataname+"_"+str(i)+".sh"
		f = open(filename,"w")
		f.writelines(lines)
		f.close()

		os.chmod(filename,stat.S_IRWXU+stat.S_IRGRP+stat.S_IXGRP+stat.S_IXOTH)

	return []


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
	parser.add_option( '-a', '--action', action='store', type='string', dest='action', help="Action to proceed: submit, harvest" )
	parser.add_option( '-d', '--dataname',  action='store', type='string', dest='dataname', help='Name of the data (see runanalysis)' )
	parser.add_option( '-j', '--jobs',  action='store', type='int',    dest='jobsNumber', help='Number of jobs' )
	parser.add_option( '-c', '--cfg' ,  action='store', type='string', dest='config', help='name of the config file (absolute path)' )
	parser.add_option( '-p', '--precompile',action='store_true', dest='shouldCompile', help='Set if exist a previous job to do compilation' \
			' (launching datamanager executable)' )
	
	( opt, args ) = parser.parse_args()
	

	#Archivo de configuracion obligatorio:
	configabspath = ''
	if opt.config is None:
		message = "\nsendcluster: ERROR the '-c' option is mandatory.\n"
		sys.exit( message )
	else:
		#Checking is a file and can be find it
		if not os.path.exists(opt.config):
			message = "\nsendcluster: ERROR Not found '"+opt.config+"'.\n"
			sys.exit( message )
		configabspath = os.path.abspath(opt.config)
	#Dataname obligatorio:
	if opt.dataname is None:
		message = "\nsendcluster: ERROR the '-d' option is mandatory.\n"
		sys.exit( message )
	# Not implemented yet
	if opt.jobsNumber != 1:
		message = "WARNING: Not implemented yet splitting jobs. Forcing '-j 1'"
		opt.jobsNumber = 1

	# Instanciation of the class manager depending the action to be done
	if opt.action is None:
		message = "\nsendcluster: ERROR the '-a' option is mandatory: 'submit' 'harvest'\n"
		sys.exit( message )

	manager = None
	if opt.action == 'submit':
		manager = clustermanager('submit',dataname=opt.dataname,\
				cfgfile=configabspath,njobs=opt.jobsNumber)

		#manager.submit()






