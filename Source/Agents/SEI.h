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
class MesMarketingSEIPreliminaryQuote;
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
    
    virtual void init(W* w_); /*!< Initialization step */
    
    
    
    //@}
    
    
    //@{
    /**
     
     Section with actions in the world
     
     */
    
    virtual void act_tick(); /*!< actions in each tick @DevStage2 might think about checking for the type of the tick in W and call proper sub tick method from there, will save on multiple boll checks, even of they are cheap. */
    
    //@}
    
    
    //@{
    /**
     
     Section relevant for marketing
     
     */
    
    virtual void request_online_quote(Household* agent_in);
    virtual void request_preliminary_quote(Household* agent_in);
    
    virtual std::shared_ptr<MesMarketingSEIPreliminaryQuote> form_online_quote(Household* agent_in); /*!< @DevStage2 think about transforming this call into interface based one, with agent_in replaced by interface and it being virtual method from the general interface. But virtual call might be more costly and unnecessary in this case, as structure of who will be requesting quotes does not change. */
    virtual std::shared_ptr<MesMarketingSEIPreliminaryQuote> form_preliminary_quote(Household* agent_in);
    
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
     
     Section with general parameters that describe SEI
     
     */
    
    
    
    std::map<EParamTypes, double> params; /** Parameters of a SEI */
    
    
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
    

};
    
    
    
    
    
    
} //end namespace solar_core




#endif
