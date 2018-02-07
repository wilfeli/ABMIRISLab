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
#include "Tools/SerializeRJ.h"
#include "Tools/RJTools.h"
#include "Tools/Log.h"
#include "UI/W.h"
#include "Agents/SolarPanel.h"
#include "Agents/Homeowner.h"
#include "Agents/SEI.h"

#define ABMS_FUDGING_UTILITY

using namespace solar_core;



std::set<EParamTypes> Homeowner::project_states_to_delete{EParamTypes::ClosedProject};

Homeowner::Homeowner(){}


Homeowner::Homeowner(const PropertyTree& pt_, W* w_) :a_time{ 0 }, get_inf_marketing_sei{10, nullptr}
{
    w = w_;
    
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

	std::size_t N_MARKETING_FROM_SEI_INIT_HO = 10;
	//reserve some space 
	
	for (auto i = 0; i < N_MARKETING_FROM_SEI_INIT_HO; ++i) 
	{
		get_inf_marketing_sei.push_back(nullptr);
		index_inf_marketing_sei.push_back(i);
	};
}

Homeowner::Homeowner(const DocumentRJ& pt_, W* w_):a_time{ 0 }, get_inf_marketing_sei{ 10, nullptr }
{


	w = w_;

	//location
	location_x = tools::get_double(pt_["location_x"]);
	location_y = tools::get_double(pt_["location_y"]);

	//House
	house = new House(pt_["House"]);

	quote_state = EnumFactory::ToEParamTypes(tools::get_string(pt_["quote_state"]));

	schedule_visits = std::vector<std::vector<std::weak_ptr<PVProject>>>(WorldSettings::instance().constraints[EConstraintParams::MaxLengthWaitPreliminaryQuote], std::vector<std::weak_ptr<PVProject>>{});
	i_schedule_visits = 0;

	quote_stage_timer = 0;
	n_preliminary_quotes = 0;
	n_pending_designs = 0;
	n_preliminary_quotes_requested = 0;

	project_states_to_delete.insert(EParamTypes::ClosedProject);

	std::size_t N_MARKETING_FROM_SEI_INIT_HO = 10;
	//reserve some space 

	for (auto i = 0; i < N_MARKETING_FROM_SEI_INIT_HO; ++i)
	{
		get_inf_marketing_sei.push_back(nullptr);
		index_inf_marketing_sei.push_back(i);
	};
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

	index_inf_marketing_sei_lock.lock();
    //saves information about advertising agent only if not already commited to installing project
    if ((marketing_state != EParamTypes::HOMarketingStateNotAccepting) && (quote_state != EParamTypes::HOStateCommitedToInstallation) && (quote_state != EParamTypes::HOStateInterconnected))
    {
		if (!index_inf_marketing_sei.empty())
		{
			auto index = index_inf_marketing_sei.front();
			get_inf_marketing_sei[index] = mes_;
			index_inf_marketing_sei.pop_front();
		}
		else 
		{
			get_inf_marketing_sei.push_back(mes_);
		};
        
        //inform world that is now active, if was inactive before
        if (marketing_state != EParamTypes::HOMarketingStateInterested)
        {
            marketing_state = EParamTypes::HOMarketingStateInterested;
            quote_state = EParamTypes::None;
            //tell world that is now interested, that it is moved to the list of active agents. Once the project is finished it will be moved from the list of active agents
            w->get_state_inf(this, marketing_state);
        };
        
        
#ifdef ABMS_DEBUG_MODE
        //check if is in active agents, should be by now
        bool FLAG_IN_ACTIVE = false;
        for (auto agent:*w->hos)
        {
            if (agent == this)
            {
                FLAG_IN_ACTIVE = true;
                break;
            };
            
        };
        
        if (!FLAG_IN_ACTIVE)
        {
            throw std::runtime_error("did not add inself into active agents");
        };
#endif
        
        
    };
    
	index_inf_marketing_sei_lock.unlock();
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
	for (auto i = 0; i < get_inf_marketing_sei.size(); ++i) 
	{
		//check that it is valid information
		if (get_inf_marketing_sei[i]) 
		{
			const auto marketing_inf = get_inf_marketing_sei[i];
			//check that it is new sei
			auto sei = std::find_if(pvprojects.begin(), pvprojects.end(), [&](std::shared_ptr<PVProject> &project) {
				if (project)
				{
					return project->sei == marketing_inf->agent;
				}
				else
				{
					return false;
				}; });


			if (sei == pvprojects.end())
			{
				//create project
				//create new project
				auto new_project = std::make_shared<PVProject>();
				//request additional information
				new_project->agent = this;
				new_project->begin_time = a_time;
				new_project->sei = marketing_inf->agent;
				if (marketing_inf->message_type == "direct") 
				{
					new_project->is_direct_marketing = true;
				};

				///requests online quote, it will be provided in a separate call
				new_project->state_project = EParamTypes::RequestedOnlineQuote;
				//assumes that there is some interest on part of an agent by the looks of it,  but in reality is used just for asynchronous calls to SEI
				marketing_inf->agent->request_online_quote(new_project);


				//save project
				pvprojects.push_back(new_project);
				marketing_inf->agent->get_project(new_project);

			};


			//add to empty pull
			get_inf_marketing_sei[i] = nullptr;
			index_inf_marketing_sei_lock.lock();
			index_inf_marketing_sei.push_back(i);
			index_inf_marketing_sei_lock.unlock();

		};
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
    
    //TODO clean up to initialize to true
    auto pool = std::vector<bool>(pvprojects.size(), false);
    
    for (auto i = 0; i < pvprojects.size(); ++i)
    {
        //check if is in the pool
        //check on Customer rating
        //if (pvprojects[i]->sei->params[EParamTypes::SEIRating] >= THETA_NCDecisions[EParamTypes::HONCDecisionSEIRating][0])
        //{
        //    pool[i] = true;
        //};

		pool[i] = true;
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
            pvprojects[i]->ac_hh_time = a_time;
        };
        
    };
    
    
    
    if ((n_preliminary_quotes_requested <= 0.0) && (pool.size() > 0.0))
    {
        quote_state = EParamTypes::HOStateDroppedOutNCDecStage;
        clean_after_dropout();
    }
    else
    {
        quote_state = EParamTypes::HOStateWaitingOnPreliminaryQuotes;
        marketing_state = EParamTypes::HOMarketingStateNotAccepting;
    };
    
    
    if (pool.size() == 0.0)
    {
        throw std::runtime_error("too early for evaluation stage");
    };
    
    
    
}



