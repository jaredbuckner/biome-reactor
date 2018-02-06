// react_test.cpp

#include <cstdio>
#include <cmath>

#include "react.hpp"
using react::reaction;
using react::product;

int main(int argc, char* argv[]) {
  react::Simulator mySim;

  double plants = 10000;
  mySim.set_params(&plants, 0.1, 0.01);
  
  // Plants reproduce
  reaction(mySim, 0.1, &plants) >> product(&plants, 2);

  // Plants crowd each other out
  reaction(mySim, 0.1 / 50000, &plants, &plants) >> product(&plants);

  double hMice = 0;
  mySim.set_params(&hMice, 0.1, 0.01);

  // Hungry mice die;
  reaction(mySim, 0.1, &hMice);
  
  double fMice = 400;
  mySim.set_params(&fMice, 0.1, 0.01);

  // Well fed mice reproduce
  reaction(mySim, 0.05, &fMice) >> product(&fMice, 2);

  // Hungry mice eat
  reaction(mySim, 0.0001, &hMice, &plants)
    >> product(&fMice);

  // Full mice get hungry
  reaction(mySim, 1.0, &fMice)
    >> product(&hMice);

  double hHawks = 0;
  mySim.set_params(&hHawks, 0.1, 0.01);
  
  // Hungry hawks die
  reaction(mySim, 0.05, &hHawks);

  double fHawks = 5;
  mySim.set_params(&fHawks, 0.01, 0.01);

  // Well fed hawks reproduce
  reaction(mySim, 0.02, &fHawks) >> product(&fHawks, 2);

  // Hungry hawks eat
  reaction(mySim, 0.002, &hHawks, &hMice) >> product(&fHawks);
  reaction(mySim, 0.002, &hHawks, &fMice) >> product(&fHawks);

  // Full hawks get hungry
  reaction(mySim, 1.0, &fHawks) >> product(&hHawks);  
  
  double t = 0.0;
  double stepSize = 1.0;
  
  for(size_t hours = 0; hours <= 24 * 365 * 5.0; ++hours) {
    double nextT = (double)hours / 24.0;
    
    printf("%6.2f : %7.0f  %5.0f (%3.0f%%)  %3.0f (%3.0f%%)\n", t,
           plants,
           (hMice + fMice), 100.0 * fMice / (hMice + fMice),
           (hHawks + fHawks), 100.0 * fHawks / (hHawks + fHawks));
    while(t < nextT) {
      if(t + stepSize > nextT) {
        stepSize = nextT - t;
      }
      stepSize = mySim.forwardStep(t, stepSize, 1.0 / 24.0 / 60.0 / 60.0 / 1000.0);
      // printf("W: %.15g P%.15g  M%.15g %.15g  H%.15g %.15g\n", stepSize, plants, hMice, fMice, hHawks, fHawks);
      t += stepSize;
      
      stepSize *= 2.1;
      if(stepSize > 1.0) { stepSize = 1.0; }
    }
  }
  
  return(0);
}
