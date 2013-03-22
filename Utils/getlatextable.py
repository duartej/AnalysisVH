#!/usr/bin/env python
"""
Extract a TH2 histogram in a latex table 
"""
from functionspool_mod import pywmanager

if __name__ == '__main__':
	import os,sys
	from optparse import OptionParser

        #Comprobando la version (minimo 2.4)
        vX,vY,vZ,_t,_t1 = sys.version_info
        if (vX > 2 and vY < 4) or vX < 2:
                message = '\033[1;31mgetlatextable ERROR\033[1;m I need python version >= 2.4'
                sys.exit( message )
        
	usage="usage: %prog [-t type1[,type2,...]]\n"
	usage+="\nPrint muon and electron weights in latex format."
	usage+="\nThe recognized weight types are: SF (scale factors), PR (prompt rates),"
	usage+="\nFR (fake rates), TR_leading (trigger eff. for leading lepton), TR_trailing,"
	usage+='\nTR_trailing (trigger eff. for trailing lepton)'
        parser = OptionParser(usage=usage)
        parser.set_defaults(wt='PR,FR')
	parser.add_option( '-t', '--types', action='store',dest='wt',metavar='T1[,T2,..]',\
			help='Weight types to dump the tables. Valid names are "PR",'\
			'"FR","SF","TR_leading","TR_trailing"')

        ( opt, args ) = parser.parse_args()

	for t in opt.wt.split(","):
		w = pywmanager(t)
		for l in [ 'Muon', 'Elec']:
			print "%s %s" % (l,t)
			print w.getlatextable(l)
			print "*"*100
	
