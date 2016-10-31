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

    
    //pregenerate range of prices
    int64_t N = 100;
    double THETA_min = 0.0;
    double THETA_max = 1.0;
    double step_size = (THETA_max - THETA_min)/N;
    
    profit_grid.resize(N+1, 2);
    
    for (auto i = 0; i < N + 1; ++i)
    {
        profit_grid(i,0) = THETA_min + i * step_size;
    };

    
    
    
    
    
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
    
    //complexity of installation in labor*hours
    complexity_install_prior = pt_.get<double>("complexity_install_prior");
    
    
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
    


    //MARK: cont.
    //is used in non-compensatory decision making, will be on generic parameters of sei
    
    
    
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
SEI::form_preliminary_quote(std::shared_ptr<PVProject> project_, double profit_margin)
{
    //from params get stuff such as average price per watt, price of a standard unit
    auto mes = std::make_shared<MesMarketingSEIPreliminaryQuote>();
    
    
    //MARK: cont. create preliminary quote estimate from real data
    //by default provide prelminary quote, but in some cases refuse to proceed with the project and close it
    //is here because otherwise later assigning will override this
    project_->state_project = EParamTypes::ProvidedPreliminaryQuote;
    
    //@DevStage3 think about changing location of this request. For now it is assumed that SEI bails out if roof is too old by its standards
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
    
    //forms design by default, for an average demand
    auto demand = WorldSettings::instance().params_exog[EParamTypes::AverageElectricityDemand]/constants::NUMBER_DAYS_IN_MONTH;
    //amount of solar irradiation in Wh/m2/day
    auto solar_irradiation = w->get_solar_irradiation(project_->agent->location_x, project_->agent->location_y);
    //distribution of permit length difficulty coefficient in different locations
    auto permit_difficulty = w->get_permit_difficulty(project_->agent->location_x, project_->agent->location_y);
    
    
    auto design = PVDesign();
    
    //assume that project is offered at 100% of utility bill
    //only 1 solar module per installer
    form_design_for_params(project_, demand, solar_irradiation, permit_difficulty, dec_project_percentages[0], profit_margin, *(dec_solar_modules.find(EParamTypes::SEIMidEfficiencyDesign)), *(dec_inverters.find(EParamTypes::TechnologyInverterCentral)), design);
    

    
    ac_estimate_savings(design, project_);


    mes->params[EParamTypes::PreliminaryQuotePrice] = design.total_costs;
    
    
    //assume that permit difficulty here in weeks
    //so that total project time is lead time and permitting time summed
    auto permit_difficulty_scale = WorldSettings::instance().params_exog[EParamTypes::AveragePermitDifficulty];
    auto total_project_time = permit_difficulty_scale * permit_difficulty + params[EParamTypes::SEILeadInProjectTime];
    
    mes->params[EParamTypes::PreliminaryQuoteTotalProjectTime] = total_project_time;
    
    //MARK: cont. change to savings percent. Add calculation of savings
    mes->params[EParamTypes::PreliminaryQuoteEstimatedNetSavings] = design.total_net_savings;
    
    
    mes->params[EParamTypes::SEIWarranty] = params[EParamTypes::SEIWarranty];
    
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
std::vector<std::shared_ptr<MesDesign>>
SEI::form_design(std::shared_ptr<PVProject> project_, double profit_margin)
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
    for (auto project_percentage:dec_project_percentages)
    {
        for (auto iter: dec_solar_modules)
        {
            for (auto iter_inv:dec_inverters)
            {
                //create design
                auto design = PVDesign();
                
                form_design_for_params(project_, demand, solar_irradiation, permit_difficulty, project_percentage, profit_margin, iter, iter_inv, design);
                
                ac_estimate_savings(design, project_);
                
                designs.push_back(design);
                
            };
        };
    };
    
    //sort by savings and present best option
    ///@DevStage2 bootstrap here for different savings dynamics depending on parameters
    std::sort(designs.begin(), designs.end(), [&](PVDesign &lhs, PVDesign &rhs){
        return lhs.total_net_savings > rhs.total_net_savings;
    });
    
    
    std::vector<std::shared_ptr<MesDesign>> mess;
    mess.push_back(std::make_shared<MesDesign>(std::make_shared<PVDesign>(designs[0])));
    mess.push_back(std::make_shared<MesDesign>(std::make_shared<PVDesign>(designs[1])));
    
        
    return mess;
}


