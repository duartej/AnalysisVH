///////////////////////////////////////////////////////////////////////
//
//    FILE: VectorUtils.h
//   CLASS: VectorUtils
// AUTHORS: I. Gonzalez Caballero
//    DATE: 21/02/2011
//
// CONTENT: Some utility functions related to vectors, angles, etc...
//
///////////////////////////////////////////////////////////////////////

#ifndef VectorUtils_h
#define VectorUtils_h 1

#include <math.h>

class VectorUtils {
 public:
  static inline float Eta(float x, float y, float z) { float t(z/sqrt(x*x+y*y)); return asinhf(t);} 

  static inline double eta3(double x, double y, double z) { double t(z/sqrt(x*x+y*y)); return asinh(t);} 

  //Calculates DeltaPhi. Returns a value between -pi and pi
  static float DeltaPhi(float phi1, float phi2);
  static double DeltaPhi(double phi1, double phi2);
  
  //DR2 = DPhi*DPhi + DEta*DEta
  static float  DeltaR2(float eta1,  float phi1, 
			float eta2,  float phi2);
  static double DeltaR2(double eta1, double phi1, 
			double eta2, double phi2);
  
  //DR2 = sqrt(DR2)
  static float  DeltaR(float eta1,  float phi1, 
		       float eta2,  float phi2);
  static double DeltaR(double eta1, double phi1, 
		       double eta2, double phi2);

  //Rho2 = x^2+y^2+z^2
  //Rho = sqrt(x^2+y^2+z^2)
  static inline float Rho2(float x, float y, float z) {return (x*x+y*y+z*z);}
  static inline double Rho2(double x, double y, double z) {return (x*x+y*y+z*z);}
  static inline float Rho(float x, float y, float z) {return sqrt(Rho2(x,y,z));}
  static inline double Rho(double x, double y, double z) {return sqrt(Rho2(x,y,z));}

};


#endif
