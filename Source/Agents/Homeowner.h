//Kelley reviewed 4/26/16 lines 117-118, 123, 125-132, 147, 163, 206, 271
//  H.h
//  ABMSolar
//
//  Created by Ekaterina Sinitskaya on 2/22/16.
//  Copyright (c) 2016 IRIS Lab. All rights reserved.
//

#ifndef ABMSolar_H_h
#define ABMSolar_H_h


#include "Tools/ExternalIncludes.h"

#include <mutex>
#include "Tools/IParameters.h"
#include "Geography/Geography.h"
#include "Agents/IAgent.h"



namespace solar_core
{

    
class MesMarketingSEIPreliminaryQuote;
class MesStateBaseHO;
class W;
class SEI;
class PVProject;
    
    
    
    
    

/**
 
 Has multiple humans, but they are not modelled as decision agents only the HO is the decision making agent.
 
 
 @wp Once survey is completed will have data: what are your choices and decisions on solar panels. The same logic as in SEI. Hidden parameter/factor is utility of accepting project. Might be more complex to estimate as will have a lot of categorical data.
 
 @DevStage3 think about using http://en.cppreference.com/w/cpp/types/result_of - very neat construction
 
 
 @DevStage2 might change from actually assigning house to assigning house type (memory considerations)
 
 
 */
class Homeowner: public IAgent
{
    friend class W;
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
    
    
    Homeowner(const PropertyTree& pt_, W* w_);
    void init(W* w_);
    
    //@}
    
    
    
    
    
    
    //@{
    /**
     
     Section with actions in the world
     
     */
    
    
    
    virtual void ac_inf_quoting_sei(); /*!< action to request information from SEI when initiative is given from the W */
    virtual void act_tick();
    
    //@}
    
    
    //@{
    /**
     
     Section relevant to marketing information

     */
    
    virtual void ac_inf_marketing_sei() override; /*!< implements actions for requesting marketing information */
    
    virtual void get_inf(std::shared_ptr<MesMarketingSEI> mes_)  override; /*!< receives marketing information */

    
    
    //@}
    
    
    
    //@{
    /**
     
     Section relevant to quoting stage

	 //will this include parameters about utility of cost considerations?
	 //will this just incorporate initial quoting or the revised quoting following the site visit
     
     */
    
    virtual std::shared_ptr<MesStateBaseHO> get_inf_online_quote(IAgent* agent_to); /*!< first request for information from SEI, provides basic information such as credit score and etc. */
    //will this command initialize relationship between HO and SEI?

	//switched order to be more chronological
    virtual void receive_online_quote(std::shared_ptr<PVProject> project_); /*!< empty for now as information is added directly to the project itself */
	virtual void receive_preliminary_quote(std::shared_ptr<PVProject> project_); /*!< empty for now as information is added directly to the project itself */
    virtual bool request_time_slot_visit(TimeUnit visit_time, std::weak_ptr<PVProject> project); /*!< check that could have a visit at this time */
    virtual bool schedule_visit(TimeUnit visit_time, std::weak_ptr<PVProject> project); /*!< schedules visit at this time, returns false if no slots are open */
	//virtual bool receive_revised_quote(std::shared_ptr<PVProject> project_); //added this virtual bool because they do get a revised quote? wasn't sure if there was a parameter specifically for this or if you make a variable for this revised quote?
	virtual bool dec_project_reroof(std::shared_ptr<PVProject> project); /*!< @DevStage2 need to ask HO how they decide to reroof */
    //@DevStage2 can receive_preliminary_quote recursively update?
    
    
    
    
    //@}
    
    

    //@{
    /**
     
     Section relevant to design phase
     
     */
    
    virtual void receive_design(std::shared_ptr<PVProject> project_); /*!< is informed that design is received */
    //@DevStage2 is this just step in process? would we be able to incorporate minor design decisions like color as well as a decision to just go with what the installer thinks is best
    //@}
    
    
    
    
    //@{
    /**
     
     Section with geographical parameters
     
     */
    