void
SEI::form_design_for_params(std::shared_ptr<const PVProject> project_, double demand, double solar_irradiation, double permit_difficulty, double project_percentage, double profit_margin, const IterTypeDecSM& iter, const IterTypeDecInverter& iter_inverter, PVDesign& design)
{
    
    double demand_adjusted = demand / (1 - WorldSettings::instance().params_exog[EParamTypes::DCtoACLoss]);
    
    design.solar_irradiation = solar_irradiation;
    
    design.permit_difficulty = permit_difficulty;
    
    
    double N_PANELS = std::ceil(demand_adjusted * project_percentage / ((solar_irradiation) * iter.second->efficiency * (iter.second->length * iter.second->width/1000000) * ( 1 - WorldSettings::instance().params_exog[EParamTypes::DCtoACLoss])));
    
    //size of one panel
    double module_area = iter.second->length * iter.second->width / 1000000;
    
    //system area size
    double system_area = N_PANELS * module_area;
    //convert sq.feet into sq. meters
    double roof_area = constants::NUMBER_SQM_IN_SQF * project_->agent->house->roof_size;
    double available_area = std::min(roof_area, system_area);
    
    //update to the actual available area
    N_PANELS = std::ceil(available_area/module_area);
    design.N_PANELS = N_PANELS;
    
    design.PV_module = iter.second;
    
    design.inverter = iter_inverter.second;
    
    design.DC_size = design.N_PANELS * iter.second->efficiency * iter.second->length * iter.second->width / 1000;
    
    
    //MARK: cont. check if need to target 100% of demand with DC or already with adjusted AC size?
    design.AC_size = design.DC_size * (1 - WorldSettings::instance().params_exog[EParamTypes::DCtoACLoss]);
    
    
    design.co2_equivalent = 0.0;
    
    
    
    
    
    //failure rate for main part of the system
    //if it is central inverter - add number of panels  and raw central inverter failure rate
    
    design.failure_rate = 0.0;
    design.failure_rate += WorldSettings::instance().params_exog[EParamTypes::SEMFailureRatePVModule] * design.N_PANELS;
    switch (design.inverter->technology) {
        case ESEIInverterType::Central:
            design.failure_rate += WorldSettings::instance().params_exog[EParamTypes::SEMFailureRateInverterCentral];
            break;
        case ESEIInverterType::PoweOptimizer:
            //add both micro and central failure rates
            design.failure_rate += WorldSettings::instance().params_exog[EParamTypes::SEMFailureRateInverterCentral];
            design.failure_rate += WorldSettings::instance().params_exog[EParamTypes::SEMFailureRateInverterPowerOptimizer] * design.N_PANELS;
            break;
        case ESEIInverterType::Micro:
            design.failure_rate += WorldSettings::instance().params_exog[EParamTypes::SEMFailureRateInverterMicro] * design.N_PANELS;
            break;
        default:
            break;
    }
    
    ac_estimate_price(design, project_, profit_margin);
    
    
}



void
SEI::ac_estimate_price(PVDesign& design, std::shared_ptr<const PVProject> project_, double profit_margin)
{
    double costs = 0.0;
    //costs of the system
    //number of panels at their price at manufacturer
    //number of inverters at their price at manufacturer
    costs += design.N_PANELS * design.PV_module->p_sem;

    switch (design.inverter->technology) {
        case ESEIInverterType::Micro:
        case ESEIInverterType::PoweOptimizer:
            costs += design.N_PANELS * design.inverter->p_sem;
            break;
        case ESEIInverterType::Central:
            costs += design.inverter->p_sem;
            break;
        default:
            break;
    }
    
    
    
    
    //cost of installation
    //time for installation
    double wage = WorldSettings::instance().params_exog[EParamTypes::LaborPrice];
    costs += complexity_install_prior * wage;
    
    
    //no assumptions on the design of a project specific time
    
    
    
    //length of permitting - assume 1 person per project
    //average of 25 days for interconnection
    auto permit_difficulty_scale = WorldSettings::instance().params_exog[EParamTypes::AveragePermitDifficulty];
    auto total_project_time = permit_difficulty_scale * design.permit_difficulty + params[EParamTypes::SEILeadInProjectTime];
    
    
    //assume that 1 person per project for the whole duration as a sales rep
    costs += wage * total_project_time * 1;
    
    
    
    //profit  margin on costs
    design.total_costs = costs * (1 + profit_margin);
    
    design.raw_costs = costs;
 
    
    
}





