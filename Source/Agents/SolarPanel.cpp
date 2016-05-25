//
//  SolarPanel.cpp
//  ABMSolar
//
//  Created by Ekaterina Sinitskaya on 3/8/16.
//  Copyright (c) 2016 IRIS Lab. All rights reserved.
//

#include "SolarPanel.h"


using namespace solar_core;

std::shared_ptr<SolarModule>
SolarModule::deserialize(const PropertyTree& pt_)
{
    return std::make_shared<SolarModule>(pt_);
}





SolarModule::SolarModule(const PropertyTree& pt_)
{
    name = pt_.get<std::string>("Name");
    efficiency = pt_.get<double>("Peak Efficiency");
    STC_power_rating = pt_.get<double>("STC Power Rating");
    p_sem = pt_.get<double>("Price (from manufacturer)");
    length = pt_.get<double>("Length");
    width = pt_.get<double>("Width");
    warranty_length = pt_.get<double>("Warranty length");
    manufacturer_id = pt_.get<std::string>("Manufacturer Id");
    degradation = pt_.get<double>("Degradation after 10 years");
}