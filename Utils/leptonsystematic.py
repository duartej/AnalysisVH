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
		from math import sqrt

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
		for leptonflavor,dictoffiles in self.__sf__.iteritems():
			for sftype,h in dictoffiles.iteritems():
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
						error = h.GetBinError(globalbin)
						# Including the systematic uncertainties on T&P recommended by MuonPOG
						# https://twiki.cern.ch/twiki/CMS/MuonTagAndProbe
						# Adding to the statistic in quadrature
						if leptonflavor == 'Muon':
							if sftype == 'id':
								if ptIn <= 20:
									error = sqrt(error**2+\
											(h.GetBinContent(globalbin)*0.015)**2.0)
								else:
									error = sqrt(error**2.+\
											(h.GetBinContent(globalbin)*0.005)**2.0)
							elif sftype == 'iso':
								if ptIn > 20:
									error = sqrt(error**2.+\
											(h.GetBinContent(globalbin)*0.002)**2.0)
						h.SetBinError(globalbinout,error)

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
	
	def __add__(self,other):
		""".. operator+(self,other) -> sf

		Adding up the yields (weighted)

		:param other: a column instance
		:type other: column

		:return: a column instance
		:rtype:  column

		"""
		# Check coherence: only can be added if
		if self.__leptontype__ != other.__leptontype__ and \
				self.__leptonid__ != other.__leptonid and \
				self.__sftype__ != other.__sftype__:
			message = '\033[1;31msf __add__ operator ERROR\033[1;m '
			message+= 'Cannot be added two instances with one of this'
			message+= ' datamember differents: lepton-id, lepton-flavor or'
			message+= ' scale factor type'
			raise RuntimeError(message)
		# Creating the new instance
		result = self.copy()

		result.__value__       += other.__value__
		result.__valueup__     += other.__valueup__
		result.__valuedown__   += other.__valuedown__
		result.__sigmaoversf__ += other.__sigmaoversf__
		
		return result
	
	def __iadd__(self,other):
		""".. operator+(self,other) -> self

		Adding up the weighted yields

		:param other: a column instance
		:type other: column

		:return: a column instance
		:rtype:  column

		"""
		# Check coherence: only can be added if
		if self.__leptontype__ != other.__leptontype__ and \
				self.__leptonid__ != other.__leptonid and \
				self.__sftype__ != other.__sftype__:
			message = '\033[1;31msf __add__ operator ERROR\033[1;m '
			message+= 'Cannot be added two instances with one of this'
			message+= ' datamember differents: lepton-id, lepton-flavor or'
			message+= ' scale factor type'
			raise RuntimeError(message)

		self.__value__       += other.__value__
		self.__valueup__     += other.__valueup__
		self.__valuedown__   += other.__valuedown__
		self.__sigmaoversf__ += other.__sigmaoversf__

		return self
		

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
	import sys

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
	
	point = nentries/100
	k = 0
	waswrite=False
	for entry in xrange(0,nentries):
		t.GetEntry(entry)
		
		# Progress bar
		if point == 0:
			entersnow = False
		else:
			entersnow = (entry % 5*point) == 0
		
		if entersnow:
			sys.stdout.write("\r\033[1;34mleptonsystematic INFO\033[1;m Processing '"+\
					inputfile+" [ "+"\b"+str(entry/point).rjust(3)+"%]")
			sys.stdout.flush()
			waswrite = True
		# Progress bar end
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
		sfw = []
		for i in xrange(len(leptons)):
			for sftype in SFTYPES[leptons[i]]:
				_sfw,_errweight = w(leptons[i],sftype,pt[i],eta[i])
				sfw.append( (_sfw,_errweight,sftype,i) )
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
	
	f.Close()
	f.Delete()
	strchannel = ''
	if nentries == 0:
		message = "\033[1;33mleptonsystematics WARNING\033[1;m The sample '%s' has 0"\
				% inputfile.split("/")[-1].replace(".root","")
		message+= " events passed the analysis cuts. No output is provided\n"
		sys.stdout.write( message )
		sys.stdout.flush()
	else:
		for i in leptons:
			strchannel += i.lower()[0]
		if waswrite:
			sys.stdout.write("\n")
			sys.stdout.flush()

	return sfvariations,strchannel

