// react.cpp

#include "react.hpp"

#include <cmath>

namespace react {

  double Simulator::rate_data::applyRate(double t) const {
    double reply = 0.0;
    
    for(rate_calc_terms_type::const_iterator termIter = terms.begin();
        termIter != terms.end(); ++termIter) {
      double factor = 1.0;
      if(termIter->first[0]) {
        factor *= *(termIter->first[0]);
        if(termIter->first[1]) {
          factor *= *(termIter->first[1]);
        }
      }

      reply += factor * termIter->second.base;
      for(callback_seq_type::const_iterator callBackIter = termIter->second.callbacks.begin();
          callBackIter != termIter->second.callbacks.end(); ++callBackIter) {
        reply += factor * (*callBackIter)(t);
      }
    }

    return(reply);
  }

  void Simulator::_fillInteraction(interaction_type& tgt,
                                   const double* v1,
                                   const double* v2) {
    if(v1 > v2) {
      tgt[0] = v1;
      tgt[1] = v2;
    } else {
      tgt[0] = v2;
      tgt[1] = v1;
    }
  }  

  void Simulator::set_params(double* valueT,
                             double maxRateStep,
                             double maxRateErr) {
    rate_data& valueRateData = _rateMap[valueT];
    valueRateData.maxRateStep = maxRateStep;
    valueRateData.maxRateErr = maxRateErr;
  }

  void Simulator::add_fixed_rate(double* valueT,
                                 double rate,
                                 const double* valueA,
                                 const double* valueB) {
    interaction_type seek;
    _fillInteraction(seek, valueA, valueB);

    rate_data& valueRateData = _rateMap[valueT];
    valueRateData.terms[seek].base += rate;    
  }

  void Simulator::add_func_rate(double* valueT,
                                rate_callback_type* rateFunc,
                                const double* valueA,
                                const double* valueB) {
    interaction_type seek;
    _fillInteraction(seek, valueA, valueB);

    rate_data& valueRateData = _rateMap[valueT];
    valueRateData.terms[seek].callbacks.push_back(rateFunc);
  }

  void Simulator::clear_rate(double* valueT,
                             const double* valueA,
                             const double* valueB) {
    
    interaction_type seek;
    _fillInteraction(seek, valueA, valueB);
    
    rate_data& valueRateData = _rateMap[valueT];
    valueRateData.terms.erase(seek);    
  }

  void Simulator::clear_all_rates(double* valueT) {
    rate_data& valueRateData = _rateMap[valueT];

    valueRateData.terms.clear();
  }

  void Simulator::erase_var(double* valueT) {
    _rateMap.erase(valueT);
  }
  
  void Simulator::clear() {
    _rateMap.clear();
  }

  double Simulator::forwardStep(double t, double stepSize, double minStepSize) {

    // First, calculate the current rates.  This calculation won't change...
    for(rate_map_type::iterator valueRateIter = _rateMap.begin();
        valueRateIter != _rateMap.end(); ++valueRateIter) {
      valueRateIter->second.rate = valueRateIter->second.applyRate(t);
    }
    
    for(bool done=false; !done; stepSize /= 2.0) {
      if(stepSize <= minStepSize) {
        stepSize = minStepSize;
        done = true;
      }

      bool minViol = false;
      for(rate_map_type::iterator valueRateIter = _rateMap.begin();
        valueRateIter != _rateMap.end(); ++valueRateIter) {
        const double& valueRef = *(valueRateIter->first);
        rate_data& rateData = valueRateIter->second;
        
        double delVal = rateData.rate * stepSize;
        double predVal = valueRef + delVal;
        double rateRev = rateData.applyRate(t + stepSize);
        double delRev = rateRev * stepSize;
        double predRev = predVal - delRev;
        double predErr = (predRev - valueRef) / 2.0;
        double corrVal = predVal - predErr;
        double stepVal = corrVal - valueRef;
        
        if(!done) {
          if(std::fabs(predErr) > rateData.maxRateErr) {
            minViol = true;
            // printf("Limited by error of %.15g\n", predErr);
            break;
          }
          if(std::fabs(stepVal) > rateData.maxRateStep) {
            minViol = true;
            // printf("Limited by step of %.15g\n", stepVal);
            break;
          }
          if(std::fabs(corrVal) > 0.0 &&
             std::fabs(stepVal / corrVal) > 0.10) {
            minViol = true;
            // printf("Limited by relative step\n");
          }
        }

        rateData.nextVal = corrVal;
      }

      if(!minViol) { done = true; }      
    }

    // Update values
    for(rate_map_type::iterator valueRateIter = _rateMap.begin();
        valueRateIter != _rateMap.end(); ++valueRateIter) {
        double& valueRef = *(valueRateIter->first);
        rate_data& rateData = valueRateIter->second;
        valueRef = rateData.nextVal;
    }

    return(stepSize);
  }

  
}

  
