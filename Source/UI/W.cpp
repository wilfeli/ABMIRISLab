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
        auto N_HHMarketingStateHighlyInterested = pt.get<long>("N_HHMarketingStateHighlyInterested");
        
        //create RNG
        //MARK: cont.
        
        
        //set internals
        //MARK: cont.
        time = 0;
        
        
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
        
        
        
        //create random number generators for locations
        //is created here to speed up generation, otherwise rng is created for each agent, so location formula is not used directly.
        //check that it is uniform distribution
        if (pt.get<std::string>("location").find("FORMULA::p.d.f.::u_int(0, size)") == std::string::npos)
        {
            throw std::runtime_error("unsupported hh specification rule");
        };
        auto max_ = world_map->g_map[0].size();
        auto pdf_location_x = boost::uniform_int<uint64_t>(0, max_);
        auto rng_location_x = boost::variate_generator<boost::mt19937&, boost::uniform_int<uint64_t>>(rand->rng, pdf_location_x);
        max_ = world_map->g_map.size();
        auto pdf_location_y = boost::uniform_int<uint64_t>(0, max_);
        auto rng_location_y = boost::variate_generator<boost::mt19937&, boost::uniform_int<uint64_t>>(rand->rng, pdf_location_y);
        
        
        auto formula_roof_age = pt.get<std::string>("House.roof_age");
        auto formula_roof_size = pt.get<std::string>("House.roof_size");
        //create rundom number generators for House
        if (formula_roof_age.find("FORMULA::p.d.f.::N_trunc") == std::string::npos)
        {
            throw std::runtime_error("unsupported hh specification rule");
        };
        if (formula_roof_size.find("FORMULA::p.d.f.::N_trunc") == std::string::npos)
        {
            throw std::runtime_error("unsupported hh specification rule");
        };
        
        double mean_roof_age = std::stod(formula_roof_age.substr(formula_roof_age.find("(") + 1, formula_roof_age.find(",") - formula_roof_age.find("(") - 1));
        double sigma2_roof_age = std::stod(formula_roof_age.substr(formula_roof_age.find(",") + 1, formula_roof_age.find(",") - formula_roof_age.find(")") - 1));
        
        auto pdf_roof_age = boost::normal_distribution<>(mean_roof_age, std::pow(sigma2_roof_age, 0.5));
        auto rng_roof_age = boost::variate_generator<boost::mt19937&, boost::normal_distribution<>>(rand->rng, pdf_roof_age);
        
        
        double mean_roof_size = std::stod(formula_roof_size.substr(formula_roof_size.find("(") + 1, formula_roof_size.find(",") - formula_roof_size.find("(") - 1));
        double sigma2_roof_size = std::stod(formula_roof_size.substr(formula_roof_size.find(",") + 1, formula_roof_size.find(",") - formula_roof_size.find(")") - 1));
        
        auto pdf_roof_size = boost::normal_distribution<>(mean_roof_size, std::pow(sigma2_roof_size, 0.5));
        auto rng_roof_size = boost::variate_generator<boost::mt19937&, boost::normal_distribution<>>(rand->rng, pdf_roof_size);
        
        
        
        //create THETA_design
        std::map<std::string, std::vector<std::string>> THETA_design_str;
        serialize::deserialize(pt.get_child("THETA_design"), THETA_design_str);
        
        auto formula_THETA = THETA_design_str[EnumFactory::FromEParamTypes(EParamTypes::HHDecPreliminaryQuote)];
        
        if (formula_THETA[0].find("FORMULA::p.d.f.::u(0, 1)") == std::string::npos)
        {
            throw std::runtime_error("unsupported hh specification rule");
        };
        
        auto pdf_THETA = boost::uniform_01<>();
        auto rng_THETA = boost::variate_generator<boost::mt19937&, boost::uniform_01<>>(rand->rng, pdf_THETA);
        
        
        std::map<EParamTypes, std::vector<double>> THETA_design;
        for (auto& iter:THETA_design_str)
        {
            THETA_design[EnumFactory::ToEParamTypes(iter.first)] = std::vector<double>{};
        };
        
        
        //create HH
        auto j = 0;
        for (auto i = 0; i < N_HH; ++i)
        {
            if (j < N_HHMarketingStateHighlyInterested)
            {
                //create few highly interested agents
                //put specific parameters into template
                pt.put("marketing_state", EnumFactory::FromEParamTypes(EParamTypes::HHMarketingStateHighlyInterested));
            };
            
            ++j;
            
            //generate location
            pt.put("location_x", rng_location_x());
            pt.put("location_y", rng_location_y());
            
            
            //generate House
            //roof_age
            //roof_size
            pt.put("House.roof_age", std::max(0.0, rng_roof_age()));
            pt.put("House.roof_size", std::max(0.0, rng_roof_size()));
            
            //create decision parameters
            THETA_design[EParamTypes::HHDecPreliminaryQuote][0] = rng_THETA();
            pt.put_child("THETA_design", serialize::serialize(THETA_design, "THETA_design").get_child("THETA_design"));
            
            
            
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