double
Homeowner::estimate_sei_utility_from_params(std::shared_ptr<PVProject> project, DecisionParams& THETA)
{
    double utility = 0.0;
    
    //here installers are evaluated
    //preliminary quote will have general savings estimation
    //collect all parameters for decisions
    utility += THETA[EParamTypes::SEIRating][project->sei->params[EParamTypes::SEIRating] - 3];
    
    //discrete variables have position in the vector of coefficients as their value. Fragile.
    utility += THETA[EParamTypes::SEIInteractionType][project->sei->params[EParamTypes::SEIInteractionType]];
    
    
    //number inside is type of equipment - directly corresponds to the position in the vector
    utility += THETA[EParamTypes::SEIEquipmentType][project->sei->params[EParamTypes::SEIEquipmentType]];
    
    //estimated project total time
    //LeadIn time if fixed for each installer and is an estimation
    //Permitting time depends on the location, but is an estimate
    int NUMBER_TICKS_IN_MONTH = 4;
    utility += THETA[EParamTypes::HOSEIDecisionTotalProjectTime][0] * project->preliminary_quote->params[EParamTypes::PreliminaryQuoteTotalProjectTime] / NUMBER_TICKS_IN_MONTH;
    
    //warranty
    //MARK: CAREFULL Warranty is in month in SEI definition and in years in conjoint
    //MARK: ERRORS IN LOGIC
//    std::map<double, int64_t> warranty_map{{5.0, 0.0}, {15.0, 1.0}, {25.0, 2.0}};
//    utility += THETA[EParamTypes::SEIWarranty][warranty_map[project->preliminary_quote->params[EParamTypes::SEIWarranty]]];
    
    
    //continious case
    utility += THETA[EParamTypes::SEIWarranty][0] * project->preliminary_quote->params[EParamTypes::SEIWarranty];
    
    
    //savings are estimated for an average homeowner
    utility += THETA[EParamTypes::HOSEIDecisionEstimatedNetSavings][0] * project->preliminary_quote->params[EParamTypes::PreliminaryQuoteEstimatedNetSavings];


#ifdef ABMS_DEBUG_MODE
	if (project->preliminary_quote->params[EParamTypes::PreliminaryQuoteEstimatedNetSavings] < 0.0)
	{
//		throw std::runtime_error("Negative savings passing DecSEI");
	}
	else 
	{
//		std::cout << "Positive savigns with low electricity prices" << std::endl; 
	};
#endif



#ifdef ABMS_FUDGING_UTILITY
	//warranty 
	//auto


	//savings 
	//skip



	//project time
	//0.5 - 4
	if ((project->preliminary_quote->params[EParamTypes::PreliminaryQuoteTotalProjectTime] / NUMBER_TICKS_IN_MONTH > 4) 
		|| (project->preliminary_quote->params[EParamTypes::PreliminaryQuoteTotalProjectTime] / NUMBER_TICKS_IN_MONTH < 0.5))
	{
		throw std::runtime_error("error in fudging");
	};




#endif


    
    return utility;
    
}




