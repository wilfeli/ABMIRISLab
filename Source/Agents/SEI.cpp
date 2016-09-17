//
//  SEI.cpp
//  ABMSolar
//
//  Created by Ekaterina Sinitskaya on 3/7/16.
//  Copyright (c) 2016 IRIS Lab. All rights reserved.
//

#include "Tools/Serialize.h"
#include "Tools/IParameters.h"
#include "Tools/WorldSettings.h"
#include "UI/W.h"
#include "Agents/SEM.h"
#include "Agents/SEI.h"
#include "Agents/Utility.h"
#include "Agents/Homeowner.h"
#include "Institutions/IMessage.h"
#include "Institutions/MarketingSystem.h"
#include "Agents/SolarPanel.h"
#include "Agents/G.h"

using namespace solar_core;

std::set<EParamTypes> SEI::project_states_to_delete{EParamTypes::ClosedProject};

SEI::SEI(const PropertyTree& pt_, W* w_)
{
    w = w_;
    
    //location
    location_x = pt_.get<long>("location_x");
    location_y = pt_.get<long>("location_y");

    
    
    //read solar modules to be used in decisions
    std::map<std::string, std::string> dec_solar_modules_str;
    serialize::deserialize(pt_.get_child("dec_solar_modules"), dec_solar_modules_str);
    
    for (auto& iter:dec_solar_modules_str)
    {
        dec_solar_modules[EnumFactory::ToEParamTypes(iter.first)] = WorldSettings::instance().solar_modules[iter.second];
    };
    
    serialize::deserialize(pt_.get_child("dec_project_percentages"),dec_project_percentages);
    
    std::vector<std::string> THETA_hard_costs_str;
    serialize::deserialize(pt_.get_child("THETA_hard_costs"), THETA_hard_costs_str);
    
    for (auto& iter:THETA_hard_costs_str)
    {
        THETA_hard_costs.push_back(serialize::solve_str_formula<double>(iter, *w->rand));
    }
    
    std::vector<std::string> THETA_soft_costs_str;
    serialize::deserialize(pt_.get_child("THETA_soft_costs"), THETA_soft_costs_str);
    for (auto& iter:THETA_soft_costs_str)
    {
        THETA_soft_costs.push_back(serialize::solve_str_formula<double>(iter, *w->rand));
    }
    
    std::vector<std::string> THETA_profit_str;
    serialize::deserialize(pt_.get_child("THETA_profit"), THETA_profit_str);
    for (auto& iter:THETA_profit_str)
    {
        THETA_profit.push_back(serialize::solve_str_formula<double>(iter, *w->rand));
    }
    
    
    
    schedule_visits = std::vector<std::vector<std::weak_ptr<PVProject>>>(WorldSettings::instance().constraints[EConstraintParams::MaxLengthWaitPreliminaryQuote], std::vector<std::weak_ptr<PVProject>>{});
    i_schedule_visits = 0;
    
    
    schedule_installations = std::vector<std::vector<std::weak_ptr<PVProject>>>(WorldSettings::instance().constraints[EConstraintParams::MaxLengthPlanInstallations], std::vector<std::weak_ptr<PVProject>>{});
    i_schedule_installations = 0;
    
    
    
    //set other parameters
    ac_designs = 0;
    
    
    sei_type = EnumFactory::ToEParamTypes(pt_.get<std::string>("sei_type"));
    money = serialize::solve_str_formula<double>(pt_.get<std::string>("money"), *w->rand);
    a_time = 0;
    
    
    //read parameters
    std::map<std::string, std::string> params_str;
    serialize::deserialize(pt_.get_child("params"), params_str);
    
    ///@DevStage2 move to W to speed up, but test before that
    for (auto& iter:params_str)
    {
        params[EnumFactory::ToEParamTypes(iter.first)] = serialize::solve_str_formula<double>(iter.second, *w->rand);
    };

    mes_marketing = std::make_shared<MesMarketingSEI>(this, sei_type);
    
    

}


void
SEI::init(W *w_)
{
    a_time = w_->time;
    
    //send marketing information out
    w->marketing->get_marketing_inf_sei(mes_marketing);
    
}



