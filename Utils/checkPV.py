#!/usr/bin/env python
"""
Script to plot primary vertices distribution after weight is applied (no selection)
"""

#Pseudo ROOT colors
kBlack=1
kRed=632
kGreen=416
kAzure=860
kCyan=432
kOrange=800
COLORSDICT = { "WW" : kRed+4, "WZTo3LNu": kOrange-2, "WJets_Madgraph": kAzure+3,
		"TTbar_2L2Nu_Powheg": kOrange+5, "TTbar_Madgraph": kOrange+5,
		"ZZ": kRed+3, "ZJets": kCyan-2, "ZJets_Madgraph": kCyan-2,
		"Data": kBlack, "Fakes": kAzure-7, 
		"TW_DR": kGreen-2, "TbarW_DR": kGreen+4,
		"DDM_ZJets": kOrange-3,
		"DDM_TTbar": kOrange+5,
		"PhotonVJets_Madgraph": kGreen-5,
		"VGamma": kGreen-5,
		"WgammaToLNuG": kGreen-5,
		"ZgammaToLLG": kGreen-6,
		"ZGToLL": kGreen-6,
		"WHToWW2L120": kRed-4,
		"WHToWW2L130": kRed-3
		}


def plotpv(sampleslist,luminosity):
	"""
	"""
	import ROOT
	from LatinoStyle_mod import LatinosStyle
	from functionspool_mod import processedsample

	lstyle = LatinosStyle() 
	lstyle.cd()
	ROOT.gStyle.SetOptStat(0)

	ROOT.gROOT.SetBatch()
	c = ROOT.TCanvas()
	frame = c.DrawFrame(0,0,30,0.12)
	frame.SetXTitle("N_{PV}")
	frame.SetYTitle("Normalized Events")
	leg = ROOT.TLegend(0.6,0.7,0.8,0.9)
	leg.SetBorderSize(0)
	leg.SetTextSize(0.03)
	leg.SetFillColor(10)

	i = 0
	for s in sampleslist:
		samplename =  s.split("cluster_")[-1]
		if samplename == "Fakes":
			continue
		rootfilename = os.path.join(os.path.join(s,"Results"),samplename+".root")
		print rootfilename
		ps = processedsample(rootfilename,lumi=luminosity)
		h = ps.gethistogram("fHNPrimaryVertices")
		h.SetNormFactor(1)
		ymax = h.GetMaximum()/h.Integral()
		if ymax > frame.GetMaximum():
			frame.GetYaxis().SetRangeUser(frame.GetBinLowEdge(1),ymax*1.20)
		try:
			color = COLORSDICT[samplename]
		except KeyError:
			color = kGreen-5
		h.SetLineColor(color)
		h.SetMarkerColor(color)
		#h.SetFillColor(color)
		#h.SetFillStyle(3002+i)
		h.SetMarkerColor(color)
		option = "PF"
		if samplename == "Data":
			h.SetMarkerStyle(20)
			h.Draw("PESAME")
			option = "PL"
		else:	
			h.Draw("SAME")
		leg.AddEntry(h,samplename,option)
		i += 1
	leg.Draw()
	c.SaveAs("pvafterreweighting.pdf")
	ROOT.gROOT.SetBatch(1)



if __name__ == '__main__':
	import os,sys
	import glob
	from optparse import OptionParser

        #Comprobando la version (minimo 2.4)
        vX,vY,vZ,_t,_t1 = sys.version_info
        if vX > 2 and vY < 4:
                message = '\033[1;31checkPV ERROR\033[1;m I need python version >= 2.4'
                sys.exit( message )
        elif vX < 2:
                message = '\033[1;31checkPV ERROR\033[1;m I need python version >= 2.4'
                sys.exit( message )
        
        parser = OptionParser()
        parser.set_defaults(luminosity="12103.3")
        parser.add_option( '-d', action='store', dest='folder', help='Paths'\
                        ' where to find the standard folder structure "cluster_blbalb"')
	parser.add_option( '-l', action='store', dest='luminosity', help='Luminosity to be used'\
			' [Default: 12103.3')
	parser.add_option( '-p', action='store', dest='printsamples', help='List of samples to'\
			' be printed and compared with the data')


        ( opt, args ) = parser.parse_args()
	
	if not opt.folder:
		folder = os.getcwd()
	else:
		folder = opt.folder

	
	# Extract the data/mc folders
	samples = filter(lambda x: os.path.isdir(x), glob.glob(os.path.join(folder,"cluster_*")))
	if opt.printsamples:
		printsamples = opt.printsamples.split(",")
		samplesprov = filter(lambda x: x.split("cluster_")[-1] in printsamples,samples)
		samples = samplesprov

	try:
		lumi = float(opt.luminosity)
	except ValueError:
		message = "\033[1;31mcheckPV ERROR:\033[1;m value introduced with option"\
				" '-l' must be numeric. It was read '%s'" % str(opt.luminosity)
		raise RuntimeError(message)
	print "+++ Plotting the Primary Vertices distribution after re-weighting"
	plotpv(samples,opt.luminosity)





	

	
