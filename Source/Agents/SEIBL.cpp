//
//  SEIBL.cpp
//  ABMSolar
//
//  Created by Ekaterina Sinitskaya on 7/9/16.
//  Copyright (c) 2016 IRIS Lab. All rights reserved.
//

#include "Tools/IRandom.h"
#include "Tools/WorldSettings.h"
#include "Agents/SEIBL.h"
#include "Agents/H.h"
#include "UI/W.h"

using namespace solar_core;

/**
 
 Create design for H and its parameters
 
 */
std::shared_ptr<PVProjectFlat>
SEIBL::form_design_for_params(H* agent_, std::shared_ptr<PVProjectFlat> project)
{
    
    //size is equal to the utility bill, assume simple electricity pricing for now
    auto estimated_demand  = agent_->params[EParamTypes::ElectricityBill] / WorldSettings::instance().params_exog[EParamTypes::ElectricityPriceUCDemand]/constants::NUMBER_DAYS_IN_MONTH;
    
    //price per watt is predetermined by the optimality choice
    auto p = params[EParamTypes::EstimatedPricePerWatt] *  estimated_demand;
    
    
    //there is only 1 solar panel + inverter combinaion to use in offering solar panels
    //calculate ROI
    
    auto roof_age = [&coef = roof_age_coef](double house_age)->double {return roof_age_coef * house_age;};
    
    
    
    auto NPV_purchase = [&project](double r)->double {
        
        //panel output
        
        
        //production each time unit
        
        
        
        
    };
    
    
    
    //find irr
    //secant method
    
    
    
    
    project.irr = irr;
    
    
    
    
}




double
SEIBL::irr_secant()
{
    
    //guess starting points
    r_n = 0.05
    r_n_1
    r_n_2
    
    
    
}



double
SEIBL::NPV_purchase(std::shared_ptr<PVProjectFlat>, double irr)
{
    //panel output
    
    
    
};




void
SEIBL::dec_base()
{
    //makes decision about ...
    
    //estimates expected profit for the current module
    
    TDesign dec_design_hat = *dec_design; /// current design first
    
    //assume price
    double p = params[EParamTypes::EstimatedPricePerWatt];
    
    
    
    
    
    double p = max_profit(dec_design_hat);
    
    
    
    

    
    //draw random SEM
    auto i = 0;
    
    if (dec_design->module->manufacturer != w->sems[i])
    {
        //evaluate new design
        
        
        auto profit_time = ;
        auto profit_new = ;
        
        
        auto p_switch = exploration_p(profit_time, profit_new);
        
        //draw and see if switches
        
        
        
        //if switches
        dec =
        
    };
    
    
    
    
    //calculate expected profit for other random design (from random SEM)
    //compare - if higher, than have chance to switch and start offering it
    
    
    
    
    
    
    
    //might be using SEM specific prior for new models?
    
    //prob of switching depends on the distance between expected profits and attitide towards switching
    
    //pick random SEM
    //use active design
    //evaluate - solve for p explicit (could solve for the actual solution here )
    
    
    
    
}


double
SEIBL::max_profit(){}



double
SEIBL::estimate_irr_from_params(std::shared_ptr<TDesign> dec_design_hat, double p)
{
    
    double irr = 0.0
    
    //MARK: cont.
    
    return irr;
    
    
}



double
SEIBL::est_profit(std::shared_ptr<TDesign> dec_design_hat, double p)
{
    
    double inflation = WorldSettings::instance().params_exog[EParamTypes::InflationRate];
    
    ///estimates profit given the proposed price
    
    ///calculate promised irr for the TDesign given average utility bill and other parameters for the design, assume that spends p requested, and savings are equal to the savings on electricity bill over the warranty length, assume increase in electricity prices, and degradation standard. Do not include down time due to maintenance - assume that it will be ideal conditions (as a sales pitch). For estimates of the profit will use actual estimated maintenance costs
    double irr_hat = est_irr_from_params(dec_design_hat, p);
    
    
    //given price parameter - get number of projects
    //estimated number of projects is Bayesian Linear Regression
    //parameters are own offered irr, offered irr of others, reputation, reputation of others
    //use ceil to get int number
    double N_hat = std::ceil(THETA_demand[0] +
                             THETA_demand[1] * irr_hat +
                             THETA_demand[2] * THETA_reputation[0] * THETA_reputation[1] +
                             THETA_demand[3] * WM_time[0] +
                             THETA_demand[4] * WM_time[1]);
    
    
    
    //estimate number of panels for an average utility bill
    auto demand = WorldSettings::instance().params_exog[EParamTypes::AverageElectricityDemand]/constants::NUMBER_DAYS_IN_MONTH;
    //solar irradiation - average number
    auto solar_irradiation = WorldSettings::instance().params_exog[EParamTypes::AverageSolarIrradiation];
    //permit length in labor*hours
    auto permit_difficulty = WorldSettings::instance().params_exog[EParamTypes::AveragePermitDifficulty];
    
    
    int N_PANELS = std::ceil(demand / ((solar_irradiation) * dec_design_hat->module->efficiency * (dec_design_hat->module->length * dec_design_hat->module->width/1000000) * (1 - WorldSettings::instance().params_exog[EParamTypes::DCtoACLoss])));
    
    double DC_size = N_PANELS * dec_design_hat->module->efficiency * dec_design_hat->module->length * dec_design_hat->module->width / 1000;
    
    double w = WorldSettings::instance().params_exog[EParamTypes::LaborPrice];
    
    
    //repeat over estimated planned horizon of 5 years for keeping the same type of equipment
    int T_planning = 5;
    double N_hat_t = 0.0;
    double complexity_install_t = dec_design_hat->complexity_install;
    double costs_t = 0.0;
    double w_t = 0.0;
    double profit_t = 0.0;
    double profit = 0.0;
    double irr_ = 0.05;
    
    for (auto t = 0; i < T_planning; ++i)
    {
        //adjust for inflation
        w_t = w * std::pow(1 + inflation, t);
        
        //calculate expected sales, assume the same formula
        N_hat_t = N_hat;
        
        //calculate installation costs - lower, as assume learning-by-doing
        costs_t += w_t * complexity_install_t * N_hat_t;
        
        //calculate expected maintenance costs (discounted)
        costs_t += est_maintenance(dec_design_hat, N_hat_t, w_t);
        
        //calculate materials costs, only panels, no inverters here
        costs_t += N_panels * N_hat_t * p_module;

        //labor costs to design each project
        costs_t += N_hat_t * params[EParamTypes::SEITimeLUForDesign] * w_t;
        
        //labor costs to get permit
        costs_t += permit_difficulty * w_t;
        
        //marketing costs - fixed number of hours in labor units
        costs_t += params[EParamTypes::SEITimeLUForMarketing] * w_t;
        
        //calculate sales
        profit_t += N_hat_t * p * DC_size;
        
        //total profit
        profit_t -= costs_t;
        
        
        //update parameters
        complexity_install_t = dec_design_hat->compexity_install * (dec_design_hat->BETA_complexity_time);
        profit_t = 0.0;
        costs_t = 0.0;
        
        //accumulate profit
        profit += profit_t / std::pow(1 + irr_, t);
        
    };

    return profit;
    
}



