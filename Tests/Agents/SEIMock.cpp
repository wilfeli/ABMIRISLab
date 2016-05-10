//
//  SEIMock.cpp
//  ABMSolar
//
//  Created by Ekaterina Sinitskaya on 5/9/16.
//  Copyright (c) 2016 IRIS Lab. All rights reserved.
//

#include "SEIMock.h"


using namespace solar_tests;
using namespace solar_core;




void
SEIMock::form_design_for_params(std::shared_ptr<PVProject> project_, double demand, double solar_radiation, double permit_difficulty, double project_percentage, const IterTypeDecSM& iter, PVDesign& design)
{
    
    SEI::form_design_for_params(project_, demand, solar_radiation, permit_difficulty, project_percentage, iter, design);
    
}