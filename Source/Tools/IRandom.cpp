//
//  IRandom.cpp
//  ABMSolar
//
//  Created by Ekaterina Sinitskaya on 4/14/16.
//  Copyright (c) 2016 IRIS Lab. All rights reserved.
//

#include "Tools/ExternalIncludes.h"
#include "Tools/IRandom.h"



using namespace solar_core;


double IRandom::r_pareto_2(double lambda, double alpha)
{
    return (lambda * std::pow((1 - ru()), -(1/alpha)) - 1);
}