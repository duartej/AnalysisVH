// -*- C++ -*-
//
// Package:    ElecSelection
// Class:      WPElecID
// 
/**\class  WPElecID

 Description: Helper class to define centrally the Electron Working Points

 Implementation: 
*/
//
// \author: Jordi Duarte Campderros  
//         Created:  Sun Oct  30 12:20:31 CET 2011
// 
// jordi.duarte.campderros@cern.ch
//
//
#ifndef WPELECID_H
#define WPELECID_H

// Helper class to deal with the working points
class WPElecID
{
	public:
		//! Working points definitions: extracted from Egamma POG
		//! https://twiki.cern.ch/twiki/bin/view/CMS/VbtfEleID2011
		enum 
		{
			WP_95,
			WP_90,
			WP_85,
			WP_80,
			WP_70,
			WP_60,
			WP_INDEF
		};
		//! Default constructor
		WPElecID( const unsigned int & workingpoint);
		~WPElecID() { }

		//! Methods to check the cuts
		bool checkBarrel( const double & sigmaietaieta, const double & deltaPhiIn,
				const double & deltaEtaIn, const double & HtoE) const;
		bool checkEndcap( const double & sigmaietaieta, const double & deltaPhiIn,
				const double & deltaEtaIn, const double & HtoE) const;

		//! Print values given a WP 
		void Print() const;

	private:
		//! Avoiding the default constructor use
		WPElecID() { }
		
		//! Working point
		unsigned int fWP;

		//! Barrel
		double sigmainin_B;
		double deltaphi_B;
		double deltaeta_B;
		double hoE_B;
		//! Endcap
		double sigmainin_EC;
		double deltaphi_EC;
		double deltaeta_EC;
		double hoE_EC;
};

#endif
