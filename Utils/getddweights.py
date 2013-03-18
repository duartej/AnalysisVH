#!/usr/bin/env python
"""
The data-driven fakeable method estimates the different Prompt and Fakes contributions
by measuring the lepton tight and fail lepton composition. To calculate the number of
events estimated of a Prompt-Prompt-Fake sample (or any other combination) it is needed
to evaluate the different tight and fail samples composition: 0-tight (Nt0),
1-tight (Nt1), 2-tight-tight (Nt2) and 3-tigth (Nt3). This measurament have to be 
weighted properly to obtain the NPPF estimation.

This script calculates the data-driven estimation (PPP, PPF, PFF and FFF as input argument)
given the Nt0, Nt1, Nt2 and Nt3 measurements. The script should be launched in a folder (or
give as option the folder) which contains the SGch standard structure (SGch stands for 
SG:=name of the process, WZ for instance, and ch:=channel, eee,mme,eem,mmm for instance). 
Below each of the SGch folders there should are the 'cluster_Fakes_Nti' directories, i=0,..,3.
"""
# Maximum short int (in 64b c++ compiler), to fix
# the negative event number bug
MAXINT = 4294967295

CHANNEL = { 0: ('Elec','Elec','Elec'), 3: ('Elec','Elec','Muon'),\
		5: ('Muon','Muon','Elec'), 1: ('Muon','Muon','Muon') }

CHANNELSTR = { 0: '3e', 3: '2e', 5: '2m', 1: '3m' }


class evtinfo(object):
	def __init__(self):
		"""class:: evtinfo() -> evtinfo instance
		Encapsulating the evtinfo tree filling in the 
		data analysis
		"""
		self.run      = -1
		self.lumi     = -1
		self.evt      = -1
		self.channel  = -1 

		self.zlep1cat = -1
		self.zlep2cat = -1
		self.wlepcat  = -1

		self.zmass    = -1
		self.zlep1pt  = -1
        	self.zlep1eta = -1 
        	self.zlep1phi = -1
        	self.zlep2pt  = -1
        	self.zlep2eta = -1
        	self.zlep2phi = -1

        	self.wmt      = -1      
        	self.wleppt   = -1
        	self.wlepeta  = -1
        	self.wlepphi  = -1

        	self.metet    = -1
        	self.metphi   = -1

		self.ordereddm = [ "run","lumi","evt","channel","zlep1cat","zlep2cat","wlepcat",\
				"zmass","zlep1pt",\
				"zlep1eta","zlep1phi","zlep2pt","zlep2eta","zlep2phi",\
				"wmt","wleppt","wlepeta","wlepphi","metet","metphi" ]
		self.dmtype = [ "i","i","i","i","i","i","i",".2f",".2f",".2f",".2f",".2f",".2f",".2f",\
				".2f",".2f",".2f",".2f",".2f",".2f" ]
	
	def get(self,attrname):
		"""method:: get(attrname) -> value
		Get the value for the leaf named 'attrname'

		:param attrname: name of the leaf
		:type attrname: str

		:return: value of the leaf
		:rtype: int (or float depending the leaf)
		"""
		return self.__getattribute__(attrname).GetValue(0)

	def __str__(self):
		"""method:: __str__() -> str
		Method to print the class. Just show all the data-members with
		format

		:rtype: str
		"""
		message = ""
		# Check the tree is initialized
		if self.channel == -1:
			raise RuntimeError("\033[1;31evtinfo ERROR\033[1;m TTree not initialized")
		for index in xrange(0,len(self.ordereddm)):
			formatdm = "%"+self.dmtype[index]+" "
			attrname = self.ordereddm[index]
			value = self.__getattribute__(attrname).GetValue(0)
			if attrname == "channel":
				if value == 3:
					value = 1
				elif value == 5:
					value = 2
				elif value == 1:
					value = 3
			# Fix negative event number bug
			if attrname == "evt" and value < 0:
		#		print value,
				value = MAXINT+value+1
		#		print value
			message += formatdm % value
		message += "\n"

		return message

