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
    friend class SEI;
    template <class T1, class T2> friend class HelperWSpecialization;
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
    
    Homeowner();
    Homeowner(const PropertyTree& pt_, W* w_);
	Homeowner(const DocumentRJ & pt_, W * w_);
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
    
    
    void get_project(std::shared_ptr<PVProject> project_);
    
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
    
     Section with general parameters that describe ho
     
    */
    
    
    
    std::map<EParamTypes, double> params; /** Parameters of a Homeowner, such as income, number of humans, etc. */
	
    
    //@}
    
    //@{
    /**
     
     Section with information relevant to potential and active projects
     
     */

    
    std::vector<std::shared_ptr<PVProject>> pvprojects; /*!< list of active and potential PV projects */
    
    std::vector<std::shared_ptr<PVProject>> pvprojects_to_add;
    std::mutex pvprojects_lock;
    
    //@}
    
    
    
    
    
    
    
    
    //@{
    /**
     
     Section relevant to marketing information
     
     */
    
    
    
    std::vector<std::shared_ptr<MesMarketingSEI>> get_inf_marketing_sei; /*!< stores list of marketing information from SEI agents that this agent is interested in geting quotes from */
	std::deque<int> index_inf_marketing_sei; /*!<  List of free indixes for information from sei.*/

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
    long n_preliminary_quotes_requested; /*!< number of requested quotes from installers, equal to total pool size */

    
    //@}
    
    //@{
    /**
     
     Section with agent's internal decisions
     
     */
    
    virtual void dec_evaluate_online_quotes(); /*!< eveluate online quotes - which to be persued further. Uses non-compensatory ruels here */
    void dec_evaluate_online_quotes_nc(); /** nc decisions continued */
    
    double estimate_sei_utility(std::shared_ptr<PVProject> project); /*!< estimate utility from param */
    
    typedef std::map<EParamTypes, std::vector<double>> DecisionParams;

    
    double estimate_sei_utility_from_params(std::shared_ptr<PVProject> project, DecisionParams& THETA); /*!< here is for C_API and generally API */

	double estimate_sei_utility_from_params(std::shared_ptr<PVProject> project, long label_i);
    
    
    virtual void dec_evaluate_preliminary_quotes(); /*!< eveluate preliminary quotes - which to be pursued further. Will correspond to SEI conjoint. Will pick best from here and request actual quotes with site visit. */
    
    
    
    
    double estimate_design_utility_from_params(std::shared_ptr<PVProject> project, DecisionParams& THETA);
	double estimate_design_utility_from_params(std::shared_ptr<PVProject> project, long label_i);
    double estimate_design_utility(std::shared_ptr<PVProject> project);
    
    
    void dec_evaluate_designs(); /*!< picks best design according to the internal preferences */ 
    
    
    std::map<EParamTypes, std::vector<double>> THETA_SEIDecisions; /*!< parameters for decision making, from installer conjoint  */
    
    std::map<EParamTypes, std::vector<double>> THETA_DesignDecisions; /*!< parameters for decision making  */
    
    std::map<EParamTypes, std::vector<double>> THETA_NCDecisions; /*!< parameters for non-compensatory decision making  */
    

	long decision_scheme_SEIDecision = 0; /*!< points to the specific decision scheme in the selection of them */
	long decision_scheme_DesignDecision = 0; /*!< points to the specific decision scheme in the selection of them */




    std::vector<double> THETA_params; /*!< here for consistency with H, is not used */
    
    
    std::deque<std::shared_ptr<PVProject>> accepted_design;
    
    long n_pending_designs;
    
    void clean_after_dropout();

    

    
    //@}
    
    
    //@{
    /**
     
     Section with general tick actions and agent's bookeeping 
     
     */
    
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
