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
    get_inf_marketing_sei.push_back(mes_);
    
    ///@DevStage2 might be addd saving of the time of the marketing message, in this case it will be saved in the form of transformed marketing messages 
    
    
    ///@DevStage3 check if this agent is interested in the marketing message
}


void
Household::ac_inf_marketing_sei()
{
    //requests quotes from SEI
    while (!get_inf_marketing_sei.empty())
    {
        auto marketing_inf = get_inf_marketing_sei.front();
        
        //Distinguishes between online and phone quote. Small installers might not have an online presence.
        ///@DevStage2 think about moving difference to the virtual call. For now it is explicit, as it is assumed that agents themselves realize that it will be online vs offline quote
        switch (marketing_inf->sei_type)
        {
            case EParamTypes::SEISmall:
                //returns quote in the form of a message
                marketing_inf->agent->request_preliminary_quote(this);
                break;
            case EParamTypes::SEILarge:
                //returns quote in the form of a message
                marketing_inf->agent->request_online_quote(this);
                break;
    
            default:
                break;
        }
    
        get_inf_marketing_sei.pop_front();
    };
    
    
}


void
Household::receive_preliminary_quote(std::shared_ptr<MesMarketingSEIPreliminaryQuote> mes_)
{
    preliminary_quotes.push_back(mes_);
}

void
Household::receive_online_quote(std::shared_ptr<MesMarketingSEIPreliminaryQuote> mes_)
{
    preliminary_quotes.push_back(mes_);
}


std::shared_ptr<MesStateBaseHH>
Household::get_inf_online_quote(IAgent* agent_to)
{
    auto mes = std::make_shared<MesStateBaseHH>();
    
    ///@DevStage1 won't work, as some parameters need to be taken from House directly, have to distinguish them in some way.
    for (auto& param:WorldSettings::instance().params_to_copy_preliminary_quote)
    {
        mes->params[param] = params[param];
    };
    
    return mes;
}




void
Household::act_tick()
{
    ///@DevStage1 add request for information/preliminary quotes that is internally triggered - call void Household::ac_inf_marketing_sei()
    
    ///@DevStage1 add selection of the best quotes from preliminary
    
    
}




void
Household::update_params()
{
    //saves parameters
    params[EParamTypes::RoofSize] = house->roof_size;
}




