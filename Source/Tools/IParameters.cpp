//
//  IParameters.cpp
//  ABMSolar
//
//  Created by Ekaterina Sinitskaya on 4/8/16.
//  Copyright (c) 2016 IRIS Lab. All rights reserved.
//

#include "IParameters.h"



std::istream& operator >> (std::istream& is, solar_core::EParamTypes& item)
{
    solar_core::EParamTypes_type value = 0;
    is >> value;
    item = static_cast<solar_core::EParamTypes>(value);
    return is;
}

std::ostream& operator << (std::ostream& os, const solar_core::EParamTypes& item)
{
    solar_core::EParamTypes_type value = static_cast<solar_core::EParamTypes_type>(item);
    os << value;
    return os;
}
