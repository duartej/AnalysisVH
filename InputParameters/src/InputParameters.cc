///////////////////////////////////////////////////////////////////////
//
//    FILE: InputParameters.C
//   CLASS: InputParameters
// AUTHORS: A.Y. Rodriguez Marrero, I. Gonzalez Caballero
//    DATE: 2010
//
///////////////////////////////////////////////////////////////////////
#include "InputParameters.h"

#include <iostream>
#include <fstream>
#include <queue>
#include <map>
#include <string>
#include <sstream>
#include <algorithm>

#ifndef __CINT__
ClassImp(InputParameters);
#endif


void InputParameters::DumpParms() {
  std::cout << " +++++++++++++++ My Input Parameters +++++++++++++ " << std::endl;
  std::cout << " + Name: " << GetName() << std::endl;
  std::cout << " + String: " << GetString() << std::endl;
  std::cout << " + Int: " << TheInt() << ", Float: " << TheFloat() << ", Double: " << TheDouble() << ", Bool: " << TheBool() << std::endl;
  std::cout << " + Named String(s): " << fMapString.size() << std::endl;
  std::map<std::string,TString>::iterator its = fMapString.begin();
  while (its != fMapString.end()) { 
    std::cout << " +   '" << its->first.c_str() << "' -> " << its->second.Data() << std::endl;
    its++; 
  }
  std::cout << " + Named Int(s): " << fMapInt.size() << std::endl;
  std::map<std::string,int>::iterator iti = fMapInt.begin();
  while (iti != fMapInt.end()) {
    std::cout << " +   '" << iti->first.c_str() << "' -> " << iti->second << std::endl; 
    iti++; 
  }
  std::cout << " + Named Float(s): " << fMapFloat.size() << std::endl;
  std::map<std::string,float>::iterator itd = fMapFloat.begin();
  while (itd != fMapFloat.end()) { 
    std::cout << " +   '" << itd->first.c_str() << "' -> " << itd->second << std::endl;
    itd++; 
  }
  std::cout << " + Named Double(s): " << fMapDouble.size() << std::endl;
  std::map<std::string,double>::iterator itdb = fMapDouble.begin();
  while (itdb != fMapDouble.end()) {
    std::cout << " +   '" << itdb->first.c_str() << "' -> " << itdb->second << std::endl;
    itdb++;
  }
  std::cout << " + Named Bool(s): " << fMapBool.size() << std::endl;
  std::map<std::string,bool>::iterator itb = fMapBool.begin();
  while (itb != fMapBool.end()) {
    std::cout << " +   '" << itb->first.c_str() << "' -> " << itb->second << std::endl;
    itb++; 
  }
  std::cout << " +++++++++++++++++++++++++++++++++++++++++++++++++ " << std::endl;
}

InputParameters * InputParameters::parser(const std::string & cfgfile)
{ 
	// auxiliar variable
	static std::map<std::string,int> typesmap;
	typesmap["int"] = 0;
	typesmap["Int_t"] = 0;
	typesmap["unsigned"] = 0;
	typesmap["long"] = 0;
	typesmap["short"] = 0;
	typesmap["float"] = 1;
	typesmap["Float_t"] = 1;
	typesmap["double"]  = 2;
	typesmap["Double_t"] = 2;
	typesmap["bool"] =3;
	typesmap["Bool_t"] = 3;
	typesmap["std::string"] = 4;
	typesmap["string"] = 4;
	typesmap["TString"] = 4;

	std::ifstream cf(cfgfile.c_str());
	if (!cf.is_open()) 
	{
		std::cerr << "InputParameter::parser ERROR: Cannot open input file '" << cfgfile
			<< "'" << std::endl;
		cf.close();
		exit(-1);
	}

	// Initialize an instance
	InputParameters * ip = new InputParameters;

	std::string line;
	// Note format: "@var Type variableName value;"
	while( ! cf.eof() )
	{
		getline(cf,line);
		if( line == "" || line == "\t" )
		{
			continue;
		}

		std::stringstream sline(line);
		std::vector<std::string> words;
		std::string wordtoken;
		while( sline >> wordtoken )
		{
			words.push_back( wordtoken );
		}
		// Return, tabulate, ..., others
		if( words.size() == 0 ) 
		{		
			continue;
		}

		// Checkings (already checked case 0)
		if( words.size() % 4 != 0)
		{
			std::cerr << "InputParameters::parser ERROR "
				<< " Config file '"<< cfgfile << "'format unknown!!"
				<< std::endl;
			exit(-1);
		}
		if( words.at(0) != "@var" )
		{
			std::cerr << "InputParameters::parser ERROR "
				<< " Config file '"<< cfgfile << "'format unknown!!"
				<< std::endl;
			exit(-1);
		}

		//First word: type
		std::string type = words.at(1);
		std::string varname = words.at(2);
		std::string value = words.at(3);
		if( typesmap[type] == typesmap["int"] )
		{
			std::istringstream buff( value);
			int val;
			buff >> val;
			ip->SetNamedInt(varname,val);
		}
		else if( typesmap[type] == typesmap["float"] )
		{
			std::istringstream buff( value);
			float val;
			buff >> val;
			ip->SetNamedFloat(varname,val);
		}
		else if( typesmap[type] == typesmap["double"] )
		{
			std::istringstream buff( value);
			double val;
			buff >> val;
			ip->SetNamedDouble(varname,val);
		}
		else if( typesmap[type] == typesmap["bool"] )
		{
			std::istringstream buff( value);
			bool val;
			buff >> val;
			ip->SetNamedBool(varname,val);
		}
		else if( typesmap[type] == typesmap["string"] )
		{
			size_t pos = value.rfind(";");
			if( pos != std::string::npos )
			{
				value.erase(pos);
			}
			ip->SetNamedString(varname,value);
		}
		else
		{
			std::cerr << "InputParameter::parser ERROR " 
				<< " Input type do not recognized '" << type << "'"
				<< std::endl;
			delete ip;
			exit(-1);
		}
	}
	
	return ip;
}