double
Homeowner::estimate_sei_utility_from_params(std::shared_ptr<PVProject> project,
											long label_i)
{

	double utility = 0.0;
	double x_i = 0.0;
	double y_i = 0.0;
	EParamTypes param = EParamTypes::None;
	auto THETA = w->ho_decisions[EParamTypes::HOSEIDecision]->HOD_distribution_scheme
		[w->ho_decisions[EParamTypes::HOSEIDecision]->labels[label_i]];



	//here installers are evaluated
	//preliminary quote will have general savings estimation
	//collect all parameters for decisions
	utility += THETA[EParamTypes::SEIRating][project->sei->params[EParamTypes::SEIRating] - 3];

	//discrete variables have position in the vector of coefficients as their value. Fragile.
	utility += THETA[EParamTypes::SEIInteractionType][project->sei->params[EParamTypes::SEIInteractionType]];


	//number inside is type of equipment - directly corresponds to the position in the vector
	utility += THETA[EParamTypes::SEIEquipmentType][project->sei->params[EParamTypes::SEIEquipmentType]];

	//estimated project total time
	//LeadIn time if fixed for each installer and is an estimation
	//Permitting time depends on the location, but is an estimate
	int NUMBER_TICKS_IN_MONTH = 4;
	x_i = project->preliminary_quote->params[EParamTypes::PreliminaryQuoteTotalProjectTime] / NUMBER_TICKS_IN_MONTH;
	param = EParamTypes::HOSEIDecisionTotalProjectTime;
	y_i = w->ho_decisions[EParamTypes::HOSEIDecision]->get_spline_value(param, x_i, label_i);

	//it is part worth, so no multiplication here
	utility += y_i;

	//warranty
	//MARK: CAREFULL Warranty is in month in SEI definition and in years in conjoint
	//MARK: ERRORS IN LOGIC
	std::map<double, int64_t> warranty_map{{5.0, 0.0}, {15.0, 1.0}, {25.0, 2.0}};
	utility += THETA[EParamTypes::SEIWarranty][warranty_map[project->preliminary_quote->params[EParamTypes::SEIWarranty]]];


	////continious case
	//utility += THETA[EParamTypes::SEIWarranty][0] * project->preliminary_quote->params[EParamTypes::SEIWarranty];


	//savings are estimated for an average homeowner
	x_i = project->preliminary_quote->params[EParamTypes::PreliminaryQuoteEstimatedNetSavings];
	param = EParamTypes::HOSEIDecisionEstimatedNetSavings;
	y_i = w->ho_decisions[EParamTypes::HOSEIDecision]->get_spline_value(param, x_i, label_i);
	utility += y_i;




	return utility;

}





double
Homeowner::estimate_sei_utility(std::shared_ptr<PVProject> project)
{
    
    
#ifndef ABMS_SPLINES
	return estimate_sei_utility_from_params(project, THETA_SEIDecisions);
#endif

#ifdef ABMS_SPLINES
	return estimate_sei_utility_from_params(project, decision_scheme_SEIDecision);
#endif    
}



