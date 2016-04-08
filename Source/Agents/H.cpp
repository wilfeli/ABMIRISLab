//
//  H.cpp
//  ABMSolar
//
//  Created by Ekaterina Sinitskaya on 3/2/16.
//  Copyright (c) 2016 IRIS Lab. All rights reserved.
//

#include "Institutions/IMessage.h"
#include "Tools/WorldSettings.h"
#include "UI/W.h"
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
    
    if (marketing_state != EParamTypes::HHMarketingStateInterested)
    {
        marketing_state = EParamTypes::HHMarketingStateInterested;
        //tell world that is now interested, that it is moved to the list of active agents. Once the project is finished it will be moved from the list of active agents
        w->get_state_inf(this, marketing_state);
    };
    
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
Household::dec_evaluate_preliminary_quotes()
{
    //sort projects by price, lower price goes first
    std::sort(pvprojects.begin(), pvprojects.end(), [](const std::shared_ptr<PVProject> lhs, const std::shared_ptr<PVProject> rhs){
        //compare only if online quote was received,
        bool compare_res = false;
        if (lhs->state_project == EParamTypes::ProvidedPreliminaryQuote && rhs->state_project == EParamTypes::ProvidedPreliminaryQuote)
        {
            compare_res = lhs->online_quote->params[EParamTypes::PreliminaryQuoteEstimatedSavings] > rhs->online_quote->params[EParamTypes::PreliminaryQuoteEstimatedSavings];
        };
        return compare_res;
    });
    
    
    std::shared_ptr<PVProject> decision = nullptr;
    //pick top project and compare price to the income
    for(auto& project:pvprojects)
    {
        if (project->state_project == EParamTypes::ProvidedPreliminaryQuote)
        {
            if (project->preliminary_quote->params[EParamTypes::PreliminaryQuotePrice] <= params[EParamTypes::Income] * THETA_design[EParamTypes::HHDecPreliminaryQuote][0])
            {
                decision = project;
                break;
            };
        }
    };
    
    if (decision)
    {
        decision->state_project == EParamTypes::AcceptedPreliminaryQuote;
        decision->sei->accepted_preliminary_quote(decision);
    };
    
}

/**
 
 
 @DevStage2 may go back and forth over the design
 
 */
void
Household::dec_evaluate_designs()
{
    //assume that best design in terms of savings is accepted?
    std::sort(pvprojects.begin(), pvprojects.end(), [&](std::shared_ptr<PVProject> &lhs, std::shared_ptr<PVProject> &rhs)
    {
        return (lhs->design && rhs->design)? lhs->design->design->total_savings > rhs->design->design->total_savings: (lhs->design)? true: false;
    });
    
    auto decision = pvprojects[0];
    
    decision->state_project = EParamTypes::AcceptedDesign;
    decision->ac_hh_time = a_time;
    decision->ac_accepted_time = a_time;
    decision->sei->accepted_design(this);
    
    accepted_design.push_back(decision);
    
}


void
Household::receive_design(std::shared_ptr<PVProject> project_)
{
    n_pending_designs++;
}


void
Household::receive_preliminary_quote(std::shared_ptr<PVProject> project_)
{
    n_preliminary_quotes++;
}

void
Household::receive_online_quote(std::shared_ptr<PVProject> project_)
{
    
}


std::shared_ptr<MesStateBaseHH>
Household::get_inf_online_quote(IAgent* agent_to)
{
    auto mes = std::make_shared<MesStateBaseHH>();
    
    ///Some parameters need to be taken from House directly, they are pushed to the general container when changed
    for (auto& param:WorldSettings::instance().params_to_copy_preliminary_quote)
    {
        mes->params[param] = params[param];
    };
    
    return mes;
}



bool
Household::request_time_slot_visit(TimeUnit visit_time, std::weak_ptr<PVProject> project)
{
    std::size_t i = (i_schedule_visits + (visit_time - a_time)) % schedule_visits.size();
    return schedule_visits[i].size() < params[EParamTypes::HHMaxNVisitsPerTimeUnit];
}

bool
Household::schedule_visit(TimeUnit visit_time, std::weak_ptr<PVProject> project)
{
    schedule_visits_lock.lock();
    std::size_t i = (i_schedule_visits + (visit_time - a_time)) % schedule_visits.size();
    bool FLAG_SCHEDULED_VISIT = false;
    
    if (schedule_visits[i].size() < params[EParamTypes::HHMaxNVisitsPerTimeUnit])
    {
        schedule_visits[i].push_back(project);
        FLAG_SCHEDULED_VISIT = true;
    };
    schedule_visits_lock.unlock();
    return FLAG_SCHEDULED_VISIT;
}



bool
Household::dec_project_reroof(std::shared_ptr<PVProject> project)
{
    //MARK: cont.
    
};



void
Household::ac_update_tick()
{
    //update internal timer
    a_time = w->time;
    
    
    //clear last day schedule
    schedule_visits[i_schedule_visits].clear();
    
    //move schedule of visits by one
    //advance index
    if (i_schedule_visits == WorldSettings::instance().constraints[EConstraintParams::MaxLengthWaitPreliminaryQuote] - 1)
    {
        i_schedule_visits = 0;
    }
    else
    {
        ++i_schedule_visits;
    };
    
    
    
}


void
Household::act_tick()
{
    //update internals for the tick
    ac_update_tick();
    
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
    
    
    //evaluate preliminary quotes and commit to the project
    if (n_preliminary_quotes >= WorldSettings::instance().constraints[EConstraintParams::MinNReceivedPreliminaryQuotes])
    {
        dec_evaluate_preliminary_quotes();
    };
    
    //evaluate preliminary quotes and commit to the project
    if (n_pending_designs >= WorldSettings::instance().constraints[EConstraintParams::MinNReceivedDesings])
    {
        dec_evaluate_designs();
    };
    
    
    for (auto& project:accepted_design)
    {
        if (auto payment = project->financing->schedule_payments[a_time - project->ac_accepted_time] > 0)
        {
            //make payment
            //assume that have enough money
            auto MesPayment = std::make_shared<MesPayment>(MesPayment(payment));
            project->sei->get_payment(MesPayment);
        };
    };
    
    //if there is accepted project - check if needs to make payments
    
    ///@DevStage1 generally actions in a tick depend on the state of an agent, either it is choosing installer or waiting for the project to finish. Might have a call back to w that will indicate that this agent has changed state. In this case w will have multiple lists of agents in different states and would call appropriate function. Or might do it internally where new state will dictate behavior in the tick. Generally have both - agent is broadcasting changed state and behaves differently depending on the state.
}




void
Household::update_params()
{
    //saves parameters
    params[EParamTypes::RoofSize] = house->roof_size;
}




