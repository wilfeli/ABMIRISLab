//
//  H.cpp
//  ABMSolar
//
//  Created by Ekaterina Sinitskaya on 3/2/16.
//  Copyright (c) 2016 IRIS Lab. All rights reserved.
//

#include "Institutions/IMessage.h"
#include "Institutions/MarketingSystem.h"
#include "Tools/WorldSettings.h"
#include "Tools/Serialize.h"
#include "UI/W.h"
#include "Agents/SolarPanel.h"
#include "Agents/Homeowner.h"
#include "Agents/SEI.h"

using namespace solar_core;



std::set<EParamTypes> Homeowner::project_states_to_delete{EParamTypes::ClosedProject};

Homeowner::Homeowner(){}


Homeowner::Homeowner(const PropertyTree& pt_, W* w_)
{
    w = w_;
    
    
//    marketing_state = EnumFactory::ToEParamTypes(pt_.get<std::string>("marketing_state"));
    
    //location
    location_x = pt_.get<long>("location_x");
    location_y = pt_.get<long>("location_y");
    
    //House
    house = new House(pt_.get_child("House"));
    
    quote_state = EnumFactory::ToEParamTypes(pt_.get<std::string>("quote_state"));
    
    schedule_visits = std::vector<std::vector<std::weak_ptr<PVProject>>>(WorldSettings::instance().constraints[EConstraintParams::MaxLengthWaitPreliminaryQuote], std::vector<std::weak_ptr<PVProject>>{});
    i_schedule_visits = 0;
    
    quote_stage_timer = 0;
    n_preliminary_quotes = 0;
    n_pending_designs = 0;
    n_preliminary_quotes_requested = 0;
    
    project_states_to_delete.insert(EParamTypes::ClosedProject);

    
}


void
Homeowner::init(W* w_)
{
    a_time = w_->time;
    
    
    if (marketing_state == EParamTypes::HOMarketingStateHighlyInterested)
    {
        //add itself to the list of agents that request initial information
        w->get_inf_marketing_sei_agents.push_back(this);
    };
    
}



void
Homeowner::get_project(std::shared_ptr<PVProject> project_)
{
    pvprojects_lock.lock();
    //save project
    pvprojects_to_add.push_back(project_);
    pvprojects_lock.unlock();
}




void
Homeowner::get_inf(std::shared_ptr<MesMarketingSEI> mes_)
{

    //saves information about advertising agent only if not already commited to installing project
    ///No mutex guards as only other operation is popping from the front, which does not invalidate anything
    
    if (marketing_state != EParamTypes::HOMarketingStateCommitedToInstallation)
    {
        get_inf_marketing_sei.push_back(mes_);
        
        if (marketing_state != EParamTypes::HOMarketingStateInterested)
        {
            marketing_state = EParamTypes::HOMarketingStateInterested;
            //tell world that is now interested, that it is moved to the list of active agents. Once the project is finished it will be moved from the list of active agents
            w->get_state_inf(this, marketing_state);
        };
    };
    
    ///@DevStage2 might be add saving of the time of the marketing message, in this case it will be saved in the form of transformed marketing messages because original message will time stamped at the moment of creation (almost at the beginning of the simulation)
    
    
    ///@DevStage3 check if this agent is interested in the marketing message
}

/**
 
 Not used now, in case if decide that some agents are actively requesting information
 */
void
Homeowner::ac_inf_marketing_sei()
{
    w->marketing->request_inf_marketing_sei(this);
}




void
Homeowner::ac_inf_quoting_sei()
{
    //requests quotes from SEI
    //will narrow down pool after gathering some information
    while (!get_inf_marketing_sei.empty())
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
        
        //uses online quote to gather general information to just narrow down the possible pool of projects
        //will use non-compensatory rules here
        switch (marketing_state)
        {
            case EParamTypes::HOMarketingStateHighlyInterested:
            case EParamTypes::HOMarketingStateInterested:
                //request online quotes, assume that it is general information to narrow down pool of installers
                //requests online quote, it will be provided in a separate call
                new_project->state_project = EParamTypes::RequestedOnlineQuote;
                //assumes that there is some interest on part of an agent by the looks of it,  but in reality is used just for asynchronous calls to SEI
                //
                marketing_inf->agent->request_online_quote(new_project);
                break;
            default:
                break;
        };
        
        get_inf_marketing_sei.pop_front();
    };
    
    
    //tick stage timer
    ++quote_stage_timer;
    
}




