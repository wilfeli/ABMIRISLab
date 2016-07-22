//
//  IRandom.cpp
//  ABMSolar
//
//  Created by Ekaterina Sinitskaya on 4/14/16.
//  Copyright (c) 2016 IRIS Lab. All rights reserved.
//

#include "Tools/ExternalIncludes.h"
#include "IRandom.h"



using solar_core;


double IRandom::r_pareto_2(double lambda, double alpha)
{
    return (lambda * std::pow((ru() + lambda / alpha), -(1/(alpha + 1))) - 1);
}