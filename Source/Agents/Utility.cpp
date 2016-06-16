//
//  Utility.cpp
//  ABMSolar
//
//  Created by Ekaterina Sinitskaya on 5/10/16.
//  Copyright (c) 2016 IRIS Lab. All rights reserved.
//

#include "Tools/Serialize.h"
#include "Agents/Utility.h"
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
                    auto rng_p = boost::variate_generator<boost::mt19937&, boost::uniform_01<>>(w->rand->rng, pdf_p);
                    
                    //grant permit with some probability
                    if (rng_p() < p)
                    {
                        //grant permit
                        project->state_project = EParamTypes::GrantedPermitForInterconnection;
                        
                        //MARK: cont. update current capacity, decrease by interconnected amount 
                    };
                    
                };
                
                project->ac_utility_time = a_time;
            };
            
        };

    };
    
    
}