void Homeowner::dec_evaluate_online_quotes_nc()
{
    //here additional NCDec after get generic prices
    auto pool = std::vector<bool>(pvprojects.size(), false);
    int64_t N_PASSED_PROJECTS = 0;
    int64_t N_ELIGIBLE_PROJECTS = 0;
    
    for (auto i = 0; i < pvprojects.size(); ++i)
    {
        //if there is quote
        if (pvprojects[i]->preliminary_quote)
        {
            ++N_ELIGIBLE_PROJECTS;
            //check if is in the pool
            //check on Customer rating
            if (pvprojects[i]->preliminary_quote->params[EParamTypes::PreliminaryQuotePrice] <= THETA_NCDecisions[EParamTypes::HONCDecisionTotalPrice][0])
            {
                pool[i] = true;
                ++N_PASSED_PROJECTS;
            }
    #ifdef ABMS_DEBUG_MODE
            else
            {
//                std::cout << "Project price: " << pvprojects[i]->preliminary_quote->params[EParamTypes::PreliminaryQuotePrice] << " Threshold price: " << THETA_NCDecisions[EParamTypes::HONCDecisionTotalPrice][0] << " Price per watt: " << pvprojects[i]->preliminary_quote->params[EParamTypes::PreliminaryQuotePrice] / pvprojects[i]->preliminary_quote->params[EParamTypes::PreliminaryQuoteDCSize] << std::endl;
            }
    #endif
            ;
        };
    };
    
    int64_t N_CLOSED_PROJECTS = 0;
    for (auto i = 0; i < pool.size(); ++i)
    {
        //pool is now narrowed down to the subset of all open projects, request further information from them (without site visit for this step)
        if (pool[i])
        {
//            auto project = pvprojects[i];
//            project->state_project = EParamTypes::RequestedPreliminaryQuote;
//            project->sei->request_preliminary_quote(project);
            
            //update number of requested quotes
//            ++n_preliminary_quotes_requested;
        }
        else
        {
            pvprojects[i]->state_project = EParamTypes::ClosedProject;
            pvprojects[i]->ac_hh_time = a_time;
            ++N_CLOSED_PROJECTS;
        };
        
    };
    
    
    
    if ((N_PASSED_PROJECTS <= 0.0) && (N_ELIGIBLE_PROJECTS > 0.0))
    {
        quote_state = EParamTypes::HOStateDroppedOutNCDecStage;
        clean_after_dropout();
    }
    else if (N_CLOSED_PROJECTS >= N_ELIGIBLE_PROJECTS)
    {
        //if all projects are closed - dropped out
        quote_state = EParamTypes::HOStateDroppedOutNCDecStage;
        clean_after_dropout();
    }
    else
    {
        quote_state = EParamTypes::HOStateWaitingOnPreliminaryQuotes;
        marketing_state = EParamTypes::HOMarketingStateNotAccepting;
    };
    
#ifdef ABMS_DEBUG_MODE
    if (pool.size() == 0.0)
    {
        throw std::runtime_error("too early for evaluation stage");
    };
#endif
    
    
    

    
}