void
Homeowner::dec_evaluate_online_quotes()
{
    //here narrowing down of the pool by non-compensatory rules will be implemented
    
    //first by information - already activated if saw neighbor install PV system.
    //second by customer rating
    //there will be groups of non-compensatory rules, each Homeowner belongs to one of the groups
    
    
    auto pool = std::vector<bool>(pvprojects.size(), false);
    
    for (auto i = 0; i < pvprojects.size(); ++i)
    {
        //check if is in the pool
        //check on Customer rating
        if (pvprojects[i]->sei->params[EParamTypes::SEIRating] >= THETA_NCDecisions[EParamTypes::SEIRating][0])
        {
            pool[i] = true;
        };
    };
    
    
    for (auto i = 0; i < pool.size(); ++i)
    {
        //pool is now narrowed down to the subset of all open projects, request further information from them (without site visit for this step)
        if (pool[i])
        {
            auto project = pvprojects[i];
            project->state_project = EParamTypes::RequestedPreliminaryQuote;
            project->sei->request_preliminary_quote(project);
            
            //update number of requested quotes
            ++n_preliminary_quotes_requested;
        }
        else
        {
            pvprojects[i]->state_project = EParamTypes::ClosedProject;
        };
        
    };
}



double
Homeowner::estimate_sei_utility_from_params(std::shared_ptr<PVProject> project, DecisionParams& THETA)
{
    double utility = 0.0;
    
    //here installers are evaluated
    //preliminary quote will have general savings estimation
    //collect all parameters for decisions
    utility += THETA[EParamTypes::SEIRating][0] * project->sei->params[EParamTypes::SEIRating];
    
    //discrete variables have position in the vector of coefficients as their value. Fragile.
    utility += THETA[EParamTypes::SEIInteractionType][project->sei->params[EParamTypes::SEIInteractionType]];
    
    
    //number inside is type of equipment - directly corresponds to the position in the vector
    utility += THETA[EParamTypes::SEIEquipmentType][project->sei->params[EParamTypes::SEIEquipmentType]];
    
    //estimated project total time
    //LeadIn time if fixed for each installer and is an estimation
    //Permitting time depends on the location, but is an estimate
    utility += THETA[EParamTypes::HOSEIDecisionTotalProjectTime][0] * project->preliminary_quote->params[EParamTypes::PreliminaryQuoteTotalProjectTime];
    
    //warranty
    //MARK: CAREFULL Warranty is in month in SEI definition and in years in conjoint
    utility += THETA[EParamTypes::SEIWarranty][project->preliminary_quote->params[EParamTypes::SEIWarranty]];
    
    
    //savings are estimated for an average homeowner
    utility += THETA[EParamTypes::HOSEIDecisionEstimatedNetSavings][0] * project->preliminary_quote->params[EParamTypes::PreliminaryQuoteEstimatedNetSavings];
    
    return utility;
    
}



double
Homeowner::estimate_sei_utility(std::shared_ptr<PVProject> project)
{
    return estimate_sei_utility_from_params(project, THETA_SEIDecisions);
    
    
}



void Homeowner::dec_evaluate_preliminary_quotes()
{
    double utility_none = THETA_SEIDecisions[EParamTypes::HOSEIDecisionUtilityNone][0];
    double error = 0.0;
    
    for (auto& project:pvprojects)
    {
        if (project->state_project == EParamTypes::ProvidedPreliminaryQuote)
        {
            //Generate random noise. Generation is here because every choice will regenerate them
            error = w->rand_ho->ru();
            project->preliminary_quote->params[EParamTypes::HOSEIDecisionEstimatedUtility] = estimate_sei_utility(project) + error;
        };
    
    
    };
    
    
    
    //sort projects by utility, higher utility goes first
    std::sort(pvprojects.begin(), pvprojects.end(), [](const std::shared_ptr<PVProject> lhs, const std::shared_ptr<PVProject> rhs){
        //compare only if online quote was received,
        bool compare_res = false;
        if (lhs->state_project == EParamTypes::ProvidedPreliminaryQuote && rhs->state_project == EParamTypes::ProvidedPreliminaryQuote)
        {
            compare_res = lhs->preliminary_quote->params[EParamTypes::HOSEIDecisionEstimatedUtility] < rhs->preliminary_quote->params[EParamTypes::HOSEIDecisionEstimatedUtility];
        };
        return compare_res;
    });
    
    
    std::shared_ptr<PVProject> decision = nullptr;
    //pick top project and request further information
    for(auto& project:pvprojects)
    {
        if (project->state_project == EParamTypes::ProvidedPreliminaryQuote)
        {
            if (project->preliminary_quote->params[EParamTypes::HOSEIDecisionEstimatedUtility] >= utility_none)
            {
                decision = project;
                break;
            };
        }
    };
    
    //if choose someone
    if (decision)
    {
        decision->state_project = EParamTypes::AcceptedPreliminaryQuote;
        decision->sei->accepted_preliminary_quote(decision);
    };
    
    
    //close other projects
    for(auto& project:pvprojects)
    {
        if (project->state_project == EParamTypes::ProvidedPreliminaryQuote)
        {
            if (project != decision)
            {
                project->state_project = EParamTypes::ClosedProject;
            };
        };
        
    };
    
    
    //exit evaluation stage
    n_preliminary_quotes = 0;
    
    
}


