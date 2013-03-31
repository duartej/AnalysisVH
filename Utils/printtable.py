#!/usr/bin/env python



# Fancy name to the samples. The dictionary is built in run-time. Whenever a sample is not
# defined here, the fancy will be the same as the default one
from cosmethicshub_mod import LEGENDSDICT as TITLEDICT
from functionspool_mod import processedsample
TITLEDICT["Other"]="Other bkg"

ORDERCOLUMNS = [ "PPP", "PPF", "Fakes", "ZZ", "Other" ]


class format(object):
	"""
	"""
	def __init__(self):
		"""
		"""
	        self.tablestart = ''
	        self.tableend   = ''
	        self.rowstart   = ''
	        self.rowend     = ''
	        self.cellstart  = ''
	        self.cellend    = ''
	        self.cellcoltitstart = ''
	        self.cellcolitend = ''
	        self.cellrowitstart = ''
	        self.cellrowitend = ''
	        self.plusminus = ''
		self.format = None

	def setformat(self,format,ncolumns):
		"""
		"""
		self.ncolumns = ncolumns
		if format.lower() == "latex" or format.lower() == "tex":
			self.tablestart = '\\begin{tabular}{ r | '
			for i in xrange(ncolumns):
				self.tablestart += ' l ' 
			self.tablestart += '}\\hline'	        	
			self.tableend   = '\\end{tabular}'
			self.rowstart = ''
			self.rowend   = '\\\\'
			self.cellstart  = ' & '
			self.cellend    = ' '
			self.cellcolitstart = ' & {\\bf '
			self.cellcolitend = ' } '
			self.cellrowitstart = ' {\\bf '
			self.cellrowitend = ' } '
			self.plusminus = ' $\\pm$ '
			self.exponentstart = '$^{'
			self.exponentend   = '}$'
			self.cdot = "$\\cdot$"
		elif format.lower() == 'html':
			self.tablestart = '<table border="0px" cellpadding="4px" cellspacing="0">'
	        	self.tableend   = '</table>'
	        	self.rowstart   = '<tr>'
	        	self.rowend     = '</tr>'
	        	self.cellstart  = '<td>'
	        	self.cellend    = '</td>'
	        	self.cellcolitstart = '<th bgcolor="lightblue">'
	        	self.cellcolitend = '</th>'
	        	self.cellrowitstart = '<th align="right" bgcolor="lightblue">'
	        	self.cellrowitend = '</th>'
	        	self.plusminus = '&plusmn;'
			self.exponentstart  = '<sup>'
			self.exponentend    = '</sup>'
			self.cdot = '&middot;'
		else:
			raise "\033[31mformat ERROR\033[m Format "+format+\
					" not implemented"

		self.format = format

		
