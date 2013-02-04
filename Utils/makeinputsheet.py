#!/usr/bin/env python
"""
Build a default configuration input sheet needed for runanalysis executable.
"""
# Indices to extract values depending the lepton type
ELEC=0
MUON=1

# Description 

DESCRIPTION = { 'General': { "NameTree": "Name of the TTree object [string]",
		                   "treeType": "The enumerate of the tree type: see  TreeManager/interface/TreeTypes.h [int]",
				   "nEvents":  "Events to be processed [int]",
				   "firstEvent":"First event to be processed [int]",
				   'Luminosity':"Luminosity of the data sample (or equivalent lumi for MC) [double]",
				   },
		'Analysis cuts': { 'MinMuPt1': "Minimum pt for the leading lepton [double]",
			           'MinMuPt2': "Minimum pt for the 2nd leading lepton [double]",
				   'MinMuPt3': "Minimum pt for the trailing lepton [double]",
				   'MaxAbsEta':"Maximum abs(eta) for the lepton [double]",
				   'MaxMuIP2DInTrackRX': "Maximum impact parameter of track associated with lepton w.r.t\n"\
						   " the Primary Vertex. [double]\n"\
					"                          X=1 for lepton pt >= 20 GeV/c region\n"\
					"                          X=2 for lepton pt <  20 GeV/c region",
				   'MaxDeltaZMu':  "Maximum dz (distance in z-coordinate) of track associated with lepton"\
						   "w.r.t the Primary Vertex [double]",
				   'MaxDeltaRMuMu': "Maximum DeltaR between Higgs candidates leptons [double]",
				   'MinMET': "Minimum aumount of MET holded by the event [double]",
				   'DeltaZMass': "Distance from the Z-boson mass peak where to cut the dileptonic"\
						   " invariant mass system [double]"
				  },
		'Lepton object definition': { 'MuonID': "Muon identification and isolation to be used: 'HWWID' for the"\
				"Latino's (Higgs-like) id and iso definition or 'VBTF' for the muon POG iso and iso"\
				"definitions [string]",
				'MaxPTIsolationRX': "Maximum Particle Flow type isolation value [double]\n"\
	               			"                          X=1 for lepton region pt <= 20 and abs(eta) <  1.479\n"\
					"                          X=2 for lepton region pt <= 20 and abs(eta) >= 1.479\n"\
					"                          X=3 for lepton region pt > 20  and abs(eta) <  1.479\n"\
					"                          X=4 for lepton region pt > 20  and abs(eta) >= 1.479",
				'MinMVAValueRX': "Minimum BDT (trained for electron id) output value [double]\n"\
					"                          X=1 for lepton region pt <  20 and abs(eta) <=  r1\n"\
					"                          X=2 for lepton region pt <  20 and r1 < abs(eta) < 1.479\n"\
					"                          X=3 for lepton region pt <  20 and abs(eta) >= 1.479\n"\
					"                          X=4 for lepton region pt >= 20 and abs(eta) <=  r1\n"\
					"                          X=5 for lepton region pt >= 20 and r1 < abs(eta) < 1.479\n"\
					"                          X=6 for lepton region pt >= 20 and abs(eta) >=  1.479\n"\
					"                          where r1=1.0 (0.8) for data taking run period 2011 (2012)",
				'MinNValidHitsSATrk': "Minimum number of valid hits in stand alone track for muon [double]",
				'MinNValidPixelHitsInTrk': "Minimum number of pixel hists in global track for muon [double]",
				'MinNValidHitsInTrk': "Minimum number of hits for the global track of muon [double]",
				'MaxNormChi2GTrk': " Maximum value for the normalized chi-square of the global track"\
						"fit [double]",
				'MinNumOfMatches': "Minimum number of inner track-muon stations matches for muon [double]",
				'MinNLayers':  "Minimum number of tracker layers with hits for muon [double]",
				'MaxDeltaPtMuOverPtMu': "Miximum value for the pt relative error [double]",
				'MaxSigmaietaietaRX': "Maximum value of the sigmaIetaIeta for electrons [double]\n"\
					"                          X=1 for lepton region abs(SCeta) < 1.479\n"\
					"                          X=2 for lepton region abs(SCeta) >=1.479",
				'MaxdeltaPhiInRX': "Maximum value for deltaPhiIn [double]\n"\
					"                          X=1 for lepton region abs(SCeta) < 1.479\n"\
					"                          X=2 for lepton region abs(SCeta) >=1.479",
				'MaxdeltaEtaInRX': "Maximum value for deltEtaIn  [double]\n"\
					"                          X=1 for lepton region abs(SCeta) < 1.479\n"\
					"                          X=2 for lepton region abs(SCeta) >=1.479",
				'MaxHtoERX': "Maximum value for energy in hadronic calorimeter over energy in"\
					"electromagnetic [double]"\
					"                          X=1 for lepton region abs(SCeta) < 1.479\n"\
					"                          X=2 for lepton region abs(SCeta) >=1.479",
				'Maxdr03TkSumPtOverPt':"Maximum value for sum of tracks in a dR<0.3 cone around the lepton"\
						" over the lepton pt [double]",  
				'Maxdr03EcalSumPtOverPt':"Maximum value for sum of energy deposits in ECAL in a dR<0.3 cone"\
						" around the lepton over the lepton pt [double]",
				'Maxdr03HcalSumPtOverPt':"Maximum value for sum of energy deposits in HCAL in a dR<0.3 cone"\
						" around the lepton over the lepton pt [double]",
				'MaxLoosed0': "Maximum value for the impact parameter in the loose definition [double]",
				'MaxLooseIso': "Maximum value for isolation in the loose definition [double]"
				},
		'Other parameters': { 'Systematic' : "Activate the systematic calculation for\n"\
					"                LEPTONSYS: sys. related with efficiencies\n"\
					"                FRSYS: sys. related with errors in FR & PR matrices determination\n"\
					"                MMSSYS: sys. related with muon momentum scale\n"\
					"                EESSYS: sys. related with electron energy scale\n"\
					"                METSYS: sys. related with energy scale and resolution for MET\n"\
					"                PUSYS: sys. related with the Pile up reweighting",
				}
		}
