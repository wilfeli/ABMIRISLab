//
//  H.h
//  ABMSolar
//
//  Created by Ekaterina Sinitskaya on 2/22/16.
//  Copyright (c) 2016 IRIS Lab. All rights reserved.
//

#ifndef ABMSolar_H_h
#define ABMSolar_H_h


#include <map>
#include <deque>


#include "Tools/IParameters.h"
#include "Geography/Geography.h"
#include "Agents/IAgent.h"



namespace solar_core
{

    
class MesMarketingQuote;
    
    

/**
 
 Has multiple humans, but they are not modelled as decision agents only the HH is the decision making agent.
 
 
 @wp Once survey is completed will have data: what are your choices and decisions on solar panels. The same logic as in SEI. Hidden parameter/factor is utility of accepting project. Might be more complex to estimate as will have a lot of categorical data.
 
 
 
 */
class Household: public IAgent
{
public:
    //@{
    /**
     
     Section with actions in the world
     
     */
    
    virtual void ac_inf_marketing_sei() override; /*!< action to request information from SEI when initiative is given from the W */
    
    
    
    //@}
    
    
protected:
    
    
    //@{
    /**
    
     Section with general parameters that describe hh
     
    */
    
    
    
    std::map<EParamTypes, double> params; /** Parameters of a household, such as income, number of humans, etc. */
    
    //@}
    
    
    
    
    //@{
    /**
     
     Section with geographical parameters
     
     */
    
    double location_x; /** Location of an agent, x coordinate. @DevStage2 think about decreasing size for this field, use uint64_t or smaller for it */
    double location_y; /** Location of an agent, y coordinate. @DevStage2 think about decreasing size for this field, use uint64_t or smaller for it */
    
    
    
    //@}
    
    
    
    
    //@{
    /**
     
     Section with parameters for solar projects, mainly House for now
     
     
     
     
     
     */
    House* house; /** Simplification: assume only 1 house per hh. If need to increase number of houses, might switch to vector of pointers to houses. Use RAII here maybe? for managining raw pointer. @DevStage3 think about using smart pointers here, it will simplify management of creation of agents. Cons: it will be bigger, management will add time. As house is created once, no need to complicated lifetime management.    */
    
    
    
    
    //@}
    
    
    
    //@{
    /**
     
     Section relevant to marketing information
     
     */
    
    std::deque<IAgent*> get_inf_marketing_sei_agents; /*!< stores list of SEI agents that is interested in geting quotes from */
    
    std::deque<std::shared_ptr<MesMarketingQuote>> quotes; /*!< have list of active quotes that need to be acted upon */
    
    
    //@}
    
    
    //@{
    /**
     
     Section with agent's internals
     
     */
    
    
    /*!<  @DevStage1 research GUID. Boost GUID is almost unique, uses machine and time, so could be repeated if used across machines or time is changed  */
    
    
    
    //@}
    
    
    
};



} //end of solar_core namespace

#endif
