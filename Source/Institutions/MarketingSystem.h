//
//  MarketingSystem.h
//  ABMSolar
//
//  Created by Ekaterina Sinitskaya on 2/22/16.
//  Copyright (c) 2016 IRIS Lab. All rights reserved.
//

#ifndef ABMSolar_MarketingSystem_h
#define ABMSolar_MarketingSystem_h

#include <map>
#include <vector>
#include "Agents/IAgent.h"
#include "Institutions/IInstitute.h"


namespace solar_core
{

    
    
class MesMarketing;
    
    
    
/**
 
 
 @DevStage1 Will inherit from Insititute
 
 
*/
class MarketingInst: public IInstitute
{
public:
    virtual void act_tick() override; /*!< */
    
protected:
    std::vector<IAgent*> interested_agents; /*!< Vector of agents that are interested in receiving marketing information  */
    
    std::map<std::string, double> params; /*!<   */
    
    std::vector<std::shared_ptr<MesMarketing>> marketing_mess; /*!< marketing messages fro SEI */
};


} //end of solar_core namespace



#endif