# To print ordered as
DESCRIPTIONORDER = { 'General': 0, 'Analysis cuts': 1, 'Lepton object definition':2, 'Other parameters':3}


# All ordered config values
CONFIGORDER = { 10 : "NameTree",              
		20 : "treeType",              
		50 : 'Luminosity',            
		60 : 'MinMuPt1',              
		70 : 'MinMuPt2',              
		80 : 'MinMuPt3',              
		90 : 'MaxAbsEta',             
		100 : 'MaxMuIP2DInTrackR1',    
		110 : 'MaxMuIP2DInTrackR2',    
		120 : 'MaxDeltaZMu',           
		130 : 'MaxDeltaRMuMu',         
		140 : 'MinMET',                
		150 : 'DeltaZMass',            
		155 : 'MuonID',
		160 : 'MaxPTIsolationR1',      
		170 : 'MaxPTIsolationR2',      
		180 : 'MaxPTIsolationR3',      
		190 : 'MaxPTIsolationR4',      
		200 : 'MinMVAValueR1',         
		210 : 'MinMVAValueR2',         
		220 : 'MinMVAValueR3',         
		230 : 'MinMVAValueR4',         
		240 : 'MinMVAValueR5',         
		250 : 'MinMVAValueR6',         
 		260 : 'MinNValidHitsSATrk',    
		270 : 'MinNValidPixelHitsInTrk',
		280 : 'MinNValidHitsInTrk',    
		290 : 'MaxNormChi2GTrk',       
		300 : 'MinNumOfMatches',       
		310 : 'MaxDeltaPtMuOverPtMu',  
		320 : 'MinNLayers',            
                330 : 'MaxSigmaietaietaR1',     
                340 : 'MaxdeltaPhiInR1',        
                350 : 'MaxdeltaEtaInR1',        
                360 : 'MaxHtoER1',              
                370 : 'MaxSigmaietaietaR2',     
                380 : 'MaxdeltaPhiInR2',        
                390 : 'MaxdeltaEtaInR2',        
                400 : 'MaxHtoER2',              
                410 : 'Maxdr03TkSumPtOverPt',   
		420 : 'Maxdr03EcalSumPtOverPt',
		430 : 'Maxdr03HcalSumPtOverPt',     	
		440 : 'MaxLoosed0',            
		450 : 'MaxLooseIso',           
		530 : "nEvents",               
		540 : "firstEvent",            
		560 : "Systematic"
		}

