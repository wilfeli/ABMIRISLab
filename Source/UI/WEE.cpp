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

        

//#ifndef DEBUG
        hos = dynamic_cast<HelperWSpecialization<WEE, ExploreExploit>*>(helper_)->create_hos(pt, mode_, path_to_dir, rng_location_x, rng_location_y, this);
        
//#endif
        
        //sem.json
        ///@DevStage2 each sem will pick initial templates by name? - could make it base creation mode
        path_to_template = path_to_dir;
        path_to_template /= "sem.json";
        path = path_to_template.string();
        read_json(path, pt);

        
        sems = dynamic_cast<HelperWSpecialization<WEE, ExploreExploit>*>(helper_)->create_sems(pt, mode_, params_d[EParamTypes::N_SEM], rng_location_x, rng_location_y, this);
        
        
        
        //sei.json
        path_to_template = path_to_dir;
        path_to_template /= "sei.json";
        path = path_to_template.string();
        read_json(path, pt);
        
        seis = dynamic_cast<HelperWSpecialization<WEE, ExploreExploit>*>(helper_)->create_seis(pt, mode_, params_d[EParamTypes::N_SEI], rng_location_x, rng_location_y, this);
        
        
        //no other institutions in this simple model
        //flags are left here just in case if decide later to add this agents back 
        
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
    

    for (auto& agent:*hos)
    {
        agent->init(this);
    };
    
    for (auto& agent:*seis)
    {
        agent->init(this);
        
        
        //add to the pool
        sorted_by_market_share_seis.push_back(agent->uid);
        
    };
    
    for (auto& agent:*sems)
    {
        agent->init(this);
    };
    
    
    //preallocate pool of designs
    for (auto i = 0; i < constants::POOL_SIZE; ++i)
    {
        pool_projects.push_back(std::make_shared<PVProjectFlat>());
    };
    
    

    //create pool of preassigned projects - mark some H as FLAG_INSTALLED_SYSTEM
    
    //pick agent by number
    auto sei = (*seis)[0];
    
    //adjust parameters for past
    std::map<std::string, double> params_t_1;
    //design
    auto dec_design_hat = sei->dec_design;
    
    params_t_1["efficiency"] = dec_design_hat->PV_module->efficiency;
    params_t_1["p_module"] = dec_design_hat->p_module;
    params_t_1["p_design"] = dec_design_hat->p_design;
    
    dec_design_hat->PV_module->efficiency = 0.15;
    double p_per_watt = 1.0;
    double panel_watts = dec_design_hat->PV_module->efficiency * (dec_design_hat->PV_module->length * dec_design_hat->PV_module->width/1000000)*1000;
    
    dec_design_hat->p_module = panel_watts * p_per_watt;
    dec_design_hat->p_design = 4.5;
    
    
    //randomly select agents and push marketing information
    auto pdf_agents = boost::uniform_int<uint64_t>(0, hos->size()-1);
    auto rng_agents = boost::variate_generator<boost::mt19937&, boost::uniform_int<uint64_t>>(rand->rng, pdf_agents);

    
    auto N_installed = 0;
    int64_t N_pre_installed = hos->size() * WorldSettings::instance().params_exog[EParamTypes::PenetrationLevel];
    std::size_t j_h = 0;
    auto max_N_trials = 50000;
    int64_t N_trials = 0;
    bool FLAG_DEC;
    
    while (N_installed < N_pre_installed)
    {
        //check that it doesn't have installation already
        while (true)
        {
            //pick h randomly
            j_h = rng_agents();
            if (!(*hos)[j_h]->FLAG_INSTALLED_SYSTEM)
            {
                break;
            };
        };
        
        ++N_trials;
        if (N_trials > max_N_trials)
        {
            break;
        };
        
        //form design for an agent
        (*seis)[0]->form_design_for_params((*hos)[j_h], pool_projects[i_pool_projects]);
        //get answer for the design
        FLAG_DEC = (*hos)[j_h]->ac_dec_design(pool_projects[i_pool_projects], this);
        //if accepted - mark as installed
        if (FLAG_DEC)
        {
            ++N_installed;
            (*hos)[j_h]->FLAG_INSTALLED_SYSTEM = true;
        };
    };
    
    //restore previous values
    dec_design_hat->PV_module->efficiency = params_t_1["efficiency"];
    dec_design_hat->p_module = params_t_1["p_module"];
    dec_design_hat->p_design = params_t_1["p_design"];



