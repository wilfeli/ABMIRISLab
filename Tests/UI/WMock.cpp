//
//  WMock.cpp
//  ABMSolar
//
//  Created by Ekaterina Sinitskaya on 5/9/16.
//  Copyright (c) 2016 IRIS Lab. All rights reserved.
//

#include "WMock.h"
using namespace solar_tests;

std::vector<solar_core::Household*>&
WMock::get_hhs()
{
    return hhs;
}