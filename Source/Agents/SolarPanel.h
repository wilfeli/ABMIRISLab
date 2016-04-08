//
//  SolarPanel.h
//  ABMSolar
//
//  Created by Ekaterina Sinitskaya on 3/8/16.
//  Copyright (c) 2016 IRIS Lab. All rights reserved.
//

#ifndef __ABMSolar__SolarPanel__
#define __ABMSolar__SolarPanel__

#include "Tools/ExternalIncludes.h"

#include "Tools/IParameters.h"

namespace solar_core
{

class Household;
class MesStateBaseHH;
class MesMarketingSEIOnlineQuote;
class MesMarketingSEIPreliminaryQuote;
class MesDesign;
class MesFinance;
class SEI;
    
    
    
    
    
class SolarModule
{
public:
    std::string name;
    double efficiency;
    double STC_power_rating;
    double p_sem; /*!< price as quoted by manufacturer */
    double length;
    double width;
};
    

    
/**
 
 Basic structure of the Solar Instalation
 http://www.gogreensolar.com/pages/solar-components-101
 
 
 
 
 
 
 */
    
class PVDesign
{
public:
    PVDesign(const PVDesign&) = default;
    double solar_radiation;
    double permit_difficulty;
    std::shared_ptr<SolarModule> PV_module;
    double N_PANELS;
    double DC_size;
    double AC_size;
    double hard_costs;
    double soft_costs;
    double total_costs;
    double total_savings;
    
    
protected:
    
    
    
    
};
    
    
    
/**
 
 PV Project by SEI
 
 */
class PVProject
{
public:
    Household* agent;/*!< for whom this project is created */
    std::shared_ptr<MesStateBaseHH> state_base_agent;/*!< additional information about the agent for whom this project is made */
    TimeUnit begin_time;
    EParamTypes state_project;
    std::shared_ptr<MesMarketingSEIOnlineQuote> online_quote;
    std::shared_ptr<MesMarketingSEIPreliminaryQuote> preliminary_quote;
    std::shared_ptr<MesDesign> design;
    std::shared_ptr<MesFinance> financing;
    SEI* sei; /*!< installer of a project */
    
    TimeUnit ac_sei_time; /*!< time of a last action on this projet by sei */
    TimeUnit ac_g_time; /*!< time of a last action on this project by g */
    TimeUnit ac_hh_time; /*!< time of a last action on the project by h */
    TimeUnit ac_accepted_time; /*!< when when the project was accepted */
    
protected:
    
    
    
};

} //end namespace solar_core

#endif /* defined(__ABMSolar__SolarPanel__) */