void Homeowner::dec_evaluate_preliminary_quotes()
{
#ifdef ABMS_DEBUG_MODE
	bool FLAG_FUDGING_NUMBERS = false; 
#endif
    
    //NC Decisions are continued here
    dec_evaluate_online_quotes_nc();
    
    if (quote_state == EParamTypes::HOStateDroppedOutNCDecStage)
    {
        return;
    };
    
	double error = 0.0;
	error = -std::log(1 / w->rand_ho->ru() - 1);
    double utility_none = THETA_SEIDecisions[EParamTypes::HOSEIDecisionUtilityNone][0] + error;
	
    
    for (auto& project:pvprojects)
    {
        if (project->state_project == EParamTypes::ProvidedPreliminaryQuote)
        {
            //Generate random noise. Generation is here because every choice will regenerate them
            error = - std::log(1/w->rand_ho->ru() - 1);
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
#ifdef ABMS_DEBUG_MODE
				//check that it is positive savings
				if (project->preliminary_quote->params[EParamTypes::PreliminaryQuoteEstimatedNetSavings] <= 0.0)
				{
					FLAG_FUDGING_NUMBERS = true;
					decision = nullptr;
				};

#endif
                
                break;
            }
            else
            {
//                std::cout << "Utility of none higher than designs" << std::endl;
            };
        }
    };
    
    //if choose someone
    if (decision)
    {
        decision->state_project = EParamTypes::AcceptedPreliminaryQuote;
        decision->sei->accepted_preliminary_quote(decision);
        quote_state = EParamTypes::HOStateWaitingOnDesigns;


    }
    else
    {
        quote_state = EParamTypes::HOStateDroppedOutSEIStage;


#ifdef ABMS_DEBUG_MODE
		if (FLAG_FUDGING_NUMBERS)
		{
			quote_state = EParamTypes::HOStateDroppedOutNCDecStage;
		};

#endif


        clean_after_dropout();
#ifdef ABMS_DEBUG_MODE
        w->get_state_inf(this, quote_state);
#endif
    };
    
    
    //close other projects
    for(auto& project:pvprojects)
    {
        if (project->state_project == EParamTypes::ProvidedPreliminaryQuote)
        {
            if (project != decision)
            {
                project->state_project = EParamTypes::ClosedProject;
                project->ac_hh_time = a_time;
            };
        };
        
    };
    
    
    //exit evaluation stage
    n_preliminary_quotes = 0;
    n_preliminary_quotes_requested = 0;
    
    
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
	//here need 5 years because it is what is asked in the survey
    utility += THETA[EParamTypes::HODesignDecisionFailures][0] * 5 * project->design->design->failure_rate;
    
    //emmision levels
    utility += THETA[EParamTypes::HODesignDecisionCO2][0] * project->design->design->co2_equivalent;
    
    //savings are estimated for an average homeowner
    utility += THETA[EParamTypes::HODesignDecisionEstimatedNetSavings][0] * project->design->design->total_net_savings;
    


#ifdef ABMS_FUDGING_UTILITY
	//constrain efficiency 
	//15.5 - 25
	if (project->design->design->PV_module->efficiency > 25)
	{
		throw std::runtime_error("error in fudging");
	};

	//visibility
	//auto

	//inverter type 
	//auto

	//number of failures 
	//0 - 3 in five years
	if ((5 * project->design->design->failure_rate) > 3)
	{
		throw std::runtime_error("error in fudging");
	};


	//emission levels
	//3 - 9
	if (project->design->design->co2_equivalent < 3)
	{
		std::stringstream ss;

		ss << "Too low co2 savings at tick " << w->time << " : " << project->design->design->co2_equivalent;

		//save information to log file
		Log::instance().log(ss.str(), "INFO: ");
		ss.str(std::string());
		ss.clear();

		//add to utility fudging
		//fudge the numbers 
		utility += THETA[EParamTypes::HODesignDecisionCO2][0] * (3.0 - project->design->design->co2_equivalent);

	}
	else if (project->design->design->co2_equivalent > 9)
	{
		std::stringstream ss;

		ss << "Too high co2 savings at tick " << w->time << " : " << project->design->design->co2_equivalent;

		//save information to log file
		Log::instance().log(ss.str(), "INFO: ");
		ss.str(std::string());
		ss.clear();

		//fudge the numbers 
		utility -= THETA[EParamTypes::HODesignDecisionCO2][0] * (project->design->design->co2_equivalent - 9.0);

	};
		


	//savings?
	//10 - 70




#endif








    return utility;
    
    
}




