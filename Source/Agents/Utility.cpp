//
//  Utility.cpp
//  ABMSolar
//
//  Created by Ekaterina Sinitskaya on 5/10/16.
//  Copyright (c) 2016 IRIS Lab. All rights reserved.
//

#include "Tools/Serialize.h"
#include "Agents/Utility.h"
#include "Agents/SEI.h"
#include "Agents/SolarPanel.h"
#include "Institutions/IMessage.h"
#include "UI/W.h"


using namespace solar_core;


Utility::Utility(const PropertyTree& pt_, W* w_)
{
    w = w_;
    
    //read parameters
    std::map<std::string, std::string> params_str;
    serialize::deserialize(pt_.get_child("params"), params_str);
    
    ///@DevStage2 move to W to speed up, but test before that
    for (auto& iter:params_str)
    {
        params[EnumFactory::ToEParamTypes(iter.first)] = serialize::solve_str_formula<double>(iter.second, *w->rand);
    };

    
    std::vector<std::string> THETA_dec_str;
    serialize::deserialize(pt_.get_child("THETA_dec"), THETA_dec_str);
    for (auto& iter:THETA_dec_str)
    {
        THETA_dec.push_back(serialize::solve_str_formula<double>(iter, *w->rand));
    }
    

    
    
    //form set with "closed" states
    project_states_to_delete.insert(EParamTypes::GrantedPermitForInterconnection);
    project_states_to_delete.insert(EParamTypes::ClosedProject);

    
    
    
}

void
Utility::init(W* w_)
{
    a_time = w->time;
}



void
Utility::request_permit_for_interconnection(std::shared_ptr<PVProject> project_)
{
    project_->ac_utility_time = a_time;
    pending_pvprojects_lock.lock();
    pending_pvprojects_to_add.push_back(project_);
    pending_pvprojects_lock.unlock();
}


void
Utility::ac_update_tick()
{
    
    a_time = w->time;
    
    
    pending_pvprojects_lock.lock();
    //pove pending projects into active projects
    pending_pvprojects.insert(pending_pvprojects.end(), pending_pvprojects_to_add.begin(), pending_pvprojects_to_add.end());
    pending_pvprojects_to_add.clear();
    
    //remove all projects that are granted permit
    
    
    pending_pvprojects.erase(std::remove_if(pending_pvprojects.begin(), pending_pvprojects.end(),
                                            [&](std::shared_ptr<PVProject> x) -> bool { return (project_states_to_delete.find(x->state_project) != project_states_to_delete.end()); }), pending_pvprojects.end());
    
    
    pending_pvprojects_lock.unlock();
    
    
    
}



void
Utility::act_tick()
{
    ac_update_tick();
    
    for (auto project:pending_pvprojects)
    {
        if (project->state_project == EParamTypes::RequestedPermitForInterconnection)
        {
            //if enough time has elapsed
            //if do not receive permit - could try again
            if ((a_time - project->ac_utility_time) >= params[EParamTypes::UtilityProcessingTimeRequiredForPermit])
            {
                
                auto free_capacity = params[EParamTypes::UtilityMaxCapacity] - params[EParamTypes::UtilityCurrentCapacity];
                
                //check how much free capacity have
                if (project->design->design->AC_size < free_capacity)
                {
                    
                    auto free_capacity_after_permit = free_capacity - project->design->design->AC_size;
                    auto p = THETA_dec[0] * std::exp(THETA_dec[0]/free_capacity_after_permit);
                    
                    //check that it is valid pdf
                    auto pdf_p = boost::uniform_01<>();
                    auto rng_p = boost::variate_generator<boost::mt19937&, boost::uniform_01<>>(w->rand_utility->rng, pdf_p);
                    
                    //grant permit with some probability
                    if (rng_p() < p)
                    {
                        //grant permit
                        project->state_project = EParamTypes::GrantedPermitForInterconnection;
                        
                        w->get_state_inf_interconnected_project(project);
                        
//#ifdef DEBUG
//                        //check if sei has this project
//                        bool FLAG_HAS_PROJECT = false;
//                        
//                        for (auto& project_test:project->sei->pvprojects)
//                        {
//                            if (project_test == project)
//                            {
//                                FLAG_HAS_PROJECT = true;
//                            };
//                        };
//                        
//                        if (!FLAG_HAS_PROJECT)
//                        {
//                            throw std::runtime_error("mismatching projects");
//                        };
//                        
//#endif
                        
                        
                        //MARK: cont. update current capacity, decrease by interconnected amount 
                    };
                    
                };
                
                project->ac_utility_time = a_time;
            };
            
        };

    };
    
    
}

