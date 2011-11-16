
#include "WPElecID.h"

#include<iostream>
#include<stdlib.h>
#include<cmath>

// Print
void WPElecID::Print() const
{
	std::cout << "=======  WP Summary cuts ======" << std::endl;
	std::cout << " WP : " << fWP << std::endl;
	std::cout << " + Barrel: " << std::endl;
	std::cout << "   sigmaietaieta=" << sigmainin_B << std::endl;
	std::cout << "   DeltaPhi_in  =" << deltaphi_B  << std::endl;
	std::cout << "   DeltaEta_in  =" << deltaeta_B  << std::endl;
	std::cout << "   H/E          =" << hoE_B       << std::endl;
	std::cout << " + Endcap: " << std::endl;
	std::cout << "   sigmaietaieta=" << sigmainin_B << std::endl;
	std::cout << "   DeltaPhi_in  =" << deltaphi_B  << std::endl;
	std::cout << "   DeltaEta_in  =" << deltaeta_B  << std::endl;
	std::cout << "   H/E          =" << hoE_B       << std::endl;
	std::cout << "================================" << std::endl;

}

// Id cuts using the working points defined at:
// https://twiki.cern.ch/twiki/bin/view/CMS/VbtfEleID2011
WPElecID::WPElecID( const unsigned int & workingPoint )
{
	/*
	Electron ID WP	95%	90%	85%	80%	70%	60%
	BARREL	  	  	  	  	  	  
	Electron ID	  	  	  	  	  	  
	   Deltainin	 0.01	 0.01	 0.01	 0.01	 0.01	 0.01
	   Deltaphi       0.8	 0.8	 0.06	 0.06	 0.03	 0.025
	   Deltaeta	 0.007	 0.007	 0.006	 0.004	 0.004	 0.004
		HoE	 0.15	 0.12	 0.04	 0.04	 0.025	 0.025
	ENDCAPS	  	  	  	  	  	  
	   Deltaini	 0.03	 0.03	 0.03	 0.03	 0.03	 0.03
	   Deltaphi	 0.7	 0.7	 0.04	 0.03	 0.02	 0.02
  	   Deltaeta	 0.01	 0.009	 0.007	 0.007	 0.005	 0.005
		HoE	 0.15	 0.15	 0.15	 0.15	 0.15	 0.15
	*/
	sigmainin_B = 0.01;
	sigmainin_EC = 0.03;
	if( workingPoint == WP_95 )
	{
		deltaphi_B  = 0.8;
		deltaeta_B  = 0.007;
		hoE_B       = 0.15;

		deltaphi_EC = 0.7;
		deltaeta_EC = 0.01;
		hoE_EC      = 0.15;
	}
	else if( workingPoint == WP_90 )
	{
		deltaphi_B  = 0.8;
		deltaeta_B  = 0.007;
		hoE_B       = 0.12;

		deltaphi_EC = 0.7;
		deltaeta_EC = 0.009;
		hoE_EC      = 0.15;
	}
	else if( workingPoint == WP_85 )
	{
		deltaphi_B  = 0.06;
		deltaeta_B  = 0.006;
		hoE_B       = 0.04;

		deltaphi_EC = 0.04;
		deltaeta_EC = 0.007;
		hoE_EC      = 0.15;
	}
	else if( workingPoint == WP_80 )
	{
		deltaphi_B  = 0.06;
		deltaeta_B  = 0.004;
		hoE_B       = 0.04;

		deltaphi_EC = 0.03;
		deltaeta_EC = 0.007;
		hoE_EC      = 0.15;
	}
	else if( workingPoint == WP_70 )
	{
		deltaphi_B  = 0.03;
		deltaeta_B  = 0.004;
		hoE_B       = 0.0025;

		deltaphi_EC = 0.02;
		deltaeta_EC = 0.005;
		hoE_EC      = 0.15;
	}
	else if( workingPoint == WP_60 )
	{
		deltaphi_B  = 0.025;
		deltaeta_B  = 0.004;
		hoE_B       = 0.025;

		deltaphi_EC = 0.02;
		deltaeta_EC = 0.005;
		hoE_EC      = 0.15;
	}
	else
	{
		std::cerr << "WPElecID::WPElecID ERROR: Working Point '"
			<< workingPoint << "' not implemented. Exiting..." << std::endl;
		exit(-1);
	}

	fWP = workingPoint;
}

bool WPElecID::checkBarrel( const double & sigmaietaieta, const double & deltaPhiIn,
		const double & deltaEtaIn, const double & HtoE) const
{
	return sigmaietaieta < sigmainin_B && fabs(deltaPhiIn) < deltaphi_B &&
		fabs(deltaEtaIn) < deltaeta_B && HtoE < hoE_B;
}

bool WPElecID::checkEndcap( const double & sigmaietaieta, const double & deltaPhiIn,
		const double & deltaEtaIn, const double & HtoE) const
{
	return sigmaietaieta < sigmainin_EC && fabs(deltaPhiIn) < deltaphi_EC &&
		fabs(deltaEtaIn) < deltaeta_EC && HtoE < hoE_EC;
}

