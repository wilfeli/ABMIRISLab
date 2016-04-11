//
//  IParameters.cpp
//  ABMSolar
//
//  Created by Ekaterina Sinitskaya on 4/8/16.
//  Copyright (c) 2016 IRIS Lab. All rights reserved.
//

#include "IParameters.h"

using namespace solar_core;

std::istream& solar_core::operator >> (std::istream& is, EParamTypes& item)
{
    EParamTypes_type value = 0;
    is >> value;
    item = static_cast<EParamTypes>(value);
    return is;
}

std::ostream& solar_core::operator << (std::ostream& os, const EParamTypes& item)
{
    EParamTypes_type value = static_cast<EParamTypes_type>(item);
    os << value;
    return os;
}
