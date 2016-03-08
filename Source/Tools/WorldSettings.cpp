//
//  WorldSettings.cpp
//  ABMSolar
//
//  Created by Ekaterina Sinitskaya on 3/7/16.
//  Copyright (c) 2016 IRIS Lab. All rights reserved.
//

#include "WorldSettings.h"

using namespace solar_core;

WorldSettings::WorldSettings()
{
}



/**
 
 Static variable is initialized only once, compiler is taking care of it.
 
 */
WorldSettings&
WorldSettings::instance()
{
    
    static WorldSettings* instance = new WorldSettings();
    return *instance;
    
}