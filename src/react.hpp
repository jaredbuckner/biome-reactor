// react.hpp
#ifndef _REACT_HPP_
#define _REACT_HPP_

#include <array>
#include <map>
#include <vector>

namespace react {

  class Simulator {
  private:
    typedef std::array<const double*,2> interaction_type;
    void _fillInteraction(interaction_type& tgt,
                          const double* v1,
                          const double* v2);

  public:
    typedef double rate_callback_type(double);

  private:
    typedef std::vector<rate_callback_type *> callback_seq_type;

    struct rate_calc_factors {
      double base;
      callback_seq_type callbacks;
      rate_calc_factors() : base(0.0), callbacks() {}
      
    }; // end struct rate_calc_terms
    
    typedef std::map<interaction_type, rate_calc_factors> rate_calc_terms_type;

    struct rate_data {
      double maxRateStep;
      double maxRateErr;
      rate_calc_terms_type terms;

      double applyRate(double t) const;
      
      // These are used during calculations
      double rate;
      double nextVal;
      
    }; // end struct rate_data

    typedef std::map<double*, rate_data> rate_map_type;

    rate_map_type _rateMap;
    
  public:
    Simulator() {}
    
    Simulator(const Simulator&) = delete;
    ~Simulator() {}

    void set_params(double* valueT,
                    double maxRateStep,
                    double maxRateErr);
    
    void add_fixed_rate(double* valueT,
                        double rate,
                        const double* valueA = 0,
                        const double* valueB = 0);
    
    void add_func_rate(double* valueT,
                       rate_callback_type* rateFunc,
                       const double* valueA = 0,
                       const double* valueB = 0);
    
    void clear_rate(double* valueT,
                    const double* valueA = 0,
                    const double* valueB = 0);
    void clear_all_rates(double* valueT);

    void erase_var(double* valueT);    
    void clear();
    
    // Returns the actual stepSize used.  Increment t by this value
    double forwardStep(double t, double stepSize, double minStepSize);
    
  }; // end class Simulator


  // reaction(sim, r, &A, &B) >> 2 * product(&c) >> 3 * product(&d);
  struct product_cont {
    double* value;
    double mult;

    product_cont(double* v, double m) : value(v), mult(m) {}
  };

  inline product_cont product(double* v, double m = 1.0) { return(product_cont(v, m)); }
  
  inline product_cont& operator*(double m, product_cont& p) { p.mult *= m; return(p); }
  inline product_cont& operator*(product_cont& p, double m) { return( m * p ); }  
  
  class reaction_cont {
    Simulator& sim;
    double rate;
    const double* valueA;
    const double* valueB;
    
  public:
    reaction_cont(Simulator& s, double r,
                  double* a,
                  double* b) :
      sim(s), rate(r), valueA(a), valueB(b) {
      if(a) sim.add_fixed_rate(a, -rate, a, b);
      if(b) sim.add_fixed_rate(b, -rate, a, b);
    }

    reaction_cont& operator>>(const product_cont& p) {
      sim.add_fixed_rate(p.value, rate * p.mult, valueA, valueB);
      return(*this);
    }

  }; // end class reaction

  inline reaction_cont reaction(Simulator& s, double r,
                                double* a = 0,
                                double* b = 0) {
    return(reaction_cont(s, r, a, b));
  }  
  
} // end namespace react

#endif
