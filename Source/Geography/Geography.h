//
//  Geography.h
//  ABMSolar
//
//  Created by Ekaterina Sinitskaya on 2/22/16.
//  Copyright (c) 2016 IRIS Lab. All rights reserved.
//

#ifndef ABMSolar_Geography_h
#define ABMSolar_Geography_h

#include "Tools/ExternalIncludes.h"


namespace solar_core
{
    


/**
 
 Belongs to a Household.
 
 
 
 */
class House
{
public:
    double roof_age; /*!< age of a house's roof in years (not required to be whole years) */
    double roof_size; /*!< size of a roof, in m^2 */
    
    
    
    
};




/**
 
 Each tile has x and y sizes.
 
 */
class Tile
{
protected:
    double size_x; /*!< length of a tile */
    double size_y; /*!< width of a tile */
    
};




/**
 
 
 @wp generally have 2-d grid for spacial information, but it might be too much for the number of agents. Might need to compress or block representation. 10^5 = 10^3 * 10^3. Might need grid with sizes, each lot has a size. 10^3 for rows and columns is OK size.
 
 Will have patches of houses, each house - 1 tile with size. Also will have empty tiles with sizes also. Will decrease total number of tiles. Will have neighborhoods with different tile sizes, house characteristics, population density as a derivative of tile size.
 
 */
class WorldMap
{
public:
    std::vector<std::vector<Tile*>> g_map; /*!< grid of location tiles */
};


} //end of namespace solar_core



#endif
