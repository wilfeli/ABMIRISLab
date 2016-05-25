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


SEIMock::SEIMock(PropertyTree& pt_, W* w_):SEI(pt_, w_){}

/**

 This way instead of virtual for speed reasons (might change to virtual later). Anyway would need to expose protected members as public in Mocks.
 
 */
void
SEIMock::form_design_for_params(std::shared_ptr<PVProject> project_, double demand, double solar_irradiation, double permit_difficulty, double project_percentage, const IterTypeDecSM& iter, PVDesign& design)
{
    SEI::form_design_for_params(project_, demand, solar_irradiation, permit_difficulty, project_percentage, iter, design);
}


void
SEIMock::ac_estimate_savings(PVDesign& design, std::shared_ptr<PVProject> project_)
{
    SEI::ac_estimate_savings(design, project_);
}





std::map<EParamTypes, std::shared_ptr<SolarModule>>&
SEIMock::get_dec_solar_modules()
{
    return dec_solar_modules;
}