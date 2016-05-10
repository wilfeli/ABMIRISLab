//
//  SEIMock.h
//  ABMSolar
//
//  Created by Ekaterina Sinitskaya on 5/9/16.
//  Copyright (c) 2016 IRIS Lab. All rights reserved.
//

#ifndef __ABMSolar__SEIMock__
#define __ABMSolar__SEIMock__

#include "Agents/SEI.h"


namespace solar_tests
{
    
    using namespace solar_core;
    
    class SEIMock: public SEI
    {
    public:
        void form_design_for_params(std::shared_ptr<PVProject> project_, double demand, double solar_radiation, double permit_difficulty, double project_percentage, const IterTypeDecSM& iter, PVDesign& design);
        
        
        
    };
    
    
} //end namespace solar_tests


#endif /* defined(__ABMSolar__SEIMock__) */
