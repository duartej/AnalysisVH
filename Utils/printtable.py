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




class table(object):
	"""
	"""
	def __init__(self,signal,**keywords):
		"""
		"""
		import glob
		import ROOT
		import os
		# First checkings:
		if not os.getenv("VHSYS"):
			raise "\033[1;31mtable ERROR\033[1;m Initialize your"+\
					" environment (VHSYS env variable needed)"

		formatprov = None

		validkeywords = [ "format" ]
		for key,value in keywords.iteritems():
			if not key in keywords.keys():
				message  = "\033[1;31mtable ERROR\033 Incorrect instantiation of 'table'"
				message += " class. Valid keywords are: "+str(validkeywords)
				raise message
			if key == 'format':
				formatprov = value


		# available filenames
		self.filenames = glob.glob("cluster_*/Results/*.root")
		if signal.find("WH") == 0:
			#Extract the other WH signals
			potentialSfiles = filter(lambda x : x.find("WH") != -1,self.filenames)
			nonsignalfiles  = filter( lambda x: x.split("/")[-1].split(".root")[0] != signal,potentialSfiles)
			# Removing
			for f in nonsignalfiles:
				self.filenames.remove(f)
		# samples names
		self.samples = map(lambda x: os.path.basename(x).split(".")[0], self.filenames )
		self.filesamplemap = dict(map(lambda x: (x,os.path.basename(x).split(".")[0]), self.filenames ))
		# Check the signal and data are there
		if not signal in self.samples:
			raise "\033[1;31mtable ERROR\033[1;m The signal introduced '"+signal+"' has not been found."
		if not "Data" in self.samples:
			raise "\033[1;31mtable ERROR\033[1;m The data is not available"
		# signal name
		self.signal  = signal
		self.data    = "Data"

		# Ordered samples for column titles
		self.columntitles = []
		for i in self.samples:
			if i == self.signal or i == self.data:
				continue
			self.columntitles.append(i)
		self.columntitles.append( "TotBkg" )
		self.columntitles.append( self.data )
		self.columntitles.append( self.signal )

		# list of cuts (ordered)
		self.cutordered = []
		f = ROOT.TFile.Open(self.filenames[0])
		# -- FIXME: Control de Errores
		histocut = f.Get("fHEventsPerCut")
		# -- FIXME: Control de Errores
		self.values = {} # {'cut': { 'sample': [ contentperbin,....] , ...]
		for i in xrange(histocut.GetNbinsX()):
			self.cutordered.append( histocut.GetXaxis().GetBinLabel(i+1) )
			#Initialization of the bin content dict
			self.values[self.cutordered[-1]] = {}
		f.Close()

		# 1) Load the InputParameters
		ROOT.gSystem.SetDynamicPath(ROOT.gSystem.GetDynamicPath()+":"+os.getenv("VHSYS")+"/libs")
		ROOT.gSystem.Load("libInputParameters.so")
		# 1a) Set the total events weighted (in the self.values data member)
		for f in self.filenames:
			self.setweightedevents(f)
		
		# format specific
		self.format = format()
		if formatprov:
			self.setformat(formatprov)

	def setweightedevents(self,filename):
		"""
		"""
		from ROOT import TFile
		from array import array

		f = TFile.Open(filename)
		if "Data.root" in filename:
			weight = 1.0
		else:
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

		# Extract all the number of events weighted in each stage
		histocut = f.Get("fHEventsPerCut")
		sample = self.filesamplemap[filename]
		for i in xrange(histocut.GetNbinsX()):
			cut = histocut.GetXaxis().GetBinLabel(i+1)
			self.values[cut][sample] = (weight*histocut.GetBinContent(i+1),weight*histocut.GetBinError(i+1))

		f.Close()

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
					self.format.exponentstart+str(exponent)+self.format.exponentend
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
	
	usage="usage: printtable <WZ|WHnnn> [options]"
	parser = OptionParser(usage=usage)
	parser.set_defaults(output="table.tex")
	parser.add_option( '-f', '--filename', action='store', type='string', dest='output', help="Output filename, the suffix defines the format," \
			" can be more than one comma separated" )

	( opt, args ) = parser.parse_args()

	if len(args) == 0 :
		message = "\033[1;31mprinttable ERROR\033[1;m Missing mandatory argument signal, see usage."
		sys.exit(message)
	signal = args[0]
	if signal != "WZ" and signal.find("WH") != 0:
		message = "\033[1;31mprinttable ERROR\033[1;m Signal '"+signal+"' not implemented, see usage."
		sys.exit(message)

	if signal.find("WH") == 0:
		signal = signal.replace("WH","WHToWW2L")


	print "\033[1;34mprinttable INFO\033[1;m Creating yields table for "+signal+" analysis",
	sys.stdout.flush()
	t = table(signal)
	print "( ",
	sys.stdout.flush()
	for fileoutput in opt.output.split(","):
		t.saveas(fileoutput)
		print fileoutput+" ",
		sys.stdout.flush()
	print ")"




