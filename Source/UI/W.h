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
class WorldMap;
class SEI;
class SEM;
class Household;


class W
{
public:
    //@{
    /**
     
     Section with initialization and serialization code.
     
     Multiple step initialization from the project file. Keep structure of a project file simple, all parameters in one file. 
     
     @DevStage1: serialization: choose custom/binary/to database. For now think that cereal will be emough, with saving to .json. All agents of the same type will be saved in the same file. Loading also from .json with simple structure.
     
     */
    
    
    W(std::string path_, std::string mode_ = "NEW"); /*!< */
    
    std::string base_path;
    
    //@}
    
    
    
    
    //@{
    /**
     
     Section with main loop
     
     */
    
    void life(); /*!< general loop */
    
    void life_hhs(); /*!< life of households */
    void life_seis(); /*!< life of seis */
    
    
    
    
    
    
    
    
    //@}
    
    
    //@{
    /**
     
     Section with general world parameters and etc.
     
     */
    
    std::atomic<bool> FLAG_IS_STOPPED;
    std::atomic<bool> FLAG_IS_STARTED;
    
    
    TimeUnit time;
    TimeUnit begin_time = 0;
    std::mutex lock_tick; /*!< lock for tick */
    std::condition_variable all_update; /*!< waits until all have updated */
    
    std::atomic<long> updated_counter;
    std::atomic<long> notified_counter;
    
    std::atomic<bool> FLAG_SEI_TICK;
    std::atomic<bool> FLAG_H_TICK;
    std::atomic<bool> FLAG_G_TICK;
    std::atomic<bool> FLAG_SEM_TICK;
    
    //@}
    
    //@{
    /**
     
     Interactions on geography
     
     */
    
    double get_solar_radiation(double location_x, double location_y) const; /*!< returns estimated amount of solar radiation for the tile */
    double get_permit_difficulty(double location_x, double location_y) const; /*!< returns permit difficulty */
    
    //@}
    
    
protected:
    std::deque<IAgent*> get_inf_marketing_sei_agents; /*!< Agents that requested information, inform SEI that there is request for information for this agent */
    
    
    
    std::vector<Household*> hhs;/*!< all H agents */
    std::vector<SEI*> seis;/*!< all SEI agents */
    std::vector<SEM*> sems; /*!< all SEM */
    /*!< H agents that are active, @DevStage3 think about splitting more fine grained */
    
    std::map<std::string, std::string> params;
    
    WorldMap* world_map;
    
    
    
    
};


}



#endif
