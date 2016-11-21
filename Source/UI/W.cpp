//
//  W.cpp
//  ABMSolar
//
//  Created by Ekaterina Sinitskaya on 3/2/16.
//  Copyright (c) 2016 IRIS Lab. All rights reserved.
//

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wc++11-compat"




#include "Tools/Serialize.h"

#include "UI/W.h"
#include "Tools/WorldSettings.h"
#include "Tools/ParsingTools.h"
#include "Tools/Simulation.h"
#include "Geography/Geography.h"
#include "Institutions/MarketingSystem.h"
#include "Agents/IAgent.h"
#include "Agents/SEI.h"
#include "Agents/SEM.h"
#include "Agents/Utility.h"
#include "Agents/G.h"
#include "Agents/Homeowner.h"
#include "UI/HelperW.h"


using boost::property_tree::read_json;


using namespace solar_core;









/**
 
 Assume that world is created from scratch
 
 */
W::W(std::string path_, HelperW* helper_, std::string mode_)
{
    
    //set ui flags
    FLAG_IS_STOPPED = true;
    
    
    //saves path to basic template
    base_path = path_;
    boost::filesystem::path path_to_model_file(path_);
    boost::filesystem::path path_to_dir = path_to_model_file.parent_path();
    boost::filesystem::path path_to_template;
    std::string path;
    //    Log::instance(path_);
    
    //preallocate stuff
    PropertyTree pt;
    std::map<std::string, std::string> params_str;
    
    
    if (mode_ == "NEW")
    {
        
        create_world(path_to_model_file, path_to_dir, path_to_template, pt, params_str);
        
        
        
        //ho.json
        path_to_template = path_to_dir;
        path_to_template /= "ho.json";
        path = path_to_template.string();
        read_json(path, pt);
        
        
        
        
        //left as an old version as need to check that formulas are consistent here with the geography definition
        //create random number generators for locations
        //is created here to speed up generation, otherwise rng is created for each agent, so location formula is not used directly.
        //check that it is uniform distribution
        if (pt.get<std::string>("location").find("FORMULA::p.d.f.::u_int(0, size)") == std::string::npos)
        {
            throw std::runtime_error("unsupported ho specification rule");
        };
        auto max_ = world_map->g_map[0].size() - 1;
        auto pdf_location_x = boost::uniform_int<uint64_t>(0, max_);
        auto rng_location_x = boost::variate_generator<boost::mt19937&, boost::uniform_int<uint64_t>>(rand->rng, pdf_location_x);
        max_ = world_map->g_map.size() - 1;
        auto pdf_location_y = boost::uniform_int<uint64_t>(0, max_);
        auto rng_location_y = boost::variate_generator<boost::mt19937&, boost::uniform_int<uint64_t>>(rand->rng, pdf_location_y);
        
        
        hos = dynamic_cast<HelperWSpecialization<W, BaselineModel>*>(helper_)->create_hos(pt, mode_, path_to_dir, rng_location_x, rng_location_y, this);
        
        
        
        
        //sem.json
        ///@DevStage2 each sem will pick initial templates by name? - could make it base creation mode
        path_to_template = path_to_dir;
        path_to_template /= "sem.json";
        path = path_to_template.string();
        read_json(path, pt);
        
        sems = dynamic_cast<HelperWSpecialization<W, BaselineModel>*>(helper_)->create_sems(pt, mode_, path_to_dir,rng_location_x, rng_location_y, this);

        
        
        
        //sei.json
        path_to_template = path_to_dir;
        path_to_template /= "sei.json";
        path = path_to_template.string();
        read_json(path, pt);
        
        seis = dynamic_cast<HelperWSpecialization<W, BaselineModel>*>(helper_)->create_seis(pt, mode_, path_to_dir,rng_location_x, rng_location_y, this);
        
        
        
        
        
        //create G
        //g.json
        path_to_template = path_to_dir;
        path_to_template /= "g.json";
        path = path_to_template.string();
        read_json(path, pt);
        g = new G(pt, this);
        
        
        //create Utility
        //utility.json
        path_to_template = path_to_dir;
        path_to_template /= "utility.json";
        path = path_to_template.string();
        read_json(path, pt);
        utility = new Utility(pt, this);
        
        
        //create marketing
        marketing = new MarketingInst(this);
        
        
        
        //set flags
        FLAG_H_TICK = true;
        FLAG_G_TICK = true;
        FLAG_UTILITY_TICK = true;
        FLAG_SEI_TICK = true;
        FLAG_SEM_TICK = true;
        FLAG_MARKET_TICK = true;
        updated_counter = 0;
        notified_counter = 0;
        
    };
}