def processresult(sfdict):
	"""
	"""
	from math import sqrt
	
	yields = sfdict.values()[0].__value__
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

def updatesysfile(foldertostore,sysdict):
	"""..function updatesysfile(sysdict) 
	
	Build or update the file which is going to be used by any other script needing systematics
	"""
	import os,sys
	from datetime import date
	import glob
	
	today = date.today()
	fileout = os.path.join(foldertostore,"systematics_mod.py")
	# Check if there is already 
	try:
		sysfile = glob.glob(foldertostore+"/systematics_mod.py")[0]
	except IndexError:
		message = '\033[1;31mupdatesysfile ERROR\033[1;m There is no "systematics_mod.py" file'
		message+= ' in "%s" folder. Please send first the "resumesys" utility to create it' % foldertostore
		raise RuntimeError(message)
	#Copying the module
	f = open(sysfile)
	lines = f.readlines()
	
	# Check there wasn't a previous modification by this function. The function should act
	# over the original systematics_mod
	if len(filter(lambda x: x.find("MODIFIED BY 'leptonsystematic'") != -1,lines)) != 0:
		message = '\033[1;31mupdatesysfile ERROR\033[1;m The "systematics_mod.py" file'
		message+= ' was already modified by this function. This behaviour is not accepted.'
		message+= ' This function must act only over the original systematics_mod.py file'
		message+= ' created by the "resumesys" utility'
		f.close()
		raise RuntimeError(message)

	# Include a comment related with this update
	lines[3] = lines[3][:-1]+" || MODIFIED BY 'leptonsystematic': "+today.strftime("%2d-%2m-%Y")+"\n"
	f.close()
	
	# Update the samples
	for name,channeldict in sysdict.iteritems():
		# find the line where starts the SYSname dict
		try:
			lineheader = filter(lambda x: lines[x].find('SYS'+name+' = {') != -1,xrange(len(lines)))[0]
		except IndexError:
			continue
		newline = "\t'LEPTON': { "
		for channel,value in channeldict.iteritems():
			newline += channel+': '+str(value)+', '
		# Check we have all the channels
		for channel in [ 'eee' , 'eem' ,'mme', 'mmm' ]:
			if not channel in channeldict.keys():
				newline+= channel+': 0.0, '
		newline = newline[:-2]+' },\n'
		lines.insert(lineheader+1,newline)
	# Backup copy
	os.rename(sysfile,sysfile.replace('.py','_backup_py'))
	print "\033[1;34mupdatesysfile INFO\033[1;m Created backup file 'systematics_mod_backup_py"
	f = open(sysfile,'w')
	f.writelines(lines)
	f.close()

