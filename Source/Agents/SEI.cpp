//
//  SEI.cpp
//  ABMSolar
//
//  Created by Ekaterina Sinitskaya on 3/7/16.
//  Copyright (c) 2016 IRIS Lab. All rights reserved.
//

#include "Tools/IParameters.h"
#include "Agents/SEI.h"
#include "Agents/H.h"
#include "Institutions/IMessage.h"


using namespace solar_core;



std::shared_ptr<MesMarketingSEIPreliminaryQuote>
SEI::get_online_quote(Household* agent_in)
{
    
    //request additional information
    auto inf_in = agent_in->get_inf_online_quote(this);
    
    //from params get stuff such as average price per watt, price of a standard unit
    auto mes = std::make_shared<MesMarketingSEIPreliminaryQuote>();
    
    double p = params[EParamTypes::EstimatedPricePerWatt] * params[EParamTypes::AveragePVPrice];
    
    mes->params[EParamTypes::PreliminaryQuote] = p;
    
    
    
   
    
    return mes;
}