/**
 
 
 
 
 
*/
void
SEI::ac_estimate_savings(PVDesign& design, std::shared_ptr<const PVProject> project_)
{
    //estimate savings for each project
    //get price of kW from the utility, assume increase due to inflation
    //total life-time of a project
    
    
    
    ///@DevStage4: calculate PPA
    ///@DevStage4: calculate lease

    //simple calculation when HO owns the system
    auto inflation = WorldSettings::instance().params_exog[EParamTypes::InflationRate];
    auto CPI = 1;
    auto AC_size_t = design.AC_size;
    //difference in definitions
    auto degradation_t = std::exp(std::log((1 - design.PV_module->degradation))/WorldSettings::instance().params_exog[EParamTypes::DegradationDefinitionLength]);
    auto production_t = 0.0;
    
    
    //loan annuity - equivalent to
    auto loan_amount = design.total_costs;
    auto interest_rate_loan = WorldSettings::instance().params_exog[EParamTypes::AverageInterestRateLoan]; //yearly terms
    double warranty_length = WorldSettings::instance().params_exog[EParamTypes::PVModuleWarrantyLength] * constants::NUMBER_TICKS_IN_YEAR;  //originally in yearly terms
    //assumed that it is equal to warranty ... BIG ASSUMPTION
    auto loan_length = warranty_length / constants::NUMBER_TICKS_IN_YEAR; //yearly dimension, divide by number of weeks in a year
    
    int NUMBER_MONTHS_IN_YEAR = 12;
    auto N_loan = loan_length * NUMBER_MONTHS_IN_YEAR;
    auto loan_annuity = (interest_rate_loan/NUMBER_MONTHS_IN_YEAR)/(1 - std::pow((1 + interest_rate_loan/NUMBER_MONTHS_IN_YEAR), - N_loan))*loan_amount;
    auto total_production = 0.0;
    double potential_energy_costs = 0.0;
    double realized_energy_income = 0.0;
    
    //It is fixed 25 years in conjoint 
    for (auto i = 0; i < warranty_length; ++i)
    {
        //estimate yearly energy production
        //MARK: cont. check again that numbers are in the correct ballpark
        production_t = AC_size_t * design.solar_irradiation * constants::NUMBER_DAYS_IN_YEAR;
        total_production += production_t;
        
        //how much have payed if decided to go without solar
        potential_energy_costs += design.AC_size * CPI * WorldSettings::instance().params_exog[EParamTypes::ElectricityPriceUCDemand];
        //how much could make if sell energy and decide not to use it
        realized_energy_income += production_t * CPI * WorldSettings::instance().params_exog[EParamTypes::ElectricityPriceUCSupply];
        AC_size_t = AC_size_t * degradation_t;
        CPI = CPI * inflation;
    };
    
    
    //adjustment for opportunity costs for diverting money into this investment
    design.total_net_savings = (realized_energy_income - loan_annuity * N_loan)/potential_energy_costs;
    design.co2_equivalent = total_production/WorldSettings::instance().params_exog[EParamTypes::EnergyToCO2];
}



void SEI::form_financing(std::shared_ptr<PVProject> project_)
{
    project_->financing = std::make_shared<MesFinance>();
    
    //assume that whole sum is due on the beginning of the project
    project_->financing->schedule_payments = {project_->design->design->total_costs};
    
    
}





PVProject* SEI::initialize_default_project()
{
    
}






/**
 
 
 Maximizing profit by changing price
 
 */
