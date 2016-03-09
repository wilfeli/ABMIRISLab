//
//  IAgent.h
//  ABMSolar
//
//  Created by Ekaterina Sinitskaya on 2/29/16.
//  Copyright (c) 2016 IRIS Lab. All rights reserved.
//

#ifndef __ABMSolar__IAgent__
#define __ABMSolar__IAgent__

#include "Tools/ExternalIncludes.h"


namespace solar_core
{
    
class MesMarketingSEI;

class IAgent
{
public:
    virtual void ac_inf_marketing_sei() = 0; /*!< action to request information from SEI when initiative is given from the W */
    virtual void get_inf(std::shared_ptr<MesMarketingSEI> mes_) = 0; /*!< receives marketing information */
    
    
    
    
    virtual void act_tick() = 0; /*!< generic actions on a tick, specialize for each type of agents */
    
    
};



} //end of solar_core namespace






#endif /* defined(__ABMSolar__IAgent__) */
