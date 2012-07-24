#!/usr/bin/env python
"""
Build a default configuration input sheet needed for runanalysis executable.
"""
# Indices to extract values depending the lepton type
ELEC=0
MUON=1

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
		'MaxLooseIso':           "double"
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
	      "Luminosity":  ({"2011": 4922.0, "2012": 5016.4 },),
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
	      'MaxLooseIso':            (None,{"2011": 0.4, "2012":-0.6}),
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
	formatted = "@var %-7s %-"+str(maxchar)+"s %7s;\n"
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
	parser.set_defaults(runperiod="2012",signal="WZ")
	parser.add_option( '-r', '--runperiod', action='store', type='string', dest='runperiod',\
			help="The run period [2011 2012<-default>]")
	parser.add_option( '-s', '--signal', action='store', dest='signal',\
			help="Signal name [WZ<-default> WH]")
	parser.add_option( '-o', '--outputname', action='store', dest='outputname',\
			help="The ouput filename [Default: analisis[signal]_*.ip]")
	parser.add_option( '-f', '--forcevalues', action='store', dest='forcelist',\
			metavar="cfgname:value,...",
			help="List of configuration values with their values introduced here [NOT IMPLEMENTED YET]")

	( opt, args ) = parser.parse_args()

	# file name
	filename = None
	if not opt.outputname:
		filename = "analisis"+opt.signal.lower()
	else:
		filename = opt.outputname.split(".")[0]

	for lt in [ ELEC,MUON ]:
		print "\033[1;34mmakeinputsheet INFO\033[1;m Created the input sheet configuration file for",
		if lt == ELEC:
			suffix = "_eee.ip"
			print "electrons: '%s'" % (filename+suffix)
		else:
			suffix = "_mmm.ip"
			print "muons: '%s'" % (filename+suffix)
		createfile(filename+suffix,lt,opt.runperiod)