class weight(object):
	"""
	Class to deal with the weights of a loose lepton. Pythonization of the
	WManager class
	"""
	def __init__(self,**keywords):
		"""class:: weight([,runperiod]) -> weight instance
		Once the instance is created, it is possible to call 
		directly to extract the Prompt-rate or Fake-rate given
		a lepton and its pt,eta
		"""
		import ROOT
		import os

		#defaul run period
		self.runperiod = '2011'

		validkeywords = [ 'runperiod' ]
		for key,value in keywords:
			if key == 'runperiod':
				self.runperiod = value

		# Loading the fakes and prompt rates
		pkgfolder = os.getenv("VHSYS")
		wmanagerdata = os.path.join(pkgfolder,'WManager/data')
		if self.runperiod == '2011':
			muonleptontype = 'vbtf'
		elif self.runperiod == '2012':
			muonleptontype = 'hwwid'
		jet = 'jet50'
		pr = 'MuPR_'
		fr = 'MuFR_'
		prfname = os.path.join(wmanagerdata,pr+self.runperiod+'_'+muonleptontype+'.root')
		frfname = prfname.replace('MuPR_','MuFR_').replace('.root','_'+jet+'.root')

		ename = lambda x: x.replace('Mu','Ele').replace('_'+muonleptontype,'')
		getth2 = lambda f: map(lambda x: f.Get(x.GetName()),\
				filter(lambda x: x.GetClassName() == 'TH2F',f.GetListOfKeys()))[0]
		
		# Extract the histos
		ROOT.TH2.AddDirectory(0)
		fprmu = ROOT.TFile(prfname)
		ffrmu = ROOT.TFile(frfname)
		fprele= ROOT.TFile(ename(prfname))
		frfele= ROOT.TFile(ename(frfname))

		self.__pr__ = { 'Muon': getth2(fprmu), 'Elec': getth2(fprele) }
		self.__fr__ = { 'Muon': getth2(ffrmu), 'Elec': getth2(frfele) }
		
		# Some modifications:
		for h in self.__pr__.values():
			ptLowmax = h.GetXaxis().GetBinCenter(h.GetNbinsX())
			for ptbin in xrange(1,h.GetNbinsX()+2):
				ptIn = h.GetXaxis().GetBinCenter(ptbin)
				ptOut= ptIn
				if ptIn >= ptLowmax:
					ptIn = ptLowmax
				for etabin in xrange(1,h.GetNbinsY()+2):
					eta = h.GetYaxis().GetBinCenter(etabin)
					globalbin = h.FindBin(ptIn,eta)
					globalbinout = h.FindBin(ptOut,eta)
					h.SetBinContent(globalbinout,h.GetBinContent(globalbin))

		for h in self.__fr__.values():
			ptLowmax = h.GetXaxis().GetBinCenter(h.GetNbinsX())
			for ptbin in xrange(1,h.GetNbinsX()+2):
				ptIn = h.GetXaxis().GetBinCenter(ptbin)
				ptOut= ptIn
				if ptIn >= ptLowmax:
					ptIn = ptLowmax
				if ptIn >= 35.0:
					ptIn = 34.0

				for etabin in xrange(1,h.GetNbinsY()+2):
					eta = h.GetYaxis().GetBinCenter(etabin)
					globalbin = h.FindBin(ptIn,eta)
					globalbinout = h.FindBin(ptOut,eta)
					h.SetBinContent(globalbinout,h.GetBinContent(globalbin))

	def __call__(self,leptype,pt,eta):
		"""method:: weight(self,pt,eta) -> (PR,FR)
		Returns the prompt-rate and fake-rate, given a lepton

		:param leptype: the lepton flavour (Elec, Muon)
		:type leptype: str (valid names are 'Elec' 'Muon')
		:param pt: the transverse momentum of the lepton
		:type pt: float
		:param eta: the pseudorapity of the lepton
		:type eta: float

		:return: the prompt-rate and fake-rate 
		:rtype: tuple(float,float)
		"""

		bin = self.__pr__[leptype].FindBin(pt,abs(eta))
		binfr = self.__fr__[leptype].FindBin(pt,abs(eta))

		return (self.__pr__[leptype].GetBinContent(bin),self.__fr__[leptype].GetBinContent(binfr))