double Homeowner::estimate_design_utility_from_params(std::shared_ptr<PVProject> project, DecisionParams& THETA)
{
    double utility = 0.0;
    
    //efficiency
    utility += THETA[EParamTypes::HODesignDecisionPanelEfficiency][0] * project->design->design->PV_module->efficiency;
    
    //visibility
    utility += THETA[EParamTypes::HODesignDecisionPanelVisibility][project->design->design->PV_module->visibility];
    
    //inverter type
    utility += THETA[EParamTypes::HODesignDecisionInverterType][static_cast<int64_t>(project->design->design->inverter->technology)];
    
    //number of failures
    utility += THETA[EParamTypes::HODesignDecisionFailures][0] * project->design->design->failure_rate;
    
    //emmision levels
    utility += THETA[EParamTypes::HODesignDecisionCO2][0] * project->design->design->co2_equivalent;
    
    //savings are estimated for an average homeowner
    utility += THETA[EParamTypes::HODesignDecisionEstimatedNetSavings][0] * project->design->design->total_net_savings;
    
    return utility;
    
    
}





double Homeowner::estimate_design_utility(std::shared_ptr<PVProject> project)
{
    return estimate_design_utility_from_params(project, THETA_DesignDecisions);
    
    
}



/**
 
 
 @DevStage4 may go back and forth over the design

 
*/
void Homeowner::dec_evaluate_designs()
{
    double error = 0.0;
    double utility_none = THETA_DesignDecisions[EParamTypes::HODesignDecisionUtilityNone][0];
    
    
    for (auto& project:pvprojects)
    {

        if (project->state_project == EParamTypes::DraftedDesign)
        {
            error = w->rand_ho->ru();
            project->design->params[EParamTypes::HODesignDecisionEstimatedUtility] = estimate_sei_utility(project) + error;
        };
    };
    
    
    
    //sort projects by utility, higher utility goes first
    std::sort(pvprojects.begin(), pvprojects.end(), [](const std::shared_ptr<PVProject> lhs, const std::shared_ptr<PVProject> rhs){
        //compare only if online quote was received,
        bool compare_res = false;
        if (lhs->state_project == EParamTypes::DraftedDesign && rhs->state_project == EParamTypes::DraftedDesign)
        {
            compare_res = lhs->design->params[EParamTypes::HODesignDecisionEstimatedUtility] < rhs->design->params[EParamTypes::HODesignDecisionEstimatedUtility];
        };
        return compare_res;
    });
    
    
    std::shared_ptr<PVProject> decision = nullptr;
    //pick top project and request further information
    for(auto& project:pvprojects)
    {
        if (project->state_project == EParamTypes::DraftedDesign)
        {
            if (project->design->params[EParamTypes::HODesignDecisionEstimatedUtility] >= utility_none)
            {
                decision = project;
                break;
            };
        }
    };
    
    //if choose someone
    if (decision)
    {
        decision->state_project = EParamTypes::AcceptedDesign;
        decision->ac_hh_time = a_time;
        decision->ac_accepted_time = a_time;
        decision->sei->accepted_design(decision);
        
        accepted_design.push_back(decision);
        
        //stop accepting marketing from SEI
        marketing_state = EParamTypes::HOMarketingStateCommitedToInstallation;
        
        //tell world that stopped accepting offers
        w->get_state_inf(this, marketing_state);

    };
    
    
    //close other projects
    for(auto& project:pvprojects)
    {
        if (project->state_project == EParamTypes::DraftedDesign)
        {
            if (project != decision)
            {
                project->state_project = EParamTypes::ClosedProject;
            };
        };
        
    };
    
    
    
}