void
SEI::get_project(std::shared_ptr<PVProject> project_)
{
    project_->state_materials = EParamTypes::None;
    pvprojects_lock.lock();
    //save project
    pvprojects_to_add.push_back(project_);
    pvprojects_lock.unlock();
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
    //if SEISmall - request information
    if ((sei_type == EParamTypes::SEISmall) && (!project_->state_base_agent))
    {
        //request additional information
        project_->state_base_agent = project_->agent->get_inf_online_quote(this);
    };
    
    
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
    

    ///@wp for now it is price per watt
    double p;
    /// in kWh
    //ElectricityBill in dollars, ElectricityPriceUCDemand in dollars per kWh
    auto estimated_demand  = project_->state_base_agent->params[EParamTypes::ElectricityBill] / WorldSettings::instance().params_exog[EParamTypes::ElectricityPriceUCDemand]/constants::NUMBER_DAYS_IN_MONTH;
    if (estimated_demand <= params[EParamTypes::AveragePVCapacity])
    {
        //if standard panel gives enough capacity - install it as a unit
        p = params[EParamTypes::EstimatedPricePerWatt] * params[EParamTypes::AveragePVCapacity] * constants::NUMBER_WATTS_IN_KILOWATT;
    }
    else
    {
        //if it is not enough use industry price per watt and estimated electricity demand from the utility bill
        p = params[EParamTypes::EstimatedPricePerWatt] * estimated_demand * constants::NUMBER_WATTS_IN_KILOWATT;
    };
    
    mes->params[EParamTypes::OnlineQuotePrice] = p;
    
    //MARK: cont. for now estimated savings are put at zero, but need to feel in actual estimation of savings. Might take it from actual interview
    mes->params[EParamTypes::OnlineQuoteEstimatedSavings] = 0.0;
    
    return mes;
}



void
SEI::collect_inf_site_visit(std::shared_ptr<PVProject> project_)
{
    //add information to the parameters of an agent
    project_->state_base_agent->params[EParamTypes::RoofAge] = project_->agent->house->roof_age;
    
    
    //if age > age threshold - ask if is willing to reroof
    if (project_->state_base_agent->params[EParamTypes::RoofAge] > params[EParamTypes::SEIMaxRoofAge])
    {
        
        auto dec = project_->agent->dec_project_reroof(project_);
        
        project_->state_base_agent->params[EParamTypes::HODecisionReroof] = dec;
        project_->state_project = EParamTypes::RequiredHOReroof;
        
    };
    
}



std::shared_ptr<MesMarketingSEIPreliminaryQuote>
SEI::form_preliminary_quote(std::shared_ptr<PVProject> project_)
{
    //from params get stuff such as average price per watt, price of a standard unit
    auto mes = std::make_shared<MesMarketingSEIPreliminaryQuote>();
    
    
    //MARK: cont. create preliminary quote estimate from real data
    //by default provide prelminary quote, but in some cases refuse to proceed with the project and close it
    //is here because otherwise later assigning will override this
    project_->state_project = EParamTypes::ProvidedPreliminaryQuote;
    
    //if roof is old and refuses to reroof - close project
    if (project_->state_project == EParamTypes::RequiredHOReroof)
    {
        //if agrees to reroof - transfer into waiting state
        //otherwise mark as closed
        if (project_->state_base_agent->params[EParamTypes::HODecisionReroof])
        {
            project_->state_project = EParamTypes::WaitingHOReroof;
        }
        else
        {
            project_->state_project = EParamTypes::ClosedProject;
        };
    };
    
    //forms design by default
    //mid percentage to be used for estimating size
    auto demand = project_->state_base_agent->params[EParamTypes::ElectricityBill]/WorldSettings::instance().params_exog[EParamTypes::ElectricityPriceUCDemand]/constants::NUMBER_DAYS_IN_MONTH;
    //amount of solar irradiation in Wh/m2/day
    auto solar_irradiation = w->get_solar_irradiation(project_->agent->location_x, project_->agent->location_y);
    //distribution of permit length in different locations
    auto permit_difficulty = w->get_permit_difficulty(project_->agent->location_x, project_->agent->location_y);
    
    
    auto design = PVDesign();
    
    form_design_for_params(project_, demand, solar_irradiation, permit_difficulty, dec_project_percentages[1], *(++dec_solar_modules.begin()), design);
    
    ac_estimate_savings(design, project_);


    mes->params[EParamTypes::PreliminaryQuotePrice] = design.total_costs;
    mes->params[EParamTypes::PreliminaryQuoteEstimatedSavings] = design.total_savings;
    
    return mes;
}










