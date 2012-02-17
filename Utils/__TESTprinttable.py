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


class row(object):
	"""
	A row is
	"""
	def __init__(self,title):
		"""
		"""
		self.title = title
		self.column= {}



class column(object):
	"""
	A column has a file where to extract the information
	Also it is assumed than the values are extracted from
	an TH1F histogram (at least in this __builddict__ 
	implementation)
	"""
	def __init__(self,file,**keywords):
		""".. class:: column(file[,title=title]) 

		A column is associated with a file containing the information. The 
		file has to get an TH1F histogram (see __builddict__ method) defining
		the rows (the x-axis of the histogram) and the values of the rows
		(the bin content of the histogram). So a column is formed by 
		(rows,values) 
		"""
		import os

		validkeywords = [ "title" ]
		self.title = None
		wildcardfiles = "*.root" # Per default in the current working directory
		for key,value in keywords.iteritems():
			if not key in keywords.keys():
				message  = "\033[1;31mcolumn ERROR\033 Incorrect instantiation of 'column'"
				message += " class. Valid keywords are: "+str(validkeywords)
				raise message
			if key == 'title':
				self.title = value

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

	def add(self,col):
		"""
		FIXME: TO BE CHECKED: TO OVERLOAD OPERATOR ADD
		"""
		from math import sqrt
		# Checks
		if self.rowvaldict.keys() != col.rowvaldict.keys():
			raise TypeError,"Cannot be added because they don't have the same"+\
					" row composition"
		
		addeddict = {}
		for cutname,(val,err) in self.rowvaldict.iteritems():
			val  += col.rowvaldict[cutname][0]
			swap = sqrt(err**2.0+col.rowvaldict[cutname][1]**2.0)
			addeddict[cutname] = (val,swap)

		self.rowvaldict = addeddict

		
