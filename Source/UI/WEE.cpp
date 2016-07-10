//
//  WEE.cpp
//  ABMSolar
//
//  Created by Ekaterina Sinitskaya on 7/9/16.
//  Copyright (c) 2016 IRIS Lab. All rights reserved.
//

#include "WEE.h"


using namespace solar_core;



void
WEE::life_hhs()
{
    
    
    //randomly select agents and push marketing information
    auto pdf_agents = boost::uniform_int<uint64_t>(0, hhs.size()-1);
    auto rng_agents = boost::variate_generator<boost::mt19937&, boost::uniform_int<uint64_t>>(rand->rng, pdf_agents);
    
    
    auto pdf_sei_agents = boost::uniform_int<uint64_t>(0, hhs.size()-1);
    auto rng_sei_agents = boost::variate_generator<boost::mt19937&, boost::uniform_int<uint64_t>>(rand->rng, pdf_sei_agents);
    
    std::size_t j_h = 0;
    std::size_t j_sei = 0;
    
    bool FLAG_DEC = false;
    
    //go through part of agents and call act_tick - staged action for them
    while (!FLAG_IS_STOPPED)
    {
        if (FLAG_H_TICK && !FLAG_IS_STOPPED)
        {
            ++notified_counter;
            FLAG_H_TICK = false;
            
            for (auto i = 0; i < WorldSettings::instance().params_exog[EParamTypes::WHMaxNToDrawPerTimeUnit]; ++i)
            {
                //pick h randomly
                j_h = rng_agents();
                
                //pick sei randomly
                j_sei = rng_sei_agents();
                
                
                //form design for an agent
                seis[j_sei]->form_design_for_params(hhs[i], pool_designs[i_pool_designs]);
                
                
                //get answer for the design
                FLAG_DEC = hhs[i]->ac_dec_design(seis[j_sei]->form_design_for_params(hhs[i]));
                
                
                
                //if accepted - save as an active project to maintain it
                if (FLAG_DEC)
                {
                    ++i_pool_designs;
                };
                
            };
            
            
            
            for (auto& agent:hhs)
            {
                //get tick
                agent->act_tick();
            };
            ++updated_counter;
        };
        
        while (!FLAG_H_TICK && !FLAG_IS_STOPPED)
        {
            //wait until new tick come
            std::unique_lock<std::mutex> l(lock_tick);
            //takes a predicate that is used to loop until it returns false
            all_update.wait_for(l, std::chrono::milliseconds(constants::WAIT_MILLISECONDS_LIFE_TICK),[this](){return (FLAG_H_TICK || FLAG_IS_STOPPED); });
        };
    };
    
    
    
    
    
}