# Types of the different config
CONFIGTYPES = { "NameTree":              "TString",
		"treeType":              "int",
		"nEvents":               "int",
		"firstEvent":            "int",
		'Luminosity':            "double",
		'MinMuPt1':              "double",
		'MinMuPt2':              "double",
		'MinMuPt3':              "double",
		'MaxAbsEta':             "double",
		'MaxMuIP2DInTrackR1':    "double",
		'MaxMuIP2DInTrackR2':    "double",
		'MaxDeltaZMu':           "double",
		'MaxDeltaRMuMu':         "double",
		'MinMET':                "double",
		'DeltaZMass':            "double",
		'MuonID':                "TString",
		'MaxPTIsolationR1':      "double",
		'MaxPTIsolationR2':      "double",
		'MaxPTIsolationR3':      "double",
		'MaxPTIsolationR4':      "double",
		'MinMVAValueR1':         "double",
		'MinMVAValueR2':         "double",
		'MinMVAValueR3':         "double",
		'MinMVAValueR4':         "double",
		'MinMVAValueR5':         "double",
		'MinMVAValueR6':         "double",
 		'MinNValidHitsSATrk':    "double",
		'MinNValidPixelHitsInTrk':"double",
		'MinNValidHitsInTrk':    "double",
		'MaxNormChi2GTrk':       "double",
		'MinNumOfMatches':       "double",
		'MaxDeltaPtMuOverPtMu':  "double",
		'MinNLayers':            "double",
                'MaxSigmaietaietaR1':    "double",  
                'MaxdeltaPhiInR1':       "double",  
                'MaxdeltaEtaInR1':       "double",  
                'MaxHtoER1':             "double",  
                'MaxSigmaietaietaR2':    "double",  
                'MaxdeltaPhiInR2':       "double",  
                'MaxdeltaEtaInR2':       "double",  
                'MaxHtoER2':             "double",  
                'Maxdr03TkSumPtOverPt':  "double",  
		'Maxdr03EcalSumPtOverPt':"double",
		'Maxdr03HcalSumPtOverPt':"double",      	
		'MaxLoosed0':            "double",
		'MaxLooseIso':           "double",
		'Systematic':		 "TString",
		}

