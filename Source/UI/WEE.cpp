//
//  WEE.cpp
//  ABMSolar
//
//  Created by Ekaterina Sinitskaya on 7/9/16.
//  Copyright (c) 2016 IRIS Lab. All rights reserved.
//





#include "Tools/Serialize.h"

#include "UI/WEE.h"
#include "Tools/WorldSettings.h"
#include "Tools/ParsingTools.h"
#include "Tools/Simulation.h"
#include "Geography/Geography.h"
#include "Institutions/MarketingSystem.h"
#include "Agents/IAgent.h"
#include "Agents/SEI.h"
#include "Agents/SEIBL.h"
#include "Agents/SEMBL.h"
#include "Agents/SEM.h"
#include "Agents/Utility.h"
#include "Agents/G.h"
#include "Agents/Homeowner.h"
#include "Agents/H.h"
#include "UI/HelperW.h"


using namespace solar_core;

/**
 
 Assume that world is created from scratch
 
 */
WEE::WEE(std::string path_, HelperW* helper_, std::string mode_)
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

        
        
        //ho.json
        path_to_template = path_to_dir;
        path_to_template /= "ho.json";
        path = path_to_template.string();
        read_json(path, pt);

        
        hos = dynamic_cast<HelperWSpecialization<WEE, ExploreExploit>*>(helper_)->create_hos(pt, mode_, path_to_dir, rng_location_x, rng_location_y, this);
        
        
        
        //sei.json
        path_to_template = path_to_dir;
        path_to_template /= "sei.json";
        path = path_to_template.string();
        read_json(path, pt);
        
        seis = dynamic_cast<HelperWSpecialization<WEE, ExploreExploit>*>(helper_)->create_seis(pt, mode_, params_d[EParamTypes::N_SEI], rng_location_x, rng_location_y, this);
        
        
        
        //sem.json
        ///@DevStage2 each sem will pick initial templates by name? - could make it base creation mode
        path_to_template = path_to_dir;
        path_to_template /= "sem.json";
        path = path_to_template.string();
        read_json(path, pt);
        
        for (auto i = 0; i < params_d[EParamTypes::N_SEM]; ++i)
        {
            sems.push_back(new SEMBL(pt, this));
        };
        
        
        max_ = sems.size() - 1;
        auto pdf_i = boost::uniform_int<uint64_t>(0, max_);
        auto rng_i = boost::variate_generator<boost::mt19937&, boost::uniform_int<uint64_t>>(rand->rng, pdf_i);
        
        
        //set producers
        for (auto iter: WorldSettings::instance().solar_modules)
        {
            if (iter.second->manufacturer_id == "FORMULA::RANDOM")
            {
                iter.second->manufacturer = sems[rng_i()];
                iter.second->manufacturer_id = iter.second->manufacturer->uid.get_string();
                
            }
            else
            {
                throw std::runtime_error("unsupported formula");
            };
        };
        
        
        
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


void
WEE::init()
{
    
    marketing->init(this);
    g->init(this);
    
    
    
    for (auto& agent:hos)
    {
        agent->init(this);
    };
    
    for (auto& agent:seis)
    {
        agent->init(this);
    };
    
    for (auto& agent:sems)
    {
        agent->init(this);
    };
    
    
    //preallocate pool of designs
    for (auto i = 0; i < constants::POOL_SIZE; ++i)
    {
        pool_projects.push_back(std::make_shared<PVProjectFlat>());
    };
    
    
    
    
}




void
WEE::life_hos()
{
    
    
    //randomly select agents and push marketing information
    auto pdf_agents = boost::uniform_int<uint64_t>(0, hos.size()-1);
    auto rng_agents = boost::variate_generator<boost::mt19937&, boost::uniform_int<uint64_t>>(rand->rng, pdf_agents);
    
    
    auto pdf_sei_agents = boost::uniform_int<uint64_t>(0, hos.size()-1);
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
            
            for (auto i = 0; i < WorldSettings::instance().params_exog[EParamTypes::WHMaxNToDrawPerTimeUnit];++i)
            {
                while (true)
                {
                    //pick h randomly
                    j_h = rng_agents();
                    if (hos[i]->FLAG_INSTALLED_SYSTEM)
                    {
                        break;
                    };
                };
                
                //pick sei randomly
                j_sei = rng_sei_agents();
                
                
                //form design for an agent
                seis[j_sei]->form_design_for_params(hos[i], pool_projects[i_pool_projects]);
                
                
                //get answer for the design
                FLAG_DEC = hos[i]->ac_dec_design(pool_projects[i_pool_projects], this);
                
                
                
                //if accepted - save as an active project to maintain it
                if (FLAG_DEC)
                {
                    //save as accepted project
                    seis[j_sei]->install_project(pool_projects[i_pool_projects], time);
                    ++i_pool_projects;
                    hos[i]->FLAG_INSTALLED_SYSTEM = true;
                };
                
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



double
WEE::get_inf(EDecParams type_, SEIBL* agent_)
{
    double ret = 0.0;
    switch (type_)
    {
        case EDecParams::Reputation_i:
        {
            double Rep_i = 0.0;
            for (std::size_t i = 0; i < seis.size(); ++i)
            {
                if (seis[i] != agent_)
                {
                    Rep_i += 1/(seis[i]->THETA_reputation[0] - 1);
                };
            };
            ret = Rep_i / (seis.size() - 1);
        }
            break;
        case EDecParams::Price_i:
        {
            double Price_i = 0.0;
            for (std::size_t i = 0; i < seis.size(); ++i)
            {
                if (seis[i] != agent_)
                {
                    Price_i += seis[i]->dec_design->irr;
                };
            };
            ret = Price_i / (seis.size() - 1);
        }
            break;
        case EDecParams::Share:
        {
            ret = market_share_seis[agent_->uid];
        }
            break;
        default:
            break;
    };
    
    return ret;
    
}


void WEE::get_state_inf_installed_project(std::shared_ptr<PVProject> project_)
{
    //save to the corresponding installer
    installed_projects_time[project_->sei->uid].push_back(std::dynamic_pointer_cast<PVProjectFlat>(project_));
    
    //save total number of installed projects
    ++N_installed_projects_time;
}





void WEE::ac_update_tick()
{
    ac_update_wm();
}


/**
 
 Calculate installed projects as a share
 
 */
void WEE::ac_update_wm()
{
    
    //save into history
    installed_projects_history.push_back(installed_projects_time);
    installed_projects_time.clear();
    
    //calculate shares
    for (auto iter:installed_projects_time)
    {
        market_share_seis[iter.first] = iter.second.size()/N_installed_projects_time;
    };
    
    N_installed_projects_time = 0;
    
    //sort by the share in descending order
    auto cmp = [&](UID const & a, UID const & b)
    {
        return market_share_seis[a] > market_share_seis[b];
    };
    
    std::sort(sorted_by_market_share_seis.begin(), sorted_by_market_share_seis.end(), cmp);
    
}