double
SEIBL::est_maintenance(std::shared_ptr<TDesign> dec_design_hat, std::size_t N_hat, double w)
{
    ///number of simulation runs
    std::size_t N_trial = 10;
    
    //calculate expected maintenance
    //draw time before next maintenance
    //create gamma for current prior - convert into boost representation
    
    //here need to write Lomax distribution for reliability - prior Gamma, data - exponential, posterior - Gamma, posterior for data - Lomax
    //for complexity prior - Normal-Inverse gamma, data - normal, posterior - Normal-Inverse gamma, posterior for data - student. Generate from usual student and shift by mean and scale
    
    
    
    
//    auto pdf_THETA_reliability = boost::random::gamma_distribution<>(dec_design_hat.THETA_reliability[0], 1/dec_design_hat.THETA_reliability[1]);
//    auto rng_THETA_reliability = boost::variate_generator<boost::mt19937&, boost::random::gamma_distribution<>>(w->rand->rng, pdf_THETA_reliability);
//    auto pdf_THETA_complexity = boost::random::gamma_distribution<>(dec_design_hat.THETA_reliability[0], 1/dec_design_hat.THETA_reliability[1]);
//    auto rng_THETA_complexity = boost::variate_generator<boost::mt19937&, boost::random::gamma_distribution<>>(w->rand->rng, pdf_THETA_reliability);
    
    
    
    double x = 0.0; //time before next breakdown
    double t = 0.0; //time of a last breakdown
    double irr_ = 0.05; //rate of financing for this installer - discounting rate for its profit calculations
    double maintenance_n_hat = 0.0; //maintenance for one simulation run
    double maintenance_hat = 0.0; //average maintenance over all simulation runs
    double inflation = WorldSettings::instance().params_exog[EParamTypes::InflationRate];
    
    
    
    //estimation for the period t, labor price is constant - change to dynamic labor price estimation in the future
    for (auto n = 0; n < N_trials; ++n)
    {
        //for each project
        for (auto i = 0; i < N_hat; ++i)
        {
            while (true)
            {
                //time until next breakdown in years
                x = rng_THETA_reliability();
                t += x;
                
                //if it is still in warranty - spend labor*hours to repair
                if (t <= dec_design_hat.module->warranty_length)
                {
                    //draw complexity of the breakdown, in labor*hours
                    y = rng_THETA_complexity();
                    //and labor costs to repair it adjusted for inflation
                    maintenance_n_hat += (y * w * std::pow(1 + inflation, t))/std::pow(1 + irr_, t);
                    
                    //? need condition of non-negativity for cashflows
                    cash_flow_maintenance_n[t] = (y * w * std::pow(1 + inflation, t));
                }
                else
                {
                    t = 0.0;
                    break;
                };
            };
            
        };
        
        maintenance_hat += maintenance_n_hat;
        maintenance_n_hat = 0.0;
    };
    
    
    //final estmate for the discounted maintenance costs
    maintenance_hat = maintenance_hat/(N_trials);

    return maintenance_hat;
    
}



void
SEIBL::wm_update()
{
    //collect average reputations for top installer in term of shares
    //collect posted irr for them, average
    
    WM_time[0] = w->get_inf(EDecParams::Reputation_i, this);
    WM_time[1] = w->get_inf(EDecParams::irr_i, this);
    
    
    //updates parameters for distribution if new information about performance is here
    
    
    //goes through actual production in the previous year, calculate percentage of the promised production,
    //update Gamma with new estimate in THETA_reputation, given the new data point
    
    
    
    
}



/**
 
 
 
 
 */
void
SEIBL::installed_project()
{
    
    //draw next time of maintenance
    
}

/**
 
 
 tick is moved to be a year
 
*/
void
SEIBL::act_tick()
{
    //updates information for decision making
    wm_update();
    
    //make price decision, based on the switching or not
    auto dec = dec_base();
    
    //switch to new offering, lock as simulateneously is offering projects to H
    lock.lock();
    dec_design = dec;
    lock.unlock();
    
    
    //MARK: cont.

    
    //go over installed projects and see if need maintenance, if need - record as costs and update time till next maintenance
    //record actual production in the year based on the maintenance length
    
    
    
    
    
}



