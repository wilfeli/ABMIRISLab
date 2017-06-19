//
//  MarketingSystem.cpp
//  ABMSolar
//
//  Created by Ekaterina Sinitskaya on 2/29/16.
//  Copyright (c) 2016 IRIS Lab. All rights reserved.
//

#include "UI/W.h"
#include "Institutions/MarketingSystem.h"
#include "Institutions/IMessage.h"
#include "Tools/Serialize.h"
#include "Tools/SerializeRJ.h"
#include "Tools/WorldSettings.h"
#include "Agents/Homeowner.h"
#include "Agents/SEI.h"


using namespace solar_core;

MarketingInst::MarketingInst(const PropertyTree& pt_, W* w_)
{
    w = w_;
    
    //read parameters
    std::map<std::string, std::string> params_str;
    serialize::deserialize(pt_.get_child("params"), params_str);
    
    ///@DevStage2 move to W to speed up, but test before that
    for (auto& iter:params_str)
    {
        params[EnumFactory::ToEParamTypes(iter.first)] = serialize::solve_str_formula<double>(iter.second, *w->rand_market);
    };
}

MarketingInst::MarketingInst(const DocumentRJ& pt_, W* w_)
{
	w = w_;

	//read parameters
	std::map<std::string, std::string> params_str;
	serialize::deserialize(pt_["params"], params_str);

	///@DevStage2 move to W to speed up, but test before that
	for (auto& iter : params_str)
	{
		params[EnumFactory::ToEParamTypes(iter.first)] = serialize::solve_str_formula<double>(iter.second, *w->rand_market);
	};
}


void
MarketingInst::init(W* w_)
{
}


/**
 
 not used for now
 
 */
void MarketingInst::request_inf_marketing_sei(IAgent* agent_)
{
    lock.lock();
    //addes for further asynchronous call
    interested_agents.push_back(agent_);
    lock.unlock();
}


void MarketingInst::get_marketing_inf_sei(std::shared_ptr<MesMarketingSEI> mes_)
{
    marketing_mess.push_back(mes_);
}




void MarketingInst::act_tick()
{

    
    //push marketing information to the selection of agents that expressed interest in receiving marketing information
    for (auto& agent:interested_agents)
    {
        //push all marketing messages
        for (auto& mes:marketing_mess)
        {
            agent->get_inf(mes);
        };
        
    };
    
    lock.lock();
    interested_agents.clear();
    lock.unlock();
    
    
    
    
    
    //randomly select agents and push marketing information
    auto pdf_agents = boost::uniform_int<uint64_t>(0, w->hos->size()-1);
    auto rng_agents = boost::variate_generator<boost::mt19937&, boost::uniform_int<uint64_t>>(w->rand_market->rng, pdf_agents);
    
    
    std::set<std::size_t> j_used;
    
    std::size_t j = 0;
//    std::cout << WorldSettings::instance().params_exog[EParamTypes::MarketingMaxNToDrawPerTimeUnit] << std::endl;
//    std::cout << w->hhs.size();
    for (auto i = 0; i < WorldSettings::instance().params_exog[EParamTypes::MarketingMaxNToDrawPerTimeUnit]; ++i)
    {
        j = rng_agents();
#ifdef ABMS_DEBUG_MODE
//        if (j_used.find(j) != j_used.end())
//        {
//            std::cout << "already used " << j << std::endl;
//        };
#endif
        j_used.insert(j);
        //push all marketing messages
        for (auto& mes:marketing_mess)
        {
            
            //check that it is in the vicinity
            if (distance((*w->hos)[j]->location_x, (*w->hos)[j]->location_y, mes->agent->location_x, mes->agent->location_y) <= params[EParamTypes::MarketingSEIMaxDistance])
            {
                (*w->hos)[j]->get_inf(mes);
            }
            else if (mes->agent->sei_type == EParamTypes::SEILarge)
            {
                (*w->hos)[j]->get_inf(mes);
            };
            
        };

    };
    
    
    
    
}





double MarketingInst::distance(double x1, double y1, double x2, double y2)
{
    
    return std::pow(std::pow(x2 - x1, 2) + std::pow(y2 - y1, 2), 0.5);

}



