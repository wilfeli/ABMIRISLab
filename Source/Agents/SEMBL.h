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

namespace solar_core {
    



class SEMBL: public SEM
{
public:
    //@{
    /**
     
     Section with actions in the world
     
     */
    
    virtual void act_tick() override;

    //@}
    
    std::map<EDecParams, std::shared_ptr<SolarModuleBL>>solar_panel_templates;/*!< list of solar panels to produce */
protected:
    std::vector<double> THETA_dist_efficiency; /*!< parameters for lognormal random walk */
    std::vector<double> THETA_dist_reliability; /*!< parameters for lognormal random walk */
    std::vector<double> THETA_dist_complexity; /*!< parameters for lognormal random walk */
    
    
    
};

} //end namespace solar_core

#endif /* defined(__ABMSolar__SEMBL__) */