#ifdef DEBUG
    N_installed = 0;
    
    //calculate number of preinstalled projects
    for (auto h: (*hos))
    {
        if (h->FLAG_INSTALLED_SYSTEM)
        {
            ++N_installed;
        };
    };
    
    
//    std::cout << N_installed << std::endl;
    
#endif
    
    
}

void WEE::life()
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
        if (updated_counter >= constants::NUMBER_AGENT_TYPES_LIFE_EE)
        {
            if (FLAG_MAX_N_ITERATIONS)
            {
                if ((time - begin_time) >= (MAX_N_ITERATIONS - 1))
                {
                    FLAG_IS_STOPPED = true;
                    FLAG_IS_STARTED = false;
                    break;
                };
            };

            if (!FLAG_IS_STOPPED)
            {
                ac_update_tick();
                
                ++time;
                updated_counter = 0;
                FLAG_SEI_TICK = true;
                FLAG_H_TICK = true;
                FLAG_G_TICK = true;
                FLAG_SEM_TICK = true;
                FLAG_MARKET_TICK = true;
                
                
                all_update.notify_all();
                while((notified_counter < constants::NUMBER_AGENT_TYPES_LIFE_EE) && !FLAG_IS_STOPPED)
                {
                    //need additional notifies, otherwise thread could wake while FLAG_TICK is false and get back to sleep. During the time it takes for it to go to sleep, notify_all could have been called and missed this thread, that was in between state
                    all_update.notify_all();
                };
                
                notified_counter = 0;
            };
        }
        else
        {
        };
    };
}


