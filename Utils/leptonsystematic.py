#!/usr/bin/env python
"""
Calculates the lepton systematic (scale factors) 
"""
# Maximum short int (in 64b c++ compiler), to fix
# the negative event number bug
MAXINT = 4294967295

CHANNEL = { 0: ('Elec','Elec','Elec'), 3: ('Elec','Elec','Muon'),\
		5: ('Muon','Muon','Elec'), 1: ('Muon','Muon','Muon') }


class weightinfo(object):
	def __init__(self):
		"""class:: weightinfo() -> weightinfo instance
		Encapsulates the weightinfo tree, which stores
		the pt,eta and leptontype of an MC sample
		"""
		self.run      = -1
		self.lumi     = -1
		self.evt      = -1
		self.channel  = -1 

		self.ntruepu  = -1

		self.zlep1pt  = -1
        	self.zlep1eta = -1 
		self.zlep1type= -1
        	
		self.zlep2pt  = -1
        	self.zlep2eta = -1
		self.zlep1type= -1

        	self.wleppt   = -1
        	self.wlepeta  = -1
		self.wleptype = -1

	def get(self,attrname):
		"""method:: get(attrname) -> val
		Get the value of the 'attrname' leaf 

		:param attrname: name of the leaf
		:type attrname: str

		:return: the value of the leaf
		:rtype: int (float depending the leaf)
		"""
		return self.__getattribute__(attrname).GetValue(0)


class weight(object):
	"""
	Mimic the WManager class, manage the scale factors and weights. Also controls
	the the terms which are split the scale factors: 
	SF Muons:     ISO and ID
	SF Electrons: RECO and IDISO
	"""
	def __init__(self,**keywords):
		"""class:: weight([,runperiod]) -> instance
		Create the TH2F maps with the scale factors split by category depending
		of the lepton:
		  * MUON SF: ID, ISO
		  * ELEC SF: RECO, IDISO

		Once the instance is created, it can be called directly to obtain the scale
		factors given a lepton, pt and eta
		"""
		import ROOT
		import os

		#defaul run period
		self.runperiod = '2011'
		# scale factors are split in:
		self.sfsplit = { 'Muon': ('iso','id'), 'Elec': ('idiso','reco') }

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
		sf = 'MuSF_'
		sffnames = { 'Muon': dict([ (i,os.path.join(wmanagerdata,\
				sf+self.runperiod+'_'+muonleptontype+'_'+i+'.root')) \
				for i in self.sfsplit['Muon'] ]) }

		ename = lambda x: x.replace('Mu','Ele').replace('_'+muonleptontype,'')
		sffnames['Elec'] = dict([ (i,os.path.join(wmanagerdata,\
				ename(sf+self.runperiod+'_'+muonleptontype+'_'+i+'.root'))) \
				for i in self.sfsplit['Elec'] ])

		getth2 = lambda f: map(lambda x: f.Get(x.GetName()),\
				filter(lambda x: x.GetClassName() == 'TH2F',f.GetListOfKeys()))[0]
		
		# Extract the histos
		ROOT.TH2.AddDirectory(0)
		self.__sf__ = dict([(i,{}) for i in sffnames.keys()])
		for lepton,dictoffiles  in sffnames.iteritems():
			for ktype,filename in dictoffiles.iteritems():
				f = ROOT.TFile(filename)
				try:
					self.__sf__[lepton][ktype] = getth2(f)
				except KeyError:
					self.__sf__[lepton] = { ktype: [getth2(f)] }
				f.Close()
				f.Delete()
		
		# Some modifications (mimic WManager class):
		for dictoffiles in self.__sf__.values():
			for h in dictoffiles.values():
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
						h.SetBinError(globalbinout,h.GetBinError(globalbin))

	def __call__(self,leptype,sftype,pt,eta,):
		"""method:: weight(leptype,sftype,pt,eta) -> (central,err)
		Return the SF central value and sigma.

		:param leptype: the lepton flavour ('Muon' or 'Elec')
		:type leptype: str
		:param sftype: the scale factor type (flavour dependent:
		                                      * Muon: iso, id
						      * Elec: reco, idiso
		:type sftype: str
		:param pt: the lepton's transverse momentum
		:type pt: float
		:param eta: the lepton's pseudorapidity
		:type eta: float

		:return: the scale factor central value and its sigma
		:rtype: tuple(float,float)
		"""

		bin = self.__sf__[leptype][sftype].FindBin(pt,abs(eta))

		return (self.__sf__[leptype][sftype].GetBinContent(bin),\
				self.__sf__[leptype][sftype].GetBinError(bin))

	def getsftypes(self,leptype):
		"""method:: getsftype(leptype) -> [sf1,sf2,...]
		Return the list of scale factors types producing the total scale factor

		:param leptype: the lepton flavor
		:type leptype: str

		:return: the list of scale factors types uses the multiply the final sf
		:rtype: list(str)
		"""
		return self.__sf__[leptype].keys()