/**
 
 @wp accroding to the CSI data set there is 50/50 split on owning and leasing SP (see Host Customer Sector and System Owner Sector fields)
 
 
 
 Example for California San Jose and REC260PE:
 Average daily use 30kWh per day. 
 For California assume 6 hours of solar. (Look at the map http://understandsolar.com/calculating-kilowatt-hours-solar-panels-produce/ ) It is kWh/m2/day. 
 (http://i1.wp.com/upload.wikimedia.org/wikipedia/commons/2/2c/Us_pv_annual_may2004.jpg?resize=1501%2C1164 )
 
 
 PTC refers to PVUSA Test Conditions, which were developed to test and compare PV systems as part of the PVUSA (Photovoltaics for Utility Scale Applications) project. PTC are 1,000 Watts per square meter solar irradiance, 20 degrees C air temperature, and wind speed of 1 meter per second at 10 meters above ground level.
 
 Logic: Peak Efficiency (15.76%) = STC Power Rating / Total panel area (=Length * Width)/1000W
 
 1 panel produces 260Wh if receives 1000Wh, so for 6kWh one panel produces 6 * 0.260 = 6 * 0.1576 * 1.65 = 1.56kWh per day of DC
 
 for AC = 1.56 * 0.8 = 1.248 (as DC -> AC conversion at 80%)
 
 for 30kWh per day need 24 panels if use STC Power Rating
 
 
 @DevStage1 need estimation on the distribution of roof sizes and usable roof sizes
 
 
*/
std::shared_ptr<MesDesign>
SEI::form_design(std::shared_ptr<PVProject> project_)
{
    
    //depending on the required size
    
    //do in multiples of three
    //check design for full electricity bill, for 80% of it and for 30% of it
    std::vector<PVDesign> designs;
    //daily electricity consumption from bill in kWh
    auto demand = project_->state_base_agent->params[EParamTypes::ElectricityBill]/WorldSettings::instance().params_exog[EParamTypes::ElectricityPriceUCDemand]/constants::NUMBER_DAYS_IN_MONTH;
    //amount of solar irradiation in kWh/m2/day
    auto solar_irradiation = w->get_solar_irradiation(project_->agent->location_x, project_->agent->location_y);
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
            
            form_design_for_params(project_, demand, solar_irradiation, permit_difficulty, project_percentage, iter, design);
            
            ac_estimate_savings(design, project_);
            
            //MARK: cont. add discard for negative profits
            
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


void
SEI::form_design_for_params(std::shared_ptr<PVProject> project_, double demand, double solar_irradiation, double permit_difficulty, double project_percentage, const IterTypeDecSM& iter, PVDesign& design)
{
    design.solar_irradiation = solar_irradiation;
    
    design.permit_difficulty = permit_difficulty;
    
    design.N_PANELS = std::ceil(demand * project_percentage / ((solar_irradiation) * iter.second->efficiency * (iter.second->length * iter.second->width/1000000) * ( 1 - WorldSettings::instance().params_exog[EParamTypes::DCtoACLoss])));
    
    design.PV_module = iter.second;
    
    design.DC_size = design.N_PANELS * iter.second->STC_power_rating / constants::NUMBER_WATTS_IN_KILOWATT;
    
    design.AC_size = design.DC_size * (1 - WorldSettings::instance().params_exog[EParamTypes::DCtoACLoss]);
    
    ///@DevStage1 add inverter technology here
    
    
    
    design.hard_costs = design.N_PANELS * iter.second->efficiency * THETA_hard_costs[0] + std::pow(design.DC_size, 2) * THETA_hard_costs[1];
    
    design.soft_costs = design.N_PANELS * THETA_soft_costs[0] + permit_difficulty * THETA_soft_costs[1];
    
    design.total_costs = (design.hard_costs + design.soft_costs) * THETA_profit[0];

}


/**
 
 
 
 
 
*/
void
SEI::ac_estimate_savings(PVDesign& design, std::shared_ptr<PVProject> project_)
{
    //estimate savings for each project
    //get price of kW from the utility, assume increase due to inflation
    //total life-time of a project
    
    
    
    ///@DevStage2: calculate PPA
    ///@DevStage2: calculate lease

    //simple calculation when HO owns the system
    auto inflation = WorldSettings::instance().params_exog[EParamTypes::InflationRate];
    auto CPI = 1;
    auto energy_costs = 0.0;
    auto AC_size = design.AC_size;
    auto degradation_t = std::exp(std::log((1 - design.PV_module->degradation))/WorldSettings::instance().params_exog[EParamTypes::DegradationDefinitionLength]);
    for (auto i = 0; i < design.PV_module->warranty_length; ++i)
    {
        //estimate yearly energy production, if PPA might be used in estimation
        //MARK: cont. check again
        auto yearly_production = AC_size * design.solar_irradiation * constants::NUMBER_DAYS_IN_YEAR;
        energy_costs += yearly_production * CPI * WorldSettings::instance().params_exog[EParamTypes::ElectricityPriceUCDemand];
        AC_size = AC_size * degradation_t;
        CPI = CPI * inflation;
    };
    
    design.energy_savings_money = energy_costs;
    design.total_savings = energy_costs - design.total_costs;
}



void
SEI::form_financing(std::shared_ptr<PVProject> project_)
{
    project_->financing = std::make_shared<MesFinance>();
    
    //assume that whole sum is due on the beginning of the project
    project_->financing->schedule_payments = {project_->design->design->total_costs};
    
    
}

/**

Here also have buying and payment
 
*/
void
SEI::install_project(std::shared_ptr<PVProject> project)
{
    auto mes = MesSellOrder(project->design->design->PV_module->name, project->design->design->N_PANELS, this);
    
    //check that has enough money
    auto money_need = mes.qn * project->design->design->PV_module->manufacturer->prices[mes.item];
    
    
    
    
    if ((money_need >= money) || (project->state_materials == EParamTypes::PendingMaterials))
    {
        
        money -= money_need;
        
        if (project->design->design->PV_module->manufacturer->sell_SolarModule(mes))
        {
            //money are already paid
        }
        else
        {
            project->state_materials = EParamTypes::PendingMaterials;
            money += money_need;
        };
        
    }
    else
    {
        project->state_materials = EParamTypes::PendingMaterials;
    };
    
    w->get_state_inf_installed_project(project);
}




void
SEI::get_payment(std::shared_ptr<MesPayment> mes_)
{
    money += mes_->q;
}



void
SEI::ac_update_tick()
{
    //update internal timer
    a_time = w->time;
    
    pvprojects_lock.lock();
    //pove pending projects into active projects
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
    if (i_schedule_visits == (WorldSettings::instance().constraints[EConstraintParams::MaxLengthWaitPreliminaryQuote] - 1))
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
            if ((a_time - project->ac_sei_time) >= params[EParamTypes::SEIProcessingTimeRequiredForSchedulingFirstSiteVisit])
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
            if ((a_time - project->ac_sei_time) >= params[EParamTypes::SEIProcessingTimeRequiredForPreliminaryQuote])
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
            if ((a_time - project->ac_sei_time) >= params[EParamTypes::SEIProcessingTimeRequiredForDesign])
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
            w->g->request_permit_for_installation(project);
            project->state_project = EParamTypes::RequestedPermitForInstallation;
        };
        
        
        if ((project->state_project == EParamTypes::GrantedPermitForInstallation) || (project->state_project == EParamTypes::ScheduleInstallation))
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
        
        
        
        if (project->state_project == EParamTypes::Installed)
        {
            //request inspection
            w->g->request_inspection(project);
            project->state_project = EParamTypes::RequestedInspectionAfterInstallation;
        };
        
        
        if (project->state_project == EParamTypes::PassedInspectionAfterInstallation)
        {
            w->utility->request_permit_for_interconnection(project);
            project->state_project = EParamTypes::RequestedPermitForInterconnection;
        };
        
        if (project->state_project == EParamTypes::GrantedPermitForInterconnection)
        {
            w->get_state_inf_interconnected_project(project);
        };
        
        
        
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
    for (auto& w_project:schedule_installations[i_schedule_installations])
    {
        //go to sites, collect information
        auto project = w_project.lock();
        //check if project is still active
        if (project && project->financing->state_payments == EParamTypes::PaymentsOnTime)
        {
            install_project(project);
            if (project->state_materials == EParamTypes::PendingMaterials)
            {
                //reschedule
                project->state_project = EParamTypes::ScheduleInstallation;
            }
            else
            {
                project->state_project = EParamTypes::Installed;
            };
            project->ac_sei_time = a_time;
        }
        else
        {
            //reschedule
            project->state_project = EParamTypes::ScheduleInstallation;
            
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


void
SEI::ac_inf_marketing_sei()
{
}

void
SEI::get_inf(std::shared_ptr<MesMarketingSEI> mes_)
{
}







