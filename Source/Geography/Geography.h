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
#include "Tools/IParameters.h"

namespace solar_core
{
    
    class W;
    class WEE;
    class Homeowner;
    class H;
    
    /**
     
     Belongs to a Homeowner.
     
     
     
     Cuurently switched to estimated roof_size from the house_size 
     -# RP  = roof pitch
     -# HS = square foot of home
     -# F = number of floors
     -# x = roof pitched length
     -# y = roof length
     -# RA = roof area
     -# R = ratio of roof area to house area
     
     Formula for calculating roof size
     \f[
     y = \sqrt{frac{HS}{F}}
     \f]
     \f[
     x = (\frac{y}{2}) \times (\cos (\arctan (RP))
     \f]
     \f[
     RA = 2 \times x \times y
     \f]
     \f[
     R = RA \div HS
     \f]
     
     
     */
    class House
    {
    public:
        House();
        House(const PropertyTree& pt_);
        double roof_age; /*!< age of a house's roof in years (not required to be whole years) */
        double roof_size; /*!< size of a roof, in m^2 */
        double house_size; /*!< size of a house in ? */
        
        
        
    };
    
    
    
    
    /**
     
     Each tile has x and y sizes.
     
     */
    class Tile
    {
    public:
        Tile(double size_x_, double size_y_, double solar_irradiation_, double permit_difficulty_, bool requires_permit_visit_): size_x(size_x_), size_y(size_y_), solar_irradiation(solar_irradiation_), permit_difficulty(permit_difficulty_), requires_permit_visit(requires_permit_visit_){}
        double size_x; /*!< length of a tile */
        double size_y; /*!< width of a tile */
        double solar_irradiation = 4.0; /*!< amount of solar irradiation per Wh/m2/day for this tile */
        double permit_difficulty = 1.0; /*!< length of a permitting process for this tile */
        bool requires_permit_visit = 1.0; /*!< if this locations needs to visit to give permit */
    };
    
    
    
    
    /**
     
     
     @wp generally have 2-d grid for spacial information, but it might be too much for the number of agents. Might need to compress or block representation. 10^5 = 10^3 * 10^3. Might need grid with sizes, each lot has a size. 10^3 for rows and columns is OK size.
     
     Will have patches of houses, each house - 1 tile with size. Also will have empty tiles with sizes also. Will decrease total number of tiles. Will have neighborhoods with different tile sizes, house characteristics, population density as a derivative of tile size.
     
     */
    class WorldMap
    {
    public:
        WorldMap(PropertyTree& pt, W* w_);
        
        std::vector<std::vector<Tile*>> g_map; /*!< grid of location tiles */
        
        std::vector<std::vector<std::vector<Homeowner*>>> h_map; /*!< location of h by tile */;
        
    };
    
    template <class Param>
    class WorldMapSpecialization: public WorldMap
    {
        
    };
    
    
    template<>
    class WorldMapSpecialization<WEE>: public WorldMap
    {
    public:
        typedef WorldMap Super;
        WorldMapSpecialization<WEE>(PropertyTree& pt, W* w_);
        std::vector<std::vector<std::vector<H*>>> h_map; /*!< location of h by tile */;
    };
    
    
} //end of namespace solar_core



#endif
