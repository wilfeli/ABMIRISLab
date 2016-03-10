//
//  SEI.cpp
//  ABMSolar
//
//  Created by Ekaterina Sinitskaya on 3/7/16.
//  Copyright (c) 2016 IRIS Lab. All rights reserved.
//

#include "Tools/IParameters.h"
#include "UI/W.h"
#include "Agents/SEI.h"
#include "Agents/H.h"
#include "Institutions/IMessage.h"
#include "Agents/SolarPanel.h"


using namespace solar_core;



void
SEI::request_online_quote(Household* agent_in)
{
    //create new project
    auto new_project = std::make_shared<PVProject>();
    //request additional information
    new_project->state_base_agent = agent_in->get_inf_online_quote(this);
    new_project->agent = agent_in;
    new_project->state_project = EParamTypes::RequestedOnlineQuote;
    new_project->begin_time = a_time;
    //save project
    pvprojects.push_back(new_project);
}


std::shared_ptr<MesMarketingSEIPreliminaryQuote>
SEI::form_online_quote(Household* agent_in)
{
    //from params get stuff such as average price per watt, price of a standard unit
    auto mes = std::make_shared<MesMarketingSEIPreliminaryQuote>();
    
    double p = params[EParamTypes::EstimatedPricePerWatt] * params[EParamTypes::AveragePVPrice];
    
    mes->params[EParamTypes::PreliminaryQuote] = p;
    
    
    //MARK: cont.
    mes->params[EParamTypes::PreliminaryQuoteEstimatedSavings] = 0.0;
    
    return mes;
}


void
SEI::act_tick()
{
    //update internal timer
    a_time = w->time;
    
    
    //go through projects, if online quote was requested - provide it
    for (auto& project:pvprojects)
    {
        ///for online quotes no check for the time elapced since the request for the quote was received, as it is assumed to be instanteneous process
        if (project->state_project == EParamTypes::RequestedOnlineQuote)
        {
            auto mes = form_online_quote(project->agent);
            project->agent->receive_online_quote(mes);
            project->state_project = EParamTypes::ProvidedOnlineQuote;
        };
    };
    
    
    
    //MARK: cont. preliminary quote - mom and pop shop will be separate class, derived from this, so this implementation is for the big SEI, which all have online quotes
    
    
    //MARK: cont. continue if interest in the project was indicated
    
    
    
}