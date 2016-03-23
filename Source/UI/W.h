//
//  W.h
//  ABMSolar
//
//  Created by Ekaterina Sinitskaya on 2/22/16.
//  Copyright (c) 2016 IRIS Lab. All rights reserved.
//

#ifndef ABMSolar_W_h
#define ABMSolar_W_h


#include <mutex>
#include "Tools/ExternalIncludes.h"
#include "Tools/IParameters.h"


namespace solar_core {

    
    
class IAgent;


class W
{
public:
    //@{
    /**
     
     Section with initialization and serialization code.
     
     Multiple step initialization from the project file. Keep structure of a project file simple, all parameters in one file. 
     
     @DevStage1: serialization: choose custom/binary/to database. For now think that cereal will be emough, with saving to .json. All agents of the same type will be saved in the same file. Loading also from .json with simple structure.
     
     */
    
    
    //@}
    
    
    
    
    //@{
    /**
     
     Section with main loop
     
     */
    
    void life_hhs(); /*!< life of households */
    
    //@}
    
    
    //@{
    /**
     
     Section with general world parameters and etc.
     
     */
    
    TimeUnit time;
    std::mutex lock_tick; /*!< lock for tick */
    std::condition_variable all_update; /*!< waits until all have updated */
    
    std::atomic<long> updated_counter;
    std::atomic<long> notified_counter;
    
    std::atomic<bool> FLAG_SEI_TICK;
    std::atomic<bool> FLAG_H_TICK;
    std::atomic<bool> FLAG_G_TICK;
    std::atomic<bool> FLAG_SEM_TICK;
    
    //@}
    
    
    
    
protected:
    std::deque<IAgent*> get_inf_marketing_sei_agents; /*!< Agents that requested information, inform SEI that there is request for information for this agent */
    
    
    
    /*!< all H agents */
    /*!< all SEI agents */
    /*!< all SEM */
    /*!< H agents that are active, @DevStage3 think about splitting more fine grained */
    

    
    
};


}



#endif
