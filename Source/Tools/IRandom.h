//
//  IRandom.h
//  ABMSolar
//
//  Created by Ekaterina Sinitskaya on 4/14/16.
//  Copyright (c) 2016 IRIS Lab. All rights reserved.
//

#ifndef __ABMSolar__IRandom__
#define __ABMSolar__IRandom__




#include <boost/random.hpp>
#include <boost/random/normal_distribution.hpp>
#include <boost/random/uniform_int_distribution.hpp>
#include "Tools/IParameters.h"
#include "Tools/ExternalIncludes.h"



namespace solar_core
{
    
    class IRandom
    {
    public:
        IRandom(double seed_):rng(seed_), rnd(rng, boost::normal_distribution<>(0.0, 1.0)), ru(rng, boost::uniform_01<>()){} /*!< creates random number generator and seeds it */
        
    public:
        boost::mt19937 rng; /*!< random number generator */
        
    public:
        boost::variate_generator<boost::mt19937&, boost::normal_distribution<>> rnd;
        boost::variate_generator<boost::mt19937&, boost::uniform_01<>> ru;
    };
} //end solar_core namespace

#endif /* defined(__ABMSolar__IRandom__) */


