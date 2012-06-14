#!/usr/bin/env

NINSTANCE = 0 

def addstack(thstack):
	""".. function:: addstack(hs) --> addhisto
	Given a THStack returns you the added up histogram

	:param hs: stack to be added
	:type hs: ROOT.THStack

	:return: a histogram with all the elements of the 
	         stack added up
	:rtype: ROOT.TH1[D|F|I]
	"""
	import ROOT
	
	ROOT.TH1.AddDirectory(False)
	classhisto = thstack.GetHists().At(0).ClassName()
	listhist = thstack.GetHists()
	outputhist = eval('ROOT.'+classhisto+'(listhist.At(0).Clone(thstack.GetName()+"_cloned_from_stack"))')
	for i in xrange(1,len(listhist)):
		outputhist.Add(listhist.At(i))
	
	return outputhist

class plfile():
	"""
	"""
	def __init__(self,filename):	
		""".. class:: plfiles(lfilename,rfilename) --> pairks
		Class to deal with a concrete extraction of some histos
		from a root file. The files have been created using the
		AnalisisVH package and there are the output of the 'plothisto'
		script in 'root' format
		"""
		import ROOT
		import os
		global NINSTANCE

		# I take care of the histos
		ROOT.TH1.AddDirectory(False)
		# 
		fileroot = ROOT.TFile(filename)
		if fileroot.IsZombie():
			raise OSError("\033[31;1mplfile ERROR\033[m File does not exist")
		self.histoname = os.path.basename(filename.replace(".root",""))
		padup = "padup_"+self.histoname
		
		# Number of plfile instance
		NINSTANCE += 1
		
		# unique identifier
		self.__id__ = filename.replace(".root","").replace("/","_")+"_"+str(NINSTANCE)

		stack = fileroot.Get("canvas").GetPrimitive(padup).GetPrimitive("hs").Clone(self.histoname+"_hs"+str(NINSTANCE))

		# histo added-up
		self.__histo__ = addstack(stack)

		# Data  histo
		self.__data__ = fileroot.Get("canvas").GetPrimitive(padup).GetPrimitive(self.histoname).Clone(self.histoname+"_data"+str(NINSTANCE))

		# Delete everything we doesn't want
		fileroot.Close()
		fileroot.Delete()
		stack.Delete()
		del fileroot,stack

	def __del__(self):
		"""
		"""
		self.__histo__.Delete()
		self.__data__.Delete()


def checkks(distr1,distr2,icl=1.0,verbose=False):
	""".. function:: checkks(distr1,distr2[,cl]) --> bool

	Check using a Kolmogorov-Smirnov statistic if the two
	distributions 'distr1' and 'dist2' are compatible with
	being emerge from the same distribution (Null hypothesis).
	If it cannot say anything with a condidence of (1-icl)% then
	returns a None

	:param distr1: first distribution
	:type distr1: numpy.array
	:param distr2: second distribution
	:type distr2: numpy.array
	:param icl: the confidence level (1-icl) we want the answer in percentage (%) [default: 1%]
	:type icl: float
	:return: the compatibility of having emerge from the same distribution
	         with a confidence level of 1-icl. If it can't say anything, returns
		 None
	:rtype: bool (None)
	"""
	from scipy.stats import ks_2samp

	ksval,pval = ks_2samp(distr1,distr2)

	pvalpc = pval*100.0
	output = None
	# If pval is lower than the icl, then we can reject the null hypothesis
	if pvalpc < icl:
		output = False
	# If pval is greater than 1-icl, then we can reject the alternative hypothesis,
	# which means accept the null hypothesis
	elif pvalpc > (100-icl):
		output = True
	
	if verbose:
		print "\033[34;1m"+"Kolmogorov-Smirnov test "+"#"*76
		print "We are rejecting the Null hypothesis (the two distributions emerge from the same one) if P < %.0f%s" % (icl,"%")
		print "Or accepting it if P > (100-%.0f)%s (i.e., rejecting the alternative hypothesis)" % (icl,"%")
		print "Found it P=%.1f%s and KS-val=%.4f" % (pvalpc,"%",ksval )
		print "#"*100+"\033[m"

	return output


def chisquaretest():
	"""
	"""
	pass