class estimator(object):
	"""
	Estimates the Prompt and Fake samples
	"""
	def __init__(self,w):
		"""class:: estimator(w) -> estimator instance
		Once the instance is created, the user can call directly that
		instance to obtain the per-event weight using different hypothesis:
		PPP, PPF, PFF and FFF

		"""
		self.weights = w
	
	def __call__(self,t1,pt1,eta1,cat1,t2,pt2,eta2,cat2,t3,pt3,eta3,cat3):
		"""method:: estimator(t1,pt1,eta1,cat1,t2,pt2,eta2,cat3,t3,pt3,eta3,cat3) 
		                -> { 'PPP': pppval, 'PPF': ppfval, 'PFF': pffval, 'FFF: fffval }
		Calculates the per-event weight

		The weight rules per tight or fail lepton are:
		    Probability to estimate a PROMPT given a TIGHT lepton: 1/(pr-fr)*pr*(1-fr)
		    Probability to estimate a FAKE   given a TIGHT lepton: 1/(pr-fr)*fr*(1-pr)

		    Probability to estimate a PROMPT given a FAIL  lepton: 1/(pr-fr)*pr*fr
		    Probability to estimate a FAKE   given a FAIL  lepton: 1/(pr-fr)*fr*pr

		The function returns the weight for each possibility: given a 3 measured leptons
		(tight or fail, this info is obtained via the self.weights data-member) it returns
		the four possibilities: all the measured leptons are Prompt estimated (PPP), the
		measured leptons are Prompt-Prompt-Fake estimated (PPF), the measured leptons are
		Prompt-Fake-Fake (PFF) estimated and all the measured leptons are Fake (FFF) estimated. 
		Note that the function takes into account the combinatories.

		:param ti: lepton type (Muon or Elec), i: 1,2,3
		:type ti: str
		:param pti: lepton's transverse momentum (i:1,2,3)
		:type pti: float 
		:param etai: lepton's pseudorapity (i:1,2,3)
		:type etai: float
		:param cati: loose category where 101: TIGHT and 102:FAIL (i:1,2,3)
		:type cati: int
		
		:return: dictionary with the weight for each possibility PPP, PPF, PFF and FFF
		:rtype: dict(str: float)
		"""
		prfrtuple = { 1: (self.weights(t1,pt1,eta1),cat1), 2: (self.weights(t2,pt2,eta2),cat2),\
				3: (self.weights(t3,pt3,eta3),cat3) }
		
		# Build the estimator weights: tights
		ewtight = map(lambda ww: ww[0], filter(lambda ww: int(ww[1]) == 101,prfrtuple.values()))
		promptpass = map(lambda ww: 1.0/(ww[0]-ww[1])*ww[0]*(1.0-ww[1]),ewtight)
		fakepass = map(lambda ww: 1.0/(ww[0]-ww[1])*ww[1]*(1.0-ww[0]),ewtight)
		# Build the estimator weights: fails
		ewfail = map(lambda ww: ww[0], filter(lambda ww: int(ww[1]) == 102,prfrtuple.values()))
		promptfail = map(lambda ww: 1.0/(ww[0]-ww[1])*ww[0]*ww[1],ewfail)
		fakefail = map(lambda ww: 1.0/(ww[0]-ww[1])*ww[1]*ww[0],ewfail)

		# Built the several cases:
		fff = 1.0
		for i in fakepass+fakefail:
			fff *= i
		ppp = 1.0
		for i in promptpass+promptfail:
			ppp *= i
		pff = 0.0
		# Indices
		p = promptpass+promptfail
		f = fakepass + fakefail
		pi =  xrange(len(p))
		for ind in pi:
			findices = filter(lambda x: x != ind, xrange(len(f)))
			pffmult = p[ind]
			for find in findices:
				pffmult *= f[find]
			pff += pffmult
		ppf = 0.0
		fi = xrange(len(f))
		for ind in fi:
			pindices = filter(lambda x: x != ind, xrange(len(p)))
			ppfmult = f[ind]
			for pind in pindices:
				ppfmult *= p[pind]
			ppf += ppfmult


		return { 'FFF': fff, 'PFF': pff, 'PPF': ppf, 'PPP': ppp }