    double location_x; /** Location of an agent, x coordinate. @DevStage2 think about decreasing size for this field, use uint64_t or smaller for it */
    double location_y; /** Location of an agent, y coordinate. @DevStage2 think about decreasing size for this field, use uint64_t or smaller for it */

    //@}
    
    /**
    

     Section with parameters for solar projects, mainly House for now
     
     
     
     
     */
    House* house; /** Simplification: assume only 1 house per hh. If need to increase number of houses, might switch to vector of pointers to houses. Use RAII here maybe? for managining raw pointer. @DevStage3 think about using smart pointers here, it will simplify management of creation of agents. Cons: it will be bigger, management will add time. As house is created once, no need to complicated lifetime management.    */
    
    
    //@}
    
    //@{
    /**
     
     
     Section with technical members
     */
    

    static std::set<EParamTypes> project_states_to_delete;
    
    //@}


protected:
    
    
    //@{
    /**
    
     Section with general parameters that describe hh
     
    */
    
    
    
    std::map<EParamTypes, double> params; /** Parameters of a Homeowner, such as income, number of humans, etc. */
    
    
    //@}
    
    //@{
    /**
     
     Section with information relevant to potential and active projects
	 //should this type of section be in H or SEI?
     
     */
    
    
    std::vector<std::shared_ptr<PVProject>> pvprojects; /*!< list of active and potential PV projects */
    
    
    
    //@}
    
    
    
    
    
    
    
    
    //@{
    /**
     
     Section relevant to marketing information
     
     */
    
    
    
    std::deque<std::shared_ptr<MesMarketingSEI>> get_inf_marketing_sei; /*!< stores list of marketing information from SEI agents that this agent is interested in geting quotes from */
    
    std::deque<std::shared_ptr<MesMarketingSEIPreliminaryQuote>> preliminary_quotes; /*!< have list of active quotes that need to be acted upon @DevStage2 think about replacing raw pointer with. @DevStage2 choose between week_ptr and shared_ptr need to think about ownership in time and time of destruction for these messages. */
    
    EParamTypes marketing_state; /*!< could be interested, very interested or not */
    
    //@}
    
    
    
    //@{
    /**
     
     Section relevant to quoting stage
     
     */
    EParamTypes quote_state; /*!< will be active quoting or inactive quoting */
    
    
    std::vector<std::vector<std::weak_ptr<PVProject>>> schedule_visits; /*!< schedule for visits for the preliminary quote, length is equal to MaxLengthWaitPreliminaryQuote */
    std::size_t i_schedule_visits;
    std::mutex schedule_visits_lock;
    
    long quote_stage_timer; /*!< number of ticks spent in a quoting stage */
    long n_preliminary_quotes; /*!< number of preliminary quotes */

    
    //@}
    
    //@{
    /**
     
     Section relevant to design stage
     
     */
    
    void dec_evaluate_designs(); /*!< picks best design according to the internal preferences */ 
    
    std::deque<std::shared_ptr<PVProject>> accepted_design; //why is this coming from a deque?
    long n_pending_designs;
    
    std::map<EParamTypes, std::vector<double>> THETA_design; /*!< parameters for decision making  */
    
    
    
    //@}
    
    //@{
    /**
     
     Section with agent's internals //internal preferences? what is the difference between this section and parameters of H
     
     */
    
    
    virtual void dec_evaluate_online_quotes(); /*!< eveluate online quotes - which to be persued further */
    virtual void dec_evaluate_preliminary_quotes(); /*!< eveluate preliminary quotes - which to be persued further */
    
    /*!< GUID research. Boost GUID is almost unique, uses machine and time, so could be repeated if used across machines or time is changed  */
    
    virtual void update_params(); /*!< is called when some part of parameters is updated that is not saved in the main map with parameters. Is used to keep all parameters synchronized. */
    
    virtual void ac_update_tick(); /*!< update internals for the tick */
    
    TimeUnit a_time; /*!< internal agent's timer */
    //@}
    
    
    //@{
    /**
     
     
     Sections with interactions with the world
     */
    
    W* w; /*!< raw pointer to the actual world */
    
    
    //@}
    
};



} //end of solar_core namespace

#endif
