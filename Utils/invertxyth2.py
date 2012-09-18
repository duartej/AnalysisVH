#!/usr/bin/env python
"""
Invert the x-axis by the y-axis in a TH2 histogram
"""

def getth2inverted(histoname,inputfile):
	"""
	"""
	import ROOT
	import array

	ROOT.TH2.AddDirectory(False)

	f = ROOT.TFile(inputfile)
	if f.IsZombie():
		message = "\033[31mgetth2inverted ERROR\033[m '%s' is not a root file" % inputfile
		raise RuntimeError(message)
	
	h = None
	if not histoname:
		# Get the first TH2
		for key in f.GetListOfKeys():
			if key.GetClassName().find("TH2") != -1:
				histoname = key.GetName()
				h = f.Get(histoname)
	else:
		h = f.Get(histoname)
	# Check
	if not h:
		message = "\033[31mgetth2inverted ERROR\033[m TH2 named '%s' not found" % str(histoname)
		raise RuntimeError(message)

	# X bins
	xbinsAD = h.GetXaxis().GetXbins()
	xbins = array.array('f',[])
	for i in xrange(xbinsAD.GetSize()):
		xbins.append( xbinsAD.At(i))
	ybinsAD = h.GetYaxis().GetXbins()
	ybins = array.array('f',[])
	for i in xrange(ybinsAD.GetSize()):
		ybins.append( ybinsAD.At(i))


	# Build new histo
	hnew = eval("ROOT."+h.ClassName()+'("'+h.GetName()+'_inv","'+h.GetTitle()+'",'+\
	 	   str(len(ybins)-1)+',ybins,'+str(len(xbins)-1)+',xbins)')
	# Fill it
	for ybinnew in xrange(1,len(xbins)+1):
		for xbinnew in xrange(1,len(ybins)+1):
			value = h.GetBinContent(ybinnew,xbinnew)
			error = h.GetBinError(ybinnew,xbinnew)
			hnew.SetBinContent(xbinnew,ybinnew,value)
			hnew.SetBinError(xbinnew,ybinnew,error)

	f.Close()
	del f
	h.Delete()
	del h
	
	return hnew


def storehisto(h,filename):
	"""
	"""
	import ROOT

	f = ROOT.TFile(filename,"UPDATE")

	h.Write()
	f.Close()




if __name__ == '__main__':
	from optparse import OptionParser,OptionGroup
	import os
	
	#Opciones de entrada
	usage = "usage: invertxyth2 inputrootfile [options]"
	parser = OptionParser(usage=usage)
	parser.set_defaults()
	parser.add_option( '-n', '--namehisto', action='store', type='string', dest='histoname',\
			help="The name of the histogram to be modified")
	parser.add_option( '-o', '--output', action='store', dest='outputfile',\
			help="The output filename")
	
	(opt,args) = parser.parse_args()

	if len(args) != 1:
		message = "\033[31minvertxyth2 ERROR\033[m mandatory input root file name as argument"
		raise RuntimeError(message)

	if not opt.outputfile:
		outputfile = args[0].split(".root")[0]+"_output.root"
	else:
		outputfile  = opt.outputfile

	h = getth2inverted(opt.histoname,args[0])

	storehisto(h,outputfile)








