//
//  H.h
//  ABMSolar
//
//  Created by Ekaterina Sinitskaya on 7/9/16.
//  Copyright (c) 2016 IRIS Lab. All rights reserved.
//

#ifndef __ABMSolar__H__
#define __ABMSolar__H__

#include "Agents/Homeowner.h"


namespace solar_core
{
    /**
     
     Simple H that could only decide on ROI
     
     */
    class H
    {
    public:
        //@{
        /**
         
         Section with geographical parameters
         
         */
        
        
        
        //@}
        
        /**
         
         
         Section with parameters for solar projects, mainly House for now
         
         
         
         
         */
        House* house; /** Simplification: assume only 1 house per h*/
        
        
        //@}
        
        
        //@{
        /**
         
         Section with general parameters that describe hh
         
         */
        
        
        
        std::map<EParamTypes, double> params; /** Parameters of a Homeowner, such as income, number of humans, etc. */
        
        
        //@}
        
        
        
    };
    

    
    
} //end namespace solar_core




#endif /* defined(__ABMSolar__H__) */