def psi(predicted,observed):
	""".. function:: psi(predicted,observed) -> value
	Function which evaluate the amount of plausability a hypothesis has (i.e., predicted) when it is found a
	particular set of observed data (i.e. observed). The unit are decibels, and more close to 0 implies a better
	reliability of the hypothesis. On the other hand, getting a psi_B = X db implies that there is another hypothesis
	that it is X db better than B. So, psi function is useful to compare two hypothesis with respect the same observed
	and see which of them has a psi nearest zero.

	:param predicted: the set of values which are predicted. Usually a MC histogram 
	:type predicted: numpy.array
	:param observed: the set of values which are observed; usually the data
	:type observed: numpy.array
	
	:return: the evaluation of the psi function
	:rtype: float
	
	See reference at 'Probability Theory. The logic of Science. T.E Jaynes, pags. 300-305. Cambridge University Press (2003)'
	"""
	from math import log10
	from numpy import array

	N_total = 0
	for n in observed:
		N_total += n
	# build the frecuency array for observed
	arrobs = array([ x/N_total for x in observed ],'d')

	# Extracting info from the predicted
	N_total_pre = 0
	for n in predicted:
		N_total_pre += n
	# and build frequency array for predicted
	arrpre = array( [ x/N_total_pre for x in predicted ], 'd' )
	
	#Consistency check: same number of measures (bins)
	if len(arrpre) != len(arrobs):
		message = "\033[31;1mpsi ERROR\033[m Different number of elements for predicted and observed"
		raise RuntimeError(message)
	#Evaluating psi (in decibels units)
	psib = 0.0
	for i in xrange(len(arrpre)):
		if not arrpre[i] > 0.0:
			continue
		try:
			psib += arrobs[i]*log10(arrobs[i]/arrpre[i])
		except ValueError:
			continue
	
	return 10.0*N_total*psib


if __name__ == '__main__':
	from optparse import OptionParser
	from array import array
	from numpy import array as numpyarray

	usage  ="usage: testks fileroot1 fileroot2 [options]"
	parser = OptionParser(usage=usage)
	parser.set_defaults(icl=1.0,verbose=False)
	parser.add_option( "-i", "--icl", action='store',dest='icl', metavar="ICL", help="Level of confidence to reject Null hypothesis in % [Default: 1.0%]")
	parser.add_option( "-v", action='store_true',dest='verbose',  help="Activating verbosity")
	
	( opt, args ) = parser.parse_args()

	if len(args) != 2:
		message = "\033[31;1mkstest ERROR\033[m Two input root files are needed"
		raise RuntimeError(message)

	plfiledict = { }
	for i in xrange(len(args)):
		prov = plfile(args[i])
		plfiledict[prov.__id__] = prov
	
	# extract array of points
	arrdict = { }
	errdict = {}
	arrdata = []
	errdata = []
	for key,val in plfiledict.iteritems():
		pointlist = []
		errlist = []
		for k in xrange(1,val.__histo__.GetNbinsX()+1):
			pointlist.append( val.__histo__.GetBinContent(k) )
			errlist.append( val.__histo__.GetBinError(k) )
		if len(arrdata) == 0:
			for k in xrange(1,val.__data__.GetNbinsX()+1):
				arrdata.append( val.__data__.GetBinContent(k) )
				errdata.append( val.__data__.GetBinError(k) )
		arrdict[key] = numpyarray( pointlist, 'd' )
		errdict[key] = numpyarray( errlist, 'd' )

	sampleslist = arrdict.keys()
	ks_dataprob = {}
	print "\033[34;1mkstest \033[m histo::"+plfiledict.values()[0].histoname
	print "   |-- kolmogorov-smirnov test"
	for i in sampleslist:
		ks_dataprob[i] = checkks(arrdict[i],arrdata,opt.icl,opt.verbose)
		try:
			print "     |-- %s and %s  emerge from the same distribution? %u " % ("data",i,ks_dataprob[i])
		except TypeError:
			print "     |-- %s and %s  emerge from the same distribution? CANNOT SAY ANYTHING " % ("data",i)
	print "   |-- psi test (0 db is the perfect hypothesis)"
	nearestzero = (None,1.0e10)
	psiBoutput = {}
	for i in sampleslist:
		psiBoutput[i] = psi(arrdict[i],arrdata)
		if nearestzero[1] > abs(psiBoutput[i]):
			nearestzero = (i,psiBoutput[i])
		print "     |-- '%s' psi_B=%.3f db " % (i,psiBoutput[i])
	print "     |-- Most supporting MC hypothesis to the data: %s with a difference of %.2f db w.r.t the other one" % \
			(nearestzero[0],abs(abs(psiBoutput[sampleslist[0]])-abs(psiBoutput[sampleslist[1]])))
	
	

