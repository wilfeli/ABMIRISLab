//
//  SEIBL.h
//  ABMSolar
//
//  Created by Ekaterina Sinitskaya on 7/9/16.
//  Copyright (c) 2016 IRIS Lab. All rights reserved.
//

#ifndef __ABMSolar__SEIBL__
#define __ABMSolar__SEIBL__

#include "Agents/SEI.h"
#include "Tools/ID.h"


namespace solar_core {
    class H;
    class WEE;
}




namespace solar_core
{
    
    class TDesign
    {
    public:
        std::shared_ptr<SolarModuleBL> module; /*!< is used to pull ID, efficiency, degradation */
        std::vector<double> THETA_reliability; /*!< prior is G(1, 1/(expected time without failures = warranty)), data is exponential, posterior is \f$ G(a + n, b + \sum_{i}\left(y_{i})\right) \f$   */
        std::vector<double> THETA_complexity; /*!< prior is Normal-Inverse Gamma(), posterior will be the same, and data distirbution is Normal with unknown mean and variance:  */
        double irr = 0.0; /*!< advertized rate of return for an average project */
        double p_module = 0.0; /*!< price of a module for this installer */
        double p_design = 0.0; /*!< price per watt for the design based on this module */
        double complexity_install = 16.0; /*!< in labor*hours for the current state of the project */
        double BETA_complexity_time = 1.0; /*!< discounting for learning on how to install */
    };
    
    
    
    class SEIBL: public SEI
    {
        friend class WEE;
    public:
        std::shared_ptr<PVProjectFlat> form_design_for_params(H* agent_, std::shared_ptr<PVProjectFlat> project);
        std::vector<double> THETA_reputation; /*!< have current estimate of a reputation by onlookers. Gamma distribution, is updated based on the realized production of installations */
        
    protected:
        
        std::map<UID, std::shared_ptr<TDesign>>  designs;  /** have current estimates on system design parameters for different learning distributions. UID of a solar_module here */
        
        std::shared_ptr<TDesign> dec_design; /*!< current design to offer */
        
        
        
        
        std::vector<double> THETA_demand; /*!< BLR estimate for demand for projects given own irr and other parameters */
        
        std::vector<double> THETA_complexity_prior;
        
        std::vector<double> THETA_reliability_prior;

        std::vector<double> WM_time; /*!< current state of information about reputation of others, promised returns of others, average for both parameters */
        
        std::vector<double> THETA_exploration; /*!< parameters for exploration distribution. Assume that probability of switching depends on the difference in profits. Two classes - with high and low probability of switching */
        
        std::mutex lock;
        
        WEE* w;
    };
    
    
    
    
} //end namespace solar_core



#endif /* defined(__ABMSolar__SEIBL__) */
