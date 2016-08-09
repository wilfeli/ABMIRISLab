//
//  SEIBL.cpp
//  ABMSolar
//
//  Created by Ekaterina Sinitskaya on 7/9/16.
//  Copyright (c) 2016 IRIS Lab. All rights reserved.
//

#include "Tools/IRandom.h"
#include "Tools/Serialize.h"
#include "Tools/WorldSettings.h"
#include "Agents/SEIBL.h"
#include "Agents/SEMBL.h"
#include "Agents/H.h"
#include "UI/W.h"
#include "UI/WEE.h"

using namespace solar_core;



SEIBL::SEIBL(const PropertyTree& pt_, WEE* w_):SEI(pt_, w_)
{
    
    //generate other parameters
    serialize::deserialize(pt_.get_child("THETA_reputation"), THETA_reputation);
    
    serialize::deserialize(pt_.get_child("THETA_complexity_prior"), THETA_complexity_prior);
    
    serialize::deserialize(pt_.get_child("THETA_reliability_prior"), THETA_reliability_prior);
    
    complexity_install_prior = pt_.get<double>("complexity_install_prior");

    
    //create empty design to offer
    dec_design = new TDesign();
    
    //reset to presets
    //might be using SEM specific prior for new models? later
    dec_design->THETA_reliability = THETA_reliability_prior;
    dec_design->THETA_complexity = THETA_complexity_prior;
    dec_design->complexity_install = complexity_install_prior;
    
    //synchronize price
    dec_design->p_design = params[EParamTypes::EstimatedPricePerWatt];
    
    
    
    //prior on V_0 - assume ridge regression
    double c = 0.5;
    V_0 = SEIWMMatrixd::Identity();
    V_0 = c * V_0;
    
    //as alternative could read from file
//    std::vector<double> V_0_std;
//    serialize::deserialize(pt_.get_child("V_0"), V_0_std);
//    Eigen::Map<Eigen::MatrixXd> V_0(V_0_std.data(), constants::N_BETA_SEI_WM, constants::N_BETA_SEI_WM);
//    a_0 = pt_.get<double>("a_0");
//    b_0 = pt_.get<double>("b_0");
//    std::vector<double> Mu_0_std;
//    serialize::deserialize(pt_.get_child("Mu_0"), Mu_0_std);
//    Eigen::Map<Eigen::MatrixXd> Mu_0(Mu_0_std.data(), constants::N_BETA_SEI_WM, 1);

    
    
    
    //prior on other parameters from BLR for the share
    a_0 = 1;
    b_0 = 1;
    
    Mu_0 = SEIWMDataType::Ones();
    X(0,0) = 1.0;
    
    
    //initialize THETA_demand from Mu_0
    THETA_demand = std::vector<double>(constants::N_BETA_SEI_WM, 0.0);
    //data generating will be MVS - take mean from there - see predictions using BLR
    for (auto i = 0; i < Mu_0.size() ; ++i)
    {
        THETA_demand[i] = Mu_0[i];
    };
    
    
    
}


void SEIBL::init(WEE* w_)
{
    w = w_;
    //finish setting starting design
    dec_design->p_module = dec_design->PV_module->p_sem;
    designs[dec_design->PV_module->uid] = dec_design;
    
}


/**
 
 Create design for H and its parameters
 
 */