double Homeowner::estimate_design_utility_from_params(std::shared_ptr<PVProject> project, 
														long label_i)
{
	//for each part - get spline index for x
	//get spline coefs from spline index
	//get y from spline coefs and x 


	double utility = 0.0;
	double x_i = 0.0;
	double y_i = 0.0;
	EParamTypes param = EParamTypes::None;
	auto THETA = w->ho_decisions[EParamTypes::HODesignDecision]->HOD_distribution_scheme
		[w->ho_decisions[EParamTypes::HODesignDecision]->labels[label_i]];

	//efficiency
	x_i = project->design->design->PV_module->efficiency;
	param = EParamTypes::HODesignDecisionPanelEfficiency;
	y_i = w->ho_decisions[EParamTypes::HODesignDecision]->get_spline_value(param, x_i, label_i);
	utility += y_i;

	//visibility
	utility += THETA[EParamTypes::HODesignDecisionPanelVisibility][project->design->design->PV_module->visibility];

	//inverter type
	utility += THETA[EParamTypes::HODesignDecisionInverterType][static_cast<int64_t>(project->design->design->inverter->technology)];

	//number of failures
	//here need 5 years because it is what is asked in the survey
	x_i = 5 * project->design->design->failure_rate;
	param = EParamTypes::HODesignDecisionFailures;
	y_i = w->ho_decisions[EParamTypes::HODesignDecision]->get_spline_value(param, x_i, label_i);
	utility += y_i;

	
	//emmision levels
	x_i = project->design->design->co2_equivalent;
	param = EParamTypes::HODesignDecisionCO2;
	y_i = w->ho_decisions[EParamTypes::HODesignDecision]->get_spline_value(param, x_i, label_i);
	utility += y_i;
	

	//savings are estimated for an average homeowner
	x_i = project->design->design->total_net_savings;
	param = EParamTypes::HODesignDecisionEstimatedNetSavings;
	y_i = w->ho_decisions[EParamTypes::HODesignDecision]->get_spline_value(param, x_i, label_i);
	utility += y_i;


	//if (x_i > 0.1) 
	//{
	//	std::cout << x_i << std::endl;
	//	std::cout << utility << std::endl;
	//};



	
	return utility;
}









double Homeowner::estimate_design_utility(std::shared_ptr<PVProject> project)
{




#ifndef ABMS_SPLINES
    return estimate_design_utility_from_params(project, THETA_DesignDecisions);
#endif

#ifdef ABMS_SPLINES
	return estimate_design_utility_from_params(project, decision_scheme_DesignDecision);
#endif
    
}



