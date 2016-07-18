//
//  SEMBL.cpp
//  ABMSolar
//
//  Created by Ekaterina Sinitskaya on 7/9/16.
//  Copyright (c) 2016 IRIS Lab. All rights reserved.
//

#include "Agents/SEMBL.h"

using namespace solar_core;

void
SEMBL::ac_update_tick()
{
    lock.lock();
    //update internal timer
    a_time = w->time;
    history_sales[a_time % history_sales.size()] = 0.0;
    
    
    lock.unlock();
}



void
SEMBL::act_tick()
{
    
    ac_update_tick();
    
    
    //if time is up - advance techonology
    //offer new panel with randomly drawn efficiency - as a random walk
    //and random reliability and complexty, ass random walks - lognormal variety
    //calibrate efficiency increase and degradation? - as a proxy to quality
    
    
    //draw new efficiency from the random walk
    //draw new reliability rate lambda from random walk
    //draw new mean and variance for complexity from random walk
    
    //assume for now they are separate distributions
    
   
    std::shared_ptr<SolarModuleBL> new_pv(new SolarModuleBL(*solar_panel_templates[EDecParams::CurrentTechnology]));
    
    //set params for new module 
    
    
    lock.lock();
    solar_panel_templates[EDecParams::CurrentTechnology] = new_pv;
    lock.unlock();
    
    
    
}