std::shared_ptr<PVProjectFlat>
SEIBL::form_design_for_params(H* agent_, std::shared_ptr<PVProjectFlat> project)
{
    
    //size is equal to the utility bill, assume simple electricity pricing for now
    auto demand  = agent_->params[EParamTypes::ElectricityConsumption]/constants::NUMBER_DAYS_IN_MONTH;

    
    //solar irradiation - average number
    auto solar_irradiation = WorldSettings::instance().params_exog[EParamTypes::AverageSolarIrradiation];
    
    int N_PANELS = std::ceil(demand / ((solar_irradiation) * dec_design->PV_module->efficiency * (dec_design->PV_module->length * dec_design->PV_module->width/1000000) * (1 - WorldSettings::instance().params_exog[EParamTypes::DCtoACLoss])));
    double DC_size = N_PANELS * dec_design->PV_module->efficiency * dec_design->PV_module->length * dec_design->PV_module->width / 1000;

    //size of one panel
    double module_area = dec_design->PV_module->length * dec_design->PV_module->width / 1000000;
    
    //system area size
    double system_area = N_PANELS * module_area;
    //convert sq.feet into sq. meters
    double roof_area = constants::NUMBER_SQM_IN_SQF * agent_->house->roof_size;
    double available_area = std::min(roof_area, system_area);
    
    
    //update to the actual available area
    N_PANELS = std::ceil(available_area/module_area);
    DC_size = N_PANELS * dec_design->PV_module->efficiency * dec_design->PV_module->length * dec_design->PV_module->width / 1000;
    
    
    //price per watt is predetermined by the optimality choice
    auto p = params[EParamTypes::EstimatedPricePerWatt] * DC_size;
    
    //calculate irr given the parameters for this project
    project->N_PANELS = N_PANELS;
    project->DC_size= DC_size;
    project->agent = agent_;
    project->AC_size = project->DC_size * (1 - WorldSettings::instance().params_exog[EParamTypes::DCtoACLoss]);
    project->p = p;
    project->PV_module = dec_design->PV_module;
    project->sei = this;
    
    //find irr
    //secant method
    project->irr_a = irr_secant(project.get());
    
    
    
    return project;
    
}




double SEIBL::irr_secant(PVProjectFlat* project)
{
    //algorithm parameters
    double precision = 0.00001; //tolerance for convergence
    auto N_iterations = 10; //max number of iterations
    
    //guess starting points
    auto r_n = 0.05;
    auto r_n_1 = 0.06;
    auto r_n1 = r_n + precision + 0.01;

    auto i = 0;

    
    while ((i < N_iterations) && std::abs(r_n1 - r_n) > precision)
    {
        r_n1 = r_n - NPV_purchase(project, r_n) * (r_n - r_n_1)/(NPV_purchase(project, r_n) - NPV_purchase(project, r_n_1));
        ++i;
    };
    
    return r_n1;
}
/**

 Replicates financial calculator from Python
 
*/
double SEIBL::NPV_purchase(PVProjectFlat* project, double irr)
{
    //solar irradiation - average number
    auto solar_irradiation = WorldSettings::instance().params_exog[EParamTypes::AverageSolarIrradiation];
    auto total_savings = 0.0;
    auto AC_size_t = project->AC_size;
    auto degradation_t = std::exp(std::log((1 - project->PV_module->degradation))/WorldSettings::instance().params_exog[EParamTypes::DegradationDefinitionLength]);
    auto inflation = WorldSettings::instance().params_exog[EParamTypes::InflationRate];
    auto production_t = 0.0;
    auto ElectricityPriceUCDemand_t = WorldSettings::instance().params_exog[EParamTypes::ElectricityPriceUCDemand];
    auto ElectricityPriceUCSupply_t = WorldSettings::instance().params_exog[EParamTypes::ElectricityPriceUCSupply];
    //purchase financials
    //MARK: cont. include tax incentives here
    auto NPV_purchase = - project->p;
    auto NPV_loan = 0.0;
    
    //loan payments
    auto loan_amount = project->p;
    //monthly payments, warranty length is in years, assume that interest rate is in yearly terms
    auto N_loan = project->PV_module->warranty_length * 12;
    //monthly payments for the loan
    auto loan_annuity = (WorldSettings::instance().params_exog[EParamTypes::AverageInterestRateLoan]/12)/(1 - std::pow((1 + WorldSettings::instance().params_exog[EParamTypes::AverageInterestRateLoan]/12), -N_loan)) * loan_amount;
    
    for (auto i = 0; i < project->PV_module->warranty_length; ++i)
    {
        //panel output
        production_t = AC_size_t * solar_irradiation * constants::NUMBER_DAYS_IN_YEAR;
        total_savings = total_savings + production_t * ElectricityPriceUCSupply_t;
        
        NPV_purchase += production_t * ElectricityPriceUCSupply_t / std::pow(1 + irr, i);
        //NPV loan income part
        NPV_loan += production_t * ElectricityPriceUCSupply_t / std::pow(1 + irr, i);
        //NPV loan costs part
        NPV_loan -= (loan_annuity * 12) / std::pow(1 + irr, i);
        
        //degradation of a panel
        AC_size_t = AC_size_t * degradation_t;
        
        //escalation of electricity price
        ElectricityPriceUCSupply_t = ElectricityPriceUCSupply_t * (1 + inflation);
        //escalation of electricity price
        ElectricityPriceUCDemand_t = ElectricityPriceUCDemand_t * (1 + inflation);
        
    };
    
    return NPV_purchase;
    
};