def datadriven(inputfile):
	"""function:: datadriven(inputfile) -> (totalweight,list(meas),list(channel),nentries)
	Calculated the weights for the data-driven event by event, adding up the events. Also
	extracts the measurament signature (how many fail and tight leptons has the inputfile) 
	and the channel of the inputfile. Returns a dict with the added up weights for the 
	four possibilities
	"""
	import ROOT

	# Get the weights
	w = weight()
	# And the estimator calculator
	est = estimator(w)
	
	f = ROOT.TFile(inputfile)
	if f.IsZombie():
		raise RuntimeError("\033[1;31mevtinfo ERROR\033[1;m ROOT file not found")

	t = f.Get("evtlist")
	if t == None:
		raise RuntimeError("\033[1;31evtinfo ERROR\033[1;m Tree 'evtlist' is not defined in the rootfile")
	nentries = t.GetEntries()
	evtinfobranch = t.GetBranch("evtinfo")
	# init instance struc
	evti = evtinfo()
	# Initialize the leaves
	for leaf in t.GetListOfLeaves():
		evti.__setattr__(leaf.GetName(),leaf)
	
	totalweight = { 'FFF': 0.0, 'PFF': 0.0, 'PPF': 0.0, 'PPP': 0.0 }
	measurement = set()
	channelset = set()
	for i in xrange(0,nentries):
		t.GetEntry(i)

		#if i % 10 == 0:
		#	print "%i-event processed (%i Event Number)" % (i,evti.evt.GetValue(0))

		# extract pt,eta,category and channel
		channel = evti.get('channel')
		channelset.add( CHANNELSTR[channel] )
		lepts = CHANNEL[channel]
		# Ordered extraction of the pt,eta of the leptons
		pt1 = evti.get('zlep1pt')
		eta1 = evti.get('zlep1eta')
		cat1 = evti.get('zlep1cat')

		pt2 = evti.get('zlep2pt')
		eta2 = evti.get('zlep2eta')
		cat2 = evti.get('zlep2cat')
		
		ptw = evti.get('wleppt')
		etaw = evti.get('wlepeta')
		wcat = evti.get('wlepcat')

		measurement.add( tuple(sorted((int(cat1),int(cat2),int(wcat)))) )

		# Using this info to weight
		wevt = est(lepts[0],pt1,eta1,cat1,lepts[1],pt2,eta2,cat2,lepts[2],ptw,etaw,wcat)
		for key,val in wevt.iteritems():
			totalweight[key]+=val

	f.Close()
	f.Delete()

	return totalweight,list(measurement),list(channelset),nentries

