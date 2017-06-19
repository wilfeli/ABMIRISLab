//
//  MarketingSystem.h
//  ABMSolar
//
//  Created by Ekaterina Sinitskaya on 2/22/16.
//  Copyright (c) 2016 IRIS Lab. All rights reserved.
//

#ifndef ABMSolar_MarketingSystem_h
#define ABMSolar_MarketingSystem_h

#include "Tools/ExternalIncludes.h"
#include "Agents/IAgent.h"
#include "Institutions/IInstitute.h"


namespace solar_core
{
    
    
    
    class MesMarketingSEI;
    class W;
    
    
    /**
     
     
     Is one of institutions
     
     
     */
    class MarketingInst: public IInstitute
    {
    public:
        virtual ~MarketingInst() = default;
        MarketingInst(const PropertyTree& pt_, W* w_);
		MarketingInst(const DocumentRJ & pt_, W * w_);
        void init(W* w_);
        virtual void act_tick() override; /*!< */
        
        //@{
        /**
         
         Section with marketing interactions
         
         */
        
        
        void get_marketing_inf_sei(std::shared_ptr<MesMarketingSEI> mes_); /*!< receives marketing information from SEI */
        void request_inf_marketing_sei(IAgent* agent_); /*!< marketing information from SEI is requested by agent */
        //@}
        
        
    protected:
        double distance(double x1, double y1, double x2, double y2);
        
        std::vector<IAgent*> interested_agents; /*!< Vector of agents that are interested in receiving marketing information  */
        
        std::map<EParamTypes, double> params; /*!<   */
        
        std::vector<std::shared_ptr<MesMarketingSEI>> marketing_mess; /*!< marketing messages from SEI */
        
        W* w;
        
        std::mutex lock;
    };
    
    
} //end of solar_core namespace



#endif
