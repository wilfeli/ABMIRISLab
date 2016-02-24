//
//  H.h
//  ABMSolar
//
//  Created by Ekaterina Sinitskaya on 2/22/16.
//  Copyright (c) 2016 IRIS Lab. All rights reserved.
//

#ifndef ABMSolar_H_h
#define ABMSolar_H_h


#include <map>


#include "Tools/IParameters.h"
#include "Geography/Geography.h"


//MARK: add namespaces, think about name


namespace solar_core {
    

/**
 
 Has multiple humans, but they are not modelled as decision agents.
 
 
 */
class Household
{
public:
    
    
    
protected:
    
    
    //@{
    /**
    
     Section with general parameters that describe hh
     
    */
    
    
    
    std::map<EParamTypes, double> params; /** Parameters of a household, such as income, number of humans, etc. */
    
    //@}
    
    
    
    
    //@{
    /**
     
     Section with geographical parameters
     
     */
    
    double location_x; /** Location of an agent, x coordinate. @DevStage2 think about decreasing size for this field, use uint64_t or smaller for it */
    double location_y; /** Location of an agent, y coordinate. @DevStage2 think about decreasing size for this field, use uint64_t or smaller for it */
    
    
    
    //@}
    
    
    
    
    //@{
    /**
     
     Section with parameters for solar projects, mainly House for now
     
     
     
     
     
     */
    House* house; /** Simplification: assume only 1 house per hh. If need to increase number of houses, might switch to vector of pointers to houses. Use RAII here maybe? for managining raw pointer. @DevStage3 think about using smart pointers here, it will simplify management of creation of agents. Cons: it will be bigger, management will add time. As house is created once, no need to complicated lifetime management.    */
    
    
    
    
    //@}
    
    
    
    
};



} //end of solar_core namespace

#endif
