#!/usr/bin/env python

"""
Python script to send jobs to cluster
"""

class clustermanager(object):
	"""
	Class to manage the sending and harvesting
	of jobs in the cluster
	"""
	def __init__(self,status,**keywords):
		"""
		Initializes
		"""
		import os
		import sys
		
		validkeys = [ 'dataname', 'cfgfile', 'njobs', 'precompile', 'packagepath',\
				'workingdir' ]
		self.pkgpath = "."
		self.precompile = False
		self.outputfiles= {}
		for key,value in keywords.iteritems():
			if key not in validkeys:
				message = "\nclustermanager: ERROR Not a valid argument '"+key+\
						"' to instanciate the class\n"
				sys.exit(message)
			
			if key == 'dataname':
				self.dataname = value
				self.originaldataname = value
				if 'WH' in value:
					self.dataname = self.dataname.replace("WH","WHToWW2L")
			elif key == 'cfgfile':
				#Checking is a file and can be find it
				if not os.path.exists(value):
					message = "\nclustermanager: ERROR Not found '"+value+"'.\n"
					sys.exit( message )
				self.cfgfile = os.path.abspath(value)
			elif key == 'njobs':
				self.njobs = int(value)
			elif key == 'precompile':
				self.precompile = True
			elif key == 'packagepath':
				# Check if exist the path and it is correct
				if not os.path.exists( value ):
					message = "\nclustermanager: ERROR Not found the analysis " \
							"package '"+value+"'\n"
					sys.exit( message )
				if not os.path.exists( os.path.join(value,"interface/AnalysisBuilder.h") ):
					message = "\nclustermanager: ERROR the path introduced '" \
							+value+"' do not contain the header interface/AnalysisBuilder.h\n"
					sys.exit( message )
				self.pkgpath = value
			elif key == 'workingdir':
				self.cwd = value
		
		self.status = status
		
		if self.status == "submit":
			# Check if we have the names of the data files
			filedatanames = self.dataname+"_datanames.dn"
			if not os.path.exists(filedatanames):
				# if not created previously
				message  = "\nclustermanager: I need the list of file names, execute:"
				message += "\n'datamanager "+self.originaldataname+" -c "+self.cfgfile+"'"
				message += "\nAnd then launch again this script\n"
				sys.exit(message)
			# Extract the total number of events and split 
			self.nevents = self.getevents(filedatanames)
			# Checking if has sense the njobs
			if self.nevents/10 < self.njobs:
				message = "clustermanager: WARNING the Number of jobs introduced '"\
						+str(self.njobs)+"' make no sense: changing to 10 "
				print message
				self.njobs = 10
			evtperjob = self.nevents/self.njobs 
			remainevt  = self.nevents % self.njobs
			self.jobidevt = []
			for i in xrange(self.njobs-1):
				self.jobidevt.append( (i,(i*evtperjob,(i+1)*evtperjob-1)) )
			# And the last
			self.jobidevt.append( (self.njobs-1,\
					((self.njobs-1)*evtperjob,self.njobs*evtperjob+remainevt ) ) )
			# Submit the jobs
			self.submit()

		if self.status == "harvest":
			# Going to the working directory
			os.chdir(self.cwd)
			# And update with the last used datamembers
			self.retrieve()

			foundoutfiles = []
			for id,bashscript in self.jobsid:
				foundoutfiles.append( self.checkjob(id) )
			
			# If we have all the outputfiles we can gathering
			if foundoutfiles == self.outputfiles:
				self.gatherfiles()

	def gatherfiles(self):
		"""Gather all the outputfiles in one
		"""
		from subprocess import Popen,PIPE
		import os
		
		print "Joining all the files in one: "
		# FIXME: Only there are 1 file, not needed the hadd
		finalfile = os.path.join("Results",self.dataname)
		command = [ 'hadd', finalfile ]
		for f in self.outputfiles.itervalues():
			command.append( f )
		p = Popen( command ,stdout=PIPE,stderr=PIPE ).communicate()
		if p[1] != "":
			message = "\nclustermanager.gatherfiles: ERROR Something wrong with hadd\n"
			message += p[1]+"\n"
			sys.exit(message)

		else:
			print "Created "+finalfile
			print "========= Process Completed ========="
			#FIXME: REMOVE THE OLD FILES



	def checkjob(self, id):
		"""Check the status of a job
		"""
		from subprocess import Popen,PIPE
		import os
		
		print "Checking the job status:"
		command = [ 'qstat','-j',id ]
		p = Popen( command ,stdout=PIPE,stderr=PIPE ).communicate()
		# The job is done and disappeared from the scheluder
		if p[1] != "":
			# Checking if the outputfiles are there
			if not os.path.exists(self.outputfiles[self.jobsidID[id]]):
				message = "\nclustermanager.checkjob: Something went wrong in the cluster:\n"
				message += "The job '"+id+"' is already finish but there is no output root file '"
				message += self.outputfiles[self.jobsidID[id]]+"'\n"
				message += "Check the cluster output file\n"
				sys.exit(message)

			# Gathering the file outputs in order to add
			return self.outputfiles[self.jobsidID[id]]
		else:
			# Still in the cluster
			print "=== Job '"+id+"' is still in the cluster"


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
		jobnames = []
		for i,evttuple in self.jobidevt:
			# Splitting the config file in the number of jobs
			iconfigname = self.createconfig(i,evttuple)
			# Creating the bash script to send the job
			if self.precompile:
				# Send the previous job to compile and update the code
				# create job to compile
				jobnames.append( self.createbash(os.path.join(self.pkgpath,'datamanagercreator'),\
						iconfigname,i) )
				break
			else:
				jobnames.append( self.createbash(os.path.join(self.pkgpath,'runanalysis'),\
						iconfigname,i) )
		# sending the jobs
		jobsid = []
		# Storing the cluster job id with the number of job (ID)
		jobsidID= {}
		for bashname,job in jobnames:
			jobsid.append( self.sendjob(bashname) )
			jobsidID[jobsid[-1][0]] = job

		self.jobsid = jobsid
		self.jobsidID = jobsidID

		#persistency
		self.store()
		
		# Returning initial directory
		os.chdir(launchDir)
	
	def retrieve(self):
		"""Retrieve and populates the class from a file
		"""
		import shelve
		import sys

		d = shelve.open(".storedmanager")
		if not d.has_key("storedmanager"):
			message = "\nclustermanager.retrieve: ERROR Not found the" \
					+" class stored in .storedmanager file"
			sys.exit(message)

		copyself = d["storedmanager"]
		
		# Putting the datamembers: FIXME: If you want all the datanames
		# do it with the __dict__ and __setattr__ methods
		self.dataname    = copyself.dataname
		self.jobsid      = copyself.jobsid
		self.outputfiles = copyself.outputfiles
		self.njobs       = copyself.njobs
		self.jobsidID    = copyself.jobsidID
		
		d.close()


	def store(self):
		"""Store the relevant info after the submission
		in order to be retrieved at another stage
		"""
		import shelve

		d = shelve.open(".storedmanager",writeback=True)
		d["storedmanager"] = self

		d.close()

		
	def getevents(self,filedatanames):
		"""Extract the number of events of a dataname
		and split them in the jobsNumber
		WARNING: As we are using possibly the 2.4
		we don't have ROOT so I used a wrapper in C++
		to extract it
		Returns a list of tuples of [(jobid,(Evt0,EvtN)),...]
		"""
		from subprocess import Popen,PIPE
		import sys

		# Extract the file names (just need the first one)
		f = open(filedatanames)
		lines = f.readlines()
		f.close()
		datafiles = set([])
		for l in lines:
			if ".root" in l:
				# FIXME Find a way to deal with blablabl_bal_numbero.root
				datafiles.add( l.split(".root")[0] )
		# Binary to extract the entries
		command = [ '../bin/extractEvents' ]
		for f in datafiles:
			command.append( f+"*" )
		p = Popen( command ,stdout=PIPE,stderr=PIPE ).communicate()
		if p[1] != "":
			message = "\nclustermanager: ERROR from 'extractEvents':\n"
			message = p[1]+"\n"
			sys.exit(message)

		totalevts = p[0]
		return int(totalevts)
	
	def createconfig(self,jobNumber,evtTuple):
	        """From a reference cfgfile, constructs a new config file
		with the tuple of events to be analysed.
		Returns the new of the config (str)
		"""
		import sys
		# Reading the config file
		f = open(self.cfgfile)
		lines  = f.readlines()
		f.close()
		newlines = []
		for l in lines:
			newlines.append( l )
			if 'nEvents' in l:
				newlines[-1] = "@var int  nEvents "+str(evtTuple[1])+";\n"
			if 'firstEvent' in l:
				newlines[-1] = "@var int  firstEvent "+str(evtTuple[0])+";\n"
		
		newcfgnamePROV = os.path.basename(self.cfgfile)
		newcfgname = newcfgnamePROV.replace( ".", "_"+str(jobNumber)+"." )
		f = open( newcfgname, "w" )
		f.writelines( newlines )
		f.close()

		return newcfgname
	
	def createbash(self,executable,cfgname,jobnumber):
		"""Create the bash scripts to be sended to the cluster
		Return a list of (bash script names (str),jobnumber)
		"""
		import os
		import stat
		
		outputname = os.path.join("Results",self.dataname+"_"+str(jobnumber)+".root")
		self.outputfiles[jobnumber] = outputname

		lines  = "#!\\bin\\bash\n"
		lines += "\n# Script created automatically by sendcluster.py utility\n"
		lines += "\nmkdir -p Results\n"
		lines += executable+" "+self.dataname+" -c "+cfgname+" -o "+outputname+"\n"
	
		filename = self.dataname+"_"+str(jobnumber)+".sh"
		f = open(filename,"w")
		f.writelines(lines)
		f.close()
		os.chmod(filename,stat.S_IRWXU+stat.S_IRGRP+stat.S_IXGRP+stat.S_IXOTH)
		
		return (filename,jobnumber)

	def sendjob(self,bashscript):
	   	"""Send to the cluster the bash script input
		Return the job id 
		"""
		from subprocess import Popen,PIPE
		
		# OJO NO HE PUESTO REQUERIMIENTOS DE MEMORIA, CPU...
		print "Sending to cluster:"+bashscript
		command = [ 'qsub','-V','-cwd','-S','/bin/bash', \
				'-P','l.gaes','-l','h_rt=00:10:00',bashscript ]
		p = Popen( command ,stdout=PIPE,stderr=PIPE ).communicate()
		if p[1] != "":
			message = "\nclustermanager: ERROR from 'qsub':\n"
			message += p[1]+"\n"
			sys.exit(message)
		
		#Extract id
		id = None
		outputline = p[0].split(' ')
		for i in outputline:
			if i.isdigit():
				id = i
		return (id,bashscript)



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
	parser.set_defaults(shouldCompile=False,jobsNumber=10)
	parser.add_option( '-a', '--action', action='store', type='string', dest='action', help="Action to proceed: submit, harvest" )
	parser.add_option( '--wd', action='store', type='string', dest='workingdir', help="Working directory used with the '-a harvest' option")
	parser.add_option( '-d', '--dataname',  action='store', type='string', dest='dataname', help='Name of the data (see runanalysis)' )
	parser.add_option( '-j', '--jobs',  action='store', type='int',    dest='jobsNumber', help='Number of jobs' )
	parser.add_option( '-c', '--cfg' ,  action='store', type='string', dest='config', help='name of the config file (absolute path)' )
	parser.add_option( '-p', '--precompile',action='store_true', dest='shouldCompile', help='Set if exist a previous job to do compilation' \
			' (launching datamanager executable)' )
	
	( opt, args ) = parser.parse_args()
	

	#Archivo de configuracion obligatorio:
	configabspath = ''
	# Instanciation of the class manager depending the action to be done
	if opt.action is None:
		message = "\nsendcluster: ERROR the '-a' option is mandatory: 'submit' 'harvest'\n"
		sys.exit( message )
	
	if opt.action == 'submit':
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
		# Instantiate and submit
		manager = None
		if opt.action == 'submit':
			manager = clustermanager('submit',dataname=opt.dataname,\
					cfgfile=configabspath,njobs=opt.jobsNumber)
	elif opt.action == 'harvest':
		if opt.workingdir is None:
			message = "\nsendcluster: ERROR the '--cw' option is mandatory.\n"
			sys.exit( message )

		if not os.path.exists(opt.workingdir):
			message = "\nsendcluster: ERROR the working path "+opt.workingdir \
					+"' does not exists\n"
			sys.exit( message )

		manager = clustermanager("harvest",workingdir=opt.workingdir)
	

