#!/usr/bin/env python

def setxs(filename):
        """..function::setxs(f) -> None
        Correct the xs for the Zgamma samples

        :param f: root filename to be used
        :type f: srt
        """
        import ROOT

        # Extracting the luminosity, efficiency weights,,...
	f = ROOT.TFile(filename,"UPDATE")
	if f.IsZombie():
		raise RuntimeError("\033[31;1msetxs ERROR\033[m Problems with the file "+filename)
	
	# 1) Load the InputParameters
	ROOT.gSystem.SetDynamicPath(ROOT.gSystem.GetDynamicPath()+":"+os.getenv("VHSYS")+"/libs")
	ROOT.gSystem.Load("libInputParameters.so")
	
	xs = 45.2 # Extracted from CMS AN2011/251 (Study of WGamma and Zgamma production at CMS)
	print "\033[34;1msetxs INFO\033[m Updating Zgamma cross-section to 45.2pb in filename '%s'" % filename
	ip = f.Get("Set Of Parameters")
	ip.SetNamedDouble("CrossSection",xs)
	
	ip.Write()
	f.Purge()
	f.Close()
	f.Delete()

	return None


if __name__ == "__main__":
	from optparse import OptionParser
	import glob
	import os,sys

	#Comprobando la version (minimo 2.4)
	vX,vY,vZ,_t,_t1 = sys.version_info
	if vX > 2 and vY < 4:
		message = 'merge3leptonfs: I need python version >= 2.4'
		sys.exit( message )
	elif vX < 2:
		message = 'merge3leptonfs: I need python version >= 2.4'
		sys.exit( message )
	#Opciones de entrada
	parser = OptionParser()
	parser.add_option('-d', '--dir' ,  action='store', type='string', dest='dir', \
			help='directory where you can found the Zgamma samples')
	
	( opt, args ) = parser.parse_args()

	if not opt.dir:
		message="\033[31;1msetZgammaXS ERROR\033[m Mandatory option '-d'"
		sys.exit(message)

	searchstr = os.path.join(os.path.abspath(opt.dir),"cluster_Zgamma*/Results/*.root")
	listrootfiles = glob.glob(searchstr)

	for filename in listrootfiles:
		setxs(filename)



