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

#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>

using namespace solar_core;

SEM::SEM(const PropertyTree& pt_, W* w_)
{
    
    w = w_;
    
    history_sales = std::vector<double>(WorldSettings::instance().constraints[EConstraintParams::MaxLengthSEMRecordHistory], 0.0);
    
    
    
    
    
}



void
SEM::init(W* w_)
{
}



bool
SEM::sell_SolarModule(solar_core::MesSellOrder &mes_)
{
    return false;
}


void
SEM::ac_update_tick()
{
    //update internal timer
    a_time = w->time;

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
        //pick random template and innovate on it
        if (params[EParamTypes::SEMNSolarPanelsResearch] > 0.0)
        {
            auto pdf_i = boost::uniform_int<uint64_t>(0, params[EParamTypes::SEMNSolarPanelsResearch] - 1);
            auto rng_i = boost::variate_generator<boost::mt19937&, boost::uniform_int<uint64_t>>(w->rand->rng, pdf_i);
            auto i = rng_i();
            std::shared_ptr<SolarModule> new_pv(new SolarModule(*solar_panel_templates[i]));
            new_pv->name = boost::uuids::to_string(boost::uuids::random_generator()());
            new_pv->efficiency *= (1 + params[EParamTypes::SEMEfficiencyUpgradeResearch]);
            
            solar_panel_templates[i] = new_pv;
            WorldSettings::instance().solar_modules[new_pv->name] = new_pv;
            
        };
        
        
    };
    
    //change prices if demand is increasing
    if ()
    {
        //
        a_time % schedule_visits.size()
        a_time - 
    };
    
    
    
    
    
    
    
}