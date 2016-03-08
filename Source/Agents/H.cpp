//
//  H.cpp
//  ABMSolar
//
//  Created by Ekaterina Sinitskaya on 3/2/16.
//  Copyright (c) 2016 IRIS Lab. All rights reserved.
//

#include "Institutions/IMessage.h"
#include "Tools/WorldSettings.h"
#include "Agents/H.h"
#include "Agents/SEI.h"

using namespace solar_core;

void
Household::get_inf(std::shared_ptr<MesMarketingSEI> mes_)
{
    //saves information about advertising agent
    ///No mutex guards as only other operation is poping from the front, which does not invalidate anything
    get_inf_marketing_sei_agents.push_back(mes_->sei);
    
    ///@DevStage2 might be addd saving of the time of the marketing message, in this case it will be saved in the form of transformed marketing messages 
    
    
    ///@DevStage3 check if this agent is interested in the marketing message
}


void
Household::ac_inf_marketing_sei()
{
    //requests quotes from SEI
    while (!get_inf_marketing_sei_agents.empty())
    {
        auto agent = get_inf_marketing_sei_agents.front();
        
        
        ///@DevStage2 think about distinguishing between online and phone quote. Small installers might not have an online presence
        
        //returns quote in the form of a message
        auto mes = agent->get_online_quote(this);
        
        preliminary_quotes.push_back(mes);
        
        get_inf_marketing_sei_agents.pop_front();
    };
    
    
    
}


std::shared_ptr<MesStateBaseHH>
Household::get_inf_online_quote(IAgent* agent_to)
{
    auto mes = std::make_shared<MesStateBaseHH>();
    
    for (auto& param:WorldSettings::instance().params_to_copy_preliminary_quote)
    {
        mes->params[param] = params[param];
    };
    
    return mes;
}




void
Household::act_tick()
{
    ///@DevStage2 add request for information that is internally triggered
    
    
    
    
}