void SEI::dec_max_profit()
{
    
    //search over margins on total costs
    //[0.0, 1.0] at 10% step
    
    
    
    //total costs are installation costs and marketing and administrative costs
    
    //values for central and micro inverters
    
    //for each profit margin estimate costs of installation
    //estimate share of the market that will go to that installer
    //1 top besides me that is not me, based on shares of the previous x periods, price ? current
    //either this and none options - easier to do - so make as a first option
    

    //simplification - assume that there is only 1 class, pick first class
    auto label = w->ho_decisions[EParamTypes::HOSEIDecision]->labels[0];

    //create project for an average homeowner
    //dummy agent with house size
    auto agent = new Homeowner();
    auto house = new House();
    agent->house = house;
    //and dummy location
    //use zero H to get location for now
    agent->location_x = (*w->hos)[0]->location_x;
    agent->location_y = (*w->hos)[0]->location_y;
    
    //electricity bill is average
    auto agent_state = std::make_shared<MesStateBaseHO>();
    agent_state->params[EParamTypes::ElectricityBill] = WorldSettings::instance().params_exog[EParamTypes::AverageElectricityDemand] * WorldSettings::instance().params_exog[EParamTypes::ElectricityPriceUCDemand];
    
   
    //house->roof_size = WorldSettings::instance().params_exog[EParamTypes::AverageRoofSize];
    //set to nonbinding constraint
    house->roof_size = 1000000;
    
    
    int64_t N_SEI_i = 2;
    
    //generic project for PV with design variations
    auto project_generic = std::make_shared<PVProject>();
    auto project_sei_i = std::make_shared<PVProject>();
    project_generic->agent = agent;
    project_generic->state_base_agent = agent_state;
    project_generic->sei = this;
    project_sei_i->agent = agent;
    project_sei_i->state_base_agent = agent_state;
    
    //two random agents to use as alternatives for estimating market share
    auto max_ = w->seis->size() - 1;
    auto pdf_i = boost::uniform_int<uint64_t>(0, max_);
    auto rng_i = boost::variate_generator<boost::mt19937&, boost::uniform_int<uint64_t>>(w->rand_sei->rng, pdf_i);

    std::vector<int64_t> sei_i;
    for (auto j = 0; j < N_SEI_i; ++j)
    {
        sei_i.push_back(rng_i());
    };
    
    double utility_den = 0.0;
    double utility_nom = 0.0;
    double utility_i = 0.0;
    double share_sei = 0.0;
    double share_design = 0.0;
    double income = 0.0;
    double expences = 0.0;
    double qn = 0.0;
    
    for (auto i = 0; i < profit_grid.rows(); ++i)
    {
        //profit margin to check - profit_grid(i,0)
        //preliminary quote with new price
        project_generic->preliminary_quote = form_preliminary_quote(project_generic, profit_grid(i,0));
        
        
        //estimate market size
        //draw other sei, request preliminary  params for quote, assume that price is fixed?
        //use the same project
        utility_den = 0.0;
        for (auto i_sei:sei_i)
        {
            project_sei_i->sei = (*w->seis)[i_sei];
            project_sei_i->preliminary_quote = (*w->seis)[i_sei]->form_preliminary_quote(project_generic, (*w->seis)[i_sei]->THETA_profit[0]);
            //estimate utility
            //use zero H to get estimation of utility for now
            utility_den += (*w->hos)[0]->estimate_sei_utility_from_params(project_sei_i, w->ho_decisions[EParamTypes::HOSEIDecision]->HOD_distribution_scheme[label]);
            
        };
        
        //utility of none
        utility_den += w->ho_decisions[EParamTypes::HOSEIDecision]->HOD_distribution_scheme[label][EParamTypes::HOSEIDecisionUtilityNone][0];
        
        //estimate own utility
        utility_nom = (*w->hos)[0]->estimate_sei_utility_from_params(project_generic, w->ho_decisions[EParamTypes::HOSEIDecision]->HOD_distribution_scheme[label]);
        
        //estimate share
        share_sei = utility_nom/utility_den;
        
        
        
        //@DevStage3 price might change if switch to multiple threads and have simulateneous profit max of multiple installers. Any way may end up with having to look at the share of installer with new price

        
        utility_den = 0.0;
        //initialize designs with micro and central inverter
        auto designs = form_design(project_generic, profit_grid(i,0));
        std::vector<double> shares_design;
        for (auto design:designs)
        {
            //update design for new parameters
            project_generic->design = design;
            
            utility_i = (*w->hos)[0]->estimate_design_utility_from_params(project_generic, w->ho_decisions[EParamTypes::HODesignDecision]->HOD_distribution_scheme[label]);
            
            utility_den += utility_i;
            
            shares_design.push_back(utility_i);
        };
        
        
        //utility of none
        utility_den += w->ho_decisions[EParamTypes::HODesignDecision]->HOD_distribution_scheme[label][EParamTypes::HODesignDecisionUtilityNone][0];
        
        
        for (auto j = 0; j < shares_design.size(); ++j)
        {
            //update to the share from raw utility
            //estimate share of the submarket that will install solar panels with particular design
            shares_design[i] = shares_design[i]/utility_den;
            
            //calculate total sales
            qn = WorldSettings::instance().params_exog[EParamTypes::TotalPVMarketSize] * share_sei * shares_design[i];
            
            //calculate income for this design type
            income += designs[i]->design->total_costs * qn;
            
            //calculate costs for this type
            expences += designs[i]->design->raw_costs * qn;
            
        };
        
        //marketing costs - fixed number of hours in labor units
        expences += params[EParamTypes::SEITimeLUForMarketing] * WorldSettings::instance().params_exog[EParamTypes::LaborPrice];
        
        //general administrative costs
        expences += params[EParamTypes::SEITimeLUForAdministration] * WorldSettings::instance().params_exog[EParamTypes::LaborPrice];

        
        //estimate total income from installation - total costs (=costs of installation + marketing and administrative)
        profit_grid(0,1) = income - expences;
        
        
    };
    

    Eigen::MatrixXd::Index maxRow, maxCol;
    double max = profit_grid.col(1).maxCoeff(&maxRow, &maxCol);

    //updating profit margin
    THETA_profit[0] = profit_grid(maxRow,0);

    
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
    
    //MARK: cont. sort out timing of updating projects - general timing of messages in the timeline
    
    
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
        
        
        //if visited site and processing time for preliminary quote has elapsed -  form preliminary quote
        if (project->state_project == EParamTypes::RequestedPreliminaryQuote)
        {
            //just processing time
            if ((a_time - project->ac_sei_time) >= params[EParamTypes::SEIProcessingTimeRequiredForPreliminaryQuote])
            {
                auto mes = form_preliminary_quote(project, THETA_profit[0]);
                project->preliminary_quote = mes;
                project->state_project = EParamTypes::ProvidedPreliminaryQuote;
                project->agent->receive_preliminary_quote(project);
                project->ac_sei_time = a_time;
            };
        };
        

        
        
        //if preliminary quote is requested and have capacity for new project, and processing time for preliminary quotes has elapced - get back and schedule time
        if (project->state_project == EParamTypes::AcceptedPreliminaryQuote)
        {
            //if preliminary quote was accepted - check that processing time after request has elapsed and contact agent to schedule visit, check capacity for visits for each future time
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
        
        
        
        
        if (project->state_project == EParamTypes::CollectedInfFirstSiteVisit)
        {
            //if enough time has elapsed
            if ((a_time - project->ac_sei_time) >= params[EParamTypes::SEIProcessingTimeRequiredForDesign])
            {
                auto mess = form_design(project, THETA_profit[0]);
                project->state_project = EParamTypes::DraftedDesign;
                project->design = mess[0];
                form_financing(project);
                project->agent->receive_design(project);
                project->ac_sei_time = a_time;
                
                for (auto i = 1; i < mess.size(); ++i)
                {
                    //create new project
                    auto project_dublicate = PVProject(*project);
                    
                    auto project_to_add = std::make_shared<PVProject>(project_dublicate);
                    
                    get_project(project_to_add);
                    
                    project_to_add->agent->get_project(project_to_add);
                    
                    //assign design
                    project_to_add->design = mess[i];
                    
                    form_financing(project_to_add);
                    project_to_add->agent->receive_design(project_to_add);
                    project_to_add->ac_sei_time = a_time;
                };
                
                

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

    
    
    
    
    //make decision on price per watt for new installations and general pricing strategy
    //estimate sales given price - estimate share of the market for installer parameters
    //estimate prob to get client given current offerings
    //assume that knows actual utility function
    
    
    
    
    
    
    

    //MARK: cont. collect information from SEM and update design examples
    if ((a_time - ac_designs) > params[EParamTypes::SEIFrequencyUpdateDesignTemplates])
    {
        //collect information about current offerings
        //sort by efficiency
        
        
        
    };
    
    
    
    
    
    
    
    
}


void
SEI::ac_inf_marketing_sei()
{
}

void
SEI::get_inf(std::shared_ptr<MesMarketingSEI> mes_)
{
}







