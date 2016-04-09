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


void
SEI::accepted_preliminary_quote(std::shared_ptr<PVProject> project_)
{
    project_->ac_sei_time = a_time;
}


void
SEI::accepted_design(std::shared_ptr<PVProject> project_)
{
    
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
SEI::collect_inf_site_visit(std::shared_ptr<PVProject> project_)
{
    ///@Kelley fill in details
    //add information to the parameters of an agent
    project_->state_base_agent->params[EParamTypes::RoofAge] = project_->agent->house->roof_age;
    
    
    //if age > age threshold - ask if is willing to reroof
    if (project_->state_base_agent->params[EParamTypes::RoofAge] > params[EParamTypes::SEIMaxRoofAge])
    {
        
        auto dec = project_->agent->dec_project_reroof(project_);
        
        project_->state_base_agent->params[EParamTypes::HHDecisionReroof] = dec;
        project_->state_project = EParamTypes::RequiredHHReroof;
        
    };
    
}



std::shared_ptr<MesMarketingSEIPreliminaryQuote>
SEI::form_preliminary_quote(std::shared_ptr<PVProject> project_)
{
    //from params get stuff such as average price per watt, price of a standard unit
    auto mes = std::make_shared<MesMarketingSEIPreliminaryQuote>();
    
    
    //MARK: cont. create preliminary quote estimate from real data
    ///@Kelley fill in details
    //by default provide preliminary quote, but in some cases refuse to proceed with the project and close it
    project_->state_project = EParamTypes::ProvidedPreliminaryQuote;
    
    
    //if roof is old and refuses to reroof - close project
    if (project_->state_project == EParamTypes::RequiredHHReroof)
    {
        //if agrees to reroof - transfer into waiting state
        //otherwise mark as closed
        if (project_->state_base_agent->params[EParamTypes::HHDecisionReroof])
        {
            project_->state_project = EParamTypes::WaitingHHReroof;
        }
        else
        {
            project_->state_project = EParamTypes::ClosedProject;
        };
    };
    

    
    
    return mes;
}

/**
 
 @wp accroding to the CSI data set there is 50/50 split on owning and leasing SP (see Host Customer Sector and System Owner Sector fields)
 
 
 
 Example for California San Jose and REC260PE:
 Average daily use 30kWh per day. 
 For California assume 6 hours of solar. (Look at the map http://understandsolar.com/calculating-kilowatt-hours-solar-panels-produce/) It is kWh/m2/day. 
 
 
 PTC refers to PVUSA Test Conditions, which were developed to test and compare PV systems as part of the PVUSA (Photovoltaics for Utility Scale Applications) project. PTC are 1,000 Watts per square meter solar irradiance, 20 degrees C air temperature, and wind speed of 1 meter per second at 10 meters above ground level.
 
 Logic: Peak Efficiency (15.76%) = STC Power Rating / Total panel area (=Length * Width)/1000W
 
 1 panel produces 260Wh if receives 1000Wh, so for 6kWh one panel produces 6 * 0.260 = 6 * 0.1576 * 1.65 = 1.56kWh per day of DC
 
 for AC = 1.56 * 0.8 = 1.248 (as DC -> AC conversion at 80%)
 
 for 30kWh per day need 24 panels if use STC Power Rating
 
 
 
 
 
*/
std::shared_ptr<MesDesign>
SEI::form_design(std::shared_ptr<PVProject> project_)
{
    
    //depending on the required size
    
    //do in multiples of three
    //check design for full electricity bill, for 80% of it and for 30% of it
    std::vector<PVDesign> designs;
    //daily electricity consumption
    auto demand = project_->state_base_agent->params[EParamTypes::ElectricityBill];
    //amount of solar radiation in Wh/m2/day
    auto solar_radiation = w->get_solar_radiation(project_->agent->location_x, project_->agent->location_y);
    //distribution of permit length in different locations
    auto permit_difficulty = w->get_permit_difficulty(project_->agent->location_x, project_->agent->location_y);
    
    //EParamTypes::ElectricityBill has daily electricity bill
    for (auto project_percentage:dec_project_percentages)
    {
        //for each preferred panel - with high, mid and low efficiency, calculate number of panels to meet the demand
        for (auto iter: dec_solar_modules)
        {
            //create design
            auto design = PVDesign();
            
            design.solar_radiation = solar_radiation;
            
            design.permit_difficulty = permit_difficulty;
            
            design.N_PANELS = std::ceil(demand * project_percentage / ((solar_radiation/1000) * iter.second->efficiency * (iter.second->length * iter.second->width/1000000) * ( 1 - WorldSettings::instance().params_exog[EParamTypes::DCtoACLoss])));
            
            design.PV_module = iter.second;
            
            design.DC_size = design.N_PANELS * iter.second->STC_power_rating;
            
            design.AC_size = design.DC_size * WorldSettings::instance().params_exog[EParamTypes::DCtoACLoss];
            
            ///@DevStage1 add inverter technology here
            
            
            
            design.hard_costs = design.N_PANELS * iter.second->efficiency * THETA_hard_costs[0] + std::pow(design.DC_size, 2) * THETA_hard_costs[1];
            
            design.soft_costs = design.N_PANELS * THETA_soft_costs[0] + permit_difficulty * THETA_soft_costs[1];
            
            design.total_costs = (design.hard_costs + design.soft_costs) * THETA_profit[0];
            
            
            ac_estimate_savings(design, project_);
            
            
            designs.push_back(design);
            
        };
    };
    
    
    //sort by savings and present best option
    ///@DevStage2 bootstrap here for different savings dynamics depending on parameters
    
    std::sort(designs.begin(), designs.end(), [&](PVDesign &lhs, PVDesign &rhs){
        return lhs.total_savings > rhs.total_savings;
    });
    auto design = std::make_shared<PVDesign>(designs[0]);
    auto mes = std::make_shared<MesDesign>(design);
    
    return mes;
}

/**
 
 
 
 
 
*/
void
SEI::ac_estimate_savings(PVDesign& design, std::shared_ptr<PVProject> project_)
{
    //estimate savings for each project
    //get price of kW from the utility, assume increase due to inflation
    //total life-time of a project
    
    
    
    //calculate PPA
    
    
    //calculate lease

    //
    auto inflation = WorldSettings::instance().params_exog[EParamTypes::InflationRate];
    auto CPI = 1;
    auto energy_costs = 0.0;
    for (auto i = 0; i < design.PV_module->warranty_length/52; ++i)
    {
        //estimate yearly energy production, if PPA might be used in estimation
//        auto yearly_production = design->AC_size * design.solar_radiation/1000 * 365.25;
        energy_costs += project_->state_base_agent->params[EParamTypes::ElectricityBill] * CPI * WorldSettings::instance().params_exog[EParamTypes::ElectricityPriceUCDemand];
        
        CPI = CPI * inflation;
    };
    
    design.total_savings = design.total_costs - energy_costs;
    
    
    
}



void
SEI::form_financing(std::shared_ptr<PVProject> project_)
{
    project_->financing = std::make_shared<MesFinance>();
    
    //assume that whole sum is due on the beginning of the project
    project_->financing->schedule_payments = {project_->design->design->total_costs};
    
    
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

    //clear last day schedule
    schedule_installations[i_schedule_installations].clear();
    
    //move schedule of visits by one
    //advance index
    if (i_schedule_installations == WorldSettings::instance().constraints[EConstraintParams::MaxLengthPlanInstallations] - 1)
    {
        i_schedule_installations = 0;
    }
    else
    {
        ++i_schedule_installations;
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
            //if preliminary quote was requested - check that processing time after request has elapsed and contact agent to schedule visit, check capacity for visits for each future time
            if ((a_time - project->ac_sei_time) >= params[EParamTypes::ProcessingTimeRequiredForSchedulingFirstSiteVisit])
            {
                bool FLAG_SCHEDULED_VISIT = false;
                std::size_t i_offset;
                std::size_t i;
                std::weak_ptr<PVProject> w_project = project;
                while (!FLAG_SCHEDULED_VISIT && i_offset < schedule_visits.size())
                {
                    //check that there is space for the visit
                    i = (i_schedule_visits + i_offset) % schedule_visits.size();
                    
                    if (schedule_visits[i].size() < params[EParamTypes::SEIMaxNVisitsPerTimeUnit])
                    {
                        auto agent_reply = project->agent->request_time_slot_visit(a_time + i_offset, w_project);
                        
                        if (agent_reply)
                        {
                            FLAG_SCHEDULED_VISIT = project->agent->schedule_visit(a_time + i_offset, w_project);
                            
                            if (FLAG_SCHEDULED_VISIT)
                            {
                                schedule_visits[i].push_back(w_project);
                                project->state_project = EParamTypes::ScheduledFirstSiteVisit;
                                project->ac_sei_time = a_time;
                            }
                        };
                        ++i_offset;
                    };
                };
            };
        };
        
        
        //if visited site and processing time for preliminary quote has elapsed -  form preliminary quote
        if (project->state_project == EParamTypes::CollectedInfFirstSiteVisit)
        {
            //if information after first site visit is collected
            if ((a_time - project->ac_sei_time) >= params[EParamTypes::ProcessingTimeRequiredForPreliminaryQuote])
            {
                auto mes = form_preliminary_quote(project);
                project->preliminary_quote = mes;
                project->agent->receive_preliminary_quote(project);
                project->ac_sei_time = a_time;
            };
        };
        
        
        
        if (project->state_project == EParamTypes::AcceptedPreliminaryQuote)
        {
            //if enough time has elapsed
            if ((a_time - project->ac_sei_time) >= params[EParamTypes::ProcessingTimeRequiredForDesign])
            {
                auto mes = form_design(project);
                project->design = mes;
                form_financing(project);
                project->agent->receive_design(project);
                project->ac_sei_time = a_time;
            };
            
        };
        
        //after design is formed - get permit for the installation
        if (project->state_project == EParamTypes::AcceptedDesign)
        {
            g->request_permit(project);
            project->state_project = EParamTypes::RequestedPermit;
        };
        
        
        if (project->state_project == EParamTypes::GrantedPermit)
        {
            //schedule installation
            bool FLAG_SCHEDULED_VISIT = false;
            std::size_t i_offset;
            std::size_t i;
            std::weak_ptr<PVProject> w_project = project;
            while (!FLAG_SCHEDULED_VISIT && i_offset < schedule_installations.size())
            {
                //check that there is space for the visit
                i = (i_schedule_installations + i_offset) % schedule_installations.size();
                
                if (schedule_installations[i].size() < params[EParamTypes::SEIMaxNInstallationsPerTimeUnit])
                {
                    auto agent_reply = project->agent->request_time_slot_visit(a_time + i_offset, w_project);
                    
                    if (agent_reply)
                    {
                        FLAG_SCHEDULED_VISIT = project->agent->schedule_visit(a_time + i_offset, w_project);
                        
                        if (FLAG_SCHEDULED_VISIT)
                        {
                            schedule_installations[i].push_back(w_project);
                            project->state_project = EParamTypes::ScheduledInstallation;
                            project->ac_sei_time = a_time;
                        }
                    };
                    ++i_offset;
                };
            };
        };
        
        //includes signing contract and starting payments if nesessary, as payment schedule will be part of the project
        
    };
    
    //visit sites and collect information
    for (auto& w_project:schedule_visits[i_schedule_visits])
    {
        //go to sites, collect information
        auto project = w_project.lock();
        //check if project is still active
        if (project)
        {
            collect_inf_site_visit(project);
            project->state_project = EParamTypes::CollectedInfFirstSiteVisit;
            project->ac_sei_time = a_time;
        };
    };
    
    
    
    //visit sites and perform installation
    //visit sites and collect information
    for (auto& w_project:schedule_installations[i_schedule_installations])
    {
        //go to sites, collect information
        auto project = w_project.lock();
        //check if project is still active
        if (project && project->financing->state_payments == EParamTypes::PaymentsOnTime)
        {
            install_project(project);
            project->state_project = EParamTypes::Installed;
            project->ac_sei_time = a_time;
        };
    };

    
    

    //MARK: cont. collect information from SEM and update design examples
    if ((a_time - ac_designs) > params[EParamTypes::SEIFrequencyUpdateDesignTemplates])
    {
        //collect information about current offerings
        //sort by efficiency
        
        
        
    };
    
    
    
    //MARK: cont. preliminary quote - mom and pop shop will be separate class, derived from this, so this implementation is for the big SEI, which all have online quotes
    
    
    
    
    
}