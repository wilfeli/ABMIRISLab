//
//  SEM.h
//  ABMSolar
//
//  Created by Ekaterina Sinitskaya on 2/22/16.
//  Copyright (c) 2016 IRIS Lab. All rights reserved.
//

#ifndef ABMSolar_SEM_h
#define ABMSolar_SEM_h

#include "Tools/IParameters.h"
#include "Tools/ExternalIncludes.h"
#include "Tools/ID.h"


namespace solar_core
{
    
    
    class W;
    class Inverter;
    class SolarModule;
    class Inverter;
    class MesSellOrder;
    
    
    
    /**
     
     @wp Research for the structure of costs
     
     Hanwha Q CELLS, 3d quater 2015 report. http://investors.hanwha-qcells.com/releasedetail.cfm?ReleaseID=943563
     Total all-in processing costs approached US$0.39 per watt in September for in-house production
     
     
     
     
     Assume that base cost is fixed - for example $0.39 for 15% efficiency. If efficiency is higher - cost becomes x*(1+theta_{1}*\frac{pv efficiency}{base efficiency})
     
     In time x becomes lower and theta_1 becomes lower
     
     Starts with the existing selection of solar panels and could innovate with some fixed frequency
     
     @DevStage2 add market for solar panels and price feedback 
     
     Production happens every tick, with fixed number of panels being produced and stored as inventories. 
     Production for invertors happens also every tick. 
     For now SEM produce both panels and inverters.
     
     
     
     */
    class SEM
    {
        template <class T1, class T2> friend class HelperWSpecialization;
    public:
        //@{
        /**
         
         Initializations
         
         */
        
        SEM(const PropertyTree& pt_, W* w_);
		SEM(const DocumentRJ & pt_, W * w_);
        void init(W* w_);
        virtual void init_world_connections();
        
        //@}
        
        
        //@{
        /**
         
         Section with actions in the world
         
         */
        
        virtual void act_tick();
        
        bool sell_SolarModule(MesSellOrder& mes_);
        
        //@}
        
        
        //@{
        /**
         
         Section with pricing decisions
         
         
         */

        std::map<std::string, double> prices; /*!< prices for solar panels */
        
        //@}
        
        

		//@{
		/**

		Section relevant to production

		*/
		std::vector<std::shared_ptr<SolarModule>>solar_panel_templates; /*!< list of solar panels to produce */

		//@}

        
        //@{
        /**
         
         Public parameters
         
         */
        
        EParamTypes sem_type;
        
        UID uid;
        std::mutex lock;
        
        //@}
        
        
    protected:
        //@{
        /**
         
         Section with internals
         
         */
        
        
        W* w;
        TimeUnit a_time; /*!< agent time */

        
        void ac_update_tick(); /*!< updates before tick */
        
        double money; /*!< money of an agent */
        std::map<EParamTypes, double> params; /*!< parameters of an agent */
        
        TimeUnit sem_production_time; /*!< last time production cycle took place */
        TimeUnit sem_research_time; /*!< last time research cycle ended */

        
        //@}
        
        
        //@{
        /**
         
         Section relevant to production
         
         */

        std::vector<std::shared_ptr<Inverter>> inverter_templates;
        
        std::map<std::string, double> inventories;
        double N_PANELS_inventories = 0.0;
        
        //@}
        
        //@{
        /**
         
         Section with pricing decisions
         
         
         */
        TimeUnit sem_dec_time; /*!< last time price decision was made */ 
        std::vector<double> history_sales;
        double costs_base = 0.39;
        std::vector<double> THETA_profit; /*!< THETA[0] - base markup, THETA[1] - */
        
        
        //@}
        
        

        
        
    };
} //end namespace solar_core


#endif
