//
//  Geography.cpp
//  ABMSolar
//
//  Created by Ekaterina Sinitskaya on 4/10/16.
//  Copyright (c) 2016 IRIS Lab. All rights reserved.
//

#include "Geography/Geography.h"
#include "Tools/Serialize.h"
#include "UI/W.h"

using namespace solar_core;

WorldMap::WorldMap(PropertyTree& pt, W* w_)
{
    auto N_TILE_X = pt.get<long>("N_TILE_X");
    auto N_TILE_Y = pt.get<long>("N_TILE_Y");
    double solar_radiation;
    double permit_difficulty;
    
    solar_radiation = serialize::solve_str_formula<decltype(solar_radiation)>(pt.get<std::string>("solar_radiation"), *w_->rand);
    permit_difficulty = serialize::solve_str_formula<decltype(permit_difficulty)>(pt.get<std::string>("permit_difficulty"), *w_->rand);
   
    
    for (auto i = 0; i < N_TILE_Y; ++i)
    {
        g_map.push_back({});
        for (auto j = 0; j < N_TILE_X; ++j)
        {
            g_map[i].push_back(new Tile(1.0, 1.0, solar_radiation, permit_difficulty));
        };
    };
    
}