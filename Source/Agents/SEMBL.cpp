//
//  SEMBL.cpp
//  ABMSolar
//
//  Created by Ekaterina Sinitskaya on 7/9/16.
//  Copyright (c) 2016 IRIS Lab. All rights reserved.
//

#include "UI/WEE.h"
#include "Agents/SEMBL.h"
#include "Agents/SolarPanel.h"

using namespace solar_core;

void SEMBL::add_connection(std::shared_ptr<SolarModuleBL> link)
{
    if (solar_panel_templates[EDecParams::NewTechnology] == link)
    {
        ++N_connections_new;
    }
    else
    {
        ++N_connections_current;
    };
}


void SEMBL::remove_connection(std::shared_ptr<SolarModuleBL> link)
{
    if (solar_panel_templates[EDecParams::NewTechnology] == link)
    {
        --N_connections_new;
    }
    else
    {
        --N_connections_current;
    };
}


void SEMBL::ac_update_tick()
{
    lock.lock();
    
    //update internal timer
    a_time = w->time;
    history_sales[a_time % history_sales.size()] = 0.0;
    
    
    //switch techonologies
    if (N_connections_current == 0)
    {
        solar_panel_templates[EDecParams::CurrentTechnology] = solar_panel_templates[EDecParams::NewTechnology];
        solar_panel_templates[EDecParams::NewTechnology] = nullptr;
        N_connections_current += N_connections_new;
        N_connections_new = 0;
    };
    
    lock.unlock();
}



void SEMBL::act_tick()
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
    
    //new efficiency from random walk
    THETA_dist_efficiency[2] = THETA_dist_efficiency[2] * std::exp(w->rand->rnd() * std::pow(THETA_dist_efficiency[1], 0.5) + THETA_dist_efficiency[0]);
    
    
    //new reliability from random walk
    
    
   
    std::shared_ptr<SolarModuleBL> new_pv(new SolarModuleBL(*solar_panel_templates[EDecParams::CurrentTechnology]));
    
    //set params for new module
    new_pv->efficiency = THETA_dist_efficiency[2];
    
    
    
    //think if there is problem that reliability is updated as random log-normal walk, but prior is assumed to be Gamma. Might need to make it gamma too?
    

    
    //update uid
    new_pv->
    

    solar_panel_templates[EDecParams::NewTechnology] = new_pv;

    
    
    
}