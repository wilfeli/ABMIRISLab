//
//  MarketingSystem.cpp
//  ABMSolar
//
//  Created by Ekaterina Sinitskaya on 2/29/16.
//  Copyright (c) 2016 IRIS Lab. All rights reserved.
//

#include "UI/W.h"
#include "Institutions/MarketingSystem.h"
#include "Institutions/IMessage.h"
#include "Tools/WorldSettings.h"
#include "Agents/H.h"


using namespace solar_core;

MarketingInst::MarketingInst(W* w_)
{
    w = w_;
}


void
MarketingInst::init(W* w_)
{
}


void
MarketingInst::request_inf_marketing_sei(IAgent* agent_)
{
    //push all marketing messages
    for (auto& mes:marketing_mess)
    {
        agent_->get_inf(mes);
    };
}


void
MarketingInst::get_marketing_inf_sei(std::shared_ptr<MesMarketingSEI> mes_)
{
    marketing_mess.push_back(mes_);
}




void
MarketingInst::act_tick()
{

    
    //push marketing information to the selection of agents that expressed interest in receiving marketing information
    for (auto& agent:interested_agents)
    {
        //push all marketing messages
        for (auto& mes:marketing_mess)
        {
            agent->get_inf(mes);
        };
        
    };
    
    
    
    //randomly select agents and push marketing information
    auto pdf_agents = boost::uniform_int<uint64_t>(0, w->hhs.size()-1);
    auto rng_agents = boost::variate_generator<boost::mt19937&, boost::uniform_int<uint64_t>>(w->rand->rng, pdf_agents);
    
    std::size_t j = 0;
    for (auto i = 0; i < WorldSettings::instance().params_exog[EParamTypes::MarketingMaxNToDrawPerTimeUnit]; ++i)
    {
        j = rng_agents();
        //push all marketing messages
        for (auto& mes:marketing_mess)
        {
            w->hhs[j]->get_inf(mes);
        };

    };
    
    
    
    
}

