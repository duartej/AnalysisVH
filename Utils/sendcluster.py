#!/usr/bin/env python

"""
Python script to send jobs to cluster
Exemple of usage:
python sendcluster.py -a submit -c ../analisiswh_mmm.ip -d WH120 -j 1 --pkgdir=../ --basedir=../../
python sendcluster.py -a harvest --wd=cluster_WHToWW2L120
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
		
		validkeys = [ 'dataname', 'cfgfilemap', 'njobs', 'precompile', 'pkgdir',\
				'workingdir', 'basedir', 'finalstate', 'analysistype' ]
		self.precompile = False
		self.outputfiles= {}
		self.leptoncfgfilemap = {}
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
			
			if key == 'dataname':
				self.dataname = value
				self.originaldataname = value
				if 'WH' in value:
					self.dataname = self.dataname.replace("WH","WHToWW2L")
			elif key == 'cfgfilemap':
				#Checking is a file and can be find it
				for lepton,cfg in value.iteritems():
					if not os.path.exists(cfg):
						message = "\033[1;31mclustermanager: ERROR\033[1;m Not found '"+value+"'.\n"
						raise message
					if lepton.lower() != "muon" and lepton.lower() != "mu" \
							and lepton.lower() != "electron" and lepton.lower() != "elec":
						message = "\033[1;31mclustermanager: ERROR\033[1;m Not valid lepton assignation"
						message +=" to the config file '"+cfg+"'. Parsed:'"+lepton.lower()+"'."
						message += " Valid keys are: muon mu electron ele"
						raise message 
					self.leptoncfgfilemap[lepton] = os.path.abspath(cfg)
			elif key == 'njobs':
				self.njobs = int(value)
			elif key == 'precompile':
				self.precompile = True
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
			elif key == 'finalstate':
				self.finalstate = value
			elif key == 'analysistype':
				self.analysistype = value
		
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
			# Check if we have the names of the data files: FIXME: Nota que con la version
			# actual del runanalysis no necesito tener el dataname pues me lo busca el mismo !!??
			self.filedatanames = os.path.join( os.getenv( "PWD" ), self.dataname+"_datanames.dn" )
			if not os.path.exists(self.filedatanames):
				cfglist = [ lepton+":"+cfg for lepton,cfg in self.leptoncfgfilemap.iteritems() ]
				cfgstr = ''
				for i in cfglist:
					cfgstr += i+','
				cfstr = cfgstr[:-1]
				# if not created previously
				message  = "\033[31;1mclustermanager: ERROR\033[0m"
				message += " I need the list of file names, execute:"
				message += "\n'datamanager "+self.originaldataname+" -c "+cfgstr+"'"
				message += "\nAnd then launch again this script"
				raise message
			# Extract the total number of events and split 
			self.nevents = self.getevents(self.filedatanames)
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
			print "clustermanager: Checking the job status for dataname '"+self.dataname+"'"
			
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
		finalfile = os.path.join("Results",self.dataname+".root")
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
		# Adding up all the final results and show them
		textresultfiles = glob.glob("./"+self.dataname+"*.sh.o*")
		showresults(textresultfiles)
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

		#command = [ 'qstat','-j',id ]
		command = [ 'qstat','-u',os.getenv("USER"),'-g','d' ]
		p = Popen( command ,stdout=PIPE,stderr=PIPE ).communicate()

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
		# Checking we have the datamembers initialized: FIXME
		#Creacion del directorio de trabajo
		launchDir = os.getenv( 'PWD' )
		self.cwd=os.path.join(launchDir,"cluster_"+self.dataname)
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
		# Extract the number of events per job
		for i,evttuple in self.jobidevt:
			# Splitting the config file in the number of jobs
			iconfigname = self.createconfig(i,evttuple)
			# And storing the name of the files
			self.outputfiles[i] = os.path.join("Results",self.dataname+"_"+str(i)+".root")
		# Preparing the name of the config files to be  sended as a tasks
		# (Note that we can use whatever we want of iconfigname 'cause 
		# all have the same output
		self.cfgnames = iconfigname
		# Creating the bash script to send the job 
		if self.precompile:
			# Send the previous job to compile and update the code
			# create job to compile
			bashscriptname = self.createbash('datamanagercreator')
		else:
			bashscriptname = self.createbash('runanalysis',iconfigname)
		
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
		for f in datafiles:
			command.append( f )
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
	
	def createconfig(self,taskNumber,evtTuple):
	        """From a reference cfgfile, constructs a new config file
		with the tuple of events to be analysed.
		Returns the config as a string of LEPTON:config,LEPTON2:config
		non task dependent name
		"""
		import sys
		
		cfgnontaskdep = ''
		# Reading the config file
		for lepton,cfgfile in self.leptoncfgfilemap.iteritems():
			f = open(cfgfile)
			lines  = f.readlines()
			f.close()
			newlines = []
			nEvents = (evtTuple[1]-evtTuple[0])+1
			for l in lines:
				newlines.append( l )
				if 'nEvents' in l:
					newlines[-1] = "@var int  nEvents "+str(nEvents)+";\n"
				if 'firstEvent' in l:
					newlines[-1] = "@var int  firstEvent "+str(evtTuple[0])+";\n"
			
			newcfgnamePROV = os.path.basename(cfgfile)
			newcfgname = newcfgnamePROV.replace( ".", "_"+str(taskNumber)+"." )
			f = open( newcfgname, "w" )
			f.writelines( newlines )
			f.close()
			# We want return the general name, using the environment variable
			cfgnontaskdepPROV = newcfgnamePROV.replace( ".", "_${SGE_TASK_ID}." )
			cfgnontaskdep     += lepton+":"+cfgnontaskdepPROV+","

		cfgnontaskdep = cfgnontaskdep[:-1]
		return cfgnontaskdep
	
	def createbash(self,executable,config):
		"""Create the bash scripts to be sended to the cluster
		Return the script filename
		"""
		import os
		import stat

		outputname = os.path.join("Results",self.dataname+"_${SGE_TASK_ID}.root")
		#self.outputfiles[tasknumber] = outputname # ?

		lines  = "#!/bin/bash\n"
		lines += "\n# Script created automatically by sendcluster.py utility\n"
		lines += "\nmkdir -p Results\n"
		lines += "export PATH=$PATH:"+os.path.join(self.basedir,"bin")+":"+os.path.join(self.pkgpath,"bin")+"\n"
		lines += "export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:"+self.libsdir+"\n"
		lines += executable+" "+self.dataname+" -a "+self.analysistype+" -c "+self.cfgnames+\
				" -d "+self.filedatanames+" -l "+self.finalstate+" -o "+outputname+"\n"
	
		filename = self.dataname+".sh"
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
		
	
def showresults(textresultfiles):
	"""
	Getting a list of output files which contain 
	the pattern:
	
	number [number%] selected events (cutname)

	and add up all the results
	"""
	import re
	from math import log10
	totaldict = {}
	cutorder = []
	regexp = re.compile("(?P<total>\d+\.*\d*)\s*\[(?P<percent>\d*\W*\d*)\%\]\s*selected\sevents\s*\((?P<cutname>\w*)\)\s*")

	for file in textresultfiles:
		f = open(file)
		lines = f.readlines()
		for line in lines:
			try:
				total,percent,cutname = regexp.search(line).groups()
			except AttributeError:
				continue
			try:
				totaldict[cutname] += float(total)
			except KeyError:
				totaldict[cutname] = float(total)

			if cutorder.count(cutname) == 0:
				cutorder.append(cutname)

	# Print 
	print "Summary of cut selection of the total sample"
	print ""
	print "N. events selected at each stage:"
	print "---------------------------------"
	maxlenght = int(log10(totaldict["AllEvents"]))+2
	for cut in cutorder:
		percent = float(totaldict[cut])/float(totaldict["AllEvents"])*100.0
		if percent < 1:
			format = "[%.2f%s]"
		elif percent < 100:
			format = "[%.1f%s]"
		else:
			format = "[%4i%s]"
		
		totalformat = str(maxlenght)+".1f"
		if str(totaldict[cut]).split(".")[-1] == '0':
			totalformat = str(maxlenght)+"i"
		
		formatstr = "%"+totalformat+" "+format+" selected events (%s)"

		print formatstr % (totaldict[cut],percent,"%",cut)



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
	usage="usage: sendcluster <submit|harvest|delete> [options]"
	parser = OptionParser(usage=usage)
	parser.set_defaults(shouldCompile=False,jobsNumber=0)
	#parser.add_option( '-a', '--analysis', action='store', type='string', dest='antype', help="Analysis to be processed WZ|WH" )
	#parser.add_option( '-w', '--wd', action='store', type='string', dest='workingdir', help="Working directory used with the '-a harvest' option")
	#parser.add_option( '-f', '--finalstate', action='store', type='string', dest='finalstate', help="Final state signature: mmm eee mme eem")
	parser.add_option( '--pkgdir', action='store', type='string', dest='pkgdir', help="Analysis package directory (where the Analysis live)")
	parser.add_option( '--basedir', action='store', type='string', dest='basedir', help="Complete package directory")
	#parser.add_option( '-d', '--dataname',  action='store', type='string', dest='dataname', help='Name of the data (see runanalysis)' )
	#parser.add_option( '-j', '--jobs',  action='store', type='int',    dest='jobsNumber', help='Number of jobs' )
	#parser.add_option('-c', '--cfg' ,  action='store', type='string', dest='config', help='name of the lepton and config file (absolute path), \':\' separated' )
	#parser.add_option( '-p', '--precompile',action='store_true', dest='shouldCompile', help='Set if exist a previous job to do compilation' \
	#		' (launching datamanager executable)' )
	
	group = OptionGroup(parser, "submit options","")
	group.add_option( '-a', '--analysis', action='store', type='string', dest='antype', help="Analysis to be processed WZ|WH" )
	group.add_option( '-f', '--finalstate', action='store', type='string', dest='finalstate', help="Final state signature: mmm eee mme eem")
	group.add_option( '-d', '--dataname',  action='store', type='string', dest='dataname', help='Name of the data (see runanalysis -h). Also'
			' not using this option, the script will use all the datafiles *_datanames.dn found in the working directory to launch process')
	group.add_option( '-j', '--jobs',  action='store', type='int',    dest='jobsNumber', help='Number of jobs. Not using this option, the script'
			' will trying to found how many jobs are needed to create a 10min job')
	group.add_option( '-c', '--cfg' ,  action='store', type='string', dest='config', help='name of the lepton and config file (absolute path), \':\' separated' )
	group.add_option( '-p', '--precompile',action='store_true', dest='shouldCompile', help='Set if exist a previous job to do compilation' \
			' (launching datamanager executable)' )
	parser.add_option_group(group)

	groupharvest = OptionGroup(parser,"harvest and delete options","")
	groupharvest.add_option( '-w', '--wd', action='store', type='string', dest='workingdir', help="Working directory used with the '-a harvest' option")
	parser.add_option_group(groupharvest)
	
	( opt, args ) = parser.parse_args()

	if len(args) == 0 :
		message = "\033[31;1msendcluster: ERROR\033[0m Note that is mandatory an action: 'submit' 'harvest'"
		message += "\nSee usage: "+parser.usage
		sys.exit(message)
	#Archivo de configuracion obligatorio:
	configabspath = ''
	# Instanciation of the class manager depending the action to be done
	#if opt.action is None:
	#	message = "\033[31;1msendcluster: ERROR\033[0m the '-a' option is mandatory: 'submit' 'harvest'"
	#	sys.exit( message )
	
	#if opt.action == 'submit':
	if args[0] == 'submit':
		leptoncfgmap = {}
		if not opt.config:
			message = "\033[31;1msendcluster: ERROR\033[0m the '-c' option is mandatory"
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
					message = "\033[34;1msendcluster: ERROR\033[0m Not found '"+config+"'"
					sys.exit( message )
				configabspath = os.path.abspath(config)
				leptoncfgmap[lepton] = configabspath
		# Dataname mandatory:
		if not opt.dataname:
			# Obtaining all the datanames from files in the current directory
			message = "\033[34;1msendcluster: INFO\033[0m obtaining the datanames"\
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
		# Instantiate and submit
		manager = None
		for dataname in datanameslist:
			print "========= Dataname: %s" % dataname
			manager = clustermanager('submit',dataname=dataname,cfgfilemap=leptoncfgmap,\
					njobs=opt.jobsNumber, pkgdir=opt.pkgdir,\
					basedir=opt.basedir,finalstate=opt.finalstate, \
					analysistype=opt.antype)

	#elif opt.action == 'harvest':
	elif args[0] == 'harvest':
		if opt.workingdir is None:
			message = "\033[31;1msendcluster: ERROR\033[0m the '--cw' option is mandatory"
			sys.exit( message )
		
		if not os.path.exists(opt.workingdir):
			message = "\033[31;1msendcluster: ERROR\033[0m the working path '"+opt.workingdir \
					+"' does not exists"
			sys.exit( message )
		
		manager = clustermanager("harvest",workingdir=opt.workingdir)
	
	elif args[0] == 'delete':
		if opt.workingdir is None:
			message = "\033[31;1msendcluster: ERROR\033[0m the '--cw' option is mandatory"
			sys.exit( message )
		
		if not os.path.exists(opt.workingdir):
			message = "\033[31;1msendcluster: ERROR\033[0m the working path '"+opt.workingdir \
					+"' does not exists"
			sys.exit( message )
		
		manager = clustermanager("delete",workingdir=opt.workingdir)
	
	else:
		message = "\033[31;1msendcluster: ERROR\033[0mNot recognized the action '"+args[0]+"'"
		sys.exit( message )

	