void
WEE::life_hos()
{
    
    
    //randomly select agents and push marketing information
    auto pdf_agents = boost::uniform_int<uint64_t>(0, hos->size()-1);
    auto rng_agents = boost::variate_generator<boost::mt19937&, boost::uniform_int<uint64_t>>(rand->rng, pdf_agents);
    
    
    auto pdf_sei_agents = boost::uniform_int<uint64_t>(0, seis->size()-1);
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
            
            if (i_pool_projects < hos->size() - 1)
            {
                for (auto i = 0; i < WorldSettings::instance().params_exog[EParamTypes::WHMaxNToDrawPerTimeUnit];++i)
                {
                    while (true)
                    {
                        //pick h randomly
                        j_h = rng_agents();
                        if (!(*hos)[j_h]->FLAG_INSTALLED_SYSTEM)
                        {
                            break;
                        };
                    };
                    
                    
                    //update for tick
                    (*hos)[j_h]->ac_update_tick(time);
                    
                    
                    //pick sei randomly
                    j_sei = rng_sei_agents();
                    
                    
                    //form design for an agent
                    (*seis)[j_sei]->form_design_for_params((*hos)[j_h], pool_projects[i_pool_projects]);
                    
                    
                    //get answer for the design
                    FLAG_DEC = (*hos)[j_h]->ac_dec_design(pool_projects[i_pool_projects], this);
                    
                    
                    
                    //if accepted - save as an active project to maintain it
                    if (FLAG_DEC)
                    {
                        //save as accepted project
                        (*seis)[j_sei]->install_project(pool_projects[i_pool_projects], time);
                        ++i_pool_projects;
                        (*hos)[j_h]->FLAG_INSTALLED_SYSTEM = true;
                        (*hos)[j_h]->time_installed = time; 
                    };
                    
                };
            }
            
            
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



void
WEE::life_seis()
{
    //    std::cout<< "test" << std::endl;
    
    while (!FLAG_IS_STOPPED)
    {
        if (FLAG_SEI_TICK && !FLAG_IS_STOPPED)
        {
            ++notified_counter;
            FLAG_SEI_TICK = false;
            
            //update wm if need
            for (auto& agent:*seis)
            {
                //get tick
                agent->act_tick_pre();
            };

            
            
            //update wm if need
            for (auto& agent:*seis)
            {
                //get tick
                agent->act_tick_wm();
            };
            
            
            
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
WEE::life_sems()
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




double
WEE::get_inf(EDecParams type_, SEIBL* agent_)
{
    double ret = 0.0;
    switch (type_)
    {
        case EDecParams::Reputation_i:
        {
            double Rep_i = 0.0;
            for (std::size_t i = 0; i < seis->size(); ++i)
            {
                if ((*seis)[i] != agent_)
                {
                    Rep_i += ((*seis)[i]->THETA_reputation[0] != 1.0 ? 1/((*seis)[i]->THETA_reputation[0] - 1) : 1.0);
                };
            };
            ret = Rep_i / (seis->size() - 1);
        }
            break;
        case EDecParams::irr_i:
        {
            double irr_i = 0.0;
            for (std::size_t i = 0; i < seis->size(); ++i)
            {
                if ((*seis)[i] != agent_)
                {
                    irr_i += (*seis)[i]->dec_design->irr;
                };
            };
            ret = irr_i / (seis->size() - 1);
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


void WEE::get_state_inf_installed_project(std::shared_ptr<PVProjectFlat> project_)
{
    //save to the corresponding installer
    installed_projects_time[project_->sei->uid].push_back(std::dynamic_pointer_cast<PVProjectFlat>(project_));
    
    //save total number of installed projects
    ++N_installed_projects_time;
}





void WEE::ac_update_tick()
{
    //update world parameters for inflation
    WorldSettings::instance().params_exog[EParamTypes::LaborPrice] *= (1 + WorldSettings::instance().params_exog[EParamTypes::InflationRate]);
    WorldSettings::instance().params_exog[EParamTypes::AverageElectricityDemand] *= (1 + WorldSettings::instance().params_exog[EParamTypes::AverageElectricityDemandGrowthRate]);
    
    ac_update_wm();
}


/**
 
 Calculate installed projects as a share
 
 */
void WEE::ac_update_wm()
{
    double N = N_installed_projects_time;
    //calculate shares
    for (auto iter:installed_projects_time)
    {
        auto N_i = iter.second.size();
//        double share = N_i / N;
//        N = 10;
//#ifdef DEBUG
//        std::cout << N_i << std::endl;
//#endif
//#ifdef DEBUG
//        std::cout << N << std::endl;
//#endif
//#ifdef DEBUG
//        std::cout << std::fixed << std::setprecision(2) << N_i/N << std::endl;
//#endif
//
//

        
        market_share_seis[iter.first] = N_i/N;
//        
//#ifdef DEBUG
//        std::cout << market_share_seis[iter.first] << std::endl;
//#endif
    };
    
    //save into history
    installed_projects_history.push_back(installed_projects_time);
    installed_projects_time.clear();
    
    
//#ifdef DEBUG
//    std::cout << time << " " << N_installed_projects_time << std::endl;
//    std::cout << time << " " << i_pool_projects + 1 << std::endl;
//#endif
    
    
    N_installed_projects_time = 0;
    
    //sort by the share in descending order
    auto cmp = [&](UID const & a, UID const & b)
    {
        return market_share_seis[a] > market_share_seis[b];
    };
    
    std::sort(sorted_by_market_share_seis.begin(), sorted_by_market_share_seis.end(), cmp);
    
    
//#ifdef DEBUG
//    
//    std::cout << time << "  ";
//    for (auto& uid:sorted_by_market_share_seis)
//    {
//        std::cout << uid.get_string() << "  ";
//        std::cout << market_share_seis[uid] << ",";
//        
//    };
//    std::cout << std::endl;
//    
//
//    
//#endif
//    
//    
//    
//#ifdef DEBUG
//    std::cout<< time << std::endl;
//    for (auto& sei:*seis)
//    {
//        std::cout<< sei->dec_design->p_design << std::endl;
//    };
//#endif
    
    
    
    //save efficiency numbers
    history_data.push_back({});
    for (auto& sei:*seis)
    {
        history_data.back().push_back(sei->dec_design->PV_module->efficiency);
    };
    
    

    
    
}


void
WEE::save_end_data()
{
    //save efficiency numbers
    history_data.push_back({});
    for (auto& sei:*seis)
    {
        history_data.back().push_back(sei->dec_design->PV_module->efficiency);
    };

}



