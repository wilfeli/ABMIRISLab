//
//  SEI.cpp
//  ABMSolar
//
//  Created by Ekaterina Sinitskaya on 3/7/16.
//  Copyright (c) 2016 IRIS Lab. All rights reserved.
//

#include "Tools/IParameters.h"
#include "Tools/WorldSettings.h"
#include "UI/W.h"
#include "Agents/SEI.h"
#include "Agents/H.h"
#include "Institutions/IMessage.h"
#include "Agents/SolarPanel.h"


using namespace solar_core;




void
SEI::get_project(std::shared_ptr<PVProject> project_)
{
    //save project
    pvprojects.push_back(project_);
}

void
SEI::request_online_quote(std::shared_ptr<PVProject> project_)
{
    //request additional information
    project_->state_base_agent = project_->agent->get_inf_online_quote(this);
}


void
SEI::request_preliminary_quote(std::shared_ptr<PVProject> project_)
{
    //update time of a last action
    project_->ac_sei_time = a_time;
}


std::shared_ptr<MesMarketingSEIOnlineQuote>
SEI::form_online_quote(std::shared_ptr<PVProject> project_)
{
    //from params get stuff such as average price per watt, price of a standard unit
    auto mes = std::make_shared<MesMarketingSEIOnlineQuote>();
    
    ///@Kelley actually form function
    ///@wp for now it is price per watt
    double p;
    auto estimated_demand  = project_->state_base_agent->params[EParamTypes::ElectricityBill] / WorldSettings::instance().params_exog[EParamTypes::ElectricityPriceUCDemand];
    if (estimated_demand <= params[EParamTypes::AveragePVCapacity])
    {
        //if standard panel gives enough capacity - install it as a unit
        p = params[EParamTypes::EstimatedPricePerWatt] * params[EParamTypes::AveragePVCapacity];
    }
    else
    {
        //if it is not enough use industry price per watt and estimated electricity demand from the utility bill
        p = params[EParamTypes::EstimatedPricePerWatt] * estimated_demand;
    };
    
    mes->params[EParamTypes::OnlineQuotePrice] = p;
    
    //MARK: cont. for now estimated savings are put at zero, but need to feel in actual estimation of savings. Might take it from actual interview
    mes->params[EParamTypes::OnlineQuoteEstimatedSavings] = 0.0;
    
    return mes;
}




void
SEI::ac_update_tick()
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
SEI::act_tick()
{
    //update internals for the tick
    ac_update_tick();
    
    
    
    
    //go through projects, if online quote was requested - provide it
    for (auto& project:pvprojects)
    {
        ///for online quotes no check for the time elapced since the request for the quote was received, as it is assumed to be instanteneous process
        if (project->state_project == EParamTypes::RequestedOnlineQuote)
        {
            auto mes = form_online_quote(project);
            project->online_quote = mes;
            project->agent->receive_online_quote(project);
            project->state_project = EParamTypes::ProvidedOnlineQuote;
            project->ac_sei_time = a_time;
        };
        
        
        //if preliminary quote is requested and have capacity for new project, and processing time for preliminary quotes has elapced - get back and schedule time
        if (project->state_project == EParamTypes::RequestedPreliminaryQuote)
        {
            if ((a_time - project->ac_sei_time) >= params[EParamTypes::ProcessingTimeRequesForPreliminaryQuote])
            {
                bool FLAG_SCHEDULED_VISIT = false;
                std::size_t i_offset;
                std::size_t i;
                while (!FLAG_SCHEDULED_VISIT && i_offset < schedule_visits.size())
                {
                    //check that there is space for the visit
                    i = (i_schedule_visits + i_offset) % schedule_visits.size();
                    
                    if (schedule_visits[i].size() < params[EParamTypes::SEIMaxNVisitsPerTimeUnit])
                    {
                        auto agent_reply = project->agent->request_slot_visit(a_time + i_offset);
                        
                        if (agent_reply)
                        {
                            project->agent->schedule_visit(a_time + i_offset);
                        };
                        
                        // MARK: cont. add guards for HH as multiple SEI could try and schedule visits at the same time if decide to parallelize SEI cycle 
                    };
                }
            };
        };
        
        
    };
    
    
    //if preliminary quote was requested - check that processing time after request has elapsed and contact agent to schedule visit, check capacity for visits for each future time
    
    
    
    
    //MARK: cont. preliminary quote - mom and pop shop will be separate class, derived from this, so this implementation is for the big SEI, which all have online quotes
    
    
    //MARK: cont. continue if interest in the project was indicated
    
    
    
}