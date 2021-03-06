//
//  G.cpp
//  ABMSolar
//
//  Created by Ekaterina Sinitskaya on 3/20/16.
//  Copyright (c) 2016 IRIS Lab. All rights reserved.
//

#include "UI/W.h"
#include "Tools/WorldSettings.h"
#include "Tools/Serialize.h"
#include "Agents/G.h"
#include "Agents/SolarPanel.h"
#include "Agents/Homeowner.h"

using namespace solar_core;



G::G(const PropertyTree& pt_, W* w_)
{
    w = w_;
    
    //create empty container
    schedule_visits = std::vector<std::vector<std::weak_ptr<PVProject>>>(WorldSettings::instance().constraints[EConstraintParams::MaxLengthWaitPreliminaryQuote], std::vector<std::weak_ptr<PVProject>>{});
    i_schedule_visits = 0;
    
    
    //read parameters
    std::map<std::string, std::string> params_str;
    serialize::deserialize(pt_.get_child("params"), params_str);
    
    ///@DevStage2 move to W to speed up, but test before that
    for (auto& iter:params_str)
    {
        params[EnumFactory::ToEParamTypes(iter.first)] = serialize::solve_str_formula<double>(iter.second, *w->rand_g);
    };
    
    
    //form set with "closed" states
    project_states_to_delete.insert(EParamTypes::GrantedPermitForInstallation);
    project_states_to_delete.insert(EParamTypes::GrantedPermitForInterconnection);
    project_states_to_delete.insert(EParamTypes::ClosedProject);
    project_states_to_delete.insert(EParamTypes::Installed);
    project_states_to_delete.insert(EParamTypes::PassedInspectionAfterInstallation);
    project_states_to_delete.insert(EParamTypes::RequestedPermitForInterconnection);
    project_states_to_delete.insert(EParamTypes::ScheduledInstallation);
    project_states_to_delete.insert(EParamTypes::ScheduleInstallation);
    
    
    
}


void
G::init(W* w_)
{
    
    a_time = w_->time;
    
}



void
G::request_permit_for_installation(std::shared_ptr<PVProject> project_)
{
    project_->ac_g_time = a_time;
    pending_pvprojects_lock.lock();
    pending_pvprojects_to_add.push_back(project_);
    pending_pvprojects_lock.unlock();
}



void
G::collect_inf_site_visit(std::shared_ptr<PVProject> project_)
{
    
}

void
G::request_inspection(std::shared_ptr<PVProject> project_)
{
    project_->ac_g_time = a_time;
    pending_pvprojects_lock.lock();
    pending_pvprojects_to_add.push_back(project_);
    pending_pvprojects_lock.unlock();

    //MARK: cont. check if some areas do not require visit after installation
    
}


void
G::approve_permit(std::shared_ptr<PVProject> project_)
{
    project_->state_project = EParamTypes::PassedInspectionAfterInstallation;
}


void
G::ac_update_tick()
{
    //update internal timer
    a_time = w->time;
    
    
    pending_pvprojects_lock.lock();
    //pove pending projects into active projects
    pending_pvprojects.insert(pending_pvprojects.end(), pending_pvprojects_to_add.begin(), pending_pvprojects_to_add.end());
    pending_pvprojects_to_add.clear();
    
    //remove all projects that are granted permit
    
    
    pending_pvprojects.erase(std::remove_if(pending_pvprojects.begin(), pending_pvprojects.end(),
                                            [&](std::shared_ptr<PVProject> x) -> bool { return (project_states_to_delete.find(x->state_project) != project_states_to_delete.end()); }), pending_pvprojects.end());
    
    
    pending_pvprojects_lock.unlock();
    
    
    
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
    for (auto& project:pending_pvprojects)
    {
        if (project->state_project == EParamTypes::RequestedPermitForInstallation)
        {
            double project_permitting_time = params[EParamTypes::GProcessingTimeRequiredForGrantingPermitForInstallation] * w->world_map->g_map[project->agent->location_y][project->agent->location_x]->permit_difficulty;
            if ((a_time - project->ac_g_time) >= project_permitting_time)
            {
                //grant permit
                project->state_project = EParamTypes::GrantedPermitForInstallation;
                project->ac_g_time = a_time;
            };
            
        };
        
        
        
        if (project->state_project == EParamTypes::RequestedInspectionAfterInstallation)
        {
            //if permit was requested - check that processing time after request has elapsed and contact agent to schedule visit, check capacity for visits for each future time
            auto project_scheduling_time = params[EParamTypes::GProcessingTimeRequiredForSchedulingPermitVisit] * w->world_map->g_map[project->agent->location_y][project->agent->location_x]->permit_difficulty;
            if ((a_time - project->ac_g_time) >= project_scheduling_time)
            {
                bool FLAG_SCHEDULED_VISIT = false;
                std::size_t i_offset = 1;
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
                    };
                    i_offset++;
                };
            };
        };
        
        if (project->state_project == EParamTypes::CollectedInfPermitVisit)
        {
            //if permit was requested - check that processing time after request has elapsed
            auto project_processing_time = params[EParamTypes::GProcessingTimeRequiredForProcessingPermit] * w->world_map->g_map[project->agent->location_y][project->agent->location_x]->permit_difficulty;
            if ((a_time - project->ac_g_time) >= project_processing_time)
            {
                approve_permit(project);
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