TDesign* SEIBL::dec_base()
{
    TDesign* dec = nullptr;
    
    
    //makes decision about switching to new design or not
    //price \f$ p_{n} \f$
    double p_n = 0.0;
    //price \f$ p_{n+1} \f$, and p_n_1 will be \f$ p_{n-1} \f$
    double p_n1 = 0.0;
    
    
    //allocate PVProjectFlat
    auto average_project = new PVProjectFlat();
    

    //estimates expected profit for the current module
    auto dec_design_hat = dec_design; /// current design first
    p_n = max_profit(dec_design_hat, average_project);
    
    //draw random SEM

    auto max_ = w->sems->size() - 1;
    auto pdf_i = boost::uniform_int<uint64_t>(0, max_);
    auto rng_i = boost::variate_generator<boost::mt19937&, boost::uniform_int<uint64_t>>(w->rand_sei->rng, pdf_i);
    
    auto i = rng_i();
    
    //MARK: cont. think if it needs new or this is OK
    auto test_design = new TDesign(*dec_design);
    test_design->PV_module = nullptr;
    
    (*w->sems)[i]->lock.lock();
    //check if it is new techonology
    if ((*w->sems)[i]->solar_panel_templates.at(EDecParams::NewTechnology) != nullptr)
    {
        test_design->PV_module = (*w->sems)[i]->solar_panel_templates[EDecParams::NewTechnology];
    }
    else
    {
        if (dec_design->PV_module != (*w->sems)[i]->solar_panel_templates[EDecParams::CurrentTechnology])
        {
            test_design->PV_module = (*w->sems)[i]->solar_panel_templates[EDecParams::CurrentTechnology];
        };
    };
    (*w->sems)[i]->lock.unlock();
    
    if (test_design->PV_module != nullptr)
    {
        
        //reset to presets
        //might be using SEM specific prior for new models? later
        test_design->THETA_reliability = THETA_reliability_prior;
        test_design->THETA_complexity = THETA_complexity_prior;
        test_design->complexity_install = complexity_install_prior;
        //assume price is the same for everyone
        test_design->p_module = test_design->PV_module->p_sem;
        
        
        p_n1 = max_profit(test_design, average_project);
        
        //evaluate new design
        auto profit_time = est_profit(dec_design_hat, average_project, p_n);
        auto profit_new = est_profit(test_design, average_project, p_n1);
        
        //calculate expected profit for other random design (from random SEM)
        //compare - if higher, than have chance to switch and start offering it
        //prob of switching depends on the distance between expected profits and attitide towards switching
        auto p_switch = exploration_p(profit_new, profit_time);
        
        //draw and see if switches
        if (w->rand_sei->ru() <= p_switch)
        {
            //inform SEM about switch
            dynamic_cast<SEMBL*>(dec_design->PV_module->manufacturer)->remove_connection(dec_design->PV_module);
            dynamic_cast<SEMBL*>(test_design->PV_module->manufacturer)->add_connection(test_design->PV_module);
            
            //if switches
            dec = test_design;
            test_design->p_design = p_n1;
        }
        else
        {
            dec = dec_design;
            delete test_design;
            //save estimated price
            dec_design->p_design = p_n;
        };
        

    }
    else
    {
        dec = dec_design;
        //save estimated price
        dec_design->p_design = p_n;
    };

    
    delete average_project;
    
    return dec;
}



