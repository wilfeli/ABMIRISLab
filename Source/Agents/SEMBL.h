//
//  SEMBL.h
//  ABMSolar
//
//  Created by Ekaterina Sinitskaya on 7/9/16.
//  Copyright (c) 2016 IRIS Lab. All rights reserved.
//

#ifndef __ABMSolar__SEMBL__
#define __ABMSolar__SEMBL__


#include "Agents/SEM.h"

namespace solar_core
{
    
class SolarModuleBL;


class SEMBL: public SEM
{
public:
    //@{
    /**
     
     
     */
    
    SEMBL(const PropertyTree& pt_, W* w_);
    
    
    //@}
    
    
    
    
    
    
    //@{
    /**
     
     Section with actions in the world
     
     */
    
    virtual void act_tick() override;
    

    //@}
    
    std::map<EDecParams, std::shared_ptr<SolarModuleBL>>solar_panel_templates;/*!< list of solar panels to produce */
    
    int N_connections_current = 0;
    int N_connections_new = 0;
    
    void add_connection(std::shared_ptr<SolarModuleBL> link);
    void remove_connection(std::shared_ptr<SolarModuleBL> link);
    
protected:
    
    //@{
    /**
     
     Section with internal actions of an agent
     
     
     */
    void ac_update_tick();
    
    
    //@}
    
    
    std::vector<double> THETA_dist_efficiency; /*!< parameters for lognormal random walk, mean, variance, current value. Mean is average increase in efficiency over the past years. Variance - small on the scale of efficiency. Starting value - 0.16 current average production efficiency. */
    std::vector<double> THETA_dist_reliability; /*!< parameters for lognormal random walk. Data generating distribution is exponential. Assume that currently it is once every 5 years, so \f$ \lambda_{0} = \frac{1}{5} \f$. */
    std::vector<double> THETA_dist_complexity; /*!< parameters for lognormal random walk. Data generating distribution is Normal. Need 4 parameters for 2 random walks and 2 current parameters. 
        Average repair time is 2 days + waiting for materials - 2 weeks, assume total 20 * 8 = 160 labor hours, and deviation of 20 labor hours.  */
    
    Eigen::MatrixXd mean_rw_complexity_dist;
    Eigen::MatrixXd sigma_rw_complexity_dist;
    Eigen::MatrixXd normTransform_rw_complexity_dist;
    Eigen::MatrixXd sample_ind_rw_complexity_dist;
    Eigen::MatrixXd sample_rw_complexity_dist ;
    
    
    double p_baseline = 150.0; /*!< base price for the panel, constant for now for simplicity */
    static int N_complexity_params; // Dimensionality (rows)
    
};

} //end namespace solar_core

#endif /* defined(__ABMSolar__SEMBL__) */