# Values for the different run periods
# ----------------------------------------
# Note that the format is a 2-tuple where (for electrons,for muons) -- see ELEC, MUON constants
# - If the same value is valid for electrons and muons, only the element 0 of the 2-tuple is filled
# - If any value need to be separated by run period, there is a dictionary inside the element of 
#   the tuple which keys are the run period year (2011, 2012 up to now)
CONFIGVAL = { "NameTree":    ("Tree",),
	      "treeType":    (0,),
	      "nEvents":     (10000,),
	      "firstEvent":  (0,),
	      "Luminosity":  ({"2011": 4922.0, "2012": 12103.3 },),
	      "MinMuPt1":    (20.0,),
	      "MinMuPt2":    (20.0,),
	      "MinMuPt3":    (10.0,),
	      "MaxAbsEta":   (2.5,2.4),
	      'MaxMuIP2DInTrackR1': (0.02,),
	      'MaxMuIP2DInTrackR2': (0.02,0.01),
	      'MaxDeltaZMu':   (0.1,),
	      'MaxDeltaRMuMu': (1.2,),
	      'MinMET':        (30.0,),
	      'DeltaZMass':    (20.0,),
	      'MuonID':        (None,"HWWID"),
	      'MaxPTIsolationR1': ({"2011": 0.13,"2012":0.15},{"2011":0.06,"2012":0.82}),
	      'MaxPTIsolationR2': ({"2011": 0.09,"2012":0.15},{"2011":0.05,"2012":0.86}),
	      'MaxPTIsolationR3': ({"2011": 0.13,"2012":0.15},{"2011":0.13,"2012":0.86}),
	      'MaxPTIsolationR4': ({"2011": 0.09,"2012":0.15},{"2011":0.09,"2012":0.82}), 
	      'MinMVAValueR1':  ({"2011": 0.139, "2012": 0.0 },None),
	      'MinMVAValueR2':  ({"2011": 0.525, "2012": 0.10},None),
	      'MinMVAValueR3':  ({"2011": 0.543, "2012": 0.62},None), 
	      'MinMVAValueR4':  ({"2011": 0.947, "2012": 0.94},None),
	      'MinMVAValueR5':  ({"2011": 0.950, "2012": 0.85},None),
	      'MinMVAValueR6':  ({"2011": 0.884, "2012": 0.92},None),
	      'MaxNormChi2GTrk':        (None,10.0),
	      'MinNumOfMatches':        (None,1),
	      'MinNValidHitsSATrk':     (None,{"2011":0, "2012": None}),
	      'MinNValidPixelHitsInTrk':(None,{"2011":0, "2012": None}),
	      'MinNValidHitsInTrk':     (None,{"2011":10,"2012": None}),
	      'MaxDeltaPtMuOverPtMu':   (None,0.1),
	      'MinNLayers':             (None,{"2011": None, "2012": 5}),
	      'MaxSigmaietaietaR1':       (0.01,None),
	      'MaxdeltaPhiInR1':          (0.15,None),
	      'MaxdeltaEtaInR1':          (0.007,None),
	      'MaxHtoER1':                (0.12,None),
	      'MaxSigmaietaietaR2':       (0.03,None),
	      'MaxdeltaPhiInR2':          (0.10,None),
	      'MaxdeltaEtaInR2':          (0.009,None),
	      'MaxHtoER2':                (0.10,None),
	      'Maxdr03TkSumPtOverPt':     (0.2,None),
	      'Maxdr03EcalSumPtOverPt':   (0.2,None),
	      'Maxdr03HcalSumPtOverPt':   (0.2,None),
	      'MaxLoosed0':             (None,0.2),
	      'MaxLooseIso':            (None,{"2011": 1.0, "2012":-0.6}),
	      'Systematic':            (None,),
	      }

#The muon 2011 vbtf id needs some different values, the CONFIGVAL dict stores the values for the hwwid
MUONID_VBTF = { 'MaxMuIP2DInTrackR1' : (CONFIGVAL['MaxMuIP2DInTrackR1'][0],{'2011':0.2,'2012':CONFIGVAL['MaxMuIP2DInTrackR1'][0]}),
	'MaxMuIP2DInTrackR2' : (CONFIGVAL['MaxMuIP2DInTrackR2'][0],{'2011':0.2,'2012':CONFIGVAL['MaxMuIP2DInTrackR2'][1]}),
	'MaxPTIsolationR1' : (CONFIGVAL['MaxPTIsolationR1'][0],{'2011':0.12,'2012':CONFIGVAL['MaxPTIsolationR1'][1]['2012']}),
	'MaxPTIsolationR2' : (CONFIGVAL['MaxPTIsolationR2'][0],{'2011':0.12,'2012':CONFIGVAL['MaxPTIsolationR2'][1]['2012']}),
	'MaxPTIsolationR3' : (CONFIGVAL['MaxPTIsolationR3'][0],{'2011':0.12,'2012':CONFIGVAL['MaxPTIsolationR3'][1]['2012']}),
	'MaxPTIsolationR4' : (CONFIGVAL['MaxPTIsolationR4'][0],{'2011':0.12,'2012':CONFIGVAL['MaxPTIsolationR4'][1]['2012']})
	}