class table(processedsample):
	"""
	"""
	def __init__(self,data,signal,**keywords):
		"""..class:: table(data,signal[,format="tex|html",\
				isreduced=True|False, wildcardfiles="dir",join="metasample",\
				subtract=metasample,force=listofsamples]) 
		
		The table is composed by several 'processedsample' instances (see functionspool_mod module), and is 
		going to be built as
		               bkg1  bkg2 ... TotBkg data signal
		

		:param data: name of the column which it will be placed before the last one. Also
		             the values of this column are not going to be added up with the other
			     columns (to create the TotBkg column)
		:type data: str
		:param signal: name of the column which it will placed the last one. Also the 
		             values of this column are not going to be added up with the other
			     columns (creating the TotBkg column)
		:type param: str
		:param format: latex|tex|html  The output format of the table
		:type format: str
		:param wildcardfiles: string with wildcards which can be used to find what files
		             have to be used as column generators
		:param join: Complex parameter to introduce the possibility of merge two or more
		             samples into one unique metasample (called in this way because the
			     metasample does not have associated any filename and is compossed by
			     two or more original samples). The argument could be a str defining
			     a pre-built metasample: DY Z+Jets Other (see getsamplecomponent);
			     or could be a list of string which are defining more than one 
			     pre-built metasample; or could be a dictionary whose keys are the
			     metasample names and the values are list containing the samples to
			     merge.
		:type join: str| [ str, str, ...] | { str: [ str, str, ... ], ... }
		:param subtract: analogous parameter than join but to subtract
		:type subtract:  dict(str,list(str))
		:param force: list of samples to be kept in the table although are inside the
		              subtract list
		:type force: list(str)
		"""
		import glob
		import ROOT
		import os
		global TITLEDICT


		formatprov = None

		validkeywords = [ "format", "isreduced", "join", "wildcardfiles", \
				"signalmc", "subtract", "force", "datadriven"]
		wildcardfiles = "*.root" # Per default
		join = []
		subtract = {}
		subtractmeta = []
		keepforce = []
		self.usermetasample = {}
		self.signalmc = None
		self.nsignalcolumn = True
		for key,value in keywords.iteritems():
			if not key in keywords.keys():
				message  = "\033[31mtable ERROR\033[m Incorrect instantiation of 'table'"
				message += " class. Valid keywords are: "+str(validkeywords)
				raise RuntimeError(message)

			if key == 'format':
				formatprov = value
			elif key == 'isreduced':
				if value:
					join = [ "DY", "Z+Jets", "Other" ]
			elif key == "wildcardfiles":
				wildcardfiles = value
			elif key == "join":
				if type(value) == list and len(value) == 0:
					# we don't want to smash the join list
					pass
				elif type(value) == list:
					join = value
				elif type(value) == dict:
					self.usermetasample = value
					join = self.usermetasample.keys()
				else:
					join.append(value)
			elif key == "subtract":
				subtract = value
			elif key == "force":
				keepforce=value
			elif key == 'signalmc': 
				self.signalmc = value
			elif key == 'datadriven':
				if value == 'PPF':
					self.nsignalcolumn = True
				elif value == 'PPP':
					self.nsignalcolumn = False
		# Update the usermetasample
		for metaname,listsamples in subtract.iteritems():
			if self.usermetasample.has_key(metaname):
				message = '\033[1;31mtable ERROR[1;m Conflicts between merge and subtract'
				message += ' sample'
				raise RuntimeError(message)
			self.usermetasample[metaname] = listsamples
			subtractmeta.append(metaname)

		# available filenames
		self.filenames = glob.glob(wildcardfiles)

		# Just to be sure that only use one WH signal (adapted 2012 MC samples)
		if signal.find("WH") == 0 or signal.find("wztt") == 0:
			#Extract the other WH signals
			# Common between 2011 and 2012 MC signal samples names
			# FIXME: Add all the signals in the last columns 
			potentialSfiles = filter(lambda x : x.find("ToWW") != -1,self.filenames)
			nonsignalfiles  = filter( lambda x: x.split("/")[-1].split(".root")[0] != signal,potentialSfiles)
			# Removing
			for f in nonsignalfiles:
				self.filenames.remove(f)
			# Added the important cuts to be used in the reduced table case
			self.importantcutsdict = { 'Pre-selection': 'Exactly3Leptons', \
					'DeltaR': 'DeltaR', 'ZVeto':'ZVeto', 'MET': 'MET'}
			self.importantcutslist = [ 'Pre-selection', 'DeltaR','ZVeto', 'MET']
		else:
			# Added the important cuts to be used in the reduced table case
			self.importantcutsdict = { 'Pre-selection' : 'Exactly3Leptons',
					'Z' : 'HasZCandidate',  'W': 'MET' }
			self.importantcutslist = [ 'Pre-selection', 'Z', 'W' ]

		# building the columns
		self.columns = {}
		for f in self.filenames:
			naturalname = os.path.basename(f).split(".")[0]
			# Putting the fancy name if has to, or using the
			# per default name (so updating the TITLEDICT global)
			try:
				coltitle = TITLEDICT[naturalname]
			except KeyError:
				TITLEDICT[naturalname] = naturalname
			col = processedsample(f)
			self.columns[col.title] = col
		
		# samples names
		self.samples = self.columns.keys()
		# Check the signal and data are there
		if not signal in self.samples+join:
			raise RuntimeError("\033[31mtable ERROR\033[m The signal introduced '"\
					+signal+"' has not been found."+\
					" Check you have not introduced the '-n' option without quotes:\n"+\
					" printtable "+signal+" -n \"whatever*..\"")
		if not data in self.samples+join:
			raise RuntimeError("\033[31mtable ERROR\033[m The data introduced '"\
					+data+"' has not been found."+\
					" Check you have not introduced the '-n' option without quotes:\n"+\
					" printtable "+signal+" -n \"whatever*..\"")

		self.signal = signal
		self.data   = data

		# Ordered samples names (column names)
		self.columntitles = []
		# Backgrounds ordered first
		self.columntitles = filter(lambda x: x in self.samples+join, ORDERCOLUMNS)
		for i in self.samples:
			# Already took into account
			if i in self.columntitles:
				continue
			# Signal and data will be put after
			if i == self.signal or i == self.data or i == self.signalmc:
				continue
			self.columntitles.append(i)
		# -- Adding the other columns
		self.columntitles.append( "TotBkg" )
		self.columntitles.append( self.data )
		if self.nsignalcolumn:
			self.columntitles.append( "Data-TotBkg" )
		self.columntitles.append( self.signal )
		if self.signalmc:
			self.columntitles.append( self.signalmc )
		# -- Avoiding repetition (mainly for  DDD-DDM case)
		dummy = map(lambda x: self.columntitles.remove(x),
				set(filter(lambda x: self.columntitles.count(x) > 1, self.columntitles)))

		# The datamember samples is superceeded by columntitles
		self.samples = self.columntitles

		# 2) Merge some samples just in one
		for metasample in join:
			samplestodelete = []
			self.columns[metasample] = processedsample("",nobuilt=True)
			for sample in self.getsamplecomponents(metasample):
				try:
					self.columns[metasample] += self.columns[sample]
					samplestodelete.append( sample )
				except KeyError:
					# Protecting the case where the pre-defined
					# samples aren't there (for instance WJets_Madgraph
					# in the 'Other' metasample)
					pass
			# See if we have any of the samples to merge, if not then 
			# it has no sense going on
			if len(samplestodelete) == 0:
				# Do not incorporate the metasample to the table
				self.columns.pop(metasample)
				# And do nothing else
				continue
			# Put the title
			self.columns[metasample].title = metasample
			# Incorporates the metasample to the global TITLEDICT
			TITLEDICT[metasample] = metasample
			# Erase the samples merged 
			for s2remove in samplestodelete:
				self.columns.pop(s2remove)
				index=self.columntitles.index(s2remove)
				self.columntitles.remove(s2remove)

			# And add to the columntitles data member if not in there
			if not metasample in self.columntitles:
				self.columntitles.insert(index,metasample)
		#2_1) Subtract some samples
		for metasample in subtractmeta:
			samplestodelete = []
			for sample in self.getsamplecomponents(metasample):
				try:
					self.columns[metasample] -= self.columns[sample]
					if sample not in keepforce:
						samplestodelete.append( sample )
				except KeyError:
					# Protecting the case where the pre-defined
					# samples aren't there (for instance WJets_Madgraph
					# in the 'Other' metasample)
					pass
			# Erase the samples merged 
			for s2remove in samplestodelete:
				self.columns.pop(s2remove)
				index=self.columntitles.index(s2remove)
				self.columntitles.remove(s2remove)

			# And add to the columntitles data member if not in there
			if not metasample in self.columntitles:
				self.columntitles.insert(index,metasample)
		# format specific
		self.format = format()
		if formatprov:
			self.setformat(formatprov)

	def getsamplecomponents(self, metasample ):
		""".. function::getsamplecomponents( metasample ) -> [ 'comp1', comp1, ... ]

		Return samples's name defining a 'metasample' (a sample composed by several
		real samples, a metasample does not have a file associated)

		:param metasample: final name of the sample which will be add up
		:type metasample: str
		
		:return: list of the samples names which will compose the meta sample
		:rtype:  [ str, str, ... ] 
		"""
		components = []
		# The user has preference
		isusersrequest = metasample in self.usermetasample.keys()
		if metasample == "DY" and not isusersrequest:
			components = [ "DYee_Powheg", "DYmumu_Powheg", "DYtautau_Powheg" ]
		elif metasample == "Z+Jets" and not isusersrequest:
			components = [ "Zee_Powheg", "Zmumu_Powheg", "Ztautau_Powheg" ]
		elif metasample == "VGamma" and not isusersrequest:
			components = [ "ZgammaToElElMad", "ZgammaToMuMuMad", "ZgammaToTauTauMad",\
					"WgammaToElNuMad", "WgammaToMuNuMad", "WgammaToTauNuMad" ]
		elif metasample == "Other" and not isusersrequest:
			#components = [ "TbarW_DR", "TW_DR", "WW", "WJets_Madgraph" ]
			components = [ "WJets_Madgraph" ] 
		elif metasample in self.usermetasample.keys():
			components = self.usermetasample[metasample]
		else:
			message  = "\033[31mgetsamplecomponents ERROR\033[m '"+metasample+"'" 
			message += " not recognized. Current valid metasamples are:"
			message += " 'DY' 'Z+Jets' 'Other' and user introduced '"
			message += str(self.usermetasample.keys())+"'"
			raise RuntimeError,message

		return components

	def getMSvalerr(self, cut,sample):
		""".. function::getMSvalerr( cut, sample ) -> valueanderror

		Extract the value and the error, given the cut and the sample name. 
		The function will find the format to return the value using only the most 
		significant decimal numbers depending on how is its error. The 
		error will be modified following the below rules:
		 - if the error is >= 1.5, then the error is an integer, same as
		 the value
		 - if the error is between 1.0 and 1.5, the error is taken with
		 two decimals 
		 - if the error is < 1.0, then return 

		:param cut: cut name
		:type  cut: str
		:param sample: sample name
		:type  sample: str

		:return: the value and its error 
		:rtype : (str,str)
		"""
		from math import sqrt
		from functionspool_mod import getvalpluserr

		# Dealing with the TotBkg sample which has to be built 
		if sample == "TotBkg":
			val = 0.0
			err2 = 0.0
			for s in filter(lambda x: x != self.data and \
					x != self.signalmc and \
					x != self.signal and x != "TotBkg" and \
					x != "Data-TotBkg",self.samples):
				(v,e) = self.columns[s].getvalue(cut)
				val += v
				err2 += e**2.0
			err = sqrt(err2)
			try:
				self.columns["TotBkg"].rowvaldict[cut] = (val,err)
			except KeyError:
				# Creating the column
				self.columns["TotBkg"] = processedsample("",nobuilt=True)
				# Inititalizing dict and all the other needed data members
				self.columns["TotBkg"].rowvaldict= { cut: (val,err) }
				self.columns["TotBkg"].cutordered = self.columns[self.data].cutordered

		# extracting the values
		try:
			val,err=self.columns[sample].getvalue(cut)
		except KeyError:
			# It's substraction data - TotBkg columns
			if sample == "Data-TotBkg":
				# Note that as it has been extract by order, Data and total background
				(valdata,errdata) = self.columns[self.data].getvalue(cut)
				(valbkg,errbkg)   = self.columns["TotBkg"].getvalue(cut)
				val = valdata-valbkg
				err = sqrt(errdata**2.0+errbkg**2.0)

		valstr,errstr = getvalpluserr(val,err)
		totalvalstr = valstr+self.format.plusminus+errstr
	
		return totalvalstr



	def printstr(self,format,isreduced=False):
		"""
		"""
		if isreduced:
			return self.__printreducedstr__(format)
		#if format != '':
		self.setformat(format)

		lines = self.format.tablestart+"\n"
		# Column titles
		lines += self.format.rowstart+self.format.cellrowitstart+"      Cuts "+self.format.cellrowitend
		for s in self.columntitles:
			fancyname = s
			try:
				fancyname = TITLEDICT[s]
			except KeyError:
				pass
			lines += self.format.cellcolitstart+fancyname+self.format.cellcolitend
		lines += self.format.rowend+" \n"
		#Content 
		# Extract the maximum lenght of the cuts
		cutordered = self.columns.values()[0].cutordered
		maxlenght = str(max(map(lambda x: len(x),cutordered)))
		cutformat = "%"+maxlenght+"s"
		# Build the table row by row (cut by cut)
		for cut in cutordered:
			lines += self.format.rowstart+self.format.cellrowitstart
			lines += cutformat % cut
			lines += self.format.cellrowitend
			for sample in self.columntitles:
				lines += self.format.cellstart+self.getMSvalerr(cut,sample)+self.format.cellend
			lines += self.format.rowend+" \n"

		lines += self.format.tableend+"\n"

		return lines

	def __printreducedstr__(self,format):
		"""
		"""
		ncolumns = 4
		self.format.setformat(format,ncolumns)
		
		lines = self.format.tablestart+"\n"
		# Column titles
		lines += self.format.rowstart+self.format.cellrowitstart+"       "+self.format.cellrowitend
		for s in self.importantcutslist:
			lines += self.format.cellcolitstart+s+self.format.cellcolitend
		lines += self.format.rowend+" \n"
		# Content
		fancycolumntitles = []
		for sample in self.columntitles:
			try:
				fancycolumntitles.append( TITLEDICT[sample] )
			except KeyError:
				fancycolumntitles.append(sample)
		# Extract the maximum lenght of the samples
		maxlenght = str(max(map(lambda x: len(x),fancycolumntitles)))
		sampleformat = "%"+maxlenght+"s"
		# Build the table row by row (sample by sample)
		for sample,fancyname in zip(self.columntitles,fancycolumntitles):
			lines += self.format.rowstart+self.format.cellrowitstart
			lines += sampleformat % fancyname
			lines += self.format.cellrowitend
			for cut in self.importantcutslist:
				realcut = self.importantcutsdict[cut]
				lines += self.format.cellstart+self.getMSvalerr(realcut,sample)+self.format.cellend
			lines += self.format.rowend+" \n"

		lines += self.format.tableend+"\n"

		return lines

	def saveas(self, filename,isreduced=False):
		"""
		"""
		# Extract the type from the file suffix
		format = filename.split(".")[-1]
		tablelines = self.printstr(format,isreduced)

		f = open(filename,"w")
		f.writelines(tablelines)
		f.close()


	def setformat(self,format):
		"""
		"""
		ncolumns = len(self.columntitles)
		self.format.setformat(format,ncolumns)



