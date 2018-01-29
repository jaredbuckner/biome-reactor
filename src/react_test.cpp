// react_test.cpp

#include <cstdio>
#include <cmath>

#include "react.hpp"
using react::reaction;
using react::product;

int main(int argc, char* argv[]) {
  react::Simulator mySim;

  constexpr double rateConstant = std::exp(1.0);

  double plants = 5000;
  mySim.set_params(&plants, 0.1, 0.01);
  
  // Growing plants
  constexpr double plantLimit = 10000;
  constexpr double plantRate = 1.0 / 15.0;
  reaction(mySim, plantRate / rateConstant, &plants)
    >> product(&plants, 2);
  reaction(mySim, - plantRate / plantLimit / rateConstant, &plants, &plants)
    >> product(&plants, 3);

  // Starving mice
  double hMice = 0;
  mySim.set_params(&hMice, 0.1, 0.01);
  constexpr double miceDRate = 1.0 / 10.0;
  reaction(mySim, miceDRate / rateConstant, &hMice); // No products!
  
  // Well fed mice
  double fMice = 100;
  mySim.set_params(&fMice, 0.1, 0.01);
  constexpr double mouseSeekRate = 24.0;
  constexpr double mousePlantsPerDay = 2.0;
  reaction(mySim, mouseSeekRate / rateConstant, &hMice, &plants)
    >> product(&fMice, 1.0 / mousePlantsPerDay)
    >> product(&hMice, 1.0 - 1.0 / mousePlantsPerDay);

  reaction(mySim, 1.0 / mousePlantsPerDay / rateConstant, &fMice)
    >> product(&hMice);

  constexpr double mouseBRate = 1.0 / 30.0;
  reaction(mySim, mouseBRate / rateConstant, &fMice)
    >> product(&fMice, 2.0);  
  
  // Starving hawks
  double hHawks = 1;
  mySim.set_params(&hHawks, 0.1, 0.01);
  constexpr double hawkDRate = 1.0 / 14.0;
  reaction(mySim, hawkDRate / rateConstant, &hHawks); // No products!
  
  // Well fed hawks
  double fHawks = 0;
  mySim.set_params(&fHawks, 0.1, 0.01);
  constexpr double hawkSeekRate = 10.0;
  constexpr double hawkMicePerDay = 3.0;
  reaction(mySim, hawkSeekRate / rateConstant, &hMice, &hHawks)
    >> product(&fHawks, 1.0 / hawkMicePerDay)
    >> product(&hHawks, 1.0 - 1.0 / hawkMicePerDay);
  
  reaction(mySim, hawkSeekRate / rateConstant, &fMice, &hHawks)
    >> product(&fHawks, 1.0 / hawkMicePerDay)
    >> product(&hHawks, 1.0 - 1.0 / hawkMicePerDay);
  
  reaction(mySim, 1.0 / hawkMicePerDay, &fHawks)
    >> product(&hHawks);
  
  constexpr double hawkBRate = 1.0 / 90.0;
  reaction(mySim, hawkBRate / rateConstant, &fHawks)
    >> product(&fHawks, 2.0);
  
  
  double t = 0.0;
  double stepSize = 1.0;
  
  for(size_t hours = 0; hours <= 24 * 365 * 5.0; ++hours) {
    double nextT = (double)hours / 24.0;
    
    printf("%6.2f : %.0f  %.0f (%2.0f%%)  %.0f (%2.0f%%)\n", t,
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
