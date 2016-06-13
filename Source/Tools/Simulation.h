//
//  Simulation.h
//  ABMSolar
//
//  Created by Ekaterina Sinitskaya on 6/9/16.
//  Copyright (c) 2016 IRIS Lab. All rights reserved.
//

#ifndef __ABMSolar__Simulation__
#define __ABMSolar__Simulation__



#include "Tools/ExternalIncludes.h"
#include "Tools/IParameters.h"


namespace solar_core
{
    namespace tools
    {
        
        void create_joint_distribution();
        
        
        /**
         
         
         Empirical univariate distribution
         
        
         */
        class EmpriricalUVD
        {
        public:
            std::string name;
            ERandomParams type;
            std::vector<double> bin_ends;
            std::vector<long> bin_values;
            
            
        };
        
        
        /**
         
         
         Empirical multi-variate distribution
         
         */
        class EmpiricalMVD
        {
        public:
            std::vector<EmpiricalUVD> mvd;
            std::vector<long> values;
            std::vector<long> freq;
            
            
        };
        
        
        
        
    } //tools
    
} //solar_core


#endif /* defined(__ABMSolar__Simulation__) */