if __name__ == '__main__':
	import sys
	import os
	from optparse import OptionParser
	from functionspool_mod import getsamplenames,parsermetasamples
	import glob
	
	#Comprobando la version (minimo 2.4)
	vX,vY,vZ,_t,_t1 = sys.version_info
	if vX > 2 and vY < 4:
		message = '\033[31printtable ERROR\033[m I need python version >= 2.4'
		sys.exit( message )
	elif vX < 2:
		message = '\033[31printtable ERROR\033[m I need python version >= 2.4'
		sys.exit( message )
	
	usage  ="usage: printtable <WZ|WHnnn|Fakes> [options]"
	parser = OptionParser(usage=usage)
	parser.set_defaults(output="table.tex",isreduced=False,dataname="Data",\
			wildcardfiles="cluster_*/Results/*.root",\
			datadriven='PPF')
	parser.add_option( '-f', '--filename', action='store', type='string', dest='output', help="Output filename, the suffix defines the format," \
			" can be more than one comma separated" )
	parser.add_option( '-r', action='store_true', dest='isreduced', help="More printable version of the table, where several samples have been merged" \
			" into one. Shortcut for '-j [DY,Z+Jetsj,Other]" )
	parser.add_option( '-d', action='store', dest='dataname', help='Name of the sample to be used in the role of "data" [Default: Data]')
	parser.add_option( '-e', action='store', dest='datadriven', metavar="PPF|PPP", \
			help='Describe what data-driven estimation was used. Using PPF, the '\
			'table is containing a extra row/column with "Data-Total Background"')
	parser.add_option( '-s', action='store', dest='signalmc', help='Name of the sample to be used '\
			'as MC signal, when dealing with PPP estimation')
	parser.add_option( '-n', action='store', dest='wildcardfiles', help='Path where to find the root '\
			'filenames to extract the table, it could be wildcards. \033[33mWARNING:\033[m'\
			' if uses wildcards, be sure to be included between quotes: "whatever*"'\
			' [Default: "cluster_*/Results/*.root"]')
	parser.add_option( '-m', action='store', dest='join', metavar="MS|MS1,MS2,...|MS1@S1,..,SN::MS2@S2_1,...,S2_2::...", \
			help=parsermetasamples())
	parser.add_option( '-S', action='store', dest='subtract', metavar="MS1@S1,S1,...::MS2@...", \
			help='Subtract to the MS sample the S samples listed'\
			' Note that the sample subtracted are going to be deleted from the plot, if you want to'\
			' keep any of them, you have to use the option --force')
	parser.add_option( "--force", action='store', dest='force', metavar='SAMPLE1,SAMPLE2,...',\
			help='Force keeping in the plot the samples subtracted using the option "-S"')
	parser.add_option( '-a', "--addname", action='store', dest='samplenames', 
			metavar="newname1@oldname1[,newname2@oldname2,...]",
			help='Change the name which a sample'\
			' will be put in the table. The standard notation comes from "SAMPLENAME_datanames.dn", so to'\
			' modify this behaviour you can use that option"')


	( opt, args ) = parser.parse_args()

	if len(args) == 0 :
		message = "\033[31mprinttable ERROR\033[m Missing mandatory argument signal, see usage."
		sys.exit(message)
	signal = args[0]

	if signal.find("WH") == 0:
		# Check the WH data names (depending of the runperiod)
		dsfrom2011 = glob.glob("*WHToWW2L*")
		dsfrom2012 = glob.glob("*wzttH*ToWW")
		if len(dsfrom2011) != 0 and len(dsfrom2012) == 0:
			signal = signal.replace("WH","WHToWW2L")
		elif len(dsfrom2012) != 0 and len(dsfrom2011) == 0: 
			signal = signal.replace("WH","wzttH")+"ToWW"
		else:
			message = "\033[31mprinttable ERROR\033[m What run period are you working on?."+\
			           " WH signal MC samples not recognized. Exiting..."
			raise RuntimeError(message)
	
	if signal.find("WZ") == 0:
		signal = signal.replace("WZ","WZTo3LNu")

	signalmc = None
	if opt.signalmc:
		signalmc = opt.signalmc
		if signalmc.find("WZ") == 0:
			signalmc = signalmc.replace("WZ","WZTo3LNu")

	if opt.join:
		join = parsermetasamples(opt.join)
	else:
		join = []

	subtract = {}
	if opt.subtract:
		for listofsamples in opt.subtract.split("::"):
			refsample = listofsamples.split('@')[0]
			try:
				sampleslist = listofsamples.split('@')[1].split(",")
			except IndexError:
				message = '\033[1;31mprinttable ERROR\033[1;m Invalid syntax for the'
				message += ' -S option. Option catched as \'%s\'' % opt.subtract
				raise SyntaxError(message)
			subtract[refsample] = [ x for x in sampleslist ] 
	forcekeep = []
	if opt.force and len(subtract) == 0:
		message = '\033[1;33mprinttable WARNING\033[1;m Ignoring --force option, it should'
		message += ' be called with -S option...' 
	elif opt.force:
		available = getsamplenames(os.getcwd())
		forcekeep = opt.force.split(",")
		for sname in forcekeep:
			if sname not in available:
				message = '\033[1;31mplothisto ERROR\033[1;m Force keep sample "%s"' % sname
				message += ' with --force option. But it is not available'
				raise RuntimeError(message)

	# If the user wants to incorporate some names 
	if opt.samplenames:
		message = "\033[31mprinttable ERROR\033[m Malformed arguments on option"
		message += " '-a','--addname'. See the help"
		dosomething=False
		for oldnew in opt.samplenames.split(","):
			oldnew_list =oldnew.split("@")
			if len(oldnew_list) != 2:
				sys.exit(message)
			TITLEDICT[oldnew_list[1]]=oldnew_list[0]
			dosomething=True
		if not dosomething:
			raise RuntimeError(message)


	print "\033[34mprinttable INFO\033[m Creating yields table for "+signal+" analysis",
	sys.stdout.flush()
	t = table(opt.dataname,signal,isreduced=opt.isreduced,\
			wildcardfiles=opt.wildcardfiles,join=join,\
			subtract=subtract,force=forcekeep,signalmc=signalmc,
			datadriven=opt.datadriven)
	print "( ",
	sys.stdout.flush()
	for fileoutput in opt.output.split(","):
		t.saveas(fileoutput,True)
		notredfilename = fileoutput.replace(".","_large.")
		t.saveas(notredfilename,False)
		print fileoutput+" ",
		sys.stdout.flush()
	print ")"