/**
 
 Returns probability to switch
 
 */
double SEIBL::exploration_p(double profit_new, double profit_old)
{
    //Logistic function (from cdf of the distribution)
    return (1/(1+std::exp(-(profit_new/profit_old - THETA_exploration[0])/THETA_exploration[1])));
    
}



double SEIBL::f_derivative(double epsilon, TDesign* dec_design_hat, PVProjectFlat* project, double x)
{
    return (est_profit(dec_design_hat, project, x + epsilon) - est_profit(dec_design_hat, project, x))/epsilon;
}


double SEIBL::max_profit(TDesign* dec_design_hat, PVProjectFlat* project)
{
    
    /// method for searching for opt solution
    /// use gradient descent
    /// https://en.wikipedia.org/wiki/Gradient_descent
    
    double epsilon = 0.01; //step in derivative
    double x_old = params[EParamTypes::EstimatedPricePerWatt]; // The value does not matter as long as abs(x_new - x_old) > precision
    double x_new = x_old + epsilon; // The algorithm starts here
    double gamma = 0.01; //step size
    double precision = 0.00001; //tolerance for convergence
    int N_steps = 100; //max number of steps
    
    
//    auto f_derivative = [epsilon, &dec_design_hat, &project, this](double x)->double {return (this->est_profit(dec_design_hat, project, x + epsilon) - this->est_profit(dec_design_hat, project, x))/epsilon;};
   
    auto i = 0;
    while ((std::abs(x_new - x_old) > precision) && (i < N_steps))
    {
        x_old = x_new;
        x_new = x_old - gamma * f_derivative(epsilon, dec_design_hat, project, x_old);
        ++i;
    };
    
    
    //if negative return 0.0
    return std::max(x_new, 0.0);
}





double SEIBL::est_irr_from_params(TDesign* dec_design_hat, PVProjectFlat* project, double p)
{
    return irr_secant(project);
}