if __name__ == '__main__':
	import os,sys
	import glob
	from optparse import OptionParser
	from math import sqrt
	from functionspool_mod import parsermetasamples

        #Comprobando la version (minimo 2.4)
        vX,vY,vZ,_t,_t1 = sys.version_info
        if (vX > 2 and vY < 4) or vX < 2:
                message = '\033[1;31mleptonsystematic ERROR\033[1;m I need python version >= 2.4'
                sys.exit( message )
        
	usage="usage: %prog filename.root [options]\n"
	usage+="\nCalculate the systematic propagation of the lepton scale factors"
	usage+="\n"
        parser = OptionParser(usage=usage)
        parser.set_defaults(mode='FU',update=False)
	parser.add_option( '-f', '--folder', action='store',dest='folders',metavar='FOLDER1[,...]',\
			help='Folder (or list of folders) where to find the "cluster_*/Results/*.root"'\
			' files. Incompatible option with "-s"')
	parser.add_option( '-s', '--signal', action='store',dest='signal',\
			help='This option assumes that inside the current directory is going'\
			' to find the channel folder structure with the cluster_*/Results/*.root' \
			' files, where each channel folder begins with SIGNAL')
	parser.add_option( '-m', '--mode', action='store',dest='mode',metavar='FU|PU',\
			help='Set the sytematic mode: FU (Fully Uncorrelated) consideres each lepton and'\
			' each scale factor independently. PU (Partial Uncorrelated), only consider'\
			' as independent the scale factor, all the leptons with the same flavour are variated'\
			' at the same direction simultaneously (Default:FU)')
	parser.add_option( '--merge', action='store',dest='merge',help=parsermetasamples())
	parser.add_option( '-u', '--update', action='store_true', dest='updatemod', help='Update'\
			', the systematic_mod.py file, previously created with the "resumesys" utility,'\
			' with the values for the LEPTON key')
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
	vetodata = lambda x,y: x.split("/")[-1].replace(".root","") != y
	vetofakes= lambda x: vetodata(x,"Fakes_Nt3") and vetodata(x,"Fakes_Nt2") \
			and vetodata(x,"Fakes_Nt1") and vetodata(x,"Fakes_Nt0") \
			and vetodata(x,"Fakes")
	for i in folders:
		prerootfiles = glob.glob(i+'/cluster_*/Results/*.root')
		rootfiles = filter(lambda x: vetodata(x,"Data") and vetofakes(x),prerootfiles)
		if len(rootfiles) == 0:
			try:
				rootfiles = [ args[0] ]
			except IndexError:
				message = '\033[1;31mleptonsystematics ERROR\033[1;m Script not '
				message+= 'properly called. See help usage'
				raise RuntimeError(message)
		samplesoutput[i] = {}
		for rf in rootfiles:
			# Extract the data/mc folders
			name = rf.split('/')[-1].replace('.root','')
			samplesoutput[i][name] = extractsystematics(rf,opt.mode)
			if samplesoutput[i][name][1] == '':
				# There wasn't any event passed the cuts
				samplesoutput[i].pop(name)
	# Considering a sample as sum of several samples
	if opt.merge:
		mergesample = parsermetasamples(opt.merge)
		for metaname,realnames in mergesample.iteritems():
			if len(filter(lambda x: x in samplesoutput.values()[0].keys(),realnames)) == 0:
				message ='\033[1;33mleptonsystematics WARNING\033[1;m There are no real'
				message+='samples to merge %s' % opt.merge
				print message
				break
			for i in folders:
				for realname in filter(lambda x: x in samplesoutput[i].keys(),realnames):
					if not samplesoutput[i].has_key(metaname):
						samplesoutput[i][metaname]= ({},samplesoutput[i][realname][1])
					for key,sfinstance in samplesoutput[i][realname][0].iteritems():
						try:
							samplesoutput[i][metaname][0][key] += sfinstance
						except KeyError:
							samplesoutput[i][metaname][0][key] = sfinstance.copy()
					samplesoutput[i].pop(realname)

	# if no update the systematics_mod file, activate verbose
	# to see anything,...
	if not opt.updatemod:
		opt.verbose = 1
	sysdict = {}
	for folder,samplesdict in samplesoutput.iteritems():
		if opt.verbose > 0:
			print "\033[1;34mleptonsystematic INFO\033[1;m '%s':" % (folder.split("/")[-1])
		for name,(sfobject,channel) in samplesdict.iteritems():
			if opt.verbose > 0:
				print "\033[1;34mleptonsystematic INFO\033[1;m [%s]" % (name)
			relerrdict = processresult(sfobject)
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
			sqrtsum = sqrt(sumup2)
			if opt.verbose > 0:
				print message[:-1]
				print 'Total squared systematic: %4.1f%s' % (sqrtsum*100.0,'%')
			try:
				sysdict[name][channel] = sqrtsum
			except KeyError:
				sysdict[name] = {channel: sqrtsum }
	# Update the systematic module
	if opt.updatemod:
		print "\033[1;34mleptonsystematics INFO\033[1;m Updating the systematics_mod.py"\
				" file with the LEPTON systematic"
		updatesysfile(os.getcwd(),sysdict)



	
	
