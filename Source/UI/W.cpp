//
//  W.cpp
//  ABMSolar
//
//  Created by Ekaterina Sinitskaya on 3/2/16.
//  Copyright (c) 2016 IRIS Lab. All rights reserved.
//

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/exceptions.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/filesystem.hpp>
using boost::property_tree::ptree;
using boost::property_tree::ptree_error;
using boost::property_tree::read_json;
using boost::property_tree::write_json;



#include "Tools/Serialize.h"

#include "UI/W.h"
#include "Tools/WorldSettings.h"
#include "Geography/Geography.h"
#include "Agents/IAgent.h"
#include "Agents/SEI.h"
#include "Agents/H.h"


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
    boost::filesystem::path path_to_model_file(path_);
    boost::filesystem::path path_to_dir = path_to_model_file.parent_path();
    boost::filesystem::path path_to_template;
//    Log::instance(path_);
    
    if (mode_ == "NEW")
    {
        std::string path = path_to_model_file.string();
        //baseline model
        PropertyTree pt;
        read_json(path, pt);
        
        auto N_SEI = pt.get<long>("N_SEI");
        auto N_HH = pt.get<long>("N_HH");
        
        
        //create RNG
        
        
        
        
        //solar_module.json
        path_to_template = path_to_dir;
        path_to_template /= "solar_module.json";
        path = path_to_template.string();
        read_json(path, pt);
        
        //create existing solar modules
        serialize::deserialize(pt.get_child("solar_modules"), WorldSettings::instance().solar_modules);
        
        
        //create grid
        path_to_template = path_to_dir;
        path_to_template /= "geography.json";
        path = path_to_template.string();
        read_json(path, pt);
        world_map = new WorldMap(pt, this);
        
        //hh.json
        path_to_template = path_to_dir;
        path_to_template /= "hh.json";
        path = path_to_template.string();
        read_json(path, pt);
        //create HH
        for (auto i = 0; i < N_HH; ++i)
        {
            //read configuration file
            //replace parameters if necessary
            hhs.push_back(new Household(pt, this));
            
        };
        
        
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
            ++time;
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
            ++notified_counter;
            FLAG_SEI_TICK = false;
            
            for (auto agent:seis)
            {
                //get tick
                agent->act_tick();
            };
            ++updated_counter;
        };
        
        
        while (!FLAG_SEI_TICK && !FLAG_IS_STOPPED)
        {
            //wait until new tick come
            std::unique_lock<std::mutex> l(lock_tick);
            //takes a predicate that is used to loop until it returns false
            all_update.wait_for(l, std::chrono::milliseconds(constants::WAIT_MILLISECONDS_LIFE_TICK),[this](){return (FLAG_SEI_TICK || FLAG_IS_STOPPED); });
        };

        
    };
    
    
}



double
W::get_solar_radiation(double location_x, double location_y) const
{
    return world_map->g_map[location_x][location_y]->solar_radiation;
}

double
W::get_permit_difficulty(double location_x, double location_y) const
{
    return world_map->g_map[location_x][location_y]->permit_difficulty;
}


void
W::get_state_inf(Household* agent_, EParamTypes state_)
{
}


