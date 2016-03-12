//
//  H.h
//  ABMSolar
//
//  Created by Ekaterina Sinitskaya on 2/22/16.
//  Copyright (c) 2016 IRIS Lab. All rights reserved.
//

#ifndef ABMSolar_H_h
#define ABMSolar_H_h


#include "Tools/ExternalIncludes.h"


#include "Tools/IParameters.h"
#include "Geography/Geography.h"
#include "Agents/IAgent.h"



namespace solar_core
{

    
class MesMarketingSEIPreliminaryQuote;
class MesStateBaseHH;
class SEI;
class PVProject;
    

/**
 
 Has multiple humans, but they are not modelled as decision agents only the HH is the decision making agent.
 
 
 @wp Once survey is completed will have data: what are your choices and decisions on solar panels. The same logic as in SEI. Hidden parameter/factor is utility of accepting project. Might be more complex to estimate as will have a lot of categorical data.
 
 @DevStage3 think about using http://en.cppreference.com/w/cpp/types/result_of - very neat construction
 
 */
class Household: public IAgent
{
public:
    
    //@{
    /**
     
     Creation and initialization section
     
     
     @DevStage2 need destructor, copy constructor, copy assignment operator, move constructor, move assignment operator
     
     C(const C&) = default;               // Copy constructor
     C(C&&) = default;                    // Move constructor
     C& operator=(const C&) & = default;  // Copy assignment operator
     C& operator=(C&&) & = default;       // Move assignment operator
     virtual ~C() { }                     // Destructor
     
     see http://stackoverflow.com/questions/4782757/rule-of-three-becomes-rule-of-five-with-c11
     
     */
    
    
    //@}
    
    
    
    
    
    
    //@{
    /**
     
     Section with actions in the world
     
     */
    
    
    
    virtual void ac_inf_quoting_sei() override; /*!< action to request information from SEI when initiative is given from the W */
    virtual void act_tick();
    
    //@}
    
    
    //@{
    /**
     
     Section relevant to marketing information
     
     */
    
    virtual void get_inf(std::shared_ptr<MesMarketingSEI> mes_)  override; /*!< receives marketing information */

    
    
    //@}
    
    
    
    //@{
    /**
     
     Section relevant to quoting stage
     
     */
    
    virtual std::shared_ptr<MesStateBaseHH> get_inf_online_quote(IAgent* agent_to); /*!< first request for information from SEI, provides basic information such as credit score and etc. */
    
    virtual void receive_preliminary_quote(std::shared_ptr<PVProject> project_); /*!< empty for now as information is added directly to the project itself */
    virtual void receive_online_quote(std::shared_ptr<PVProject> project_); /*!< empty for now as information is added directly to the project itself */
    
    
    long quote_stage_timer; /*!< number of ticks spent in a quoting stage */
    
    
    
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
     
     Section with information relevant to potential and active projects
     
     */
    
    
    std::vector<std::shared_ptr<PVProject>> pvprojects; /*!< list of active and potential PV projects */
    
    
    
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
    
    
    
    std::deque<std::shared_ptr<MesMarketingSEI>> get_inf_marketing_sei; /*!< stores list of marketing infiormation from SEI agents that this agent is interested in geting quotes from */
    
    std::deque<std::shared_ptr<MesMarketingSEIPreliminaryQuote>> preliminary_quotes; /*!< have list of active quotes that need to be acted upon @DevStage2 think about replacing raw pointer with. @DevStage1 choose between week_ptr and shared_ptr need to think about ownership in time and time of destruction for these messages. */
    EParamTypes marketing_state; /*!< could be interested, very interested or not */
    
    //@}
    
    
    
    //@{
    /**
     
     Section relevant to quoting stage
     
     */
    EParamTypes quote_state; /*!< will be active quoting or inactive quoting */
    
    
    //@}
    
    
    
    
    //@{
    /**
     
     Section with agent's internals
     
     */
    
    
    /*!<  @DevStage1 GUID research. Boost GUID is almost unique, uses machine and time, so could be repeated if used across machines or time is changed  */
    
    virtual void update_params(); /*!< is called when some part of parameters is updated that is not saved in the main map with parameters. Is used to keep all parameters synchronized. */
    
    TimeUnit a_time; /*!< internal agent's timer */
    //@}
    
    
    
};



} //end of solar_core namespace

#endif