class sf(object):
	"""
	Class encapsulating the full calculation of scale factors
	"""
	def __init__(self,leptonid,leptontype,sftype,initval=0.0,**keywords):
		"""class sf(leptonid,leptontype,sftype[,initval=0.0,mode=FU|PU]
		This class encapsulates the scale factor systematic calculation, 
		either event by event or as sum of events. A sf object is fully
		defined giving a leptontype, a identification to the lepton and
		the scale factor type.		
		Using it to calculate the scale factor of an event, the initval 
		has to be set to 1.0, otherwise if the sf class instance is going
		to calculate the total weighted events, the initval argument it 
		must be set to 0.0 (default). 
		The weight calculation algorithm is depending of the mode introduced
		by the user:
		   * mode=FU, stands for FullyUncorrelated: the systematic is considered
		   fully uncorrelated, the sigma variations are done independently for each
		   lepton and for each scale factor type
		   * mode=PU, stands for ParticalUncorrelated: the systematic is considered
		   fully uncorrelated regarding the scale factor type, but the sigma variations 
		   are moved simultaneously for every lepton with the same flavour
		"""
		self.mode = 'FU'
		for key,val in keywords.iteritems():
			if key == 'mode':
				if not val in [ 'FU', 'PU' ]:
					message = '\033[1;31msf ERROR\033[1;m Invalid mode "%s"' % val
					message+= '. Valid modes are "PU" or "FU". See help'
					raise RuntimeError(message)
				self.mode = val
			else:
				message = '\033[1;31msf ERROR\033[1;m Invalid instance key "%s"' % key
				message+= '. Valid keys are "mode"'
				raise RuntimeError(message)

		self.__leptontype__  = leptontype
		self.__leptonid__    = leptonid
		self.__sftype__      = sftype
		self.__value__       = initval
		self.__valueup__     = initval
		self.__valuedown__   = initval
		self.__sigmaoversf__ = 0.0

	def __str__(self):
		"""method:: __str__() -> str
		Representation 
		"""
		return str((self.__value__,self.__valueup__,self.__valuedown__))

	def copy(self):
		"""method:: copy() -> sf instance
		Performs a copy of the object

		:rtype: sf
		"""
		other = sf(self.__leptonid__,self.__leptontype__,self.__sftype__,self.__value__)
		other.__valueup__      = self.__valueup__
		other.__valuedown__    = self.__valuedown__
		other.__sigmaoversf__  = self.__sigmaoversf__

		return other


	def setsfanderr(self,leptonid,sfval,err,sftype):
		"""method:: setsfanderr(leptonid,sfval,err,sftype)
		Update the sf weight related with this leptonid (or lepton flavour type)
		The update is done as follows:
		  * the central value is multiply it by the old central value
		  * if the leptonid (or the leptonflavour if mode=PU) matches with the 
		  leptonid of the instance:
		     * the up (down) scale factor is multiplied by (sfval+(-)err)
		  * else,
		     * the up (down) scale factor is multiplied by sfval
		
		:param leptonid: unique int that identifies the lepton in the event if mode FU,
		                 or lepton flavour if mode PU
		:type leptonid: int (if mode=FU) or str (if mode=PU)
		:param sfval: scale factor central value
		:type sfval: float
		:param err: scale factor error (sigma)
		:type err: float
		:param sftype: the scale factor type which is sfval and err
		:type err: str
		"""
		# Behaviour depending of the mode
		leptonattr = self.__leptonid__
		if self.mode == 'PU':
			leptonattr = self.__leptontype__
		self.__value__ *= sfval
		if sftype == self.__sftype__ \
				and leptonid == leptonattr :
			self.__valueup__      *= (sfval+err)
			self.__valuedown__    *= (sfval-err)
			self.__sigmaoversf__  += err/sfval
		else:
			self.__valueup__   *= sfval
			self.__valuedown__ *= sfval

	def addevent(self,sfobject):
		"""method:: addevent(sfobject) 
		Update the sf instance with the values of the sfobject,
		just adding the relevant data-members. Function used
		to add events up

		:param sfobject: sf instance object to be added
		:type sfobject: sf
		"""
		self.__value__       += sfobject.__value__
		self.__valueup__     += sfobject.__valueup__
		self.__valuedown__   += sfobject.__valuedown__
		self.__sigmaoversf__ += sfobject.__value__*sfobject.__sigmaoversf__
	
	def getvariations(self):
		"""method:: getvariations() -> (up,down)
		Return the up and down weighted yields

		:return: up and down weighted yields
		:rtype: tuple(float,float)
		"""
		return (self.__valueup__,self.__valuedown__)

	def getsigma(self):
		"""method:: getsigma() -> sigma
		Return the sum of sigma_sftype/sfval_sftype
		
		:return: sigma_sftype/sfval_sftype
		:rtype: float
		"""
		return self.__sigmaoversf__



