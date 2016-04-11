//
//  Geography.cpp
//  ABMSolar
//
//  Created by Ekaterina Sinitskaya on 4/10/16.
//  Copyright (c) 2016 IRIS Lab. All rights reserved.
//

#include "Geography/Geography.h"
using namespace solar_core;

WorldMap::WorldMap(PropertyTree& pt)
{
    auto N_TILE_X = pt.get<long>("N_TILE_X");
    auto N_TILE_Y = pt.get<long>("N_TILE_Y");
    
    
    
    for (auto i = 0; i < N_TILE_Y; ++i)
    {
        g_map.push_back({});
        for (auto j = 0; j < N_TILE_X; ++j)
        {
            g_map[i].push_back(new Tile());
        };
    };
    
}