class configvar:
	"""
	Class to deal with the configuration keys. Just the name and the type 
	are data-members. Once it is instantiated one can call the instance 
	as a function whose arguments are the lepton type and the run period
	"""
	def __init__(self,name):
		self.name = name
		try:
			self.type = CONFIGTYPES[self.name]
		except KeyError:
			raise RuntimeError("\033[1;31mconfigvar instantation\033[1;m: not defined the '"+\
					self.name+"' configuration value'. This script should be"+\
					" updated with this new configuration issue")
	def __call__(self,leptontype,runperiod):
		""" .. configvar.__call__(leptontype,runperiod,value=None)
		Getting the values
		"""

		if leptontype != ELEC and leptontype != MUON:
			raise RuntimeError("\033[1;31mconfigvar instance\033[1;m: not recognized the'"+\
					" lepton type '"+str(leptontype)+"'")
		
		preval = None
		try:
			preval = CONFIGVAL[self.name][leptontype]
		except IndexError:
			# There is the same value for muons and electrons
			preval = CONFIGVAL[self.name][0]
	
		val = None
		try:
			val = preval[runperiod]
		except TypeError:
			# There is no difference between periods
			val = preval
		except KeyError:
			raise RuntimeError("\033[1;31mconfigvar instance\033[1;m: not found"+\
					" the runperiod '"+runperiod+"'")
		return val

	def __str__(self):
		for vartype,dictofvar in DESCRIPTION.iteritems():
			try:
				return dictofvar[self.name]
			except KeyError:
				pass
		raise ValueError("\033[1;31mconfigvar instance\033[1;m: '"+self.name+"' not"+\
				"described. Modify this module to insert it")




def createfile(filename,leptontype,runperiod):
	"""
	"""
	cfgval = []
	maxchar = 0 
	for id,name in sorted(CONFIGORDER.iteritems()):
		cfgval.append( configvar(name) )
		maxchar = max(maxchar,len(name))

	lines = ""
	vis = 0
	formatted = "@var %-7s %-"+str(maxchar)+"s %13s;\n"
	for cfg in cfgval:
		value = cfg(leptontype,runperiod)
		if value == None:
			continue
		lines += formatted % (cfg.type,cfg.name,value)
		if vis % 4 == 0:
			lines += "\n"
		vis += 1
	
	f = open(filename,"w")
	f.writelines(lines)
	f.close()

	return 0
		
	