def extractsystematics(inputfile,mode='FU'):
	"""
	"""
	import ROOT

	# Check valid modes
	if not mode in [ 'FU', 'PU' ]:
		message = '\033[1;31mextractsystematics ERROR\033[1;m Invalid mode "%s"' % mode
		message+= '. Valid modes are "PU" or "FU". See help'
		raise RuntimeError(message)

	# Get the weights
	w = weight()
	
	f = ROOT.TFile(inputfile)
	if f.IsZombie():
		raise RuntimeError("\033[1;31mleptonsystematic ERROR\033[1;m ROOT file not found")

	t = f.Get("weighttree")
	if t == None:
		raise RuntimeError("\033[1;31mleptonsystematic ERROR\033[1;m Tree 'weighttree' '\
				'is not defined in the rootfile")
	nentries = t.GetEntries()
	winfobranch = t.GetBranch("weighttree")
	# init instance struc
	wi = weightinfo()
	# Initialize the leaves
	for leaf in t.GetListOfLeaves():
		wi.__setattr__(leaf.GetName(),leaf)

	SFTYPES  = { 'Muon': w.getsftypes('Muon'), 'Elec': w.getsftypes('Elec') }
	sfvariations = {}
	yields = 0.0

	for entry in xrange(0,nentries):
		t.GetEntry(entry)

		if entry % 1000 == 0:
			print "%i-event processed (%i Event Number)" % (entry,wi.evt.GetValue(0))
		# extract pt,eta,category and channel
		channel = wi.get('channel')
		leptons = CHANNEL[channel]
		# Ordered extraction of the pt,eta of the leptons
		# Note that the order is fixed 
		pt1 = wi.get('zlep1pt')
		eta1 = wi.get('zlep1eta')

		pt2 = wi.get('zlep2pt')
		eta2 = wi.get('zlep2eta')
		
		ptw = wi.get('wleppt')
		etaw = wi.get('wlepeta')

		pt = (pt1,pt2,ptw)
		eta= (eta1,eta2,etaw)
		
		currsf = {}
		if mode == 'FU':
			for i in xrange(len(leptons)):
				for sftype in SFTYPES[leptons[i]]:
					currsf[(sftype,i)] = sf(i,leptons[i],sftype,1.0)
		else:
			for leptontype,sftypelist in SFTYPES.iteritems():
				for sftype in sftypelist:
					currsf[(sftype,leptontype)] = sf(-1,leptontype,sftype,1.0,mode='PU')

		# Using this info to weight
		currentyield = 1.0
		sfw = []
		for i in xrange(len(leptons)):
			for sftype in SFTYPES[leptons[i]]:
				_sfw,_errweight = w(leptons[i],sftype,pt[i],eta[i])
				sfw.append( (_sfw,_errweight,sftype,i) )
				currentyield *= _sfw
		if mode == 'FU':
			for (_sfw,errweight,sftype,leptonid) in sfw:
				for sfobject in currsf.values():
					sfobject.setsfanderr(leptonid,_sfw,errweight,sftype)
		else:
			for sfobject in currsf.values():
				for (_sfw,errweight,sftype,leptonid) in sfw:
					sfobject.setsfanderr(leptons[leptonid],_sfw,errweight,sftype)

		# Updating general info
		for (sftype,i),sfobject in currsf.iteritems():
			try:
				sfvariations[(sftype,i)].addevent(sfobject)
			except KeyError:
				sfvariations[(sftype,i)] = sfobject.copy()

		yields += currentyield
	
	f.Close()
	f.Delete()

	return yields,sfvariations

