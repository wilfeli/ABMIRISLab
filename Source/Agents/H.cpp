//
//  H.cpp
//  ABMSolar
//
//  Created by Ekaterina Sinitskaya on 3/2/16.
//  Copyright (c) 2016 IRIS Lab. All rights reserved.
//

#include "Agents/H.h"


using namespace solar_core;

void
Household::get_inf(std::shared_ptr<MesMarketing> mes)
{
    ///@DevStage3 check if is interested in the marketing message
}


void
Household::ac_inf_marketing_sei()
{
    //requests quotes from SEI
    while (!get_inf_marketing_sei_agents.empty())
    {
        auto agent = get_inf_marketing_sei_agents.front();
        
        //returns quote in the form of a message
        auto mes = agent->get_quote(this);
        
        quotes.push_back(mes);
        
        get_inf_marketing_sei_agents.pop_front();
    };
    
    
    
}







void
Household::act_tick()
{
    ///@DevStage2 add request for information that is internally triggered
    
    
    
    
}