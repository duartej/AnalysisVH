#!/usr/bin/env python
"""
Extract the Event info tree from a root file
"""
# Maximum short int (in 64b c++ compiler), to fix
# the negative event number bug
MAXINT = 4294967295

class evtinfo(object):
	def __init__(self):
		"""
		"""
		self.run      = -1
		self.lumi     = -1
		self.evt      = -1
		self.channel  = -1 

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

		self.ordereddm = [ "run","lumi","evt","channel","zmass","zlep1pt",\
				"zlep1eta","zlep1phi","zlep2pt","zlep2eta","zlep2phi",\
				"wmt","wleppt","wlepeta","wlepphi","metet","metphi" ]
		self.dmtype = [ "i","i","i","i",".2f",".2f",".2f",".2f",".2f",".2f",".2f",\
				".2f",".2f",".2f",".2f",".2f",".2f" ]


	def __str__(self):
		"""
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


def evtinfowrite(inputfile,outputfile):
	"""
	"""
	import ROOT
	
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
	if evtinfobranch == None: # TO BE DEPRECATED
		evtinfobranch = t.GetBranch("evtlist")
	# Initialize the leaves
	for leaf in t.GetListOfLeaves():
		#TO BE DEPRECATED (strip)
		evti.__setattr__(leaf.GetName().strip(),leaf)
	
	lines = "#"
	for i in evti.ordereddm:
		lines += " "+i
	lines += "\n"
	for i in xrange(0,nentries):
		t.GetEntry(i)

		lines += evti.__str__()
		if i % 10 == 0:
			print "%i-event processed (%i Event Number)" % (i,evti.evt.GetValue(0))

	f.Close()
	f.Delete()

	ftxt = open(outputfile,"w")
	ftxt.writelines(lines)
	ftxt.close()

	return nentries

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
        
	usage="usage: %prog filename.root [options]"
        parser = OptionParser(usage=usage)
        parser.set_defaults()
	parser.add_option( '-o', action='store', dest='outputname', help='Output filename')

        ( opt, args ) = parser.parse_args()

	if len(args) == 0:
                message = '\033[1;31mgetevtlist ERROR\033[1;m Input root file mandatory!'
                sys.exit( message )
	
	if not opt.outputname:
		outputname = "evtlist.txt"
	else:
		outputname = opt.outputname

	
	# Extract the data/mc folders
	print "\033[1;34mgetevtlist INFO\033[1;m Extracting event relevant info from '%s'" % args[0],
	print "and printing-out to '%s'" % outputname
	totalent = evtinfowrite(args[0],outputname)
	print "\033[1;34mgetevtlist INFO\033[1;m Processed a total %i events" % totalent





	

	
