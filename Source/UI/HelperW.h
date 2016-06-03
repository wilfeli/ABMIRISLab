//
//  HelperW.h
//  ABMSolar
//
//  Created by Ekaterina Sinitskaya on 5/20/16.
//  Copyright (c) 2016 IRIS Lab. All rights reserved.
//

#ifndef __ABMSolar__HelperW__
#define __ABMSolar__HelperW__

#include "Tools/ExternalIncludes.h"
#include "Tools/IRandom.h"
#include "Agents/SEI.h"
#include "../Tests/Agents/SEIMock.h"


namespace solar_core {

    
    class W;

    class BaselineModel {};
    class UnitTestModel {};
    class HelperW
    {
    public:
        virtual std::vector<SEI*> create_seis(PropertyTree& pt_, std::string mode_, long N_SEI, long N_SEILarge, boost::variate_generator<boost::mt19937&, boost::uniform_int<uint64_t>>& rng_location_x, boost::variate_generator<boost::mt19937&, boost::uniform_int<uint64_t>>& rng_location_y, W* w_) = 0;
    };
    

    template <class T, class Param>
    class HelperWSpecialization: public HelperW
    {
    public:
        
        std::vector<SEI*> create_seis(PropertyTree& pt_, std::string mode_, long N_SEI, long N_SEILarge, boost::variate_generator<boost::mt19937&, boost::uniform_int<uint64_t>>& rng_location_x, boost::variate_generator<boost::mt19937&, boost::uniform_int<uint64_t>>& rng_location_y, W* w_) override
        {
            
        }
    };
    
    
    
    template <class T>
    class HelperWSpecialization<T, BaselineModel>: public HelperW
    {
    public:

        std::vector<SEI*> create_seis(PropertyTree& pt, std::string mode_, long N_SEI, long N_SEILarge, boost::variate_generator<boost::mt19937&, boost::uniform_int<uint64_t>>& rng_location_x, boost::variate_generator<boost::mt19937&, boost::uniform_int<uint64_t>>& rng_location_y, W* w_)
        {
            std::vector<SEI*> seis;
            
            T* w = static_cast<T*>(w_);
            
            //create random number generators for locations
            //is created here to speed up generation, otherwise rng is created for each agent, so location formula is not used directly.
            //check that it is uniform distribution
            if (pt.get<std::string>("location").find("FORMULA::p.d.f.::u_int(0, size)") == std::string::npos)
            {
                throw std::runtime_error("unsupported hh specification rule");
            };
            
            
            //create SEI - use template for parameters, use model file for additional parameters
            //create sei_type
            auto j = 0;
            for (auto i = 0; i < N_SEI; ++i)
            {
                //put sei_type
                pt.put("sei_type", EnumFactory::FromEParamTypes(EParamTypes::SEISmall));
                if (j < N_SEILarge)
                {
                    pt.put("sei_type", EnumFactory::FromEParamTypes(EParamTypes::SEILarge));
                };
                ++j;
                
                
                //generate location
                pt.put("location_x", rng_location_x());
                pt.put("location_y", rng_location_y());
                
                
                seis.push_back(new SEI(pt, w));
                
                
            };
            return seis;
            
        }
    };
    
    
    
    template <class T>
    class HelperWSpecialization<T, UnitTestModel>: public HelperW
    {
    public:
        
        std::vector<SEI*> create_seis(PropertyTree& pt, std::string mode_, long N_SEI, long N_SEILarge, boost::variate_generator<boost::mt19937&, boost::uniform_int<uint64_t>>& rng_location_x, boost::variate_generator<boost::mt19937&, boost::uniform_int<uint64_t>>& rng_location_y, W* w_)
        {
            std::vector<SEI*> seis;
            
            T* w = static_cast<T*>(w_);
            
            //create random number generators for locations
            //is created here to speed up generation, otherwise rng is created for each agent, so location formula is not used directly.
            //check that it is uniform distribution
            if (pt.get<std::string>("location").find("FORMULA::p.d.f.::u_int(0, size)") == std::string::npos)
            {
                throw std::runtime_error("unsupported hh specification rule");
            };
            
            
            //create SEI - use template for parameters, use model file for additional parameters
            //create sei_type
            auto j = 0;
            for (auto i = 0; i < N_SEI; ++i)
            {
                //put sei_type
                pt.put("sei_type", EnumFactory::FromEParamTypes(EParamTypes::SEISmall));
                if (j < N_SEILarge)
                {
                    pt.put("sei_type", EnumFactory::FromEParamTypes(EParamTypes::SEILarge));
                };
                ++j;
                
                
                //generate location
                pt.put("location_x", rng_location_x());
                pt.put("location_y", rng_location_y());
                
                
                seis.push_back(new solar_tests::SEIMock(pt, w));
                
                
            };
            return seis;
            
        }
    };

    

    
} // solar_core namespace



#endif /* defined(__ABMSolar__HelperW__) */