def processresult(yields,sfdict):
	"""
	"""
	from math import sqrt
	relerr = lambda x: abs(x-yields)/yields

	relerrdict = {}
	sigma = 0.0
	for (sftype,id),sfobject in sfdict.iteritems():
		val = max(map(lambda x: relerr(x),sfobject.getvariations()))
		sigma += sfobject.getsigma()**2.0
		try:
			relerrdict[(id,sfobject.__leptontype__)][sftype] = val
		except KeyError:
			relerrdict[(id,sfobject.__leptontype__)] = {sftype: val}
	return relerrdict


if __name__ == '__main__':
	import os,sys
	import glob
	from optparse import OptionParser
	from math import sqrt

        #Comprobando la version (minimo 2.4)
        vX,vY,vZ,_t,_t1 = sys.version_info
        if (vX > 2 and vY < 4) or vX < 2:
                message = '\033[1;31mleptonsystematic ERROR\033[1;m I need python version >= 2.4'
                sys.exit( message )
        
	usage="usage: %prog filename.root [options]\n"
	usage+="\nCalculate the systematic propagation of the lepton scale factors"
	usage+="\n"
        parser = OptionParser(usage=usage)
        parser.set_defaults(verbose=False,mode='FU')
	parser.add_option( '-f', '--folder', action='store',dest='folders',metavar='FOLDER1[,...]',\
			help='Folder (or list of folders) where to find the Nt0, Nt1, Nt2 and Nt3'\
			' Fakes measurements. Incompatible option with "-s"')
	parser.add_option( '-s', '--signal', action='store',dest='signal',\
			help='This option assumes that inside the current directory is going'\
			' to find the channel folder structure with the Nt0, Nt1, Nt2 and Nt3' \
			' calculations, where each channel folder begins with SIGNAL')
	parser.add_option( '-m', '--mode', action='store',dest='mode',metavar='FU|PU',\
			help='Set the sytematic mode: FU (Fully Uncorrelated) consideres each lepton and'\
			' each scale factor independently. PU (Partial Uncorrelated), only consider'\
			' as independent the scale factor, all the leptons with the same flavour are variated'\
			' at the same direction simultaneously (Default:FU)')
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
			message = '\033[1;31mleptonsystematic ERROR\033[1;m Input root file mandatory!'
			raise RuntimeError( message )
		folders = [ args[0] ]
	# Check the mode
	if not opt.mode in [ 'FU', 'PU' ]:
		message = '\033[1;31mextractsystematics ERROR\033[1;m Invalid mode "%s"' % opt.mode
		message+= '. Valid modes are "PU" or "FU". See help'
		raise RuntimeError(message)

	samplesoutput = {}
	for i in folders:
		rootfiles = glob.glob(i+'/cluster_*/Results/*.root')
		if len(rootfiles) == 0:
			rootfiles = [ args[0] ]
		samplesoutput[i] = {}
		for rf in rootfiles:
			# Extract the data/mc folders
			print "\033[1;34mleptonsystematic INFO\033[1;m Evaluating scale factor systematics from '%s'" % \
					(rf)
			name = rf.split('/')[-1].replace('.root','')
			samplesoutput[i][name] = extractsystematics(rf,opt.mode)
	
	for folder,samplesdict in samplesoutput.iteritems():
		print "\033[1;34mleptonsystematic INFO\033[1;m '%s':" % (folder.split("/")[-1])
		for name,(yields,sfobject) in samplesdict.iteritems():
			relerrdict = processresult(yields,sfobject)
			sumup2 = 0.0
			message = ''
			for (ind,leptontype),sfdict in relerrdict.iteritems():
				try:
					# 'FU' mode
					message += "Systematic [%i::%s]: " % (ind,leptontype)
				except TypeError:
					# 'PU' mode
					message += "Systematic [%s]: " % (leptontype)
				for sftype,value in sfdict.iteritems():
					if value == 0.0:
						message += '%4s%s (%5s) ' % ('--','%',sftype)
					else:
						message += '%4.1f%s (%5s) ' % (value*100.0,'%',sftype)
						sumup2 += value**2.0
				message += '\n'
			print message[:-1]
			print 'Total squared systematic: %4.1f%s' % (sqrt(sumup2)*100.0,'%')

	
	
