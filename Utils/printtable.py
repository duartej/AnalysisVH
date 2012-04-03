#!/usr/bin/env python


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
			self.tablestart += '}'	        	
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
			raise "\033[1;31mformat ERROR\033[1;m Format "+format+\
					" not implemented"

		self.format = format


class column(object):
	"""
	A column has a file where to extract the information
	Also it is assumed than the values are extracted from
	an TH1F histogram (at least in this __builddict__ 
	implementation)
	"""
	def __init__(self,file,**keywords):
		""".. class:: column(file[,title=title,nobuilt=True|False]) 

		A column is associated with a file containing the information. The 
		file has to get an TH1F histogram (see __builddict__ method) defining
		the rows (the x-axis of the histogram) and the values of the rows
		(the bin content of the histogram). So a column is formed by 
		(rows,values).
		Also, if it is called with 'nobuilt=True' it is created a instance
		without fill any of its datamember, so the user must do that.
		"""
		import os

		validkeywords = [ "title", "nobuilt" ]
		self.title = None
		wildcardfiles = "*.root" # Per default in the current working directory
		for key,value in keywords.iteritems():
			if not key in keywords.keys():
				message  = "\033[1;31mcolumn ERROR\033 Incorrect instantiation of 'column'"
				message += " class. Valid keywords are: "+str(validkeywords)
				raise message
			if key == 'title':
				self.title = value
			if key == 'nobuilt':
				self.title = None
				self.filename = None
				self.cutordered = None
				self.rowvaldict = None
				return

		self.filename = file
		# FIXME: Must be a root file, checks missing
		# Define the title of the column as the last word 
		# before the .root (if the client doesn't provided it)
		if not self.title:
			self.title = os.path.basename(self.filename).split(".")[0]

		# Extract the values and stores to a dict
		self.rowvaldict = self.__builddict__()

	def __builddict__(self):
		"""..method: __builddict__() -> { 'cutname1': (val,err), ... } 
		This could be a pure virtual, in order to be used by anyone
		and  any storage method (not just a histo). So this is concrete
		for my analysis: fHEventsPerCut  histogram
		Build a dict containing the values already weighted by its luminosity
		"""
		import ROOT
		from array import array
		import os

		f = ROOT.TFile(self.filename)
		# Including the luminosity, efficiency weights,,...
		if "Data.root" in self.filename:
			weight = 1.0
		elif "Fakes.root" in self.filename:
			weight = 1.0
		else:
			# 1) Load the InputParameters
			ROOT.gSystem.SetDynamicPath(ROOT.gSystem.GetDynamicPath()+":"+os.getenv("VHSYS")+"/libs")
			ROOT.gSystem.Load("libInputParameters.so")

			weight = 1.0
			xs = array('d',[0.0])
			luminosity = array('d',[0.0])
			neventsample = array('i',[0])
			neventsskim  = array('i',[0])
			ip = f.Get("Set Of Parameters")
			ip.TheNamedDouble("CrossSection",xs)
			ip.TheNamedInt("NEventsSample",neventsample)
			ip.TheNamedInt("NEventsTotal",neventsskim)
			ip.TheNamedDouble("Luminosity",luminosity)
			weight  = xs[0]*luminosity[0]/neventsample[0]
		
		h = f.Get("fHEventsPerCut")
		self.cutordered = []
		valdict = {}
		#FIXME: Control de errores: histo esta
		for i in xrange(h.GetNbinsX()):
			self.cutordered.append( h.GetXaxis().GetBinLabel(i+1) )
			#Initialization of the bin content dict
			valdict[self.cutordered[-1]] = (weight*h.GetBinContent(i+1),weight*h.GetBinError(i+1))
		f.Close()

		return valdict	

	def __add__(self,other):
		""".. operator+(other) -> column 

		Adding up the rowvaldict, so the two columns have to contain the
		same rows. Note that

		:param other: a column instance
		:type other: column

		:return: a column instance
		:rtype:  column

		"""
		from math import sqrt
		# Checks
		# Allowing the a += b operation (when a was instantied using
		# the 'nobuilt=True' argument, in this case rowvaldict=None
		try:
			if set(self.rowvaldict.keys()) != set(other.rowvaldict.keys()):
				raise TypeError,"Cannot be added because they don't have the same"+\
						" row composition"
			hasdict=True
		except AttributeError:
			hasdict=False

		# Case when self was called as a += b
		if not hasdict:
			self.rowvaldict = other.rowvaldict
			self.cutordered = other.cutordered
			return self			
		
		addeddict = {}
		for cutname,(val,err) in self.rowvaldict.iteritems():
			val  += other.rowvaldict[cutname][0]
			swap = sqrt(err**2.0+other.rowvaldict[cutname][1]**2.0)
			addeddict[cutname] = (val,swap)

		#self.rowvaldict = addeddict
		result = column("",nobuilt=True)
		result.rowvaldict = addeddict
		result.cutordered = self.cutordered

		return result

	def getvalerr(self,cutname):
		""".. method:: getvalerr(cutname) -> (val,err)
		"""
		try:
			return self.rowvaldict[cutname]
		except KeyError:
			raise RuntimeError,"column.getvalerr:: the cut '"+cutname+"' is not "+\
					"defined"

		