class table(object):
	"""
	"""
	def __init__(self,data,signal,**keywords):
		"""..class:: table(data,signal[,format=fr, isreduced=True|False, fakemode=True|False, wildcardfiles=dir]) 
		
		The table is composed by several 'columns' (see 'column' class), and is 
		going to be built as
		               bkg1  bkg2 ... TotBkg Data signal
		

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
		"""
		import glob
		import ROOT
		import os
		# First checkings:
		if not os.getenv("VHSYS"):
			raise "\033[1;31mtable ERROR\033[1;m Initialize your"+\
					" environment (VHSYS env variable needed)"

		# Reduced table or not
		self.reduced   = False
		self.fakemode  = False

		formatprov = None

		validkeywords = [ "format", "isreduced", "join", "wildcardfiles" ]
		wildcardfiles = "*.root" # Per default
		join = []
		for key,value in keywords.iteritems():
			if not key in keywords.keys():
				message  = "\033[1;31mtable ERROR\033 Incorrect instantiation of 'table'"
				message += " class. Valid keywords are: "+str(validkeywords)
				raise message

			if key == 'format':
				formatprov = value
			elif key == 'isreduced':
				self.reduced = value
			elif key == "wildcardfiles":
				wildcardfiles = value
			elif key == "join":
				join = value

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

		# building the columns
		self.columns = {}
		for f in self.filenames:
			col = column(f)
			self.columns[col.title] = col
		
		# samples names
		self.samples = self.columns.keys()
		# Check the signal and data are there
		if not signal in self.samples:
			raise "\033[1;31mtable ERROR\033[1;m The signal introduced '"+signal+"' has not been found."
		if not data in self.samples:
			raise "\033[1;31mtable ERROR\033[1;m The data introduced '"+data+"' has not been found."

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
		self.columntitles.append( self.signal )

		# 2) Merge Z+Jets, Drell-Yan and other backgrounds in order to
		# get a more printable table, just if it was demanded
		if self.reduced:
			valdict = {}
			for cut,sampledict in self.values.iteritems():
				valdict[cut] = {}
				for totalsample in [ "DY", "Z+Jets", "Other" ]:
					valdict[cut][totalsample] = self.addupsample(cut,totalsample)

			# Deleting the old keys and substituing by the new ones
			for cut in self.cutordered:
				for sample2add in [ "DY", "Z+Jets", "Other" ]:
					for sample2del in self.getsamplecomponents(sample2add):
						# Deleting
						self.values[cut].pop(sample2del)
					# New total samples
					self.values[cut][sample2add] = valdict[cut][sample2add]
			# Redoing the column title and sample datamember
			self.columntitles = []
			self.samples = []
			for sample in self.values[self.cutordered[0]].iterkeys():
				self.samples.append( sample )
				if sample == self.signal or sample == self.data:
					continue
				if self.fakemode and (sample == "DY" or sample == "Z+Jets"):
					continue
				self.columntitles.append(sample)
			self.columntitles.append( "TotBkg" )
			self.columntitles.append( self.data )
			self.columntitles.append( self.signal )

		# format specific
		self.format = format()
		if formatprov:
			self.setformat(formatprov)

	def getsamplecomponents(self, totalsample ):
		""".. function::getsamplecomponents( totalsample ) -> [ 'comp1', comp1, ... ]

		Return the names of the samples which define a 'totalsample'

		:param totalsample: final name of the sample which will be add up
		:type totalsample: str
		
		:return: list of the samples names which will compose the total sample
		:rtype:  [ str, str, ... ] 
		"""
		components = []
		if totalsample == "DY":
			components = [ "DYee_Powheg", "DYmumu_Powheg", "DYtautau_Powheg" ]
		elif totalsample == "Z+Jets":
			components = [ "Zee_Powheg", "Zmumu_Powheg", "Ztautau_Powheg" ]
		elif totalsample == "Other":
			components = [ "TbarW_DR", "TW_DR", "WW", "WJets_Madgraph" ]
		else:
			message  = "\033[1;31mgetsamplecomponents ERROR\033 '"+totalsample+"'" 
			message += " not recognized. Valid samples are: DY Z+Jets Other"
			raise message

		return components


	def getvaluestr(self, cut,sample):
		""".. function::getvaluestr( cut, sample ) -> valueanderror

		Extract the value and the error, already weighted to the correct
		luminosity, given the cut and the sample name. The function will
		find the format to return the value using only the most 
		significant decimal numbers depending on how is its errror. The 
		error will be modified following the below rules:
		 - if the error is >= 1.5, then the error is an integer, same as
		 the value
		 - if the error is between 1.0 and 1.5, the error is taken with
		 two decimals 
		 - if the error is < 1.0, then return 

		:param cut: cut name, must be contained in 'self.cutordered'
		:type  cut: str
		:param sample: sample name, must be contained in 'self.samples'
		:type  sample: str

		:return: the value and its error 
		:rtype : (str,str)
		"""
		from math import sqrt

		try:
			val,err=self.values[cut][sample]
		except KeyError:
			# Is total background,
			val = 0.0
			err2 = 0.0
			for s in filter(lambda x: x != self.data and x != self.signal,self.samples):
				val += self.values[cut][s][0]
				err2 += (self.values[cut][s][1])**2.0
			err = sqrt(err2)

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



	def printstr(self,format=''):
		"""
		"""
		if format != '':
			self.setformat(format)

		lines = self.format.tablestart+"\n"
		# Column titles
		lines += self.format.rowstart+self.format.cellrowitstart+"      Cuts "+self.format.cellrowitend
		for s in self.columntitles:
			lines += self.format.cellcolitstart+s+self.format.cellcolitend
		lines += self.format.rowend+" \n"
		#Content 
		# Extract the maximum lenght of the cuts
		maxlenght = str(max(map(lambda x: len(x),self.values.keys())))
		cutformat = "%"+maxlenght+"s"
		for cut in self.cutordered:
			lines += self.format.rowstart+self.format.cellrowitstart
			lines += cutformat % cut
			lines += self.format.cellrowitend
			for sample in self.columntitles:
				lines += self.format.cellstart+self.getvaluestr(cut,sample)+self.format.cellend
			lines += self.format.rowend+" \n"

		lines += self.format.tableend+"\n"

		return lines

		
	def saveas(self, filename):
		"""
		"""
		# Extract the type from the file suffix
		format = filename.split(".")[-1]
		tablelines = self.printstr(format)

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
	
	usage="usage: printtable <WZ|WHnnn|Fakes> [options]"
	parser = OptionParser(usage=usage)
	parser.set_defaults(output="table.tex",isreduced=False,fakemode=False,fakeasdata=False)
	parser.add_option( '-f', '--filename', action='store', type='string', dest='output', help="Output filename, the suffix defines the format," \
			" can be more than one comma separated" )
	parser.add_option( '-F', action='store_true', dest='fakemode', help="Activate the fake mode where the Z+Jets, DY and ttbar are estimated with" \
			" the fakeable object method. Incompatible with '\033[1;39mFakes\033[1;m' signal" )
	parser.add_option( '-r', action='store_true', dest='isreduced', help="More printable version of the table, where several samples have been merged" \
			" into one" )
	parser.add_option( '-d', action='store', dest='dataname', help='Name of the sample to be used in the role of "data"')

	( opt, args ) = parser.parse_args()

	if len(args) == 0 :
		message = "\033[1;31mprinttable ERROR\033[1;m Missing mandatory argument signal, see usage."
		sys.exit(message)
	signal = args[0]
	if signal != "WZ" and signal.find("WH") != 0 and signal != "Fakes":
		message = "\033[1;31mprinttable ERROR\033[1;m Signal '"+signal+"' not implemented, see usage."
		sys.exit(message)

	if signal.find("WH") == 0:
		signal = signal.replace("WH","WHToWW2L")

	if opt.fakeasdata and opt.fakemode:
		message = "\033[1;31mprinttable ERROR\033[1;m Incompatible options '-F' and '-k'. See help."
		sys.exit(message)

	print "\033[1;34mprinttable INFO\033[1;m Creating yields table for "+signal+" analysis",
	sys.stdout.flush()
	t = table(signal,isreduced=opt.isreduced,fakemode=opt.fakemode,dataname=opt.dataname)
	print "( ",
	sys.stdout.flush()
	for fileoutput in opt.output.split(","):
		t.saveas(fileoutput)
		print fileoutput+" ",
		sys.stdout.flush()
	print ")"




