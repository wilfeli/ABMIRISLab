//
//  W.h
//  ABMSolar
//
//  Created by Ekaterina Sinitskaya on 2/22/16.
//  Copyright (c) 2016 IRIS Lab. All rights reserved.
//

#ifndef ABMSolar_W_h
#define ABMSolar_W_h

#include <deque>



namespace solar_core {

    
    
class IAgent;


class W
{
public:
    //@{
    /**
     
     Section with initialization and serialization code.
     
     Multiple step initialization from the project file. Keep structure of a project file simple, all parameters in one file. 
     
     @DevStage1: serialization: choose custom/binary/to database 
     
     */
    
    
    //@}
    
    
    
    
    //@{
    /**
     
     Section with main loop
     
     */
    
    void life_hhs(); /*!< life of households */
    
    //@}
    
    
protected:
    std::deque<IAgent*> get_inf_marketing_sei_agents; /*!< Agents that requested information, inform SEI that there is request for information for this agent */
};


}



#endif
