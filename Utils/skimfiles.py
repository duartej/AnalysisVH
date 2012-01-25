#!/usr/bin/env python

"""
Python script to send jobs to cluster
Exemple of usage:
python skimfiles.py -a submit -c ../analisiswh_mmm.ip -d WH120 -j 1 --pkgdir=../ --basedir=../../
python skimfiles.py -a harvest --wd=cluster_WHToWW2L120
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
		
		validkeys = [ 'inputfile', 'njobs', 'pkgdir', 'outputfile', \
				'workingdir', 'basedir', 'cutid', 'cutfile' ]
		self.precompile = False
		self.outputfiles= {}
		self.datanames = None
		# Trying to extract the env variables to define 
		# the path of the General package
		if os.getenv("VHSYS"):
			self.basedir = os.path.abspath(os.getenv("VHSYS"))
			self.libsdir = os.path.join(self.basedir,"libs")
			if not os.path.exists( os.path.join(self.basedir,"CutManager") ):
				message = "\033[31;1mclustermanager: ERROR\033[0m the path introduced '" \
						+value+"' is not the base directory for the package 'VHAnalysis'\n"
				raise message 
		# for the analysis specific package
		if os.getenv("ANALYSISSYS"):
			self.pkgpath = os.path.abspath(os.getenv("ANALYSISSYS"))
			if not os.path.exists( os.path.join(self.pkgpath,"interface/AnalysisBuilder.h") ):
				message = "\033[31;1mclustermanager: ERROR\033[0m the path introduced '" \
						+value+"' do not contain the header interface/AnalysisBuilder.h\n"
				raise message 

		for key,value in keywords.iteritems():
			if key not in validkeys:
				message = "\nclustermanager: ERROR Not a valid argument '"+key+\
						"' to instanciate the class\n"
				raise message
			
			if key == 'inputfile':
				self.datanames = value
				self.nameID = self.datanames.split("_")[0].replace("/","").replace(".","")
			elif key == 'njobs':
				self.njobs = int(value)
			elif key == 'pkgdir':
				if not value:
					continue
				# Check if exist the path and it is correct
				if not os.path.exists( os.path.join(value,"interface/AnalysisBuilder.h") ):
					message = "\033[1;31mclustermanager: ERROR\033[1;m the path introduced '" \
							+value+"' do not contain the header interface/AnalysisBuilder.h"
					raise message
				self.pkgpath = os.path.abspath(value)
			elif key == 'basedir':
				if not value:
					continue
				# Check if exist the path and it is correct
				if not os.path.exists( os.path.join(value,"CutManager") ):
					message = "\033[1;31mclustermanager: ERROR\033[1;m the path introduced '" \
							+value+"' is not the base directory"
					raise message 
				self.basedir = os.path.abspath(value)
				self.libsdir = os.path.join(self.basedir,"libs")
			elif key == 'workingdir':
				self.cwd = os.path.abspath(value)
			elif key == 'outputfile':
				#FIXME: CHeck if value == ''
				self.outputfile = value
			elif key == 'cutid':
				self.cutid = value
			elif key == 'cutfile':
				self.cutfile = value
		
		# Checked if basedir and pkgpath are there
		try:
			dummy = self.basedir
		except AttributeError:
		        message  = "\033[31;1mclustermanager: ERROR\033[0m You have to introduce the base directory for the package VHAnalysis"
			message += "Or you can use an environment variable: 'export VHSYS=path'"
			raise message
		try:
			dummy = self.pkgpath
		except AttributeError:
		        message  = "\033[31;1mclustermanager: ERROR\033[0m You have to introduce the base analysis directory"
			message += "Or you can use an environment variable: 'export ANALYSISSYS=path'"
			raise message


		self.status = status
		
		if self.status == "submit":
			# Extract the total number of events and split 
			self.nevents = self.getevents(self.datanames)
			# We want some thing quick, the estimation is between 500-1000 e/sec,
			# so we are trying to send 10minutes-jobs: ~450000 evt per job  (changed to 300000-> New calculations)
			if self.njobs == 0:
				message = "\033[34;1mclustermanager: INFO\033[0m Guessing the number of tasks "\
						+"to send 10 minutes jobs. Found: "
				self.njobs = self.nevents/300000   #450000 
				message += str(self.njobs)
				print message
			# Checking if has sense the njobs
			if self.njobs < 1:
				message = "\033[33;1mclustermanager: WARNING\033[0m the Number of jobs introduced '"\
						+str(self.njobs)+"' make no sense: changing to 1 "
				print message
				self.njobs = 1
			evtperjob = self.nevents/self.njobs 
			# Recall: the first event is 0, the last event: Entries-1
			remainevt  = (self.nevents % self.njobs)-1 
			self.jobidevt = []
			self.tasksID  = []
			for i in xrange(self.njobs-1):
				self.jobidevt.append( (i+1,(i*evtperjob,(i+1)*evtperjob-1)) )
				self.tasksID.append( i+1 )
			# And the last
			self.tasksID.append( self.njobs )
			self.jobidevt.append( (self.njobs,\
					((self.njobs-1)*evtperjob,self.njobs*evtperjob+remainevt ) ) )
			# Submit the jobs
			self.submit()

		if self.status == "harvest":
			# Going to the working directory
			os.chdir(self.cwd)
			# And update with the last used datamembers
			self.retrieve()

			foundoutfiles = []
			print "clustermanager: Checking the job status"
			
			self.taskstatus = { 'r': [], 'qw': [], 'Undefined': [], 'Done': []}
			for taskid,dummy in self.jobidevt:
				foundoutfiles.append( self.checkjob(taskid) )
			# Print status 
			havetoprint = False
			for l in self.taskstatus.itervalues():
				if len(l) != 0:
					havetoprint = True
					break
			if havetoprint:
				getcolor = lambda x,color: "\033[1;"+str(color)+"m"+x+"\033[1;m"
				outputmessage = ''
				textstatusdict = { "r": getcolor("Running",32), "qw": getcolor("Queued",30), \
						"Undefined": getcolor("Undefined",35), "Done": getcolor("Done",34) }
				for status,tasklist in self.taskstatus.iteritems():
					if len(tasklist) == 0:
						continue
					outputmessage += "   "+textstatusdict[status]+": ["
					for task in set(tasklist):
						outputmessage += str(task)+","
					outputmessage = outputmessage[:-1]+"]\n"
				print outputmessage
			
			# If we have all the outputfiles we can gathering
			if foundoutfiles == self.outputfiles.values():
				self.gatherfiles()

		if self.status == "delete":
			# Going to the working directory
			os.chdir(self.cwd)
			# And update with the last used datamembers
			self.retrieve()
			self.delete()

	
	def delete(self):
		"""Delete the job"""
		from subprocess import Popen,PIPE
		import sys
		import os
		import shutil

		print "\033[1;37mDeleting the '"+self.dataname+"' job:\033[1;m "+self.jobsid+" and data .",
		sys.stdout.flush()
		command = [ 'qdel',str(self.jobsid) ]
		p = Popen( command ,stdout=PIPE,stderr=PIPE ).communicate()
		print ".",
		sys.stdout.flush()
		os.chdir("..")
		shutil.rmtree(self.cwd)
		print ".",
		sys.stdout.flush()
		print " DONE!  "


	def gatherfiles(self):
		"""Gather all the outputfiles in one
		"""
		from subprocess import Popen,PIPE
		import os
		import tarfile
		import glob
		
		print "=== ",self.dataname,": Joining all the files in one"
		# FIXME: Only there are 1 file, not needed the hadd
		finalfile = os.path.join("Results",self.outputfile)
		# FIXED BUG: just cp when there is only one file, otherwise
		#            there are problems with the TTree
		if len(self.outputfiles) == 1:
			# Note that when there is only 1 file, always its #task=1
			command = [ 'cp', self.outputfiles[1], finalfile ]
		else:
			command = [ 'haddPlus', finalfile ]
			for f in self.outputfiles.itervalues():
				command.append( f )
		p = Popen( command ,stdout=PIPE,stderr=PIPE ).communicate()
		# Checking if everything was allright
		totalevts = self.getevents(finalfile,True)
		if totalevts != self.nevents:
			message  = "\033[33;1mclustermanager.gatherfiles: WARNING\033[0m the total file"
			message += "'"+finalfile+"' do not contain all the events:\n"
			message += "Total events to be processed:"+str(self.nevents)+"\n"
			message += "Total events in '"+finalfile+"':"+str(totalevts)+"\n"
			print message
			return 
		# If everything was fine, deleting the files 
		# and cleaning the directory
		for f in self.outputfiles.itervalues():
			os.remove( f )
		# Taring and compressing
		filestotar = glob.glob("./*.*")
		filestotar.append( ".storedmanager")
		tar = tarfile.open(os.path.basename(self.cwd)+".tar.gz","w:gz")
		for f in filestotar:
			tar.add(f)
		tar.close()
		# if everything was fine, deleting the files
		if os.path.exists(os.path.basename(self.cwd)+".tar.gz"):
			for f in filestotar:
				os.remove(f)
		else:
			message  = "\033[33;1mclustermanager.gatherfiles: WARNING\033[0m I can't manage\n"
			message += "to create the backup .tar.gz file\n"
			print message

		print "Created "+finalfile
		print "========= Process Completed ========="


	def checkjob(self, taskid):
		"""Check the status of a job
		"""
		from subprocess import Popen,PIPE
		import os

		try:
			p = self.qstatoutput
		except:
			#command = [ 'qstat','-j',id ]
			command = [ 'qstat','-u',os.getenv("USER"),'-g','d' ]
			p = Popen( command ,stdout=PIPE,stderr=PIPE ).communicate()
			self.qstatoutput = p

		isincluster = False
		taskstatus = {}
		for line in p[0].split("\n"):
			if not str(self.jobsid) in line:
				continue
			parseline = line.split()
			status= parseline[4]
			try:
				task  = int(parseline[9])
			except IndexError:
				# Implies it is still waiting
				task  = int(parseline[8])
			taskstatus[task] = status
			isincluster = True

		if not isincluster:
			# Checking if the outputfiles are there
			if not os.path.exists(self.outputfiles[taskid]):
				message = "\033[1;31mclustermanager.checkjob: Something went wrong in the cluster:\033[1;m"
				message += "The task '"+str(taskid)+"' of the job '"+str(self.jobsid)
				message += "' is already finish but there is no output root file '"
				message += self.outputfiles[taskid]+"'\n"
				message += "Check the cluster outputs file"
				raise message

			# Gathering the file outputs in order to add
			self.taskstatus["Done"].append(taskid)
			return self.outputfiles[taskid]

		# Still in cluster
		#statustaskdict = dict( [ (status,[]) for status in taskstatus.values() ] )
		for task,status in taskstatus.iteritems():
			if status == "r" or status == "t":
				self.taskstatus["r"].append(task)
			elif status == "qw":
				self.taskstatus["qw"].append(task)
			else:
				self.taskstatus["Undefined"].append(task)

	def submit(self):
		"""Submit the job
		"""
		import os
		import sys
		#Creacion del directorio de trabajo
		launchDir = os.getenv( 'PWD' )
		self.cwd=os.path.join(launchDir,"cluster_"+self.nameID)
		try:
			os.mkdir( self.cwd )
		except OSError:
			# FIXME
			message  = "\033[1;31mclustermanager: ERROR\033[1;m I cannot create the directory '"+self.cwd+"'"
			message += "\nPossibly the job is currently in use. Remove the directory if you "
			message += "want to send it again."
			# FIXME: Comprobar si hay jobs activos---> usa status method
			raise message
		# Moving to that dir
		os.chdir( self.cwd )

		# FIXME: DEPENDENT!!!
		lines = 'LINE 0: NOT TO USE\n'
		# Extract the number of events per job
		for i,evttuple in self.jobidevt:
			# Splitting the config file in the number of jobs
			lines = self.createconfig(lines,evttuple)
			# Storing the name of the files
			self.outputfiles[i] = self.outputfile.replace(".root","_"+str(i)+".root")
		# File containing the event init and event end to be evaluated 
		# in each row corresponding to the jobid
		self.eventsfile = "seedevents.txt"
		f = open(self.eventsfile,"w")
		f.writelines(lines)
		f.close()
		# Creating the bash script to send the job 
		bashscriptname = self.createbash('skimfile')
		
		# Sending the job and storing the cluster job id
		self.jobsid = self.sendjob(bashscriptname)

		#persistency
		self.store()
		
		# Returning initial directory
		os.chdir(launchDir)
	
	def retrieve(self):
		"""Retrieve and populates the class from a file
		"""
		import shelve
		import sys
		import glob

		d = shelve.open(".storedmanager")
		if not d.has_key("storedmanager"):
			# Check if already is done the file
			if len(glob.glob("*.tar.gz")) != 0:
				message = "clustermanager.retrive: The job is already DONE!"
			else:
				message = "\nclustermanager.retrieve: ERROR Not found the" \
					+" class stored in .storedmanager file"
			sys.exit(message)

		copyself = d["storedmanager"]
		
		# Putting the datamembers: FIXME: If you want all the datanames
		# do it with the __dict__ and __setattr__ methods
		self.dataname     = copyself.dataname
		self.jobsid       = copyself.jobsid
		self.jobidevt     = copyself.jobidevt
		self.tasksID      = copyself.tasksID
		self.outputfiles  = copyself.outputfiles
		self.njobs        = copyself.njobs
		self.basedir      = copyself.basedir
		self.pkgpath      = copyself.pkgpath
		self.libsdir      = copyself.libsdir
		self.filedatanames= copyself.filedatanames
		self.nevents      = copyself.nevents
		try:
			self.taskstatus   = copyself.taskstatus
		except AttributeError:
			# It means we have not yet done a previous harvest
			pass
		
		d.close()


	def store(self):
		"""Store the relevant info after the submission
		in order to be retrieved at another stage
		"""
		import shelve

		d = shelve.open(".storedmanager",writeback=True)
		d["storedmanager"] = self

		d.close()

		
	def getevents(self,filedatanames,direct=False):
		"""Extract the number of events of a dataname
		and split them in the jobsNumber.
		There are two modes:
		 1. direct = True
		    The arg. 'filedatanames' is directly the 
		    file to inspect
		 2. direct = False
		    The arg. 'filedatanames' is the path of 
		    a files containing the paths of the root
		    files to inspect
		WARNING: As we are using possibly the 2.4
		we don't have ROOT so I used a wrapper in C++
		to extract it
		Returns the total number of events
		"""
		from subprocess import Popen,PIPE
		import sys
		import os

		# Extract the file names: Two modes:
		# the arg filedatanames is directly the file to inspect
		if direct:
			datafiles = set([os.path.abspath(filedatanames)])
		# the arg filedatanames contain a list of files to inspect
		else:
			f = open(filedatanames)
			lines = f.readlines()
			f.close()
			datafiles = set([])
			for l in lines:
				if ".root" in l:
					datafiles.add(l.replace("\n",""))
		
		# Binary to extract the entries
		binexe = os.path.join(self.basedir,"bin/extractEvents")
		command = [ binexe ]
		# Initialize the inputfiles
		self.inputfiles = []
		for f in datafiles:
			command.append( f )
			self.inputfiles.append(f)
		# The flag to evaluate processed file
		if direct:
			command.append( "-h" )
		p = Popen( command ,stdout=PIPE,stderr=PIPE ).communicate()
		if p[1] != "":
			message = "\nclustermanager: ERROR from 'extractEvents':\n"
			message = p[1]+"\n"
			raise message

		totalevts = p[0]
		# Just checking the exponential format
		return int(totalevts)
	
	def createconfig(self,line,evtTuple):
	        """From a reference cfgfile, constructs a new config file
		with the tuple of events to be analysed.
		FIXME: DEPENDENT
		"""
		line += str(evtTuple[0])+","+str(evtTuple[1])+"\n"

		return line
	
	def createbash(self,executable,**keywords):
		"""Create the bash scripts to be sended to the cluster
		Return the script filename
		FIXME: DEPENDENT
		"""
		import os
		import stat

		outputname = os.path.join("Results",self.outputfile.replace(".root","_${SGE_TASK_ID}.root"))
		# Extract the input files
		inputfiles = ""
		for f in self.inputfiles:
			inputfiles += f+","
		inputfiles = inputfiles[:-1]

		lines  = "#!/bin/bash\n"
		lines += "\n# Script created automatically by skimfiles.py utility\n"
		lines += "\nmkdir -p Results\n"
		lines += "export PATH=$PATH:"+os.path.join(self.basedir,"bin")+":"+os.path.join(self.pkgpath,"bin")+"\n"
		lines += "export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:"+self.libsdir+"\n"
		lines += "\n"
		lines += "EVENTFILE="+self.eventsfile+"\n"
		lines += "EVENTS=$(cat $EVENTFILE | head -n $SGE_TASK_ID | tail -n 1)\n"
		lines += executable+" "+self.cutid+" -i "+inputfiles+" -c "+self.cutfile+\
				" -e $EVENTS -o "+outputname+"\n"
	
		filename = self.nameID+".sh"
		f = open(filename,"w")
		f.writelines(lines)
		f.close()
		os.chmod(filename,stat.S_IRWXU+stat.S_IRGRP+stat.S_IXGRP+stat.S_IXOTH)
		
		return filename

	def sendjob(self,bashscript):
	   	"""Send to the cluster the bash script input
		Return the job id 
		"""
		from subprocess import Popen,PIPE
		
		# Recall map is ordered 
		taskidinit = self.jobidevt[0][0]
		taskidend  = self.jobidevt[-1][0]
		# OJO NO HE PUESTO REQUERIMIENTOS DE MEMORIA, CPU...
		print "Sending to cluster: "+bashscript
		command = [ 'qsub','-V','-cwd','-S','/bin/bash', \
				'-t',str(taskidinit)+"-"+str(taskidend),\
				'-P','l.gaes','-l', 'immediate', '-l','h_rt=02:00:00',bashscript ]
		p = Popen( command ,stdout=PIPE,stderr=PIPE ).communicate()
		if p[1] != "":
			message = "\033[1;31mclustermanager: ERROR\033[1;m from 'qsub':\n"
			message += p[1]+"\n"
			raise message
		
		#Extract id
		id = None
		outputline = p[0].split(' ')
		for i in outputline:
			if i.isdigit():
				id = i
			if len(i.split(":")) == 2:
				id = i.split(".")[0]
		return id


if __name__ == '__main__':
	import sys
	import os
	import glob
	from optparse import OptionParser,OptionGroup

	#Comprobando la version (minimo 2.4)
	vX,vY,vZ,_t,_t1 = sys.version_info
	if vX > 2 and vY < 4:
		message = 'skimfiles: I need python version >= 2.4'
		sys.exit( message )
	elif vX < 2:
		message = 'skimfiles: I need python version >= 2.4'
		sys.exit( message )
	#Opciones de entrada
	usage="usage: skimfiles <submit|harvest|delete> [options]"
	parser = OptionParser(usage=usage)
	parser.set_defaults(jobsNumber=0)
	parser.add_option( '--pkgdir', action='store', type='string', dest='pkgdir', help="Analysis package directory (where the Analysis live)")
	parser.add_option( '--basedir', action='store', type='string', dest='basedir', help="Complete package directory")
	
	group = OptionGroup(parser, "submit options","")
	group.add_option( '-i', '--input', action='store', type='string', dest='inputfile', help="The name of the .dn file which contain the list" \
			" of root files to be skimmed (see runanalysis code and sendcluster script)" )
	group.add_option( '-o', '--output', action='store', type='string', dest='outputfile', help="The input file after the processing "\
			"(i.e. the name of the output file")
	group.add_option( '-c', '--cutfile',  action='store', type='string', dest='cutfile', help='The name of the root file containing the cuts. This file'\
			' is used to skim the INPUTFILE with the events which pass the cut CUTID')
	group.add_option( '-t', '--threshold' ,  action='store', type='string', dest='cutid', help='Cut threshold, each event of INPUTFILE has to have'\
			' an cut greater or equal CUTID in its "friend" equivalent file CUTFILE' )
	group.add_option( '-j', '--jobs',  action='store', type='int',    dest='jobsNumber', help='Number of jobs. Not using this option, the script'\
			' will trying to found how many jobs are needed to create a 10min job')
	parser.add_option_group(group)

	groupharvest = OptionGroup(parser,"harvest and delete options","")
	groupharvest.add_option( '-w', '--wd', action='store', type='string', dest='workingdir', help="Working directory used with the '-a harvest' option")
	parser.add_option_group(groupharvest)
	
	( opt, args ) = parser.parse_args()

	if len(args) == 0 :
		message = "\033[31;1mskimfiles: ERROR\033[0m Note that is mandatory an action: 'submit' 'harvest'"
		message += "\nSee usage: "+parser.usage
		sys.exit(message)

	print "\033[1;31m*********************************************************\033[1;m"
	print "\033[1;31m*WARNING*WARNING*WARNING*WARNING*WARNING*WARNING*WARNING*\033[1;m"
	print "\033[1;31m*\033[1;m        This code is not stable. Use with caution      \033[1;31m*\033[1;m"
	print "\033[1;31m*WARNING*WARNING*WARNING*WARNING*WARNING*WARNING*WARNING*\033[1;m"
	print "\033[1;31m*********************************************************\033[1;m"
	
	if args[0] == 'submit':
		# Inputfile mandatory:
		if not opt.inputfile:
			message = "\033[31;1mskimfiles: ERROR\033[0m the '-i' option is mandatory"
			sys.exit( message )
			
		# Also cut file:
		if not opt.cutfile:
			message = "\033[31;1mskimfiles: ERROR\033[0m the '-c' option is mandatory"
			sys.exit( message )
		# Also cut id
		if not opt.cutid:
			message = "\033[31;1mskimfiles: ERROR\033[0m the '-t' option is mandatory"
			sys.exit( message )

		if not opt.outputfile:
			opt.outputfile = opt.inputfile.split('.root')[0]+"_skimcuts.root"

		# Instantiate and submit
		manager = None
		manager = clustermanager('submit',inputfile=opt.inputfile,\
				njobs=opt.jobsNumber, pkgdir=opt.pkgdir,\
				basedir=opt.basedir,cutfile=opt.cutfile, \
				cutid=opt.cutid,outputfile=opt.outputfile)

	elif args[0] == 'harvest':
		if opt.workingdir is None:
			message = "\033[31;1mskimfiles: ERROR\033[0m the '--cw' option is mandatory"
			sys.exit( message )
		
		if not os.path.exists(opt.workingdir):
			message = "\033[31;1mskimfiles: ERROR\033[0m the working path '"+opt.workingdir \
					+"' does not exists"
			sys.exit( message )
		
		manager = clustermanager("harvest",workingdir=opt.workingdir)
	
	elif args[0] == 'delete':
		if opt.workingdir is None:
			message = "\033[31;1mskimfiles: ERROR\033[0m the '--cw' option is mandatory"
			sys.exit( message )
		
		if not os.path.exists(opt.workingdir):
			message = "\033[31;1mskimfiles: ERROR\033[0m the working path '"+opt.workingdir \
					+"' does not exists"
			sys.exit( message )
		
		manager = clustermanager("delete",workingdir=opt.workingdir)
	
	else:
		message = "\033[31;1mskimfiles: ERROR\033[0mNot recognized the action '"+args[0]+"'"
		sys.exit( message )

	


