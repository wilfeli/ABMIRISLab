//
//  H.cpp
//  ABMSolar
//
//  Created by Ekaterina Sinitskaya on 7/9/16.
//  Copyright (c) 2016 IRIS Lab. All rights reserved.
//


#include "Tools/Serialize.h"
#include "Agents/SolarPanel.h"

#include "Agents/H.h"
#include "Agents/SEIBL.h"
#include "UI/WEE.h"
#include "Tools/WorldSettings.h"

using namespace solar_core;


H::H(const PropertyTree& pt_, WEE* w_):THETA_decision(3, 0.0)
{
    //location
    location_x = pt_.get<long>("location_x");
    location_y = pt_.get<long>("location_y");
    
    //House
    house = new House(pt_.get_child("House"));

    

}



void H::ac_update_tick(TimeUnit time_)
{
    //check for how much to update
    auto delta_time = time_ - time_updated_params;
    
    //update Income for inflation
    params[EParamTypes::Income] *= std::pow((1 + WorldSettings::instance().params_exog[EParamTypes::InflationRate]), delta_time);
    //update Energy Consumption for growth rates - historical
    params[EParamTypes::ElectricityConsumption] *= std::pow((1 + WorldSettings::instance().params_exog[EParamTypes::AverageElectricityDemandGrowthRate]), delta_time);
    
    
}



void H::init(WEE* w_)
{
    
    time_updated_params = w_->time;
    
    //calculate probabilty of switching given the parameters of this h
    THETA_decision[0] = 1/THETA_params[1] * std::pow((1 + THETA_params[0]/THETA_params[1] * params[EParamTypes::Income] / 1000),-1/(THETA_params[0] + 1));
    //variance equivalent is fixed for everyone
    THETA_decision[1] = THETA_params[2];
    THETA_decision[2] = 0.5;
    
    
    //adjust income for inflation, assume that it is 2016, and data is from 2009
    //it is 6 years, so
    params[EParamTypes::Income] *= std::pow((1 + WorldSettings::instance().params_exog[EParamTypes::InflationRate]), 6);
    //adjust electricity consumption for growth rates
    params[EParamTypes::ElectricityConsumption] *= (1 + WorldSettings::instance().params_exog[EParamTypes::AverageElectricityDemandHistoricalGrowth]);
    
    //MARK: cont. adjust ElectricityBill for other model
}


bool H::ac_dec_design(std::shared_ptr<PVProjectFlat> project_, WEE* w_)
{
    
    //get irr and reputation of an installer (mean from distribution
    auto irr = project_->irr_a * ((project_->sei->THETA_reputation[0] != 1)? 1/(project_->sei->THETA_reputation[0] - 1) * project_->sei->THETA_reputation[1] : 1);
    
//    if (irr < 0.0)
//    {
//        std::cout << std::endl;
//    };
    
    
    //Logistic function (from cdf of the distribution)
    auto p_switch = (1/(1+std::exp(-((irr) * THETA_decision[2] - THETA_decision[0])/THETA_decision[1])));
    
//#ifdef DEBUG
//    std::cout << std::fixed << std::setprecision(2) << w_->time << ": irr: " << irr << ", p_switch: " << p_switch << std::endl;
//#endif
    
    return (w_->rand_ho->ru() <= p_switch);
}