void
Homeowner::receive_design(std::shared_ptr<PVProject> project_)
{
    ++n_pending_designs;
}


void
Homeowner::receive_preliminary_quote(std::shared_ptr<PVProject> project_)
{
    ++n_preliminary_quotes;
}

void
Homeowner::receive_online_quote(std::shared_ptr<PVProject> project_)
{
    
}


std::shared_ptr<MesStateBaseHO>
Homeowner::get_inf_online_quote(IAgent* agent_to)
{
    auto mes = std::make_shared<MesStateBaseHO>();
    
    ///Some parameters need to be taken from House directly, they are pushed to the general container when changed
    for (auto& param:WorldSettings::instance().params_to_copy_preliminary_quote)
    {
        mes->params[param] = params[param];
    };
    
    return mes;
}



bool
Homeowner::request_time_slot_visit(TimeUnit visit_time, std::weak_ptr<PVProject> project)
{
    std::size_t i = (i_schedule_visits + (visit_time - a_time)) % schedule_visits.size();
    return schedule_visits[i].size() < params[EParamTypes::HOMaxNVisitsPerTimeUnit];
}

bool
Homeowner::schedule_visit(TimeUnit visit_time, std::weak_ptr<PVProject> project)
{
    schedule_visits_lock.lock();
    std::size_t i = (i_schedule_visits + (visit_time - a_time)) % schedule_visits.size();
    bool FLAG_SCHEDULED_VISIT = false;
    
    if (schedule_visits[i].size() < params[EParamTypes::HOMaxNVisitsPerTimeUnit])
    {
        schedule_visits[i].push_back(project);
        FLAG_SCHEDULED_VISIT = true;
    };
    schedule_visits_lock.unlock();
    return FLAG_SCHEDULED_VISIT;
}



bool
Homeowner::dec_project_reroof(std::shared_ptr<PVProject> project)
{
    //MARK: cont.
    return false;
}



void
Homeowner::ac_update_tick()
{
    //update internal timer
    a_time = w->time;
    
    
    pvprojects_lock.lock();
    //move pending projects into active projects
    pvprojects.insert(pvprojects.end(), pvprojects_to_add.begin(), pvprojects_to_add.end());
    pvprojects_to_add.clear();

    
    
    //delete closed projects
    pvprojects.erase(std::remove_if(pvprojects.begin(), pvprojects.end(),
                                    [&](std::shared_ptr<PVProject> x) -> bool { return (project_states_to_delete.find(x->state_project) != project_states_to_delete.end()); }), pvprojects.end());
    pvprojects_lock.unlock();

    
    
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
Homeowner::act_tick()
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
        //moves to the evaluation stage
        dec_evaluate_online_quotes();
    };
    
    
    //evaluates preliminary quotes and commits to the project
    //once all requested quotes are in - evaluate offerings
    if (n_preliminary_quotes >= n_preliminary_quotes_requested)
    {
        dec_evaluate_preliminary_quotes();
    };
    
    
    
    //evaluates designs
    if (n_pending_designs >= WorldSettings::instance().constraints[EConstraintParams::MinNReceivedDesings])
    {
        dec_evaluate_designs();
    };
    
    
    for (auto& project:accepted_design)
    {
        //@DevStage2 need to double check to see if sending payments before system is installed
        if (auto payment = project->financing->schedule_payments[a_time - project->ac_accepted_time] > 0)
        {
            //make payment
            //assume that have enough money
            auto mes = std::make_shared<MesPayment>(payment);
            project->sei->get_payment(mes);
        };
    };
    
    //@DevStage2 if there is an accepted project - check if needs to make payments

    
    ///@DevStage2 generally actions in a tick depend on the state of an agent, either it is choosing installer or waiting for the project to finish. Might have a call back to w that will indicate that this agent has changed state. In this case w will have multiple lists of agents in different states and would call appropriate function. Or might do it internally where new state will dictate behavior in the tick. Generally have both - agent is broadcasting changed state and behaves differently depending on the state.
}




void
Homeowner::update_params()
{
    //saves parameters
    params[EParamTypes::RoofSize] = house->roof_size;
}




