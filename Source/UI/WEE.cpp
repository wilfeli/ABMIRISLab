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

void WEE::ac_update_tick()
{
    installed_projects_history.push_back(installed_projects_time);
    installed_projects_time.clear();
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
            
            for (auto i = 0; i < WorldSettings::instance().params_exog[EParamTypes::WHMaxNToDrawPerTimeUnit]; ++i)
            {
                //pick h randomly
                j_h = rng_agents();
                
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
                    Rep_i += seis[i]->reputation;
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


void W::get_state_inf_installed_project(std::shared_ptr<PVProject> project_)
{
    //save to the corresponding installer
    installed_projects_time[project_->sei->uid].push_back(project_);
    
    //save total number of installed projects
    ++N_installed_projects_time;
}





/**
 
 Calculate installed projects as a share
 
 */
void
WEE::ac_update_wm()
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






