//
//  SEI.h
//  ABMSolar
//
//  Created by Ekaterina Sinitskaya on 2/22/16.
//  Copyright (c) 2016 IRIS Lab. All rights reserved.
//

#ifndef ABMSolar_SEI_h
#define ABMSolar_SEI_h



#include "Tools/ExternalIncludes.h"

#include "Tools/IParameters.h"
#include "Agents/IAgent.h"
#include "Agents/SolarPanel.h"



namespace solar_core
{
    

    
/**
 
 
 
 */
class MesMarketingSEI;
class MesMarketingSEIOnlineQuote;
class MesMarketingSEIPreliminaryQuote;
class MesDesign;
class MesPayment;
class W;
class Household;
    

/**
 
 For marketing - use marketing institute as an intermediary. SEI submits marketing information, marketing institute passes it to H based on the parameters, such as effectiveness
 
 
 
 
 @wp For choice function could try neural nets - train them, include two sets of parameters, one will be parameters of an installer and the second will be parameters of a project. The resulting node will be probability of accepting project. Estimation will be done on a database with all installed solar panels. It is equivalent to density estimation using NN. Could also use PCA when the estimated component is assumed to be profit from the project. Could use Python with sci-kit or check TensorFlow. Once estimation is done - could use it as a simple linear function to produce estimation of profit - and thus estimation for the chance of accepting project. Basically dataset will produce profit estimation in some form, should still work even if have only accepted projects. For new project - will get profit estimation. 
 
  
 
 
 @DevStage1 might add timer to model that giving estimate might take time. Also might have queue and maximum capacity, thus receiving quote in place might actually take some time before requesting online visit and actually performing this visit
 
 */
class SEI: public IAgent
{
public:
    //@{
    /**
    
     Section containing initialization information
     
     */
    SEI(const PropertyTree& pt_, W* w_);
    virtual void init(W* w_); /*!< Initialization step */
    
    
    
    //@}
    
    
    //@{
    /**
     
     Section with actions in the world
     
     */
    
    virtual void act_tick(); /*!< actions in each tick @DevStage2 might think about checking for the type of the tick in W and call proper sub tick method from there, will save on multiple boll checks, even of they are cheap. */
    
    virtual void get_project(std::shared_ptr<PVProject> project_); /*!< receives project - technical method to synchronoze lists of projects between agents */
    
    
    //@}
    
    
    //@{
    /**
     
     Section relevant for marketing
     
     */
    
    virtual void request_online_quote(std::shared_ptr<PVProject> project_);
    virtual void request_preliminary_quote(std::shared_ptr<PVProject> project_);
    virtual void ac_inf_marketing_sei() override;
    virtual void get_inf(std::shared_ptr<MesMarketingSEI> mes_) override;
    
    //@}
    
    
    //@{
    /**
     
     Section relevant to quoting phase
     
     */

    virtual void accepted_preliminary_quote(std::shared_ptr<PVProject> project_); /*!< indicated that preliminary quote was accepted */
    
    
    //@}
    
    //@{
    /**
     
     Section relevant to design phase
     
     */
    
    
    virtual void accepted_design(std::shared_ptr<PVProject> project_); /*!< indicates that design was accepted */
    
    
    //@}
    
    
    
    
    //@{
    /**
     
     Section relevant to payments
     
     */
    
    
    void get_payment(std::shared_ptr<MesPayment> mes_);
    
    
    //@}
    
    
    
protected:
    //@{
    /**
     
     Section relevant to marketing information
     
     */
    
    MesMarketingSEI* mes_marketing; /*!< Contains basic marketing information. is created with agent and updated to reflect new marketing policy */
    
    
    //@}
    
    //@{
    /**
     
     Section relevant to quoting phase
     
     */
    
    virtual std::shared_ptr<MesMarketingSEIOnlineQuote> form_online_quote(std::shared_ptr<PVProject> project_); /*!< @DevStage2 think about transforming this call into interface based one, with agent_in replaced by interface and it being virtual method from the general interface. But virtual call might be more costly and unnecessary in this case, as structure of who will be requesting quotes does not change. */
    virtual std::shared_ptr<MesMarketingSEIPreliminaryQuote> form_preliminary_quote(std::shared_ptr<PVProject> project_);
    
    
    std::vector<std::vector<std::weak_ptr<PVProject>>> schedule_visits; /*!< schedule for visits for the preliminary quote, length is equal to the MaxLengthWaitPreliminaryQuote */
    std::size_t i_schedule_visits;
    
    
    void collect_inf_site_visit(std::shared_ptr<PVProject> project_); /*!< collects information on site */
    
    //@}
    
    
    //@{
    /**
     
     Section relevant to design phase
     
     */
    
    
    virtual std::shared_ptr<MesDesign> form_design(std::shared_ptr<PVProject> project_); /*!< creates design based on the project's parameters */
    
    std::map<EParamTypes, std::shared_ptr<SolarModule>> dec_solar_modules; /*!< choices for different modules to create design with */
    std::vector<double> dec_project_percentages; /*!< percentage of a utility bill to cover */
    std::vector<double> THETA_hard_costs; /*!< THETA[0] - price per efficiency unit, THETA[1] discount for the size of a project */
    std::vector<double> THETA_soft_costs; /*!< THETA[0] - labor costs for installation, THETA[1] - additional labor costs due to the permitting difficulty */
    std::vector<double> THETA_profit; /*!< THETA[0] - profit margin */
    
    
    void ac_estimate_savings(PVDesign& design, std::shared_ptr<PVProject> project_); /*!< estimate savings for the project */
    
    void form_financing(std::shared_ptr<PVProject> project_); /*!< create financing options to choose from */
    
    TimeUnit ac_designs; /*!< last time information about SEM was updated */
    
    //@}
    
    
    //@{
    /**
     
     
     Section relevant to installation phase
     
     */
    
    std::vector<std::vector<std::weak_ptr<PVProject>>> schedule_installations; /*!< schedule for installations, length is equal to the MaxLengthPlanInstallation */
    
    std::size_t i_schedule_installations;
    
    void install_project(std::shared_ptr<PVProject> project); /*!< performs actual installation */
    
    //@}
    
    
    
    //@{
    /**
     
     Section with general parameters that describe SEI
     
     */
    
    
    
    std::map<EParamTypes, double> params; /** Parameters of a SEI */
    EParamTypes sei_type;
    double money = 0.0; 
    
    TimeUnit a_time; /*!< internal agent's timer */
    
    
    //@}

    
    
    //@{
    /**
     
     Section with information relevant to potential and active projects
     
     */
    
    
    std::vector<std::shared_ptr<PVProject>> pvprojects; /*!< list of active and potential PV projects */
    
    
    
    //@}
    
    
    
    
    //@{
    /**
     
     
     Sections with interactions with the world
     */
    
    W* w; /*!< raw pointer to the actual world */
    
    
    //@}
    
    
    
    //@{
    /**
     
     Section with internals of an agent
     
     */
    
    void ac_update_tick(); /*!< update internals for the tick */
    
    //@}
    

};
    
    
    
    
    
    
} //end namespace solar_core




#endif
