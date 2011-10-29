///////////////////////////////////////////////////////////////////////
//
//    FILE: VectorUtils.h
//   CLASS: VectorUtils
// AUTHORS: I. Gonzalez Caballero
//    DATE: 21/02/2011
//
///////////////////////////////////////////////////////////////////////

#include "VectorUtils/interface/VectorUtils.h"

float VectorUtils::DeltaPhi(float phi1, float phi2){ 
  float pi = M_PI;
  float result = phi1 - phi2;
  while (result > pi) result -= 2*pi;
  while (result <= -pi) result += 2*pi;
  return result;
}

float VectorUtils::DeltaR2(float eta1, float phi1, 
			   float eta2, float phi2){
  float deta = eta1 - eta2;
  float dphi = VectorUtils::DeltaPhi(phi1, phi2);
  return deta*deta + dphi*dphi;
}

float VectorUtils::DeltaR(float eta1, float phi1, 
			  float eta2, float phi2){
  return sqrt(VectorUtils::DeltaR2(eta1, phi1, eta2, phi2));
}

double VectorUtils::DeltaPhi(double phi1, double phi2){ 
  double pi = M_PI;
  double result = phi1 - phi2;
  while (result > pi) result -= 2*pi;
  while (result <= -pi) result += 2*pi;
  return result;
}

double VectorUtils::DeltaR2(double eta1, double phi1, 
			    double eta2, double phi2){
  double deta = eta1 - eta2;
  double dphi = VectorUtils::DeltaPhi(phi1, phi2);
  return deta*deta + dphi*dphi;
}

double VectorUtils::DeltaR(double eta1, double phi1, 
			   double eta2, double phi2){
  return sqrt(VectorUtils::DeltaR2(eta1, phi1, eta2, phi2));
}