if __name__ == '__main__':
	import os,sys
	import glob
	from optparse import OptionParser

        #Comprobando la version (minimo 2.4)
        vX,vY,vZ,_t,_t1 = sys.version_info
        if vX > 2 and vY < 4:
                message = '\033[1;31mcheckPV ERROR\033[1;m I need python version >= 2.4'
                sys.exit( message )
        elif vX < 2:
                message = '\033[1;31mcheckPV ERROR\033[1;m I need python version >= 2.4'
                sys.exit( message )
        
	usage="usage: %prog filename.root <PPP|PPF|PFF|FFF> [options]\n"
	usage+="\nExtract the weights for the data-driven estimation based in a"
	usage+=" processed sample. The sample should be a 'Fakes' one."
        parser = OptionParser(usage=usage)
        parser.set_defaults(verbose=False)
	parser.add_option( '-f', '--folder', action='store',dest='folders',metavar='FOLDER1[,...]',\
			help='Folder (or list of folders) where to find the Nt0, Nt1, Nt2 and Nt3'\
			' Fakes measurements. Incompatible option with "-s"')
	parser.add_option( '-s', '--signal', action='store',dest='signal',\
			help='This option assumes that inside the current directory is going'\
			' to find the channel folder structure with the Nt0, Nt1, Nt2 and Nt3' \
			' calculations, where each channel folder begins with SIGNAL')
	parser.add_option( '-v', '--verbose', action='count', dest='verbose', help='Verbose mode'\
			', get also the number of raw events (not weighted) at each cut')

        ( opt, args ) = parser.parse_args()

	folders = []
	if opt.signal and opt.folders:
		message = '\033[1;31mgetddweights ERROR\033[1;m Incompatible options "-s" and "-f"'
		raise RuntimeError(message)

	# Assumes full folder content
	if opt.signal:
		folders = glob.glob(opt.signal+'*')
	elif opt.folders:
		folders = opt.folders.split(",")
	else:
		if len(args) < 1:
			message = '\033[1;31mgetddweigths ERROR\033[1;m Input root file mandatory!'
			raise RuntimeError( message )
		folders = [ args[0] ]
	
	totalchannel = {}
	rawentrieschannel = {}
	for i in folders:
		rootfiles = glob.glob(i+'/cluster_Fakes_Nt*/Results/Fakes_Nt*.root')
		if len(rootfiles) == 0:
			rootfiles = [ args[0] ]
		for rf in rootfiles:
			# Extract the data/mc folders
			print "\033[1;34mgetddweigths INFO\033[1;m Evaluating data-driven estimations from '%s'" % \
					(rf)
			totalent,measurement,channelstr,rawentries = datadriven(rf)

			# Get the measurament, number of tight: 101
			if len(measurement) != 1:
				print "\033[1;33mgetddweigths WARNING\033[1;m Number"\
						" of measured type of leptons inconsistent"
				print "Found: ",
				for i in measurement:
					print map(lambda x: int(x),i),
				print
			ntights = len(filter(lambda x: int(x) == 101,measurement[0]))
		
			if len(channelstr) != 1:
				print "\033[1;33mgetddweigths WARNING\033[1;m Number of channels more than one!"
				print "          Found: ",
				for i in channelstr:
					print i,
				print
			channel = channelstr[0]
			try:
				totalchannel[channel][ntights] = totalent
			except KeyError:
				totalchannel[channel] = { ntights : totalent }
			try:
				rawentrieschannel[channel][ntights] = rawentries
			except KeyError:
				rawentrieschannel[channel] = { ntights: rawentries }
		
			if opt.verbose >= 2:
				print "="*50
				print "CHANNEL: %s   Measurement: Nt%i, Raw entries:%i" % (channel,ntights,rawentries)
				print "Nt%i contribution to the: " % (ntights)
				message = ''
				for i in [ 'PPP', 'PPF', 'PFF', 'FFF' ]:
					message += "           +  %4s estimation: %10.6f\n" % (i,totalent[i])
				print message[:-1]
				print "="*50
	if opt.verbose >= 1:
		print "="*50
		print "\033[1;34mgetddweights INFO\033[1;m Raw entries by channel and measurement"
		m = '%10s  ' % ''
		for ch in sorted(rawentrieschannel.keys()):
			m += '|| %6s ' % ch
		m += '\n'
		for ntight in sorted(rawentrieschannel.values()[0].keys()):
			m += '%8sNt%i  ' % ('',ntight)
			for ch in sorted(rawentrieschannel.keys()):
				m += '|| %3i ' % rawentrieschannel[ch][ntight]
			m += '\n'
		print m
		print "="*50


	if len(totalchannel.values()[0].values()) == 1:
		sys.exit(0)
	
	# (nt0,nt1,nt2,nt3)
	RULES = { 'PPP': (-1.0,1.0,-1.0,1.0), 'PPF': (1.0,-1.0,1.0,-1.0),\
			'PFF': (-1.0,1.0,-1.0,1.0), 'FFF': (1.0,-1.0,1.0,-1.0) }
	channelest = {}
	sumchannel = {}
	for ch,totaldmeas in totalchannel.iteritems():
		sumchannel[ch] = 0.0
		for est in ['PPP','PPF','PFF','FFF']:
			summingup = 0.0
			for nt in totaldmeas.keys():
				summingup += RULES[est][nt]*totaldmeas[nt][est]
			try:
				channelest[est][ch] = summingup
			except KeyError:
				channelest[est] = { ch: summingup }
			sumchannel[ch] += channelest[est][ch]
			
	m= '%10s  || %8s  || %8s  || %8s  || %8s\n' % ('','3e','2e','2m','3m')
	for est in ['PPP','PPF','PFF','FFF']:
		m += '%10s' % (est)
		for ch in ['3e', '2e', '2m', '3m']:
			try:
				m += '  || %3.4f ' % (channelest[est][ch])
			except KeyError:
				m += '  || %8s ' % ('-')
				sumchannel[ch] = 0.0
		m +='\n'
	m += '%12s' % (' ')
	for ch in ['3e', '2e', '2m', '3m']:
		m += '|| %3.4f  ' % sumchannel[ch]
	print m
	
