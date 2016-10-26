//
//  SEM.cpp
//  ABMSolar
//
//  Created by Ekaterina Sinitskaya on 4/28/16.
//  Copyright (c) 2016 IRIS Lab. All rights reserved.
//

#include "Tools/WorldSettings.h"
#include "UI/W.h"
#include "Agents/SEM.h"
#include "Agents/SolarPanel.h"
#include "Institutions/IMessage.h"
#include "Tools/Serialize.h"

#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>

using namespace solar_core;

SEM::SEM(const PropertyTree& pt_, W* w_)
{
    
    w = w_;
    
    history_sales = std::vector<double>(WorldSettings::instance().constraints[EConstraintParams::SEMMaxLengthRecordHistory], 0.0);
    
    
    
    //read parameters
    std::map<std::string, std::string> params_str;
    serialize::deserialize(pt_.get_child("params"), params_str);
    
    ///@DevStage2 move to W to speed up, but test before that
    for (auto& iter:params_str)
    {
        params[EnumFactory::ToEParamTypes(iter.first)] = serialize::solve_str_formula<double>(iter.second, *w->rand);
    };
    
    
    std::vector<std::string> THETA_profit_str;
    serialize::deserialize(pt_.get_child("THETA_profit"), THETA_profit_str);
    for (auto& iter:THETA_profit_str)
    {
        THETA_profit.push_back(serialize::solve_str_formula<double>(iter, *w->rand));
    }

    money = serialize::solve_str_formula<double>(pt_.get<std::string>("money"), *w->rand);
    
    costs_base = serialize::solve_str_formula<double>(pt_.get<std::string>("costs_base"), *w->rand);
    
    
    N_PANELS_inventories = pt_.get<double>("N_PANELS_inventories");
    
    sem_production_time = -params[EParamTypes::SEMFrequencyProduction];
    sem_research_time = 0;
    sem_dec_time = 0;
    a_time = w->time;
    
}



void
SEM::init(W* w_)
{
    //collect solar panels
    for (auto iter:WorldSettings::instance().solar_modules)
    {
        if (iter.second->manufacturer == this)
        {
            solar_panel_templates.push_back(iter.second);
        };
    };
    
    
    //for each solar module create inventories
    for (auto iter:solar_panel_templates)
    {
        inventories[iter->name] = N_PANELS_inventories;
    };
    
    //set prices
    for (auto iter:solar_panel_templates)
    {
        auto efficiency_differential = iter->efficiency/params[EParamTypes::SEMPriceBaseEfficiency];
    
        prices[iter->name] = costs_base * THETA_profit[0] * (1 + params[EParamTypes::SEMPriceMarkupEfficiency] * std::pow(-1, 1 - std::signbit(efficiency_differential - 1)));
    };
    
}


void SEM::init_world_connections()
{
    //MARK: cont. add price setting for inverters and PV modules
    
    
}



bool
SEM::sell_SolarModule(solar_core::MesSellOrder &mes_)
{
    bool FLAG_TRANSACTION = false;
    if (inventories.count(mes_.item) > 0 && inventories[mes_.item] >= mes_.qn)
    {
        inventories[mes_.item] -= mes_.qn;
        FLAG_TRANSACTION = true;
    };
    
    lock.lock();
    history_sales[a_time % history_sales.size()] += mes_.qn;
    lock.unlock();
    
    return FLAG_TRANSACTION;
}


void
SEM::ac_update_tick()
{
    lock.lock();
    //update internal timer
    a_time = w->time;
    history_sales[a_time % history_sales.size()] = 0.0;
    lock.unlock();
}




void
SEM::act_tick()
{
 
    //update internals for the tick
    ac_update_tick();

    
    
    //produce inverters and solar panels according to templates
    if ((a_time - sem_production_time) >= params[EParamTypes::SEMFrequencyProduction])
    {
        for (auto& solar_panel:solar_panel_templates)
        {
            //add production quantity to the inventory
            inventories[solar_panel->name] += params[EParamTypes::SEMProductionQuantity];
        };
        
        sem_production_time = a_time;
    };
    
    
    //innovate, change parameters for templates
    if ((a_time - sem_research_time) >= params[EParamTypes::SEMFrequencyResearchTemplates])
    {
        //MARK: see if this parameter needs to be changed? ot iterated over in the cycle
        //pick random template and innovate on it
        if (params[EParamTypes::SEMNSolarPanelsResearch] > 0.0)
        {
            auto pdf_i = boost::uniform_int<uint64_t>(0, solar_panel_templates.size() - 1);
            auto rng_i = boost::variate_generator<boost::mt19937&, boost::uniform_int<uint64_t>>(w->rand_sem->rng, pdf_i);
            
            std::set<std::size_t> templates_to_replace;
            
            while (templates_to_replace.size() < std::min((double)solar_panel_templates.size(), params[EParamTypes::SEMNSolarPanelsResearch]))
            {
                templates_to_replace.insert(rng_i());
            };
            
            
            for (auto i:templates_to_replace)
            {
                std::shared_ptr<SolarModule> new_pv(new SolarModule(*solar_panel_templates[i]));
                new_pv->name = boost::uuids::to_string(boost::uuids::random_generator()());
                new_pv->efficiency *= (1 + params[EParamTypes::SEMEfficiencyUpgradeResearch]);
                
                solar_panel_templates[i] = new_pv;
                WorldSettings::instance().solar_modules[new_pv->name] = new_pv;
            };
            
        };
        
        
    };
    
    //change prices if demand is changing
    //@DevStage2 change to avoid check at every cycle
    if (((a_time - sem_dec_time) >= params[EParamTypes::SEMFrequencyPriceDecisions]) && (a_time >= 2))
    {
        
        auto qn_t = history_sales[(a_time - 1) % history_sales.size()];
        auto qn_t_1 = history_sales[(a_time - 2) % history_sales.size()];
        
        if (qn_t > 0.0 && qn_t_1 > 0.0)
        {
            //base markup is here
            THETA_profit[0] *= qn_t/qn_t_1;
        };
        
        
        for (auto iter:prices)
        {
            auto efficiency = WorldSettings::instance().solar_modules[iter.first]->efficiency;
            auto efficiency_differential = efficiency/params[EParamTypes::SEMPriceBaseEfficiency];
            
            iter.second = costs_base * THETA_profit[0] * (1 + params[EParamTypes::SEMPriceMarkupEfficiency] * std::pow(-1, 1 - std::signbit(efficiency_differential - 1)));
            
        };
        
        
        sem_dec_time = a_time;
        
        
    };
    
    
    
    
    
    
    
}