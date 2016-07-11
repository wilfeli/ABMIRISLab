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
#include "Tools/Serialize.h"
#include "Tools/IRandom.h"
#include "Agents/SEI.h"
#include "Agents/Homeowner.h"
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
        
        
        
        /** Old version of generating H - with independent parameters  */
        std::vector<Homeowner*> create_hhs(PropertyTree& pt, std::string mode_, long N_HO, long N_HOMarketingStateHighlyInterested, boost::variate_generator<boost::mt19937&, boost::uniform_int<uint64_t>>& rng_location_x, boost::variate_generator<boost::mt19937&, boost::uniform_int<uint64_t>>& rng_location_y, IRandom* rand)
        {
            std::vector<Homeowner*> hhs;
            
            //create THETA_design
            std::map<std::string, std::vector<std::string>> THETA_design_str;
            serialize::deserialize(pt.get_child("THETA_design"), THETA_design_str);
            
            auto formula_THETA = THETA_design_str[EnumFactory::FromEParamTypes(EParamTypes::HODecPreliminaryQuote)];
            
            if (formula_THETA[0].find("FORMULA::p.d.f.::u(0, 1)") == std::string::npos)
            {
                throw std::runtime_error("unsupported hh specification rule");
            };
            
            auto pdf_THETA = boost::uniform_01<>();
            auto rng_THETA = boost::variate_generator<boost::mt19937&, boost::uniform_01<>>(rand->rng, pdf_THETA);
            
            
            std::map<EParamTypes, std::vector<double>> THETA_design;
            for (auto& iter:THETA_design_str)
            {
                THETA_design[EnumFactory::ToEParamTypes(iter.first)] = std::vector<double>{};
            };
            
            
            auto formula_roof_age = pt.get<std::string>("House.roof_age");
            auto formula_roof_size = pt.get<std::string>("House.roof_size");
            
            //create rundom number generators for House
            if (formula_roof_age.find("FORMULA::p.d.f.::N_trunc") == std::string::npos)
            {
                throw std::runtime_error("unsupported hh specification rule");
            };
            if (formula_roof_size.find("FORMULA::p.d.f.::N_trunc") == std::string::npos)
            {
                throw std::runtime_error("unsupported hh specification rule");
            };
    
            double mean_roof_age = std::stod(formula_roof_age.substr(formula_roof_age.find("(") + 1, formula_roof_age.find(",") - formula_roof_age.find("(") - 1));
            double sigma2_roof_age = std::stod(formula_roof_age.substr(formula_roof_age.find(",") + 1, formula_roof_age.find(",") - formula_roof_age.find(")") - 1));
    
            auto pdf_roof_age = boost::normal_distribution<>(mean_roof_age, std::pow(sigma2_roof_age, 0.5));
            auto rng_roof_age = boost::variate_generator<boost::mt19937&, boost::normal_distribution<>>(rand->rng, pdf_roof_age);
    
    
            double mean_roof_size = std::stod(formula_roof_size.substr(formula_roof_size.find("(") + 1, formula_roof_size.find(",") - formula_roof_size.find("(") - 1));
            double sigma2_roof_size = std::stod(formula_roof_size.substr(formula_roof_size.find(",") + 1, formula_roof_size.find(",") - formula_roof_size.find(")") - 1));
    
            auto pdf_roof_size = boost::normal_distribution<>(mean_roof_size, std::pow(sigma2_roof_size, 0.5));
            auto rng_roof_size = boost::variate_generator<boost::mt19937&, boost::normal_distribution<>>(rand->rng, pdf_roof_size);

            
            
            //create HO
            auto j = 0;
            for (auto i = 0; i < N_HO; ++i)
            {
                if (j < N_HOMarketingStateHighlyInterested)
                {
                    //create few highly interested agents
                    //put specific parameters into template
                    pt.put("marketing_state", EnumFactory::FromEParamTypes(EParamTypes::HOMarketingStateHighlyInterested));
                };
                
                ++j;
                
                //generate location
                pt.put("location_x", rng_location_x());
                pt.put("location_y", rng_location_y());
                
            
                
                //generate House
                //roof_age
                //roof_size
                pt.put("House.roof_age", std::max(0.0, rng_roof_age()));
                pt.put("House.roof_size", std::max(0.0, rng_roof_size()));
                
                //create decision parameters
                THETA_design[EParamTypes::HODecPreliminaryQuote] = std::vector<double>{rng_THETA()};
                pt.put_child("THETA_design", serialize::serialize(THETA_design, "THETA_design").get_child("THETA_design"));
                
                
                
                
                
                
                //read configuration file
                //replace parameters if necessary
                hhs.push_back(new Homeowner(pt, this));
                
                
                
                
            };

            
            
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
                
                
                //only difference is here - to differenciate SEI and SEIMock
                seis.push_back(new solar_tests::SEIMock(pt, w));
                
                
            };
            return seis;
            
        }
    };

    

    
} // solar_core namespace



#endif /* defined(__ABMSolar__HelperW__) */
