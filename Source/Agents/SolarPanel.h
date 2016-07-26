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

class Homeowner;
class H;
class MesStateBaseHO;
class MesMarketingSEIOnlineQuote;
class MesMarketingSEIPreliminaryQuote;
class MesDesign;
class MesFinance;
class SEI;
class SEM;
    
    
    
    
    class Inverter
    {
    public:
        std::string name;
        EParamTypes technology; 
    protected:
        
        
    };
    
    
    
    class SolarModule
    {
    public:

        static std::shared_ptr<SolarModule> deserialize(const PropertyTree& pt_);
        SolarModule(const PropertyTree& pt_);
        
        
        std::string name;
        double efficiency;
        double STC_power_rating;
        double p_sem; /*!< price as quoted by manufacturer */
        double length;
        double width;
        double warranty_length; /*!< in weeks, as it is time period */
        std::string manufacturer_id = "";
        SEM* manufacturer = nullptr;
        double degradation = 0.0;
        UID uid;
        
        
        
        
    };
    
    
    class SolarModuleBL: public SolarModule
    {
    public:
        /** parameters of a true reliability distribution (exponential) */
        std::vector<double> THETA_reliability;
        /** parameters of a true reliability distribution (normal) */
        std::vector<double> THETA_complexity;
    protected:
    };
    

    
    /**
     
     Basic structure of the Solar Instalation
     http://www.gogreensolar.com/pages/solar-components-101
     
     
     
     
     
     
     */
    class PVDesign
    {
    public:
        PVDesign() = default;
        PVDesign(const PVDesign&) = default;
        double solar_irradiation = 0.0;
        double permit_difficulty = 0.0;
        std::shared_ptr<SolarModule> PV_module;
        double N_PANELS = 0.0;
        double DC_size = 0.0;
        double AC_size = 0.0;
        double hard_costs = 0.0;
        double soft_costs = 0.0;
        double total_costs = 0.0;
        double total_savings = 0.0;
        double energy_savings_money = 0.0;
        
        
    protected:
        
        
        
        
    };
    
    
    
    class PVProjectFlat
    {
    public:
        std::shared_ptr<SolarModuleBL> PV_module;
        double N_PANELS = 0.0;
        double DC_size = 0.0;
        double AC_size = 0.0;
        TimeUnit begin_time;
        SEI* sei; /*!< installer of a project */
        H* agent;/*!< for whom this project is created */
        double production_time = 0.0; /*!< realized production for t period */
        double production_time_1 = 0.0; /*!< realized production for t-1 period */
        double irr_e = 0.0; /*!< expected irr of a project */
        double irr_a = 0.0; /*!< advertized by installer irr */
        TimeUnit maintenance_time;
        TimeUnit maintenance_time_1;
        double maintenance_complexity;
        double p; /*!< total price for this project */
        
    };
    
    
    
    /**
     
     PV Project by SEI
     
     */
    class PVProject
    {
    public:
        Homeowner* agent;/*!< for whom this project is created */
        std::shared_ptr<MesStateBaseHO> state_base_agent;/*!< additional information about the agent for whom this project is made */
        TimeUnit begin_time;
        EParamTypes state_project;
        EParamTypes state_materials;
        std::shared_ptr<MesMarketingSEIOnlineQuote> online_quote;
        std::shared_ptr<MesMarketingSEIPreliminaryQuote> preliminary_quote;
        std::shared_ptr<MesDesign> design;
        std::shared_ptr<MesFinance> financing;
        SEI* sei; /*!< installer of a project */
        
        TimeUnit ac_sei_time = 0; /*!< time of a last action on this projet by sei */
        TimeUnit ac_g_time = 0; /*!< time of a last action on this project by g */
        TimeUnit ac_hh_time = 0; /*!< time of a last action on the project by h */
        TimeUnit ac_accepted_time = 0; /*!< when when the project was accepted */
        TimeUnit ac_utility_time = 0; /*! when request was accepted by utility company */
        
    protected:
        
        
        
    };

} //end namespace solar_core

#endif /* defined(__ABMSolar__SolarPanel__) */
