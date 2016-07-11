//
//  WMock.cpp
//  ABMSolar
//
//  Created by Ekaterina Sinitskaya on 5/9/16.
//  Copyright (c) 2016 IRIS Lab. All rights reserved.
//

#include "Tests/UI/WMock.h"
#include "Tests/Agents/SEIMock.h"

using namespace solar_tests;


WMock*
WMock::create(std::string path_, std::string mode_)
{
    //create helper
    auto helper = new solar_core::HelperWSpecialization<solar_core::W, solar_core::UnitTestModel>();
    
    return new WMock(path_, helper, mode_);
}




std::vector<solar_core::Homeowner*>&
WMock::get_hos()
{
    return hos;
}


std::vector<solar_core::SEI*>&
WMock::get_seis()
{
    return seis;
}