void W::create_world(boost::filesystem::path& path_to_model_file, boost::filesystem::path& path_to_dir, boost::filesystem::path& path_to_template, PropertyTree& pt, std::map<std::string, std::string>& params_str)
{
    std::map<std::string, std::string> parsed_model;
    
    tools::parse_model_file(path_to_model_file.string(), parsed_model);
    
    std::string w_file_name = "";
    if (parsed_model.count("path_to_save") > 0)
    {
        params["path_to_save"] = parsed_model["path_to_save"];
    }
    else
    {
        throw std::runtime_error("Wrong configuration file");
    };
    
    
    path_to_template = path_to_dir;
    path_to_template /= "w.json";
    
    std::string path = path_to_template.string();
    
    //baseline model
    read_json(path, pt);
    
    params_d[EParamTypes::N_SEI] = pt.get<long>("N_SEI");
    params_d[EParamTypes::N_SEILarge] = pt.get<long>("N_SEILarge");
    params_d[EParamTypes::N_SEM] = pt.get<long>("N_SEM");
    params_d[EParamTypes::N_SEMPVProducer] = pt.get<long>("N_SEMPVProducer");
    params_d[EParamTypes::N_SEMInverterProducer] = pt.get<long>("N_SEMInverterProducer");
    params_d[EParamTypes::N_HO] = pt.get<long>("N_HO");
    params_d[EParamTypes::N_HOMarketingStateHighlyInterested] = pt.get<long>("N_HOMarketingStateHighlyInterested");
    
    //create RNG
    rand_sei = new IRandom(pt.get<double>("SEED"));
    rand = new IRandom(pt.get<double>("SEED"));
    rand_sem = new IRandom(pt.get<double>("SEED"));
    rand_ho = new IRandom(pt.get<double>("SEED"));
    rand_g = new IRandom(pt.get<double>("SEED"));
    rand_market = new IRandom(pt.get<double>("SEED"));
    rand_utility = new IRandom(pt.get<double>("SEED"));
    
    //create parameters
    serialize::deserialize(pt.get_child("WorldSettings.params_exog"),params_str);
    
    for (auto& iter:params_str)
    {
        WorldSettings::instance().params_exog[EnumFactory::ToEParamTypes(iter.first)] = serialize::solve_str_formula<double>(iter.second, *rand);
    };
    
    
    
    params_str.clear();
    serialize::deserialize(pt.get_child("WorldSettings.constraints"),params_str);
    for (auto& iter:params_str)
    {
        WorldSettings::instance().constraints[EnumFactory::ToEConstraintParams(iter.first)] = serialize::solve_str_formula<double>(iter.second, *rand);
    };
    
    serialize::deserialize(pt.get_child("params"),params);
    
    //set internals
    time = 0;
    
    
    //solar_module.json
    path_to_template = path_to_dir;
    path_to_template /= "solar_equipment.json";
    path = path_to_template.string();
    read_json(path, pt);
    
    //create existing solar modules
    serialize::deserialize(pt.get_child("solar_modules"), WorldSettings::instance().solar_modules);
    
    
    //create existing inverters
    serialize::deserialize(pt.get_child("inverters"), WorldSettings::instance().inverters);
    
    
    //create grid
    path_to_template = path_to_dir;
    path_to_template /= "geography.json";
    path = path_to_template.string();
    read_json(path, pt);
    world_map = new WorldMap(pt, this);
    
    
    //update permitting difficulty
    //average permit difficulty is connected to processing time required by g
    //
    
    

}