double SEIBL::est_profit(TDesign* dec_design_hat, PVProjectFlat* project, double p)
{
    
    double inflation = WorldSettings::instance().params_exog[EParamTypes::InflationRate];
    
    ///estimates profit given the proposed price
    //estimate number of panels for an average utility bill
    auto demand = WorldSettings::instance().params_exog[EParamTypes::AverageElectricityDemand]/constants::NUMBER_DAYS_IN_MONTH;
    //solar irradiation - average number
    auto solar_irradiation = WorldSettings::instance().params_exog[EParamTypes::AverageSolarIrradiation];
    //permit length in labor*hours
    auto permit_difficulty = WorldSettings::instance().params_exog[EParamTypes::AveragePermitDifficulty];
    
    
    int N_PANELS = std::ceil(demand / ((solar_irradiation) * dec_design_hat->PV_module->efficiency * (dec_design_hat->PV_module->length * dec_design_hat->PV_module->width/1000000) * (1 - WorldSettings::instance().params_exog[EParamTypes::DCtoACLoss])));
    
    double DC_size = N_PANELS * dec_design_hat->PV_module->efficiency * dec_design_hat->PV_module->length * dec_design_hat->PV_module->width / 1000;

    //create average project for this design
    project->PV_module = dec_design_hat->PV_module;
    project->DC_size = DC_size;
    project->AC_size = project->DC_size * (1 - WorldSettings::instance().params_exog[EParamTypes::DCtoACLoss]);
    project->p = DC_size * p;

    
    
    ///calculate promised irr for the TDesign given average utility bill and other parameters for the design, assume that spends p requested, and savings are equal to the savings on electricity bill over the warranty length, assume increase in electricity prices, and degradation standard. Do not include down time due to maintenance - assume that it will be ideal conditions (as a sales pitch). For estimates of the profit will use actual estimated maintenance costs
    double irr_hat = est_irr_from_params(dec_design_hat, project, p);
    
    
    //given price parameter - get number of projects
    //estimated number of projects is Bayesian Linear Regression
    //parameters are own offered irr, offered irr of others, reputation, reputation of others
    //use ceil to get int number
    double N_hat = std::min(std::ceil(THETA_demand[0] +
                             THETA_demand[1] * irr_hat +
                             THETA_demand[2] * (THETA_reputation[0] > 1.0? THETA_reputation[1]/(THETA_reputation[0] - 1) : 1.0) +
                             THETA_demand[3] * X(0, 2) +
                             THETA_demand[4] * X(0, 3)), 1.0);
    
    //adjust for the general market size
    N_hat = N_hat * WorldSettings::instance().params_exog[EParamTypes::TotalPVMarketSize];
    
    //??? some other type of estimation
    double wage = WorldSettings::instance().params_exog[EParamTypes::LaborPrice];
    
    
    //repeat over estimated planned horizon of 5 years for keeping the same type of equipment
    int T_planning = 5;
    double N_hat_t = 0.0;
    double complexity_install_t = dec_design_hat->complexity_install;
    double costs_t = 0.0;
    double w_t = 0.0;
    double profit_t = 0.0;
    double profit = 0.0;
    double irr_ = 0.05;
    
    for (auto t = 0; t < T_planning; ++t)
    {
        //adjust for inflation
        w_t = wage * std::pow(1 + inflation, t);
        
        //calculate expected sales, assume the same formula
        N_hat_t = N_hat;
        
        //calculate installation costs - lower, as assume learning-by-doing
        costs_t += w_t * complexity_install_t * N_hat_t;
        
        //calculate expected maintenance costs (discounted)
        costs_t += est_maintenance(dec_design_hat, N_hat_t, w_t);
        
        //calculate materials costs, only panels, no inverters here
        costs_t += N_PANELS * N_hat_t * dec_design_hat->p_module;

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
        
        //accumulate profit
        profit += profit_t / std::pow(1 + irr_, t);
        
        
        //update parameters
        complexity_install_t = dec_design_hat->complexity_install * (dec_design_hat->BETA_complexity_time);
        profit_t = 0.0;
        costs_t = 0.0;
        

        
    };

    return profit;
    
}





