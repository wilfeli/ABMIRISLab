//
//  MarketingSystem.cpp
//  ABMSolar
//
//  Created by Ekaterina Sinitskaya on 2/29/16.
//  Copyright (c) 2016 IRIS Lab. All rights reserved.
//

#include "Institutions/MarketingSystem.h"


using namespace solar_core;


void
MarketingInst::act_tick()
{

    
    //push marketing information to selection
    
    for (auto& agent:interested_agents)
    {
        //check if
        
        agent->get_inf();
    };
    
    
    
    //randomly select agents and push marketing information 
    
    
}

