//
//  SolarPanel.h
//  ABMSolar
//
//  Created by Ekaterina Sinitskaya on 3/8/16.
//  Copyright (c) 2016 IRIS Lab. All rights reserved.
//

#ifndef __ABMSolar__SolarPanel__
#define __ABMSolar__SolarPanel__

#include "Tools/ExternalIncludes.h"

#include "Tools/IParameters.h"

namespace solar_core
{

class Household;
class MesStateBaseHH;
    
/**
 
 PV Project by SEI
 
 */
class PVProject
{
public:
    Household* agent;/*!< for whom this project is created */
    std::shared_ptr<MesStateBaseHH> state_base_agent;/*!< additional information about the agent for whom this project is made */
    TimeUnit begin_time;
    EParamTypes state_project;
    
    
    
protected:
    
    
    
};

} //end namespace solar_core

#endif /* defined(__ABMSolar__SolarPanel__) */
