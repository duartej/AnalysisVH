#!/usr/bin/env python
"""
:mod:`clustermanager_mod` -- clustermanager class
.. module:: clustermanager_mod
   :platform: Unix
   :synopsis: clustermanager class definition for dealing with 
              the cluster
.. moduleauthor:: Jordi Duarte Campderros <duarte@ifca.unican.es>
"""

# Number of maximum jobs supported at uniovi cluster
MAXJOBSUNIOVI=70

def showresults(textresultfiles):
	"""..function:: showresults( [file1,file2,..] 
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

	# N muons and electrons no tight (in the Fake case)
	nmuons = 0
	nelecs = 0
	nevents= 0
	retevts = re.compile("======= Nt(?P<tights>\d): (?P<total>\d*) =======")
	ntights = -1

	for file in textresultfiles:
		f = open(file)
		lines = f.readlines()
		for line in lines:
			try:
				total,percent,cutname = regexp.search(line).groups()
			except AttributeError:
				try:
					nmuons += int(line.split("======= Number of no Tight Muons:")[-1].strip())
				except ValueError:
					pass
				try:
					nelecs += int(line.split("======= Number of no Tight Elecs:")[-1].strip())
				except ValueError:
					pass
				try:
					ntights,total = retevts.search(line).groups()
					nevents += int(total)
				except AttributeError:
					pass
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

		if format % (percent,"%") == "[0.00%]":
			format = "[%s%s]"
			percent= "<.01"
		
		totalformat = str(maxlenght)+".1f"
		if str(totaldict[cut]).split(".")[-1] == '0':
			totalformat = str(maxlenght)+"i"
		
		formatstr = "%"+totalformat+" "+format+" selected events (%s)"

		print formatstr % (totaldict[cut],percent,"%",cut)

	if ntights != -1:
		print "-------------------------------------------------"
		print "N of muons and electrons no Tight"
		print "nMuons = %d    nElecs = %d" % (nmuons,nelecs)
		print "Number of Nt%s events passing all selection: %d" % (ntights,nevents)
		print "-------------------------------------------------"

def getedges(numberlist):
	"""..function:: getedges(numberlist) -> [(init,endgroup1),(init,endgroup2),...]
	Extract from a list of integers the edges of consecutive number
	"""
	from itertools import groupby
	from operator import itemgetter

	try:
		dum = map(lambda x: int(x),numberlist)
	except ValueError:
		message="\033[1;31mgetedges ERROR\033[1;m Invalid literals from"
		message+=" the list %s. All components must be integers" % str(numberlist)
		raise RuntimeError(message)

	edges = []
	for k, g in groupby(enumerate(numberlist), lambda (i,x):i-x):
		auxlist = map(itemgetter(1),g)
		if len(auxlist) == 1:
			edgetup = (auxlist[0],auxlist[0])
		else:
			edgetup = (auxlist[0],auxlist[-1])
		edges.append(edgetup)

	return edges


class clustermanager(object):
	"""
	Class to manage the sending and harvesting
	of jobs in the cluster
	"""
	def __init__(self,workingdir='',**keywords):
		"""
		Initializes
		"""
		import os
		import sys
		import socket
		
		validkeys = [ 'dataname', 'cfgfilemap', 'njobs', 'precompile',\
				'finalstate', 'analysistype',
				'fakeable' ]
		
		if workingdir != '':
			self.cwd = os.path.abspath(workingdir)
		else:
			# Sent for submit, no needed for working dir, but check
			# there are others keywords needed
			if 'dataname' not in keywords.keys():
				message = "\033[1;31mclustermanager ERROR\033[1;m Instantiation needs"\
						" some minimum keywords when it is not introduced the"\
						" working dir argument"
				raise RuntimeError(message)

		self.precompile = False
		self.outputfiles= {}
		self.leptoncfgfilemap = {}
		self.fakeable = False
		self.hostname = socket.gethostname()
		# Checking the environment, and setting self.basedir self.pkgdir
		self.__checkenv()

		# Parsing the keywords
		for key,value in keywords.iteritems():
			if key not in validkeys:
				message = "\033[1;31mclustermanager\033[1;m: ERROR Not a valid argument '"+key+\
						"' to instanciate the class\n"
				raise RuntimeError(message)
			
			if key == 'dataname':
				self.dataname = value
				self.originaldataname = value
				if 'WH' in value:
					self.dataname = self.dataname.replace("WH","WHToWW2L")
			elif key == 'cfgfilemap':
				#Checking is a file and can be find it
				for lepton,cfg in value.iteritems():
					if not os.path.exists(cfg):
						message = "\033[31mclustermanager: ERROR\033[m Not found '"+cfg+"'.\n"
						raise RuntimeError(message)
					if lepton.lower() != "muon" and lepton.lower() != "mu" \
							and lepton.lower() != "electron" and lepton.lower() != "elec":
						message = "\033[31mclustermanager: ERROR\033[m Not valid lepton assignation"
						message +=" to the config file '"+cfg+"'. Parsed:'"+lepton.lower()+"'."
						message += " Valid keys are: muon mu electron ele"
						raise RuntimeError(message)
					self.leptoncfgfilemap[lepton] = os.path.abspath(cfg)
			elif key == 'njobs':
				self.njobs = int(value)
			elif key == 'precompile':
				self.precompile = True
			elif key == 'finalstate':
				self.finalstate = value
			elif key == 'analysistype':
				self.analysistype = value
			elif key == 'fakeable':
				try:
					leptons = value.split(",")
					self.nLeptons = leptons[0]
					self.nTights  = leptons[1]
					self.fakeable = True
				except AttributeError:
					self.nLeptons = None
					self.nTights = None
		# Some needed data
		try:
			dum = self.dataname
		except AttributeError:
			self.dataname = os.path.basename(self.cwd).split("_")[-1]
		self.action = None
	
		self.__init__ = True
		# The class is initialize. Now the process method can be called
	

	def process(self,action):
		"""..method:: process([action])
		Performs the action requiered by the user, submit a job, harvesting
		or deleting. If it is called with arguments the action mode will be 
		change to that one introduced
		"""
		import os

		if not self.__init__:
			message = "\033[1;31clustermanager ERROR\033[1;m: It is needed an"\
					" 'clustermanager' instance to call the 'process' method."
			raise RuntimeError(message)

		self.__processcall__ = True
		
		# Change the state
		if not action in [ 'submit','resubmit','harvest','delete','resubmit.stalled'] :
			message = "\033[1;31mclustermanager ERROR\033[1;m: Invalid action '%s'."\
					" Valid actions are: submit harvest resubmit "\
					" resubmit.stalled delete" % self.action
			raise RuntimeError(message)

		self.action = action
		
		# In the harvesting or deleting, extracting the clustermanager object
		# previously stored
		if self.action == "harvest" or self.action == "resubmit" \
				or self.action == "resubmit.stalled" or self.action == "delete":
			PWD = os.getcwd()
			os.chdir(self.cwd)
			swapobj = self.retrieve()
			if swapobj:
				preserveaction = self.action
				self = swapobj
				self.action = preserveaction
			else:
				# Already done
				return None

		if self.action == "submit":
			self.__doSubmit()
		elif self.action == "harvest":
			self.__doHarvest()
		elif self.action == "delete":
			self.__doDelete()
		elif self.action == "resubmit":
			self.__doResubmit()
		elif self.action == 'resubmit.stalled':
			self.__doResubmit_Stalled()
		else:
			message = "\033[1;31mclustermanager ERROR\033[1;m: Invalid action '%s'."\
					" Valid actions are: submit harvest delete" % self.action
			raise RuntimeError(message)
	

	def __checkenv(self):
		"""..method:: __checkenv() -> bool
		Perfom some environmental checks to be sure the jobs could be
		sent
		"""
		import os

		# Trying to extract the env variables to define 
		# the path of the General package
		if os.getenv("VHSYS"):
			self.basedir = os.path.abspath(os.getenv("VHSYS"))
			self.libsdir = os.path.join(self.basedir,"libs")
			if not os.path.exists( os.path.join(self.basedir,"CutManager") ):
				message = "\033[31;2mclustermanager: ERROR\033[0m the path introduced '" \
						+value+"' is not the base directory for the package 'VHAnalysis'\n"
				raise message 
		else: 
			message = "\033[1;31mclustermanager ERROR\033[1;m:  The enviroment variable"\
					" 'VHSYS' is not present. Initialize the AnalysisVH package"
			raise RuntimeError(message)
		# for the analysis specific package
		if os.getenv("ANALYSISSYS"):
			self.pkgpath = os.path.abspath(os.getenv("ANALYSISSYS"))
			if not os.path.exists( os.path.join(self.pkgpath,"interface/AnalysisBuilder.h") ):
				message = "\033[31;2mclustermanager: ERROR\033[0m the path introduced '" \
						+value+"' do not contain the header interface/AnalysisBuilder.h\n"
				raise message 
		return True
		


	def __doSubmit(self):
		"""..method:: doSubmit()
		Submit all the jobs to the cluster
		"""
		import os
		import shutil
		import glob

		# Checking this method was called by process method
		try: 
			dummy =self.__processcall__
		except AttributeError:
			message = "\033[1;31mclustermanager ERROR\033[1;m Internal method. "\
					"The __doSubmit method must"\
					" be called trough the process method."
			raise RuntimeError(message)
		self.__processcall__ = False

		# FIXME: Check we have all the datamembers we need

		# Check if we have the names of the data files: FIXME: Nota que con la version
		# actual del runanalysis no necesito tener el dataname pues me lo busca el mismo !!??
		self.filedatanames = os.path.join( os.getenv( "PWD" ), self.dataname+"_datanames.dn" )
		if not os.path.exists(self.filedatanames):
			if "dataname" in self.originaldataname:
				message  = "\033[31mclustermanager: ERROR\033[m"
				message += " Syntax mistake in '-d dataname' option. The argument 'dataname' must be just"
				message += " the name, without '_dataname.dn'. So '"+self.originaldataname+"' is not valid"
				raise RuntimeError(message)
			# if not created previously
			message  = "\033[31mclustermanager: ERROR\033[m"
			message += " I need the list of file names, execute:"
			message += "\n'datamanager "+self.originaldataname+" -f "+self.finalstate+"'"
			message += "\nAnd then launch again this script"
			raise RuntimeError(message)
		# Extract the total number of events and split 
		self.nevents = self.getevents(self.filedatanames)
		# We want some thing quick, the estimation is between 500-1000 e/sec,
		# so we are trying to send 10minutes-jobs: ~450000 evt per job  (changed to 200000-> New calculations)
		if self.njobs == 0:
			message = "\033[34;2m[clustermanager INFO]\033[m Guessing the number of tasks "\
					+"to send 10 minutes jobs. Found: "
			self.njobs = self.nevents/200000   #450000 
			mess2 = ""
			if self.hostname.find("uniovi") != -1 and self.njobs > MAXJOBSUNIOVI:
				self.njobs = MAXJOBSUNIOVI
				mess2 = " (Forced due to UNIOVI cluster limitation)"
			message += str(self.njobs)+mess2
			print message
		# Checking if has sense the njobs
		if self.njobs < 1:
			message = "\033[33;2m[clustermanager WARNING]\033[m the Number of jobs introduced '"\
					+str(self.njobs)+"' make no sense: changing to 1 "
			print message
			self.njobs = 1
		evtperjob = self.nevents/self.njobs 
		# Recall: the first event is 0, the last event: Entries-1
		remainevt  = (self.nevents % self.njobs)-1 
		self.taskidevt = []
		self.tasksID  = []
		for i in xrange(self.njobs-1):
			self.taskidevt.append( (i+1,(i*evtperjob,(i+1)*evtperjob-1)) )
			self.tasksID.append( i+1 )
		# And the last tasksIDs and events range per task
		self.tasksID.append( self.njobs )
		self.taskidevt.append( (self.njobs,\
				((self.njobs-1)*evtperjob,self.njobs*evtperjob+remainevt ) ) )
		# Submitting the jobs ========================================================
		# Checking we have the datamembers initialized: FIXME
		#Creacion del directorio de trabajo
		launchDir = os.getenv( 'PWD' )
		self.cwd=os.path.join(launchDir,"cluster_"+self.dataname)
		try:
			os.mkdir( self.cwd )
		except OSError:
			# FIXME
			message  = "\033[31mclustermanager: ERROR\033[m I cannot create the directory '"+self.cwd+"'"
			message += "\nPossibly the job is currently in use. Remove the directory if you "
			message += "want to send it again."
			# FIXME: Comprobar si hay jobs activos---> usa status method
			raise RuntimeError(message)
		# Moving to that dir
		os.chdir( self.cwd )
		# Extract the number of events per job
		for i,evttuple in self.taskidevt:
			# Splitting the config file in the number of jobs
			iconfigname = self.createconfig(i,evttuple)
			# And storing the name of the files
			self.outputfiles[i] = os.path.join("Results",self.dataname+"_"+str(i)+".root")
		# Preparing the name of the config files to be sended as a tasks
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
		
		self.bashscriptname = bashscriptname
		
		# Sending the job and storing the cluster job id
		self.jobid = self.sendjob(bashscriptname)

		# Creating the status tasks dict
		self.taskstatus = { 'Done': [], 'Undefined': [], 'r': [], 'qw': [], \
				'Stalled': [], 'Failed': [] }

		#persistency
		self.store()
		
		# If data or Fakes check and copy if exists a blacklist
		# FIXME: Probably it is has to be introduced other names
		if self.dataname == "Data" or self.dataname.find("Fakes") == 0:
			try:
				blfilename = "blacklist.evt"
				originalbl = glob.glob(os.path.join(launchDir,blfilename))[0]
				shutil.copyfile(originalbl,blfilename)
			except IndexError:
				pass
		
		# Returning initial directory
		os.chdir(launchDir)

	def __doResubmit(self):
		"""..method:: __doResubmit()
		Send again a list of failed jobs
		"""
		import os
		# Checking this method was called by process method
		try: 
			dummy =self.__processcall__
		except AttributeError:
			message = "\033[1;31mclustermanager ERROR\033[1;m Internal method. "\
					"The __doHarvest method must"\
					" be called trough the process method."
			raise RuntimeError(message)
		self.__processcall__ = False

		# Moving to the directory of work
		prepwd = os.getenv("PWD")
		os.chdir(self.cwd)

		newtasklist = []
		# Extracting list of jobs failed and not in cluster (XXX)
		for task in list(set(self.taskstatus["Failed"])):
		#for task,filestr in self.outputfiles.iteritems():
		#	if not os.path.isfile(filestr):
				newtasklist.append(task)

		# Checking the list of jobs are already defined
		for taskstr in newtasklist:
			task = int(taskstr)
			if task not in map(lambda x: x[0],self.taskidevt):
				message = "\033[1;31mclustermanager ERROR\033[1;m Not possible"
				message += " re-send a task which wasn't previously defined."
				message += " First submit a job"
				raise RuntimeError(message)
			# Checking the jobs haven't result
			dummy = self.outputfiles[task]
			if os.path.isfile(dummy):
				message = "\033[1;31mclustermanager ERROR\033[1;m The output"
				message += " root file for the task %i is already there." % task
				message += " Canceling re-submit..."
				raise RuntimeError(message)
		# send the jobs
		taskranges = getedges(newtasklist)
		self.jobsid = []
		message = "\033[1;34m[clustermanager INFO]\033[1;m Re-submit '%s'" % self.dataname
		message += " Tasks: ["
		for tasktup in taskranges:
			self.jobsid.append( self.sendjob(self.bashscriptname,tasktup) )
			message += "(%i,%i)," % (tasktup[0],tasktup[1])
		message = message[:-1]+"]"
		print message
		# Clearing the taskstatus list
		for taskid in newtasklist:
			self.taskstatus["Failed"].remove(taskid)

		self.store()

		# coming back
		os.chdir(prepwd)
	
	def __doResubmit_Stalled(self):
		"""..method:: __doResubmit()
		Send again a list of STALLED jobs
		"""
		from subprocess import Popen,PIPE
		
		listtask = []
		for task in self.taskstatus["Stalled"]:
			listtask.append(task)
		listtask = list(set(listtask))
		for taskid in listtask:
			command = [ 'qmod','-rj',self.jobid+"."+str(taskid) ]
			p = Popen( command ,stdout=PIPE,stderr=PIPE ).communicate()
			print "\033[1;33m[clustermanager.__checkjob INFO]\033[1;m: Re-submitting "\
					" task job in error status: %s.%i" % (self.jobid,taskid)


	def __doHarvest(self):
		"""..method:: _doHarvest()
		Retrieve the jobs if they have finished, otherwise stores its status
		"""
		# Checking this method was called by process method
		try: 
			dummy =self.__processcall__
		except AttributeError:
			message = "\033[1;31mclustermanager ERROR\033[1;m Internal method. "\
					"The __doHarvest method must"\
					" be called trough the process method."
			raise RuntimeError(message)
		self.__processcall__ = False

		foundoutfiles = []

		jobsdict = self.getstat(self.hostname)
		getexceptionerr = []
		for taskid,dummy in self.taskidevt:
			try:
				foundoutfiles.append( self.__checkjob(jobsdict,taskid) )
			except RuntimeError,e:
				getexceptionerr.append( e.args[0] )
			# __checkjob returning None object if the task it wasn't finished
			if not foundoutfiles[-1]:
				foundoutfiles = foundoutfiles[:-1]
		print "\033[34;1m[clustermanager INFO]\033[m Checking the job status for dataname '"+self.dataname+"'"
		# Print status 
		getcolor = lambda x,color: "\033["+str(color)+";1m"+x+"\033[m"
		outputmessage = ''
		textstatusdict = { "r": getcolor("Running",32), "qw": getcolor("Queued",30), \
				"Undefined": getcolor("Undefined",35), "Done": getcolor("Done",34),
				"Stalled": getcolor("Stalled",33), "Failed": getcolor("Failed",31) }
		for status,tasklistunord in self.taskstatus.iteritems():
			tasklist = sorted(list(set(tasklistunord)))
			if len(tasklist) == 0:
				continue
			outputmessage += "   "+textstatusdict[status]+": ["
			edgeslist = getedges(tasklist)
			for edges in edgeslist:
				outputmessage += str(edges[0])+"-"+str(edges[1])+","
			outputmessage = outputmessage[:-1]+"]\n"
		print outputmessage[:-1]
		
		# If we have all the outputfiles we can gathering
		if foundoutfiles == self.outputfiles.values():
			self.__gatherfiles()
		# persistency
		self.store()

		# If we got exceptions, dump the info
		if len(getexceptionerr) != 0:
			message = '\033[31mclustermanager.__checkjob: Something went wrong in the cluster\033[1;m: '
			message += 'Tasks: ['
			for i in getexceptionerr:
				message += "%s," % (i.split(":")[-1].split()[2])
			message = message[:-1]+"]"
			message += " are already finished but there is no output root file"
			raise RuntimeError(message)
	
	def __doDelete(self):
		"""::method __doDelete() 
		Delete the job"""
		from subprocess import Popen,PIPE
		import sys
		import os
		import shutil

		# Checking this method was called by process method
		try: 
			dummy =self.__processcall__
		except AttributeError:
			message = "\033[1;31mclustermanager ERROR\033[1;m Internal method. "\
					"The __doDelete method must"\
					" be called trough the process method."
			raise RuntimeError(message)
		self.__processcall__ = False

		print "\033[37mDeleting the '"+self.dataname+"' job:\033[m "+self.jobid+" and data .",
		sys.stdout.flush()
		command = [ 'qdel',str(self.jobid) ]
		p = Popen( command ,stdout=PIPE,stderr=PIPE ).communicate()
		print ".",
		sys.stdout.flush()
		os.chdir("..")
		shutil.rmtree(self.cwd)
		print ".",
		sys.stdout.flush()
		print " DONE!  "


	def __gatherfiles(self):
		"""..method:: __gatherfiles() 
		Gather all the outputfiles in one
		"""
		from subprocess import Popen,PIPE
		import os
		import tarfile
		import glob
		
		print "\033[1;34m[clustermanager INFO]\033[1;m %s:: Joining all the files in one" % self.dataname
		finalfile = os.path.join("Results",self.dataname+".root")
		# Just cp when there is only one file, otherwise
		# there are problems with the TTree
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
			message  = "\033[33;2mclustermanager.__gatherfiles WARNING\033[m the total file"
			message += "'"+finalfile+"' do not contain all the events:\n"
			message += "Total events to be processed:"+str(self.nevents)+"\n"
			message += "Total events in '"+finalfile+"':"+str(totalevts)+"\n"
			print message
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
			message  = "\033[33;2mclustermanager.__gatherfiles WARNING\033[m I can't manage\n"
			message += "to create the backup .tar.gz file\n"
			print message
		
		# Tagging the jobs done
		self.taskstatus["Done"] = self.outputfiles.keys()

		print "Created "+finalfile
		print "\033[1;34m[clustermanager INFO]\033[1;m",self.dataname,": Harvest completed"
	
	
	def __checkjob(self, jobsdict,_taskid):
		"""..method:: __checkjob(taskid)
		Check and update the status of a job
		"""
		import os

		# be sure to be an int
		taskid = int(_taskid)
		
		# Resubmision case
		if type(self.jobid) == list:
			# Checking every id
			for jobid in self.jobsid:
				swapjobid = self.jobid
				self.jobid = jobid
				self.__checkjob(jobsdict,taskid)
				self.jobid = swapjobid
		try:
			jobidpresent = jobsdict[self.jobid]
			rawstatus = jobsdict[self.jobid][taskid]
			isincluster = True
		except (KeyError,TypeError):
			# The task was done either the self.jobid is not 
			# (it means the full job is done) or the taskid
			# is not (it means the task is done)
			rawstatus='Done'
			isincluster = False

		# If not in the cluster, it should be finished and get available the outputs
		if not isincluster:
			# Checking if the outputfiles are there
			if not os.path.exists(self.outputfiles[taskid]):
				message = "\033[31mclustermanager.__checkjob: Something went wrong in the cluster:\033[m"
				message += "The task '"+str(taskid)+"' of the job '"+str(self.jobid)
				message += "' is already finished but there is no output root file '"
				message += self.outputfiles[taskid]+"'\n"
				message += "Check the cluster outputs file"
				# Update the status
				self.taskstatus['Failed'].append(taskid)
				# And remove it from the other dicts
				for outstatus,tasklist in filter(lambda (x,y): x != 'Failed', self.taskstatus.iteritems()):
					try:
						self.taskstatus[outstatus].remove(taskid)
					except ValueError:
						pass
				raise RuntimeError( message )
			# Gathering the file outputs in order to add
			self.taskstatus["Done"].append(taskid)
			# And remove it from the other dicts
			for outstatus,tasklist in filter(lambda (x,y): x != 'Done', self.taskstatus.iteritems()):
				try:
					self.taskstatus[outstatus].remove(taskid)
				except ValueError:
					pass
			return self.outputfiles[taskid]

		# Parsing status
		if rawstatus == "r" or rawstatus == 'Rr':
			status = 'r'
		elif rawstatus == 't':
			status = 'Stalled'
		elif rawstatus == "qw":
			status = 'qw'
		# UNIOVI
		elif rawstatus == "R":
			status = 'r'
		elif rawstatus == "Q":
			status = 'qw'
		#elif status == "C" or status == "E":
		#        # The C -- DONE but E is error. ANyway the error
		#        # is due to the imposibility of copy the stderr and out   
		#	self.taskstatus["Done"].append(taskid)
		#	return self.outputfiles[taskid]
		else:
			if self.hostname.find("ifca") and (rawstatus == "Eqw" or rawstatus == 'ERq'):
				from subprocess import Popen,PIPE

				command = [ 'qmod','-cj',self.jobid+"."+str(taskid) ]
				p = Popen( command ,stdout=PIPE,stderr=PIPE ).communicate()
				print "\033[1;33m[clustermanager.__checkjob INFO]\033[1;m: Re-submitting "\
						" task job in error status: %s.%i" % (self.jobid,taskid)
				status = 'Undefined'

		# Cualquier otro status que no hemos tenido en cuenta, no creara 'status' y petara
		
		# Update the taskstatus (but the Done)
		for outstatus,tasklist in filter(lambda (x,y): y != 'Done', self.taskstatus.iteritems()):
			try:
				self.taskstatus[outstatus].remove(taskid)
			except ValueError:
				pass
		self.taskstatus[status].append(taskid)
		return None

	@staticmethod
	def getstat(hostname):
		"""..staticmethod:: getstat() -> { 'jobid': {'taskid': status}, ... }
		Get and parse string with the result of the 'qstat' cluster utility
		"""
		from subprocess import Popen,PIPE
		import os

                if hostname.find("uniovi") != -1:
			print "\033[1;32mWARNING\033[1;m New functionalities added"\
					" to clustermanager. Still not debugged for "\
					" UNIOVI cluster. Use with caution"
			command = [ 'qstat','-t','-u',os.getenv("USER") ]
		else:
			command = [ 'qstat','-u',os.getenv("USER"),'-g','d' ]
		p = Popen( command ,stdout=PIPE,stderr=PIPE ).communicate()
		
		jobs = {}
		for line in p[0].split("\n")[2:]:
			parseline = line.split()
			try:
				jobid = parseline[0]
			except IndexError:
				# There is no job in the cluster
				continue
			# Check is not catching an interactive job
			jobname = parseline[2]
			if jobname == "QRLOGIN":
				continue

			status= parseline[4]
			try:
				task  = int(parseline[9])
			except IndexError:
				# Implies it is still waiting
				task  = int(parseline[8])
			except ValueError:
                                # Working at fanae 
				jobid  = parseline[0].split(".")[0].split("[")[0]
				print  parseline[0].split(".")[0].split("[")[0]
                                task   = parseline[0].split(".")[0].split("[")[1].replace("]","")
                                status = parseline[9] 
			try:
				jobs[jobid][task] = status
			except KeyError:
				jobs[jobid] = { task: status }

		if len(jobs) == 0:
			jobs = None
			
		return jobs

	
	@staticmethod
	def retrieve(filename='.storedmanager'):
		"""..method:: retrieve()
		Retrieve and populates the class from a file 
		"""
		import shelve
		import glob

		d = shelve.open(filename)
		if not d.has_key("storedmanager"):
			# Check if already is done the file
			if len(glob.glob("*.tar.gz")) != 0 \
					or len(glob.glob("WARNING_FOLDER_GENERATED_FROM_SCRIPT.txt")) != 0:
				message = "clustermanager.retrieve: The job is already DONE!"
				print message
				return None
			else:
				message = "\033[1;31mclustermanager.retrieve ERROR]\033[1;m Not found the" \
					+" class stored in .storedmanager file"
				raise RuntimeError(message)

		copyself = d["storedmanager"]

		# Check if we are done: NEW 
		try:
			if len(copyself.taskstatus["Done"]) == len(copyself.outputfiles.keys()):
				message = "clustermanager.retrieve: The job is already DONE!"
				print message
				d.close()
				return None
		except AttributeError:
			message = "\033[1;31mclustermanager.retrieve ERROR]\033[1;m Massive job failure. See" \
					+" the *.sh.e* files to see what happened"
			raise RuntimeError(message)

		return copyself
		


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
			message = "\033[31mclustermanager\033[m ERROR from 'extractEvents':\n"
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
		import os
		
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
                        if self.hostname.find("uniovi") != -1:
                                arrayvariable = "PBS_ARRAYID"
			else:
                                arrayvariable = "SGE_TASK_ID"
			# We want return the general name, using the environment variable
			cfgnontaskdepPROV = newcfgnamePROV.replace( ".", "_${"+arrayvariable+"}." )
			cfgnontaskdep     += lepton+":"+cfgnontaskdepPROV+","

		cfgnontaskdep = cfgnontaskdep[:-1]
		return cfgnontaskdep
	
	def createbash(self,executable,config):
		"""Create the bash scripts to be sended to the cluster
		Return the script filename
		"""
		import os
		import stat

                if self.hostname.find("uniovi") != -1:
                        arrayvariable = "PBS_ARRAYID"
                        # PATCH TO BE FIXED
                        redictout = " 1> "+self.dataname+".sh.o-${"+arrayvariable+"}"
                        redicterr = " 2> "+self.dataname+".sh.e-${"+arrayvariable+"}"
                        PROVoutputfile = redictout+redicterr
		else:
                        arrayvariable = "SGE_TASK_ID"
                        PROVoutputfile = "" 
		outputname = os.path.join("Results",self.dataname+"_${"+arrayvariable+"}.root")

		lines  = "#!/bin/bash\n"
		lines += "\n# Script created automatically by sendcluster.py utility\n"
		lines += "\nmkdir -p Results\n"
		lines += "export PATH=$PATH:"+os.path.join(self.basedir,"bin")+":"+os.path.join(self.pkgpath,"bin")+"\n"
		lines += "export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:"+self.libsdir+"\n"
		lines += executable+" "+self.dataname+" -a "+self.analysistype+" -c "+self.cfgnames+\
				" -d "+self.filedatanames+" -l "+self.finalstate+" -o "+outputname+PROVoutputfile+"\n"
		if self.fakeable:
			lines = lines[:-1]+" -F "+self.nLeptons+","+self.nTights+"\n"
	
		filename = self.dataname+".sh"
		f = open(filename,"w")
		f.writelines(lines)
		f.close()
		os.chmod(filename,stat.S_IRWXU+stat.S_IRGRP+stat.S_IXGRP+stat.S_IXOTH)
		
		return filename

	def sendjob(self,bashscript,tasktuplepre=None):
		"""..method:: sendjob() -> jobid
		Send to the cluster the bash script input
		"""
		from subprocess import Popen,PIPE

		if tasktuplepre:
			# Getting the edges
			taskidinit= tasktuplepre[0]
			taskidend = tasktuplepre[1]
		else:
			# Getting all the jobs
			# Recall map is ordered 
			taskidinit = self.taskidevt[0][0]
			taskidend  = self.taskidevt[-1][0]

		# OJO NO HE PUESTO REQUERIMIENTOS DE MEMORIA, CPU...
		print "Sending to cluster: "+bashscript
		if self.hostname.find("uniovi") != -1:
                        import os
			command = [ 'qsub','-V','-d',os.getcwd(), \
                                 '-t',str(taskidinit)+"-"+str(taskidend),bashscript ]
		else:
			command = [ 'qsub','-r','yes','-V','-cwd','-S','/bin/bash', \
			        '-t',str(taskidinit)+"-"+str(taskidend),\
				'-P','l.gaes','-l', 'immediate', '-l','h_rt=02:00:00',bashscript ]
		p = Popen( command ,stdout=PIPE,stderr=PIPE ).communicate()
		if p[1] != "":
			message = "\033[31mclustermanager: ERROR\033[m from 'qsub':\n"
			message += p[1]+"\n"
			raise message
		
		#Extract id
		id = None
		outputline = p[0].split(' ')
		# Depending the host
		if self.hostname.find("uniovi") != -1:
                        id = outputline[0].split('.')[0].replace("[]","")
		else:
			for i in outputline:
				if i.isdigit():
					id = i
				if len(i.split(":")) == 2:
					id = i.split(".")[0]
		return id
	
