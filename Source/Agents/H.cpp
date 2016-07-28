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
#include "UI/WEE.h"


using namespace solar_core;


H::H(const PropertyTree& pt_, WEE* w_)
{


}




void H::init(WEE* w_)
{
    
    //calculate probabilty of switching given the parameters of this h
    THETA_decision[0] = 1/THETA_params[1] * std::pow((1 + THETA_params[0]/THETA_params[1] * params[EParamTypes::Income] / 1000),-1/(THETA_params[0] + 1));
    //variance equivalent is fixed for everyone
    THETA_decision[1] = THETA_params[2];
    

}


bool H::ac_dec_design(std::shared_ptr<PVProjectFlat> project_, WEE* w_)
{
    //get irr and reputation of an installer (mean from distribution)
    auto irr = (project_->irr_a * project_->sei->THETA_reputation[0]);
    
    //Logistic function (from cdf of the distribution)
    auto p_switch = (1/(1+std::exp(-(irr - THETA_decision[0])/THETA_decision[1])));
    
    return (w_->rand->ru() <= p_switch);
}