//
//  SolarPanel.cpp
//  ABMSolar
//
//  Created by Ekaterina Sinitskaya on 3/8/16.
//  Copyright (c) 2016 IRIS Lab. All rights reserved.
//

#include "Agents/SolarPanel.h"
#include "Agents/SEMBL.h"

using namespace solar_core;

double SolarModule::initialization_tolerance = 0.1;



std::shared_ptr<Inverter>
Inverter::deserialize(const PropertyTree& pt_)
{
    //placeholder to be replaced by other type if needed
    if (pt_.get<std::string>("TYPE") == "solar_core::Inverter")
    {
        return std::make_shared<Inverter>(pt_);
    }
    else
    {
        return std::make_shared<Inverter>(pt_);
    };
}


Inverter::Inverter(const PropertyTree& pt_)
{
    name = pt_.get<std::string>("Name");
    technology = EnumFactory::ToESEIInverterType(pt_.get<std::string>("Type"));
    
}


std::shared_ptr<SolarModule>
SolarModule::deserialize(const PropertyTree& pt_)
{
    
    if (pt_.get<std::string>("TYPE") == "solar_core::SolarModuleBL")
    {
        
        return std::make_shared<SolarModuleBL>(pt_);
    }
    else
    {
        return std::make_shared<SolarModule>(pt_);
    };
}





SolarModule::SolarModule(const PropertyTree& pt_)
{
    name = pt_.get<std::string>("Name");
    efficiency = pt_.get<double>("Peak Efficiency");
    STC_power_rating = pt_.get<double>("STC Power Rating");
    p_sem = pt_.get<double>("Price (from manufacturer)");
    length = pt_.get<double>("Length");
    width = pt_.get<double>("Width");
    warranty_length = WorldSettings::instance().params_exog[EParamTypes::PVModuleWarrantyLength];
    manufacturer_id = pt_.get<std::string>("Manufacturer Id");
    degradation = pt_.get<double>("Degradation after 10 years");
    
    
    //added double check on parameters consistency
    double STC_power_rating_from_ef = efficiency * length * width / 1000;
    
    
    if (std::abs(STC_power_rating_from_ef - STC_power_rating) > initialization_tolerance)
    {
        throw std::runtime_error("inconsistent solar panel parameters");
    };
    
    
    //convert multiple parameters into indicator of visiibility
    std::string color = pt_.get<std::string>("Color");
    std::string pattern = pt_.get<std::string>("Pattern");
    
    //depending on the pattern will become low visibility
    if ((pattern == "Solid") || (pattern == "Lines"))
    {
        visibility = 1.0;
    };
    
    
    
    
}


SolarModuleBL::SolarModuleBL(const PropertyTree& pt_):SolarModule(pt_)
{
    warranty_length = pt_.get<double>("Warranty length");


}

void SolarModuleBL::init()
{
    //
    THETA_reliability = std::vector<double>(1, 0.0);
    THETA_complexity = std::vector<double>(2, 0.0);
    
    //get parameters from manufacturer
    auto sem = dynamic_cast<SEMBL*>(manufacturer);
    
    //update efficiency to the efficiency of the panel from the .json specification
    sem->THETA_dist_efficiency[2] =  efficiency;
    //update parameters for the distribution
    THETA_reliability[0] = sem->THETA_dist_reliability[2];
    THETA_complexity[0] = sem->THETA_dist_complexity[SEMBL::N_complexity_params * 3];
    THETA_complexity[1] = sem->THETA_dist_complexity[SEMBL::N_complexity_params * 3 + 1];
    
}