class table(object):
	"""
	"""
	def __init__(self,data,signal,**keywords):
		"""..class:: table(data,signal[,format="tex|html", isreduced=True|False, wildcardfiles="dir",join="metasample"]) 
		
		The table is composed by several 'columns' (see 'column' class), and is 
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
		"""
		import glob
		import ROOT
		import os
		# First checkings:
		if not os.getenv("VHSYS"):
			raise "\033[1;31mtable ERROR\033[1;m Initialize your"+\
					" environment (VHSYS env variable needed)"

		formatprov = None

		validkeywords = [ "format", "isreduced", "join", "wildcardfiles" ]
		wildcardfiles = "*.root" # Per default
		join = []
		self.usermetasample = {}
		for key,value in keywords.iteritems():
			if not key in keywords.keys():
				message  = "\033[1;31mtable ERROR\033 Incorrect instantiation of 'table'"
				message += " class. Valid keywords are: "+str(validkeywords)
				raise message

			if key == 'format':
				formatprov = value
			elif key == 'isreduced':
				if value:
					join = [ "DY", "Z+Jets", "Other" ]
			elif key == "wildcardfiles":
				wildcardfiles = value
			elif key == "join":
				if type(value) == list:
					join = value
				elif type(value) == dict:
					self.usermetasample = value
					join = self.usermetasample.keys()
				else:
					join = [ value ]

		# available filenames
		self.filenames = glob.glob(wildcardfiles)

		# Just to be sure that only use one WH signal
		if signal.find("WH") == 0:
			#Extract the other WH signals
			potentialSfiles = filter(lambda x : x.find("WH") != -1,self.filenames)
			nonsignalfiles  = filter( lambda x: x.split("/")[-1].split(".root")[0] != signal,potentialSfiles)
			# Removing
			for f in nonsignalfiles:
				self.filenames.remove(f)
			# Added the important cuts to be used in the reduced table case
			self.importantcutsdict = { } # FIXME: TO BE DONE!!
			self.importantcutslist = []
		else:
			# Added the important cuts to be used in the reduced table case
			# FIXME WARNING HARDCODED!! --> THis is a temporal patch...
			self.importantcutsdict = { 'Pre-selection' : 'AtLeast3Leptons',
					'Z' : 'HasZCandidate',  'W': 'MET' }
			self.importantcutslist = [ 'Pre-selection', 'Z', 'W' ]

		# building the columns
		self.columns = {}
		for f in self.filenames:
			col = column(f)
			self.columns[col.title] = col
		
		# samples names
		self.samples = self.columns.keys()
		# Check the signal and data are there
		if not signal in self.samples:
			raise "\033[1;31mtable ERROR\033[1;m The signal introduced '"+signal+"' has not been found."+\
					" Check you have not introduced the '-n' option without quotes:\n"+\
					" printtable "+signal+" -n \"whatever*..\"" 
		if not data in self.samples:
			raise "\033[1;31mtable ERROR\033[1;m The data introduced '"+data+"' has not been found."+\
					" Check you have not introduced the '-n' option without quotes:\n"+\
					" printtable "+signal+" -n \"whatever*..\"" 

		self.signal = signal
		self.data   = data

		# Ordered samples names (column names)
		self.columntitles = []
		for i in self.samples:
			if i == self.signal or i == self.data:
				continue
			self.columntitles.append(i)
		self.columntitles.append( "TotBkg" )
		self.columntitles.append( self.data )
		self.columntitles.append( "Nobs-Nbkg" )
		self.columntitles.append( self.signal )

		# The datamember samples is superceeded by columntitles
		self.samples = self.columntitles

		# 2) Merge some samples just in one
		for metasample in join:
			samplestodelete = []
			self.columns[metasample] = column("",nobuilt=True)
			for sample in self.getsamplecomponents(metasample):
				self.columns[metasample] += self.columns[sample]
				samplestodelete.append( sample )
			# Put the title
			self.columns[metasample].title = metasample
			# Erase the samples merged 
			for i in xrange(len(samplestodelete)-1):
				self.columns.pop(samplestodelete[i])
				self.columntitles.remove(samplestodelete[i])
			self.columns.pop(samplestodelete[-1])
			# And substitute the last sample by the metasample
			indexlast = self.columntitles.index(samplestodelete[-1])
			self.columntitles[indexlast] = metasample


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
		if metasample == "DY":
			components = [ "DYee_Powheg", "DYmumu_Powheg", "DYtautau_Powheg" ]
		elif metasample == "Z+Jets":
			components = [ "Zee_Powheg", "Zmumu_Powheg", "Ztautau_Powheg" ]
		elif metasample == "Other":
			components = [ "TbarW_DR", "TW_DR", "WW", "WJets_Madgraph" ]
		elif metasample in self.usermetasample.keys():
			components = self.usermetasample[metasample]
		else:
			message  = "\033[1;31mgetsamplecomponents ERROR\033[1;m '"+metasample+"'" 
			message += " not recognized. Current valid metasamples are:"
			message += " 'DY' 'Z+Jets' 'Other' and user introduced '"
			message += str(self.usermetasample.keys())+"'"
			raise RuntimeError,message

		return components


	def getMSvalerr(self, cut,sample):
		""".. function::getMSvalerr( cut, sample ) -> valueanderror

		Extract the value and the error, given the cut and the sample name. 
		The function will find the format to return the value using only the most 
		significant decimal numbers depending on how is its errror. The 
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
		# FIXME: TO BE MODIFIED: IT NEEDS SOME IMPROVEMENTS (THERE ARE MINOR BUGS)
		from math import sqrt
	
		# Dealing with the TotBkg sample which has to be built 
		if sample == "TotBkg":
			val = 0.0
			err2 = 0.0
			for s in filter(lambda x: x != self.data and x != self.signal and x != "TotBkg" and x != "Nobs-Nbkg",self.samples):
				(v,e) = self.columns[s].getvalerr(cut)
				val += v
				err2 += e**2.0
				err = sqrt(err2)
				try:
					self.columns["TotBkg"].rowvaldict[cut] = (val,err)
				except KeyError:
					# Creating the column
					self.columns["TotBkg"] = column("",nobuilt=True)
					# Inititalizing dict
					self.columns["TotBkg"].rowvaldict= { cut: (val,err) }

		# extracting the values
		try:
			val,err=self.columns[sample].getvalerr(cut)
		except KeyError:
			# It's substraction data - TotBkg columns
			if sample == "Nobs-Nbkg":
				# Note that as it has been extract by order, Data and total background
				(valdata,errdata) = self.columns["Data"].getvalerr(cut)
				(valbkg,errbkg)   = self.columns["TotBkg"].getvalerr(cut)
				val = valdata-valbkg
				err = sqrt(errdata**2.0+errbkg**2.0)

		# Begin the formatting
		# Found the last significant value: we get the first value
		# > 0 
		#errstr      = str(err)
		#valstr      = str(val)
		nafterpoint = 0 
		# Case > 1.5
		if abs(err) < 1e-30:
			return ("%i" % val)
		elif err >= 1.5 and err < 2.0:
			errstr = "%.1f" % err
			valstr = "%.1f" % val
			nafterpoint = 1
		# Case 1 > err > 1.5
		elif err >= 1.0 and err < 1.5 :
			errstr = "%.2f" % err
			valstr = "%.2f" % val
			nafterpoint = 2
		elif err >= 2.0:
			errstr = "%i" % round(err)
			valstr = "%i" % round(val)
			nafterpoint = 0
		elif err < 1.0 and err > 0.0:
			getdecimal = False
			errstrPRE = str(err).split(".")[-1]
			numbuilt = ''
			for n,index in zip(errstrPRE,xrange(len(errstrPRE))):
				nafterpoint += 1
				if int(n) > 0:
					numbuilt += n
				try:
					dum = int(numbuilt)
				except ValueError:
					continue
				if int(numbuilt) >= 2: 
					# done
					# --- Check the first is not 1
					nsignumberformat = "%.0e"
					if numbuilt[0] == "1":
						nsignumberformat = "%.1e"
					if nafterpoint > 3:
						errstr = nsignumberformat % err
						exponent= int(errstr.split("e-")[-1])
						errstr = errstr.split("e-")[0]
						# - keeping the integer part 
						#   FIXME: assume at least 10^-3...
						valstr = '%i' % val
						decpart= val-int(valstr)
						if valstr == '0':
							valstr = ''
						if exponent != nafterpoint:
							missing = nafterpoint-exponent
							decpartstrformat = "%."+str(exponent+missing)+"f" 
						else:
							decpartstrformat = "%."+str(exponent)+"f" 
						# To rounding properly
						decpartstrPRE = decpartstrformat % decpart
						# get only the decimal part
						decpartstrONLY = decpartstrPRE.split(".")[-1]
						# Moving down the decimal point
						decpartstr = ''
						for i in xrange(exponent):
							decpartstr += decpartstrONLY[i]
						decpartstr = str(int(decpartstr))

						if numbuilt[0] == "1":
							valstr += decpartstr[:-1]+"."+decpartstr[-1]
						else:
							valstr += decpartstr
					else:
						howmany0     = nafterpoint-len(numbuilt)
						errstrformat = "%."+str(nafterpoint)+"f"
						errstr       = errstrformat % err
						#errstr       = "0."+"0"*howmany0+numbuilt
						valstrformat = "%."+str(howmany0+len(numbuilt))+"f"
						valstr  = valstrformat % val

					break
	
		if nafterpoint > 3:
			totalvalstr = "("+valstr+self.format.plusminus+errstr+")"+self.format.cdot+"10"+\
					self.format.exponentstart+"-"+str(exponent)+self.format.exponentend
		else:
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
			lines += self.format.cellcolitstart+s+self.format.cellcolitend
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
		# Extract the maximum lenght of the samples
		maxlenght = str(max(map(lambda x: len(x),self.columntitles)))
		sampleformat = "%"+maxlenght+"s"
		# Build the table row by row (sample by sample)
		for sample in self.columntitles:
			lines += self.format.rowstart+self.format.cellrowitstart
			lines += sampleformat % sample
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
	from optparse import OptionParser
	
	#Comprobando la version (minimo 2.4)
	vX,vY,vZ,_t,_t1 = sys.version_info
	if vX > 2 and vY < 4:
		message = '\033[1;31printtable ERROR\033[1;m I need python version >= 2.4'
		sys.exit( message )
	elif vX < 2:
		message = '\033[1;31printtable ERROR\033[1;m I need python version >= 2.4'
		sys.exit( message )
	
	usage  ="usage: printtable <WZ|WHnnn|Fakes> [options]"
	parser = OptionParser(usage=usage)
	parser.set_defaults(output="table.tex",isreduced=False,dataname="Data",wildcardfiles="cluster_*/Results/*.root")
	parser.add_option( '-f', '--filename', action='store', type='string', dest='output', help="Output filename, the suffix defines the format," \
			" can be more than one comma separated" )
	parser.add_option( '-r', action='store_true', dest='isreduced', help="More printable version of the table, where several samples have been merged" \
			" into one. Shortcut for '-j [DY,Z+Jetsj,Other]" )
	parser.add_option( '-d', action='store', dest='dataname', help='Name of the sample to be used in the role of "data" [Default: Data]')
	parser.add_option( '-n', action='store', dest='wildcardfiles', help='Path where to find the root '\
			'filenames to extract the table, it could be wildcards. \033[1;33mWARNING:\033[1;m'\
			' if uses wildcards, be sure to be included between quotes: "whatever*"'\
			' [Default: "cluster_*/Results/*.root"]')
	parser.add_option( '-j', action='store', dest='join', metavar="MS|MS1,MS2,...|MS1@S1,..,SN::MS2@S2_1,...,S2_2::...", \
			help='Merge two or more samples into one unique metasample,'\
			' it could be a str defining a pre-built metasample: "DY" "Z+Jets" "Other";'\
			' it could be a list of strings comma separated which are defining more than one pre-built'\
			' metasample; or could be metasample names and a list of samples')


	( opt, args ) = parser.parse_args()

	if len(args) == 0 :
		message = "\033[1;31mprinttable ERROR\033[1;m Missing mandatory argument signal, see usage."
		sys.exit(message)
	signal = args[0]

	if signal.find("WH") == 0:
		signal = signal.replace("WH","WHToWW2L")
	
	if signal.find("WZ") == 0:
		signal = signal.replace("WZ","WZTo3LNu")

	if opt.join:
		if opt.join.find("@") != -1:
			# Dictionary build
			keyvallist = opt.join.split("::")
			join = {}
			for items in keyvallist:
				kv = items.split("@")
				join[kv[0]] = kv[1].split(",")
			# FIXME: Comprobacion de errores?
		elif opt.join.find(","):
			join = opt.join.split(",")
		else:
			join = opt.join
	else:
		join = []


	print "\033[1;34mprinttable INFO\033[1;m Creating yields table for "+signal+" analysis",
	sys.stdout.flush()
	t = table(opt.dataname,signal,isreduced=opt.isreduced,wildcardfiles=opt.wildcardfiles,join=join)
	print "( ",
	sys.stdout.flush()
	for fileoutput in opt.output.split(","):
		t.saveas(fileoutput,True)
		notredfilename = fileoutput.replace(".","_large.")
		t.saveas(notredfilename,False)
		print fileoutput+" ",
		sys.stdout.flush()
	print ")"