double SEIBL::est_maintenance(TDesign* dec_design_hat, std::size_t N_hat, double wage)
{
    ///number of simulation runs
    std::size_t N_trials = 10;
    
    //calculate expected maintenance
    //draw time before next maintenance
    auto rng_THETA_reliability = [&]()
    {
        return w->rand_sei->r_pareto_2(dec_design_hat->THETA_reliability[1], dec_design_hat->THETA_reliability[0]);
    };
    

    auto pdf_THETA_complexity_base = boost::random::student_t_distribution<>(2 * dec_design_hat->THETA_complexity[2]);
    auto rng_THETA_complexity_base = boost::variate_generator<boost::mt19937&, boost::random::student_t_distribution<>>(w->rand_sei->rng, pdf_THETA_complexity_base);
    auto rng_THETA_complexity = [&]()
    {
        return dec_design_hat->THETA_complexity[0] + rng_THETA_complexity_base() * (dec_design_hat->THETA_complexity[3] * (dec_design_hat->THETA_complexity[1] + 1))/(dec_design_hat->THETA_complexity[1] * dec_design_hat->THETA_complexity[2]);
    };
    

    double x = 0.0; //time before next breakdown
    double y = 0.0; //complexity of a repair
    double t = 0.0; //time of a last breakdown
    double irr_ = 0.05; //rate of financing for this installer - discounting rate for its profit calculations
    double maintenance_n_hat = 0.0; //maintenance for one simulation run
    double maintenance_hat = 0.0; //average maintenance over all simulation runs
    double inflation = WorldSettings::instance().params_exog[EParamTypes::InflationRate];
//    std::vector<double> cash_flow_maintenance_n{dec_design_hat->PV_module->warranty_length, 0.0};
    int64_t N_hat_i = std::min(N_hat, N_trials * 1);
    
    
    //estimation for the period t, labor price is constant - change to dynamic labor price estimation in the future
    for (auto n = 0; n < N_trials; ++n)
    {
        //for each project
        for (auto i = 0; i < N_hat_i; ++i)
        {
            while (true)
            {
                //time until next breakdown in years
                x = rng_THETA_reliability();
                t += x;
                
                //if it is still in warranty - spend labor*hours to repair
                if (t <= dec_design_hat->PV_module->warranty_length)
                {
                    //draw complexity of the breakdown, in labor*hours
                    y = rng_THETA_complexity();
                    //and labor costs to repair it adjusted for inflation
                    maintenance_n_hat += (y * wage * std::pow(1 + inflation, t))/std::pow(1 + irr_, t);
                    
                    //? need condition of non-negativity for cashflows
//                    cash_flow_maintenance_n[t] = (y * wage * std::pow(1 + inflation, t));
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
    maintenance_hat = (maintenance_hat * N_hat / N_hat_i)/(N_trials);

    return maintenance_hat;
    
}



void
SEIBL::wm_update()
{
    
    //collect average reputations for top installers in term of shares
    //collect posted irr for them, average
    
    X(0,1) = (THETA_reputation[0] > 1.0 ? THETA_reputation[1]/(THETA_reputation[0] - 1) : 1.0);
    X(0,2) = dec_design->irr;
    X(0,3) = w->get_inf(EDecParams::Reputation_i, this);
    X(0,4) = w->get_inf(EDecParams::irr_i, this);
    //observed share over the past year - y
    Y(0,0) = w->get_inf(EDecParams::Share, this);
    

    //implement BLR (https://en.wikipedia.org/wiki/Bayesian_linear_regression )
    //details http://www.biostat.umn.edu/~ph7440/pubh7440/BayesianLinearModelGoryDetails.pdf
    //assume incremental updates
    Mu_n = (X.transpose() * X + V_0.inverse()).inverse() * (V_0.inverse() * Mu_0 + X.transpose() * Y);
    V_n = (X.transpose() * X + V_0.inverse()).inverse();
    a_n = a_0 + 0.5;
    b_n = b_0 + (0.5 * (Y.transpose() * Y + Mu_0.transpose() * V_0.inverse() * Mu_0 + Mu_n.transpose() * V_n.inverse() * Mu_n))(0,0);
    
    //update to new values
    Mu_0 = Mu_n;
    V_0 = V_n;
    a_0 = a_n;
    b_0 = b_n;
    
    
    //data generating will be MVS - take mean from there - see predictions using BLR
    for (auto i = 0; i < THETA_demand.size() ; ++i)
    {
        THETA_demand[i] = Mu_0[i];
    };
    
    //update reputation
    //updates parameters for distribution if new information about performance is here
    //goes through actual production in the previous year, calculate percentage of the promised production,
    //update Inv-Gamma with new estimate in THETA_reputation, given the new data point
    //assume method of moments with only parameter being  \f$/alpha \$f
    double mean = 0.0;
    auto i_max = pvprojects.size() - 1;
    for(auto i = 0; i < i_max ; ++i)
    {
        mean += pvprojects[i]->production_time;
    };
    
    if (i_max > 0)
    {
        //average production
        mean = mean / i_max;
        
        //number of data points, assume update every step
        uint64_t n = a_time;
        
        if (n > 0)
        {
            //updated parameter for reputation
            THETA_reputation[0] = (1 + 1/ (1/(THETA_reputation[0] - 1)*(n/(n+1)) + mean / (n+1)));
        };
    };
}



/**
 
 
 
 
 */
void SEIBL::install_project(std::shared_ptr<PVProjectFlat> project_, TimeUnit time_)
{
    //fill in details for the project
    //uses true distribution here - exponential
    auto rng_THETA_reliability = [&](double lambda)
    {
        
        return boost::variate_generator<boost::mt19937&, boost::exponential_distribution<>>(w->rand_sei->rng, boost::exponential_distribution<>(lambda))();
    };

    
    
    //draw next time of maintenance
    project_->begin_time = time_;
    //will break when, might be longer than project_->PV_module->warranty_length
    project_->maintenance_time = rng_THETA_reliability(dec_design->PV_module->THETA_reliability[0]) + time_;
    //start counter for length before break-down from the start of the project
    project_->maintenance_time_1 = time_;
    
    
    w->get_state_inf_installed_project(project_);
    
    pvprojects.push_back(project_);
    
}


void SEIBL::projects_update()
{
    
    //go though installed projects, record failures
    std::map<UID, std::vector<double>> failures;
    //go though installed projects record failure complexity
    std::map<UID, std::vector<double>> failures_complexity;

    
    //uses true distribution here
    auto rngs_THETA_reliability_base = new std::map<UID, boost::variate_generator<boost::mt19937*, boost::exponential_distribution<>>*>();
    
    for (const auto& iter:designs)
    {
        rngs_THETA_reliability_base->emplace(std::make_pair(iter.first, new boost::variate_generator<boost::mt19937*, boost::exponential_distribution<>>(&w->rand_sei->rng, boost::exponential_distribution<>(iter.second->PV_module->THETA_reliability[0]))));
    };

    
    //reliability generator
    auto rng_THETA_reliability = [](boost::variate_generator<boost::mt19937*, boost::exponential_distribution<>>* rng_THETA_reliability_base_)
    {
        return (*rng_THETA_reliability_base_)();
    };
    
    
    
    //complexity generators
    auto rng_THETA_complexity = [](boost::variate_generator<boost::mt19937*, boost::normal_distribution<>>* rng_THETA_complexity_base_, std::shared_ptr<SolarModuleBL> pv_module_)
    {
        return pv_module_->THETA_complexity[0] + (*rng_THETA_complexity_base_)() * std::pow(pv_module_->THETA_complexity[1], 0.5);
    };

    auto rngs_THETA_complexity_base = new std::map<UID, boost::variate_generator<boost::mt19937*, boost::normal_distribution<>>*>();
    for (auto iter:designs)
    {
        rngs_THETA_complexity_base->emplace(std::make_pair(iter.first, new boost::variate_generator<boost::mt19937*, boost::normal_distribution<>>(&w->rand_sei->rng, boost::normal_distribution<>(0.0, 1.0))));
    };
    
    
    
    auto i_max = pvprojects.size() - 1;
    for (int64_t i = i_max; i >= 0; --i)
    {
        if (pvprojects[i]->maintenance_time <= a_time)
        {
            //record as failure
            failures[pvprojects[i]->PV_module->uid].push_back(pvprojects[i]->maintenance_time);
            
            //restart counter until next break-down
            pvprojects[i]->maintenance_time_1 = a_time;
            
            //draw next maintenance time
            pvprojects[i]->maintenance_time = rng_THETA_reliability(rngs_THETA_reliability_base->at(pvprojects[i]->PV_module->uid)) + a_time;
            
            //draw maintenance complexity
            pvprojects[i]->maintenance_complexity = rng_THETA_complexity(rngs_THETA_complexity_base->at(pvprojects[i]->PV_module->uid), pvprojects[i]->PV_module);
            
            //save actual complexity level
            failures_complexity[pvprojects[i]->PV_module->uid].push_back(pvprojects[i]->maintenance_complexity);
            
            
            //record into costs for the period
            costs_time += pvprojects[i]->maintenance_complexity * WorldSettings::instance().params_exog[EParamTypes::LaborPrice] ;
            
            //records down period into production time, hard coded that TimeUnit here is 1 year - so share of down time as a percentage of days in a year - 365.
            //because maintenance complexity is in labor*hours and solar radiation defines number of hours per day solar panels are active, the result will be number of days solar system is down, convert into percentage of a year
            pvprojects[i]->production_time = pvprojects[i]->maintenance_complexity / (WorldSettings::instance().params_exog[EParamTypes::AverageSolarIrradiation]) / (constants::NUMBER_DAYS_IN_TICK);
            
        }
        else
        {
            pvprojects[i]->production_time = 1.0; // always up, no failures this year
        };
    };
    
    
    
    //updates Gamma - prior to Gamma posterior, with Exponential data
    for (auto failure:failures)
    {
        //\f$ a' = a + n \f$
        designs[failure.first]->THETA_reliability[0] += failure.second.size();
        //\f$ b' = b + \sum_{i}x_{i} \f$
        designs[failure.first]->THETA_reliability[1] += std::accumulate(failure.second.begin(), failure.second.end(), 0.0);
    };
    
    
    double mu_complexity_n = 0.0;
    double v_complexity_n = 0.0;
    double alpha_complexity_n = 0.0;
    double beta_complexity_n = 0.0;
    //go though each panel and update estimates for reliability and complexity
    for (auto failure:failures_complexity)
    {
        //transform into Eigen X matrix (n,1)
        Eigen::Map<Eigen::MatrixXd> X(failure.second.data(), failure.second.size(), 1);
        Eigen::MatrixXd centered = X.rowwise() - X.colwise().mean();
        
        auto X_bar = X.mean();
        auto n = failure.second.size();
        auto design = designs[failure.first];
        
        //cont. updating posterior normal-inverse-gamma for complexity - formulas is in https://en.wikipedia.org/wiki/Conjugate_prior - for normal with mu and sigma
        mu_complexity_n = (design->THETA_complexity[1] * design->THETA_complexity[0] + n * X_bar) / (design->THETA_complexity[1] + n);
        v_complexity_n = design->THETA_complexity[1] + n;
        alpha_complexity_n = design->THETA_complexity[2] + 0.5 * n;
        beta_complexity_n = design->THETA_complexity[3] + 0.5 * (centered.transpose() * centered)(0,0) + 0.5 * (n * design->THETA_complexity[1]) / (design->THETA_complexity[1]+ n) * std::pow(X_bar - design->THETA_complexity[0], 0.5);
        
        
        design->THETA_complexity[0] = mu_complexity_n;
        design->THETA_complexity[1] = v_complexity_n;
        design->THETA_complexity[2] = alpha_complexity_n;
        design->THETA_complexity[3] = beta_complexity_n;
        
        
    };

    
    //MARK: cont. clean after itself
    //delete new allocations
    for (auto& iter:*rngs_THETA_reliability_base)
    {
        delete iter.second;
    };
    
    for (auto& iter:*rngs_THETA_complexity_base)
    {
        delete iter.second;
    };
    
    delete rngs_THETA_reliability_base;
    delete rngs_THETA_complexity_base;
    
}



void SEIBL::ac_update_tick()
{
    //update internal timer
    a_time = w->time;
    
    
    //add projects to add to avoid iterating over changing collection
}


/**
 
 
 tick is moved to be a year
 
*/
void SEIBL::act_tick()
{
    //update basic parameters
    ac_update_tick();
    
    
    //skip first update
    if (a_time > 0)
    {
        //updates information for decision making
        wm_update();
    };
    //projects update
    //go over installed projects and see if need maintenance, if need - record as costs and update time till next maintenance
    //record actual production in the year based on the maintenance length
    projects_update();
    
    //make price decision, based on the switching or not
    auto dec = dec_base();
//    TDesign* dec = dec_design;
    
    //switch to new offering, lock as simulateneously is offering projects to H
    lock.lock();
    if (dec_design != dec)
    {
        dec_design = dec;
        designs[dec->PV_module->uid] = dec;
        params[EParamTypes::EstimatedPricePerWatt] = dec_design->p_design;
    };
    lock.unlock();
    
    
}



