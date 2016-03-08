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

    
    //push marketing information to the selection of agents that expressed interest in receiving marketing information
    for (auto& agent:interested_agents)
    {
        //push all marketing messages
        for (auto& mes:marketing_mess)
        {
            agent->get_inf(mes);
        };
        
    };
    
    
    
    //randomly select agents and push marketing information 
    //MARK: cont.
    
    
    
    
    
}

