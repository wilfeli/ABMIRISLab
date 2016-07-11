//
//  SEIBL.cpp
//  ABMSolar
//
//  Created by Ekaterina Sinitskaya on 7/9/16.
//  Copyright (c) 2016 IRIS Lab. All rights reserved.
//

#include "Agents/SEIBL.h"
#include "Agents/H.h"

using namespace solar_core;

/**
 
 Create design for H and its parameters
 
 */
std::shared_ptr<PVDesign>
SEIBL::form_design_for_params(H* agent_)
{
    
    //size is equal to the utility bill, assume simple electricity pricing for now
    auto estimated_demand  = agent_->params[EParamTypes::ElectricityBill] / WorldSettings::instance().params_exog[EParamTypes::ElectricityPriceUCDemand]/constants::NUMBER_DAYS_IN_MONTH;
    
    //price per watt is predetermined by the optimality choice
    auto p = params[EParamTypes::EstimatedPricePerWatt] *  estimated_demand;
    
    
    //there is only 1 solar panel + inverter combinaion to use in offering solar panels
    //calculate ROI
    [EparamTypes::SEIMidEfficiencyDesign]
    
    
    auto roof_age = [&coef = roof_age_coef](double house_size)->double {return coef*house_size;};
    
    
    
    auto NPV_purchase = [& =  ](double r)->double {
        
        //panel output
        
        
        //production each time unit
        
        
        
        
    };
    
    
    
    //find irr
    //secant method
    
    
    
    
    project.irr = irr;
    
    
    
    
}





void
SEI::dec_base()
{
    //makes decision about ...
    
    
    
    
    //
    
    
}



void
SEI::act_tick()
{
    
    
    //make price decision, based on the switching or not
    dec_base();
    
    
    //MARK: cont. 

    
    
}



