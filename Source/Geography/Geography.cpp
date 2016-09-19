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


House::House(const PropertyTree& pt_)
{
    roof_age = pt_.get<double>("roof_age");
    roof_size = pt_.get<double>("roof_size");
    house_size = pt_.get<double>("house_size");
    
}


WorldMap::WorldMap(PropertyTree& pt, W* w_)
{
    auto N_TILE_X = pt.get<long>("N_TILE_X");
    auto N_TILE_Y = pt.get<long>("N_TILE_Y");
    double solar_irradiation;
    double permit_difficulty;
    bool requires_permit_visit;
    
    solar_irradiation = serialize::solve_str_formula<decltype(solar_irradiation)>(pt.get<std::string>("solar_irradiation"), *w_->rand);
    permit_difficulty = serialize::solve_str_formula<decltype(permit_difficulty)>(pt.get<std::string>("permit_difficulty"), *w_->rand);
    requires_permit_visit = serialize::solve_str_formula<decltype(requires_permit_visit)>(pt.get<std::string>("requires_permit_visit"), *w_->rand);

    
    for (auto i = 0; i < N_TILE_Y; ++i)
    {
        g_map.push_back({});
        for (auto j = 0; j < N_TILE_X; ++j)
        {
            g_map[i].push_back(new Tile(1.0, 1.0, solar_irradiation, permit_difficulty, requires_permit_visit));
        };
    };
    
}