void
W::init()
{
    
    marketing->init(this);
    g->init(this);
    utility->init(this);
    
    
    for (auto& agent:*hos)
    {
        agent->init(this);
    };
    
    for (auto& agent:*seis)
    {
        agent->init(this);
    };
    
    for (auto& agent:*sems)
    {
        agent->init(this);
    };
    

    
    
    
    //update permitting difficulty
    //average permit difficulty is connected to processing time required by g
    //
    int LaborUnitsPerTick = 40;
    WorldSettings::instance().params_exog[EParamTypes::AveragePermitDifficulty] = ((g->params[EParamTypes::GProcessingTimeRequiredForGrantingPermitForInstallation]
    + g->params[EParamTypes::GProcessingTimeRequiredForSchedulingPermitVisit]
    + g->params[EParamTypes::GProcessingTimeRequiredForProcessingPermit]) +
     utility->params[EParamTypes::UtilityProcessingTimeRequiredForPermit] ) * LaborUnitsPerTick;

    
    
    
}



void
W::life()
{
    bool FLAG_MAX_N_ITERATIONS = params.count("N_ITERATIONS");
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
            ac_update_tick();
            
            ++time;
            updated_counter = 0;
            FLAG_SEI_TICK = true;
            FLAG_H_TICK = true;
            FLAG_G_TICK = true;
            FLAG_SEM_TICK = true;
            FLAG_MARKET_TICK = true;
            FLAG_UTILITY_TICK = true;
            
            
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

void W::ac_update_tick()
{
    
    
    //MARK: cont. add update of labor price
    
    
}


void
W::life_hos()
{
    
    //@DevStage3 move to main cycle maybe? As of now it will be called only in the very beginning
    //go through Homeowners that indicated desire to request information and inform them that action to request information could be taken
    //those that are highly interested
    for (auto& agent:get_inf_marketing_sei_agents)
    {
        ///@DevStage3 might consider moving this call to tasks, to speed up cycle. Might not be worth it as have to include the time to set up and tear down the task itself and the calls might be relatively quick. Need to profile this place.
        agent->ac_inf_marketing_sei();
    };
    
    
    
    
    //go through part of agents and call act_tick - staged action for them
    while (!FLAG_IS_STOPPED)
    {
        if (FLAG_H_TICK && !FLAG_IS_STOPPED)
        {
            ++notified_counter;
            FLAG_H_TICK = false;
            
            for (auto i = 0; i < active_hos.size(); ++i)
            {
                auto agent = active_hos[i];
                if (agent)
                {
                    if (agent->marketing_state != EParamTypes::HOMarketingStateCommitedToInstallation)
                    {
                        //get tick
                        agent->act_tick();
                    }
                    else
                    {
                        //TODO check that is nullptr to the agent in the list and not to actual agent
                        agent = nullptr;
                    };
                };
                

            };
            ++updated_counter;
        };
        
        
        //@DevStage3 add cleaning of active_hos from time to time based on timer
        
        
        while (!FLAG_H_TICK && !FLAG_IS_STOPPED)
        {
            //wait until new tick come
            std::unique_lock<std::mutex> l(lock_tick);
            //takes a predicate that is used to loop until it returns false
            all_update.wait_for(l, std::chrono::milliseconds(constants::WAIT_MILLISECONDS_LIFE_TICK),[this](){return (FLAG_H_TICK || FLAG_IS_STOPPED); });
        };
    };
    
    
    
    
    
}


void
W::life_seis()
{
//    std::cout<< "test" << std::endl;
    
    while (!FLAG_IS_STOPPED)
    {
        if (FLAG_SEI_TICK && !FLAG_IS_STOPPED)
        {
            ++notified_counter;
            FLAG_SEI_TICK = false;
            
            for (auto& agent:*seis)
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



void
W::life_sems()
{
    while (!FLAG_IS_STOPPED)
    {
        if (FLAG_SEM_TICK && !FLAG_IS_STOPPED)
        {
            ++notified_counter;
            FLAG_SEM_TICK = false;
            
            for (auto& agent:*sems)
            {
                //get tick
                agent->act_tick();
            };
            ++updated_counter;
        };
        
        while (!FLAG_SEM_TICK && !FLAG_IS_STOPPED)
        {
            //wait until new tick come
            std::unique_lock<std::mutex> l(lock_tick);
            //takes a predicate that is used to loop until it returns false
            all_update.wait_for(l, std::chrono::milliseconds(constants::WAIT_MILLISECONDS_LIFE_TICK),[this](){return (FLAG_SEM_TICK || FLAG_IS_STOPPED); });
        };
    };
}





void
W::life_gs()
{
    while (!FLAG_IS_STOPPED)
    {
        if (FLAG_G_TICK && !FLAG_IS_STOPPED)
        {
            ++notified_counter;
            FLAG_G_TICK = false;
            
            auto& agent = g;
            
            //get tick
            agent->act_tick();
            
            ++updated_counter;
        };
        
        while (!FLAG_G_TICK && !FLAG_IS_STOPPED)
        {
            //wait until new tick come
            std::unique_lock<std::mutex> l(lock_tick);
            //takes a predicate that is used to loop until it returns false
            all_update.wait_for(l, std::chrono::milliseconds(constants::WAIT_MILLISECONDS_LIFE_TICK),[this](){return (FLAG_G_TICK || FLAG_IS_STOPPED); });
        };
    };
}


void
W::life_markets()
{
    while (!FLAG_IS_STOPPED)
    {
        if (FLAG_MARKET_TICK && !FLAG_IS_STOPPED)
        {
            ++notified_counter;
            FLAG_MARKET_TICK = false;
            
            auto& agent = marketing;
            
            //get tick
            agent->act_tick();
            
            ++updated_counter;
        };
        
        while (!FLAG_MARKET_TICK && !FLAG_IS_STOPPED)
        {
            //wait until new tick come
            std::unique_lock<std::mutex> l(lock_tick);
            //takes a predicate that is used to loop until it returns false
            all_update.wait_for(l, std::chrono::milliseconds(constants::WAIT_MILLISECONDS_LIFE_TICK),[this](){return (FLAG_MARKET_TICK || FLAG_IS_STOPPED); });
        };
    };
}


void
W::life_utility()
{
    while (!FLAG_IS_STOPPED)
    {
        if (FLAG_UTILITY_TICK && !FLAG_IS_STOPPED)
        {
            ++notified_counter;
            FLAG_UTILITY_TICK = false;
            
            auto& agent = utility;
            
            //get tick
            agent->act_tick();
            
            ++updated_counter;
        };
        
        while (!FLAG_UTILITY_TICK && !FLAG_IS_STOPPED)
        {
            //wait until new tick come
            std::unique_lock<std::mutex> l(lock_tick);
            //takes a predicate that is used to loop until it returns false
            all_update.wait_for(l, std::chrono::milliseconds(constants::WAIT_MILLISECONDS_LIFE_TICK),[this](){return (FLAG_UTILITY_TICK || FLAG_IS_STOPPED); });
        };
    };
}





double
W::get_solar_irradiation(double location_x, double location_y) const
{
    return world_map->g_map[location_x][location_y]->solar_irradiation;
}



double
W::get_permit_difficulty(double location_x, double location_y) const
{
    return world_map->g_map[location_x][location_y]->permit_difficulty;
}


void
W::get_state_inf(Homeowner* agent_, EParamTypes state_)
{
    
    //depending on the state either set to null if no longer active or add to the list
    switch (state_)
    {
        case EParamTypes::HOMarketingStateHighlyInterested:
        case EParamTypes::HOMarketingStateInterested:
            active_hos.push_back(agent_);
            break;
        case EParamTypes::HOMarketingStateCommitedToInstallation:
            break;
        default:
            break;
            
    };
    
    
    
}



void
W::get_state_inf_installed_project(std::shared_ptr<PVProject> project_)
{
    
    
    
    
}


void
W::get_state_inf_interconnected_project(std::shared_ptr<PVProject> project_)
{
    interconnected_projects.push_back(project_);
    auto mes = project_->sei->mes_marketing;
    auto h = project_->agent;
    
    //send info in the net
    for (auto agent:world_map->h_map[h->location_x][h->location_y])
    {
        agent->get_inf(mes);
    };
    
}




#pragma clang diagnostic pop




