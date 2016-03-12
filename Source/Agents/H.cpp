//
//  H.cpp
//  ABMSolar
//
//  Created by Ekaterina Sinitskaya on 3/2/16.
//  Copyright (c) 2016 IRIS Lab. All rights reserved.
//

#include "Institutions/IMessage.h"
#include "Tools/WorldSettings.h"
#include "Agents/SolarPanel.h"
#include "Agents/H.h"
#include "Agents/SEI.h"

using namespace solar_core;

void
Household::get_inf(std::shared_ptr<MesMarketingSEI> mes_)
{
    //saves information about advertising agent
    ///No mutex guards as only other operation is poping from the front, which does not invalidate anything
    get_inf_marketing_sei.push_back(mes_);
    
    ///@DevStage2 might be addd saving of the time of the marketing message, in this case it will be saved in the form of transformed marketing messages because original message will time stamped at the moment of creation (almost at the beginning of the simulation)
    
    
    ///@DevStage3 check if this agent is interested in the marketing message
}


void
Household::ac_inf_quoting_sei()
{
    //requests quotes from SEI
    //restricts number of projects
    while ((!get_inf_marketing_sei.empty()) && (pvprojects.size() <= WorldSettings::instance().constraints[EConstraintParams::MaxNOpenProjectsHH]))
    {
        auto marketing_inf = get_inf_marketing_sei.front();
        
        //create project
        //create new project
        auto new_project = std::make_shared<PVProject>();
        //request additional information
        new_project->agent = this;
        new_project->begin_time = a_time;
        new_project->sei = marketing_inf->agent;
        //save project
        pvprojects.push_back(new_project);
        marketing_inf->agent->get_project(new_project);
        
        //Distinguishes between online and phone quote. Small installers might not have an online presence.
        ///@DevStage2 think about moving difference to the virtual call. For now it is explicit, as it is assumed that agents themselves realize that it will be online vs offline quote
        switch (marketing_state)
        {
            case EParamTypes::HHMarketingStateHighlyInterested:
                //requests preliminary quote with site visit
                new_project->state_project = EParamTypes::RequestedPreliminaryQuote;
                marketing_inf->agent->request_preliminary_quote(new_project);
            case EParamTypes::HHMarketingStateInterested:
                switch (marketing_inf->sei_type)
                {
                case EParamTypes::SEISmall:
                    //returns quote in the form of a message
                    // assume no online presence, so requests preliminary quote
                    new_project->state_project = EParamTypes::RequestedPreliminaryQuote;
                    marketing_inf->agent->request_preliminary_quote(new_project);
                    break;
                case EParamTypes::SEILarge:
                    //requests online quote, it will be provided in a separate call
                    new_project->state_project = EParamTypes::RequestedOnlineQuote;
                    marketing_inf->agent->request_online_quote(new_project);
                    break;
                    
                default:
                    break;
                }

            default:
                //otherwise do nothing as is not interested
                break;
        };
        
        get_inf_marketing_sei.pop_front();
    };
    
    
    ++quote_stage_timer;
    
}

void
Household::dec_evaluate_online_quotes()
{
    //sort projects by price, lower price goes first
    std::sort(pvprojects.begin(), pvprojects.end(), [](const std::shared_ptr<PVProject> lhs, const std::shared_ptr<PVProject> rhs){
        //compare only if online quote was received,
        bool compare_res = false;
        if (lhs->state_project == EParamTypes::ProvidedOnlineQuote && rhs->state_project == EParamTypes::ProvidedOnlineQuote)
        {
            compare_res = lhs->online_quote->params[EParamTypes::OnlineQuotePrice] < rhs->online_quote->params[EParamTypes::OnlineQuotePrice];
        };
        return compare_res;
    });
    
    
    //mark first N projects for requesting preliminary quotes and request them
    auto n_request_quotes = 0;
    auto max_n_request_quotes = WorldSettings::instance().constraints[EConstraintParams::MaxNRequestedPreliminaryFromOnlineQuotes];
    while (n_request_quotes < max_n_request_quotes)
    {
        auto project = pvprojects[n_request_quotes];
        project->state_project = EParamTypes::RequestedPreliminaryQuote;
        project->sei->request_preliminary_quote(project);
        n_request_quotes++;
    };
    
    
}


void
Household::receive_preliminary_quote(std::shared_ptr<PVProject> project_)
{
    
}

void
Household::receive_online_quote(std::shared_ptr<PVProject> project_)
{
    
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
    if (quote_stage_timer < WorldSettings::instance().constraints[EConstraintParams::MaxNTicksToCollectQuotes])
    {
        //initiates and continues collection of quoting information
        ac_inf_quoting_sei();
    }
    else
    {
        //move to evaluation stage
        dec_evaluate_online_quotes();
    };
    
    
    ///@DevStage1 add selection of the best quotes from preliminary
    
    ///@DevStage1 generally actions in a tick depend on the state of an agent, either it is choosing installer or waiting for the project to finish. Might have a call back to w that will indicate that this agent has changed state. In this case w will have multiple lists of agents in different states and would call appropriate function. Or might do it internally where new state will dictate behavior in the tick. Generally have both - agent is broadcasting changed state and behaves differently depending on the state.
}




void
Household::update_params()
{
    //saves parameters
    params[EParamTypes::RoofSize] = house->roof_size;
}




