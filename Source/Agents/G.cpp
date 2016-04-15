//
//  G.cpp
//  ABMSolar
//
//  Created by Ekaterina Sinitskaya on 3/20/16.
//  Copyright (c) 2016 IRIS Lab. All rights reserved.
//

#include "UI/W.h"
#include "Tools/WorldSettings.h"
#include "Agents/G.h"
#include "Agents/SolarPanel.h"
#include "Agents/H.h"

using namespace solar_core;


void
G::request_permit(std::shared_ptr<PVProject> project_)
{
    project_->ac_g_time = a_time;
    
    
    pending_projects.push_back(project_);
    
    
    
}



void
G::collect_inf_site_visit(std::shared_ptr<PVProject> project_)
{
    
}


void
G::grant_permit(std::shared_ptr<PVProject> project_)
{
    project_->state_project = EParamTypes::GrantedPermit;
}


void
G::ac_update_tick()
{
    //update internal timer
    a_time = w->time;
    
    
    //clear last day schedule
    schedule_visits[i_schedule_visits].clear();
    
    //move schedule of visits by one
    //advance index
    if (i_schedule_visits == WorldSettings::instance().constraints[EConstraintParams::MaxLengthWaitPermitVisit] - 1)
    {
        i_schedule_visits = 0;
    }
    else
    {
        ++i_schedule_visits;
    };
    
    
    
}



void
G::act_tick()
{
    //update internals for the tick
    ac_update_tick();

    
    //go through projects, if permit has been requested and processing time has elapsed - schedule a visit
    //if preliminary quote is requested and have capacity for new project, and processing time for preliminary quotes has elapced - get back and schedule time
    for (auto& project:pending_projects)
    {
        if (project->state_project == EParamTypes::RequestedPermit)
        {
            //if permit was requested - check that processing time after request has elapsed and contact agent to schedule visit, check capacity for visits for each future time
            if ((a_time - project->ac_g_time) >= params[EParamTypes::GProcessingTimeRequiredForSchedulingPermitVisit])
            {
                bool FLAG_SCHEDULED_VISIT = false;
                std::size_t i_offset;
                std::size_t i;
                std::weak_ptr<PVProject> w_project = project;
                while (!FLAG_SCHEDULED_VISIT && i_offset < schedule_visits.size())
                {
                    //check that there is space for the visit
                    i = (i_schedule_visits + i_offset) % schedule_visits.size();
                    
                    if (schedule_visits[i].size() < params[EParamTypes::GMaxNVisitsPerTimeUnit])
                    {
                        auto agent_reply = project->agent->request_time_slot_visit(a_time + i_offset, w_project);
                        
                        if (agent_reply)
                        {
                            FLAG_SCHEDULED_VISIT = project->agent->schedule_visit(a_time + i_offset, w_project);
                            
                            if (FLAG_SCHEDULED_VISIT)
                            {
                                schedule_visits[i].push_back(w_project);
                                project->state_project = EParamTypes::ScheduledPermitVisit;
                                project->ac_g_time = a_time;
                            }
                        };
                        i_offset++;
                    };
                };
            };
        };
        if (project->state_project == EParamTypes::RequestedPermit)
        {
            //if permit was requested - check that processing time after request has elapsed and contact agent to schedule visit, check capacity for visits for each future time
            if ((a_time - project->ac_g_time) >= params[EParamTypes::GProcessingTimeRequiredForProcessingPermit])
            {
                grant_permit(project);
            };
        };
    };
    
    //visit sites and collect information
    for (auto& w_project:schedule_visits[i_schedule_visits])
    {
        //go to sites, collect information
        auto project = w_project.lock();
        if (project)
        {
            collect_inf_site_visit(project);
            project->state_project = EParamTypes::CollectedInfPermitVisit;
            project->ac_g_time = a_time;
        };
    };
    
}