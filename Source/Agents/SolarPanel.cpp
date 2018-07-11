//
//  SolarPanel.cpp
//  ABMSolar
//
//  Created by Ekaterina Sinitskaya on 3/8/16.
//  Copyright (c) 2016 IRIS Lab. All rights reserved.
//

#include "Tools/WorldSettings.h"
#include "Agents/SolarPanel.h"
#include "Agents/SEMBL.h"
#include "Agents/Homeowner.h"
#include "Physics/SolarPanelBL.h"


using namespace solar_core;

double SolarModule::initialization_tolerance = 0.1;



std::shared_ptr<Inverter>
Inverter::deserialize(const PropertyTree& pt_)
{
    //placeholder to be replaced by other type if needed
    if (pt_.get<std::string>("ClassType") == "solar_core::Inverter")
    {
        return std::make_shared<Inverter>(pt_);
    }
    else
    {
        return std::make_shared<Inverter>(pt_);
    };
}


std::shared_ptr<Inverter>
Inverter::deserialize(const DocumentNode& pt_)
{
	//placeholder to be replaced by other type if needed
	if (tools::get_string(pt_["ClassType"]) == "solar_core::Inverter")
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
    manufacturer_id = pt_.get<std::string>("Manufacturer Id");
    name = pt_.get<std::string>("Name");
    technology = EnumFactory::ToESEIInverterType(pt_.get<std::string>("Type"));
    
}


Inverter::Inverter(const DocumentNode& pt_)
{
	manufacturer_id = tools::get_string(pt_["Manufacturer Id"]);
	name = tools::get_string(pt_["Name"]);
	technology = EnumFactory::ToESEIInverterType(tools::get_string(pt_["Type"]));

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

std::shared_ptr<SolarModule>
SolarModule::deserialize(const DocumentNode& pt_)
{

	if (tools::get_string(pt_["TYPE"]) == "solar_core::SolarModuleBL")
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
	origin = pt_.get<std::string>("Origin");
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

SolarModule::SolarModule(const DocumentNode& pt_)
{
	name = tools::get_string(pt_["Name"]);
	origin = tools::get_string(pt_["Origin"]);
	efficiency = tools::get_double(pt_["Peak Efficiency"]);
	STC_power_rating = tools::get_double(pt_["STC Power Rating"]);
	p_sem = tools::get_double(pt_["Price (from manufacturer)"]);
	length = tools::get_double(pt_["Length"]);
	width = tools::get_double(pt_["Width"]);
	warranty_length = WorldSettings::instance().params_exog[EParamTypes::PVModuleWarrantyLength];
	manufacturer_id = tools::get_string(pt_["Manufacturer Id"]);
	degradation = tools::get_double(pt_["Degradation after 10 years"]);


	//added double check on parameters consistency
	double STC_power_rating_from_ef = efficiency * length * width / 1000;


	if (std::abs(STC_power_rating_from_ef - STC_power_rating) > initialization_tolerance)
	{
		throw std::runtime_error("inconsistent solar panel parameters");
	};


	//convert multiple parameters into indicator of visiibility
	std::string color = tools::get_string(pt_["Color"]);
	std::string pattern = tools::get_string(pt_["Pattern"]);

	//depending on the pattern will become low visibility
	if ((pattern == "Solid") || (pattern == "Lines"))
	{
		visibility = 1.0;
	};




}




PVProject::~PVProject()
{
    if (is_temporary)
    {
        delete agent->house;
        delete agent;
    };
}