/**
 
 
 @DevStage4 may go back and forth over the design

 
*/
void Homeowner::dec_evaluate_designs()
{
#ifdef ABMS_DEBUG_MODE
	bool FLAG_FUDGING_NUMBERS = false;
#endif
    double error = 0.0;
	error = -std::log(1 / w->rand_ho->ru() - 1);
    double utility_none = THETA_DesignDecisions[EParamTypes::HODesignDecisionUtilityNone][0] + error;
    
    
    for (auto& project:pvprojects)
    {

        if (project->state_project == EParamTypes::DraftedDesign)
        {
            error = -std::log(1 / w->rand_ho->ru() - 1);
            project->design->params[EParamTypes::HODesignDecisionEstimatedUtility] = estimate_design_utility(project) + error;
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

#ifdef ABMS_DEBUG_MODE
				//do not accept negative savings
				if (project->design->design->total_net_savings <= 0.0)
				{
					decision = nullptr;
					FLAG_FUDGING_NUMBERS = true; 
				};

#endif // ABMS_DEBUG_MODE
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
        
        quote_state = EParamTypes::HOStateCommitedToInstallation;
        
        //tell world that stopped accepting offers
        w->get_state_inf(this, quote_state);

#ifdef ABMS_DEBUG_MODE
		std::stringstream ss;
		//check if it is negative savings
		if (decision->design->design->total_net_savings <= 0.1)
		{
			auto demand = decision->state_base_agent->params[EParamTypes::ElectricityBill]
				/ WorldSettings::instance().params_exog[EParamTypes::ElectricityPriceUCDemand] 
				/ constants::NUMBER_DAYS_IN_MONTH;

			

			//record utility bill, price of the system, total net savings, roof size?
			ss << decision->design->design->DC_size;

			ss << " ";

			ss << decision->design->design->total_costs; 

			ss << " ";

			//price per watt
			ss << decision->design->design->total_costs / decision->design->design->DC_size;

			ss << " ";

			ss << demand;

			ss << " ";

			ss << constants::NUMBER_SQM_IN_SQF * decision->agent->house->roof_size * decision->agent->house->roof_effective_size;

			ss << " ";

			ss << decision->design->design->total_net_savings;
			
			//save information to log file
			Log::instance().log(ss.str(), "INFO: NEGATIVE SAVINGS ");
			ss.str(std::string());
			ss.clear();
		};


		if (decision->design->design->N_PANELS <= 10) 
		{
			//check its size
			ss << decision->design->design->N_PANELS;
			//save information to log file
			Log::instance().log(ss.str(), "INFO: SMALL SYSTEM ");
			ss.str(std::string());
			ss.clear();
		};



#endif

    }
    else
    {
        
		//MARK: cont. add decision timing to hh and check that project is closed next tick after hh decision to close it
		quote_state = EParamTypes::HOStateDroppedOutDesignStage;

#ifdef ABMS_DEBUG_MODE
		if (FLAG_FUDGING_NUMBERS) 
		{
			quote_state = EParamTypes::HOStateDroppedOutNCDecStage;
		};

#endif
		


        clean_after_dropout();
    };
    
    
    //close other projects
    for(auto& project:pvprojects)
    {
        if (project->state_project == EParamTypes::DraftedDesign)
        {
            if (project != decision)
            {
                project->state_project = EParamTypes::ClosedProject;
                project->ac_hh_time = a_time;
            };
        };
        
    };
    
    
    //exit design stage
    n_pending_designs = 0;
    
    
}






void
Homeowner::receive_design(std::shared_ptr<PVProject> project_)
{
    ++n_pending_designs;
}


void
Homeowner::receive_preliminary_quote(std::shared_ptr<PVProject> project_)
{

    
#ifdef ABMS_DEBUG_MODE
    if (n_preliminary_quotes_requested <= 0.0)
    {
		//TODO: check why throwing here
//        throw std::runtime_error("mismatched number of quotes");
    };
    
    if (n_preliminary_quotes > n_preliminary_quotes_requested)
    {
        std::cout << "mismatched number of quotes" << std::endl;
    };
    
#endif
    
    
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
    return true;
}





void Homeowner::clean_after_dropout()
{
    
    //close all projects
    //close other projects
    for(auto& project:pvprojects)
    {
        project->state_project = EParamTypes::ClosedProject;
        project->ac_hh_time = a_time;
    };

    pvprojects_lock.lock();
    //delete closed projects
    pvprojects.erase(std::remove_if(pvprojects.begin(), pvprojects.end(),
                                    [&](std::shared_ptr<PVProject> x) -> bool { return (project_states_to_delete.find(x->state_project) != project_states_to_delete.end()); }), pvprojects.end());
    pvprojects_lock.unlock();
    
    
    //null all timers
    quote_stage_timer = 0;
    
    n_preliminary_quotes = 0;
    n_preliminary_quotes_requested = 0;
    
    marketing_state = EParamTypes::None;

    
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
    
    if ((quote_stage_timer < WorldSettings::instance().constraints[EConstraintParams::MaxNTicksToCollectQuotes]) && (marketing_state == EParamTypes::HOMarketingStateInterested))
    {
        //initiates and continues collection of quoting information
        ac_inf_quoting_sei();
        quote_state = EParamTypes::HOStateWaitingOnOnlineQuotes;
    }
    else if (quote_state == EParamTypes::HOStateWaitingOnOnlineQuotes)
    {
        //no longer accepting new information
        marketing_state = EParamTypes::HOMarketingStateNotAccepting;
        //moves to the evaluation stage
        dec_evaluate_online_quotes();

    };
    
    
    //evaluates preliminary quotes and commits to the project
    //once all requested quotes are in - evaluate offering
    if ((n_preliminary_quotes >= n_preliminary_quotes_requested) && (n_preliminary_quotes > 0.0))
    {
        dec_evaluate_preliminary_quotes();
    };
    
    
    
    
    //evaluates designs
    //all sei send the same number of proposals in this case
    if (n_pending_designs >= WorldSettings::instance().constraints[EConstraintParams::MinNReceivedDesings])
    {
        dec_evaluate_designs();
    };
    
    
    
    
    for (auto& project:accepted_design)
    {
        //search
        for (long i = 0; i < std::min((long)project->financing->schedule_payments.size(), (long)(a_time - project->ac_accepted_time + 1)); ++i)
        {
            if (auto payment = project->financing->schedule_payments[i] > 0)
            {
                //make payment
                //assume that have enough money
                auto mes = std::make_shared<MesPayment>(payment);
                if (project->sei->get_payment(mes, project))
                {
                    project->financing->schedule_payments[i] = 0.0;
                };
            };
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