if __name__ == '__main__':
	from optparse import OptionParser
	import os
	
	#Opciones de entrada
	parser = OptionParser()
	parser.set_defaults(runperiod="2012",signal="WZ",muonid='VBTF')
	parser.add_option( '-d',  action='store', metavar="CONFIGNAME", dest='describe',\
			help="List and describe the configurable 'CONFIGNAME'. Print "\
			" 'all' to show all accepted configurables. Also is posible to"\
			" extract all the configurables splitted by subsets: general, analysis,"\
			" lepton or other" )
	parser.add_option( '-r', '--runperiod', action='store', type='string', dest='runperiod',\
			help="The run period [2011 2012<-default>]")
	parser.add_option( '-s', '--signal', action='store', dest='signal',\
			help="Signal name [WZ<-default> WH]")
	parser.add_option( '-o', '--outputname', action='store', dest='outputname',\
			help="The ouput filename [Default: analisis[signal]_*.ip]")
	parser.add_option( '--muonid', action="store", dest="muonid",\
			help="The ID and Iso to be used for the 2011 muon."\
			" Valid values are HWWID and VBTF. [Default: VBTF]")
	parser.add_option( '--sys', action='store', dest='systematic',\
			metavar="SYSNAME:UP|DOWN",
			help="Introduce the systematic to be evaluated and splitted by"\
					" a colon, the variation UP or DOWN. Known and"\
					"valid systematic names are: 'LEPTONSYS', 'FRSYS'"\
					" 'MMSSYS', 'EESSYS', 'METSYS' and 'PUSYS'. See 'makeinputsheet -d"\
					" other' for detailed info about them")
	parser.add_option( '-f', '--forcevalues', action='store', dest='forcelist',\
			metavar="cfgname:valueElectron@valueMuon,...",
			help="List of configuration values with their values introduced here")

	( opt, args ) = parser.parse_args()

	# Listing and exit
	if opt.describe:
		import sys
		message = ""
		sections = [ 'general', 'analysis', 'lepton', 'other' ]
		if opt.describe.lower() == 'all':
			describelist = sections
		else:
			describelist = [opt.describe.lower()]

		if describelist[0] in sections or describelist == sections:
			maxvalvar  = 0
			listmessage = {}
			for sect in describelist:
				try:
					_typesval = filter(lambda x: x.lower().find(sect) != -1,DESCRIPTION.keys())[0]
				except IndexError:
					raise RuntimeError("\033[1;31mmakeinputsheet ERROR\033[1;m: The global section"+\
						" '"+sect+"' is not defined as configurable section. See the help"+\
						" (option '-d')")
				dictvar = DESCRIPTION[_typesval]
				listmessage[_typesval] = []
				for var,description in dictvar.iteritems():
					maxvalvar = max(maxvalvar,len(var))
					listmessage[_typesval].append((var,description))
			for _typesval,listvar in sorted(listmessage.iteritems(),\
					key=lambda (key,ord): DESCRIPTIONORDER[key] ):
				message += "\033[1;34m%s configuration type\033[1;m:\n" % _typesval
				for var,descript in listvar:
					metaline = "\033[1;37m%"+str(maxvalvar)+"s\033[1;m: %s\n"
					message += metaline % (var,descript)
			print message+"\n"
			sys.exit(0)


		var = configvar(opt.describe)
		print "\033[1;37m%s\033[1;m: %s\n" % (opt.describe,var)
		sys.exit(0)


	# file name
	filename = None
	if not opt.outputname:
		filename = "analisis"+opt.signal.lower()
	else:
		filename = opt.outputname.split(".")[0]

	# Systematic
	if opt.systematic:
		# Parsing input
		try:
			sys,mode = opt.systematic.split(":")
		except ValueError:
			raise RuntimeError("\033[1;31mmakeinputsheet ERROR\033[1;m: Not valid argument"+\
					" for '--sys' option. Format is SYS:VAL, where VAL must be UP or DOWN")
		validsys = [ 'LEPTONSYS', 'FRSYS', 'MMSSYS', 'EESSYS', 'METSYS', 'PUSYS' ]
		if not sys in validsys:
			message = "\033[1;31mmakeinputsheet ERROR\033[1;m: Not valid argument"+\
					" for '--sys' option. Valid systematics are:"
			for i in validsys:
				message += " '%s'" % i
			raise RuntimeError(message)
		if not mode in [ 'UP', 'DOWN']:
			raise RuntimeError("\033[1;31mmakeinputsheet ERROR\033[1;m: Not valid argument"+\
					" for '--sys option. Valid variation are 'UP' 'DOWN'")
		CONFIGVAL['Systematic'] = (sys+":"+mode,)


	if opt.muonid:
		if opt.muonid != "HWWID" and opt.muonid != "VBTF":
			raise RuntimeError("\033[1;31mmakeinputsheet ERROR\033[1;m: Not valid argument"+\
					" for '--muonid' option. Valid ID are 'HWWID' 'VBTF'")
		# Change the CONFIGVAL dict if proceed to the values of VBTF
		if opt.muonid == "VBTF":
			for key, value in MUONID_VBTF.iteritems():
				CONFIGVAL[key] = value
		try:
			opt.forcelist += "MuonID:None@"+opt.muonid
		except TypeError:
			opt.forcelist = "MuonID:None@"+opt.muonid
	
	# The user forces to put the value
	if opt.forcelist:
		namevallist = opt.forcelist.split(",")
		if len(namevallist) == 0:
			raise RuntimeError("\033[1;31mmakeinputsheet ERROR\033[1;m: '-f' option not parsed"+\
					" correctly. See the help")
		namevaldir = dict( map(lambda x: (x.split(":")[0],x.split(":")[-1]), namevallist) )
		for cfgname,valuetuple in namevaldir.iteritems():
			try:
				electronval = valuetuple.split("@")[0]
				muonval     = valuetuple.split("@")[-1]
			except:
				raise RuntimeError("\033[1;31mmakeinputsheet ERROR\033[1;m: '-f' option not parsed"+\
						" correctly. See the help")
			# The None key
			try:
				electronval =eval(electronval)
			except NameError:
				pass
			try:
				muonval = eval(muonval)
			except NameError:
				pass
			CONFIGVAL[cfgname] = (electronval,muonval)


	for lt in [ ELEC,MUON ]:
		print "\033[1;34mmakeinputsheet INFO\033[1;m Created the input sheet configuration file for",
		if lt == ELEC:
			suffix = "_eee.ip"
			print "electrons: '%s'" % (filename+suffix)
		else:
			suffix = "_mmm.ip"
			print "muons: '%s'" % (filename+suffix)
		createfile(filename+suffix,lt,opt.runperiod)




