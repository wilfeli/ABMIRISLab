//
//  W.cpp
//  ABMSolar
//
//  Created by Ekaterina Sinitskaya on 3/2/16.
//  Copyright (c) 2016 IRIS Lab. All rights reserved.
//

#include "UI/W.h"
#include "Agents/IAgent.h"


using namespace solar_core;

/**
 
 Assume that world is created from scratch
 
*/
W::W(std::string path_, std::string mode_)
{
    
    //set ui flags
    FLAG_IS_STOPPED = true;
    
    
    //saves path to basic template
    base_path = path_;
    Log::instance(path_);
    
    if (mode_ == "NEW")
    {
        
        //create existing solar modules
        
        
        
        //create SEI - use template for parameters, use model file for additional parameters
        
        
        
        
        
    };
}



void
W::life()
{
    bool FLAG_MAX_N_ITERATIONS = params.count("N_ITERATIOMS");
    long MAX_N_ITERATIONS = 0;
    if (FLAG_MAX_N_ITERATIONS)
    {
        MAX_N_ITERATIONS = std::stol(params["N_ITERATIONS"]);
    };
    
    while (!FLAG_IS_STOPPED)
    {
        //stop if max number iterations is exceeded
        if (FLAG_MAX_N_ITERATIONS)
        {
            if ((time - begin_time) > MAX_N_ITERATIONS)
            {
                FLAG_IS_STOPPED = true;
                FLAG_IS_STARTED = false;
                break;
            };
        };
        
        if (updated_counter >= constants::NUMBER_AGENT_TYPES_LIFE)
        {
            time++;
            updated_counter = 0;
            FLAG_SEI_TICK = true;
            FLAG_H_TICK = true;
            FLAG_G_TICK = true;
            FLAG_SEM_TICK = true;
            
            
            all_update.notify_all();
            while((notified_counter < constants::NUMBER_AGENT_TYPES_LIFE) && !FLAG_IS_STOPPED)
            {
                //need additional notifies, otherwise thread could wake while FLAG_TICK is false and get back to sleep. During the time it takes for it to go to sleep, notify_all could have been called and missed this thread, that was in between state
                all_update.notify_all();
            };
            
            notified_counter = 0;
        }
        else
        {
        };
    };
}




void
W::life_hhs()
{
    
    
    //go through households that indicated desire to request information and inform them that action to request information could be taken
    for (auto& agent:get_inf_marketing_sei_agents)
    {
        ///@DevStage3 might consider moving this call to tasks, to speed up cycle. Might not be worth it as have to include the time to set up and tear down the task itself and the calls might be relatively quick. Need to profile this place.
        agent->ac_inf_marketing_sei();
    };
    
    
    
    
    //go through part of agents and call act_tick - staged action for them
    //MARK: cont.
    
}


void
W::life_seis()
{
    while (!FLAG_IS_STOPPED)
    {
        if (FLAG_SEI_TICK && !FLAG_IS_STOPPED)
        {
            notified_counter++;
            FLAG_SEI_TICK = false;
            
            for (auto agent:seis)
            {
                //get tick
                agent->act_tick(time);
            };
            updated_counter++;
        };
        
        
        while (!FLAG_SEI_TICK && !FLAG_IS_STOPPED)
        {
            //wait until new tick come
            std::unique_lock<std::mutex> l(lock_tick);
            //takes a predicate that is used to loop until it returns false
            all_update.wait_for(l, std::chrono::milliseconds(constants::WAIT_MILLISECONDS_LIFE_TICK),[this](){return (FLAG_SEI_TICK || FLAG_IS_STOPPED); });
        };

        
    };
    
    
};
