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
#include "Tools/Simulation.h"
#include "Tools/WorldSettings.h"
#include "Agents/H.h"
#include "Agents/SEI.h"
#include "Agents/SEIBL.h"
#include "Agents/SEMBL.h"
#include "Agents/Homeowner.h"
#include "../Tests/Agents/SEIMock.h"


namespace solar_core {

    
    class W;

    class BaselineModel {};
    class UnitTestModel {};
    class ExploreExploit {};
    
    
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
        
        
        
        /** Old version of generating H - with independent parameters. Is not used in BaselineModel  */
        std::vector<Homeowner*> create_hhs_independent_params(PropertyTree& pt, std::string mode_, long N_HO, long N_HOMarketingStateHighlyInterested, boost::variate_generator<boost::mt19937&, boost::uniform_int<uint64_t>>& rng_location_x, boost::variate_generator<boost::mt19937&, boost::uniform_int<uint64_t>>& rng_location_y, IRandom* rand)
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
    
    
    template <class T>
    class HelperWSpecialization<T, ExploreExploit>: public HelperW
    {
    public:
        /** Here it will be used by dynamically casting to the proper class to get access to this specific initialization   */
        std::vector<SEIBL*>* create_seis(PropertyTree& pt, std::string mode_, long N_SEI, boost::variate_generator<boost::mt19937&, boost::uniform_int<uint64_t>>& rng_location_x, boost::variate_generator<boost::mt19937&, boost::uniform_int<uint64_t>>& rng_location_y, W* w_)
        {
            T* w = static_cast<T*>(w_);
            auto seis = new std::vector<SEIBL*>();
            
            //SEM connections
            auto max_ = w->sems->size() - 1;
            auto pdf_i = boost::uniform_int<uint64_t>(0, max_);
            auto rng_i = boost::variate_generator<boost::mt19937&, boost::uniform_int<uint64_t>>(w->rand->rng, pdf_i);
            int64_t j_sem = 0;
            
            //check how many exploreres to create
            int64_t N_Explorer = pt.get<double>("THETA_exploration.Share::explorer") * N_SEI;
            std::vector<double> THETA_explorer;
            std::vector<double> THETA_exploiter;
            serialize::deserialize(pt.get_child("THETA_exploration.FORMULA::explorer"), THETA_explorer);
            serialize::deserialize(pt.get_child("THETA_exploration.FORMULA::exploiter"), THETA_exploiter);

            
            
            for (auto i = 0; i < N_SEI; ++i)
            {
                //put sei_type
                pt.put("sei_type", EnumFactory::FromEParamTypes(EParamTypes::SEISmall));
                
                //generate location
                pt.put("location_x", rng_location_x());
                pt.put("location_y", rng_location_y());
                
                
                seis->push_back(new SEIBL(pt, w));
                
                if (i < N_Explorer)
                {
                    //create explorer
                    seis->back()->THETA_exploration = THETA_explorer;
                }
                else
                {
                    seis->back()->THETA_exploration = THETA_exploiter;
                };
                
                

                
                //set manufacturer connection
                if (pt.get<std::string>("dec_design") == "FORMULA::RANDOM")
                {
                    j_sem = rng_i();
                    //connect to random sem manufacturer
                    seis->back()->dec_design->PV_module = (*w->sems)[j_sem]->solar_panel_templates[EDecParams::CurrentTechnology];
                    //record as connection
                    (*w->sems)[j_sem]->add_connection(seis->back()->dec_design->PV_module);
                    
                }
                else
                {
                    throw std::runtime_error("unsupported specification");
                };
            };
            
            
            //set market shares for them
            for (auto& sei: *seis)
            {
                w->market_share_seis[sei->uid] = 1/N_SEI;
            };
            
            
            
            return seis;
        }
        
        
        
        std::vector<SEMBL*>* create_sems(const PropertyTree& pt, std::string mode_, long N_SEM, boost::variate_generator<boost::mt19937&, boost::uniform_int<uint64_t>>& rng_location_x, boost::variate_generator<boost::mt19937&, boost::uniform_int<uint64_t>>& rng_location_y, W* w_)
        {
            T* w = static_cast<T*>(w_);
            auto sems = new std::vector<SEMBL*>();
            
            for (auto i = 0; i < N_SEM; ++i)
            {
                sems->push_back(new SEMBL(pt, w_));
            };
            
            
            
            //create connections
            auto max_ = WorldSettings::instance().solar_modules.size() - 1;
            auto pdf_i = boost::uniform_int<uint64_t>(0, max_);
            auto rng_i = boost::variate_generator<boost::mt19937&, boost::uniform_int<uint64_t>>(w->rand->rng, pdf_i);
            
            
            
            //Every SEM has one solar_module
            for (auto iter:*sems)
            {
                while (true)
                {
                    auto it = WorldSettings::instance().solar_modules.begin();
                    std::advance(it, rng_i());
                    auto module = it->second;
                    if (module->manufacturer_id == "FORMULA::RANDOM")
                    {
                        //could use this module
                        module->manufacturer = iter;
                        module->manufacturer_id = module->manufacturer->uid.get_string();
                        
                        //intrusive setting, UGLY
                        dynamic_cast<SEMBL*>(module->manufacturer)->solar_panel_templates[EDecParams::CurrentTechnology] = std::static_pointer_cast<SolarModuleBL>(module);
                        //set parameters for the module
                        dynamic_cast<SEMBL*>(module->manufacturer)->solar_panel_templates[EDecParams::CurrentTechnology]->init();

                        break;
                    };
                };
            };
            
            return sems;
        }
        
        
        
        std::vector<H*>* create_hos(PropertyTree& pt, std::string mode_, boost::filesystem::path& path_to_dir, boost::variate_generator<boost::mt19937&, boost::uniform_int<uint64_t>>& rng_location_x, boost::variate_generator<boost::mt19937&, boost::uniform_int<uint64_t>>& rng_location_y, W* w_)
        {
            
            
            auto hos = new std::vector<H*>();
            
            std::map<std::string, std::string> params_str;
            T* w = static_cast<T*>(w_);
            
            //read json with distribution parameters
            auto path_to_data = path_to_dir;
            path_to_data /= "joint_distribution.csv";
            auto path_to_scheme = path_to_dir;
            path_to_scheme /= "distribution.json";
            
            
            auto e_dist = tools::create_joint_distribution(path_to_scheme.string(), path_to_data.string());
            
            //will draw electricity bill for each, roof size as a constant percent of a house size, income
            auto formula_roof_age = pt.get<std::string>("House.roof_age");
            auto formula_roof_size = pt.get<std::string>("House.roof_size");
            auto roof_size_coef = 0.0;
            auto roof_age_coef = 0.0;
            std::regex e("");
            
            if (formula_roof_size.find("FORMULA::house_size::") == std::string::npos)
            {
                throw std::runtime_error("unsupported hh specification rule");
            }
            else
            {
                e.assign("FORMULA\\u003A\\u003Ahouse_size\\u003A\\u003A");
                formula_roof_size = std::regex_replace(formula_roof_size, e, "");
                roof_size_coef = serialize::solve_str_formula<double>(formula_roof_size, *(w->rand));
            };
            
            auto roof_size = [&coef = roof_size_coef](double house_size)->double {return coef*house_size;};
            
            if (formula_roof_age.find("FORMULA::house_age::") == std::string::npos)
            {
                throw std::runtime_error("unsupported hh specification rule");
            }
            else
            {
                e.assign("FORMULA\\u003A\\u003Ahouse_age\\u003A\\u003A");
                formula_roof_age = std::regex_replace(formula_roof_age, e, "");
                roof_age_coef = serialize::solve_str_formula<double>(formula_roof_age, *(w->rand));
            };
            
            auto roof_age = [&coef = roof_age_coef](double house_age)->double {return coef*house_age;};
            
            
            //in the order of drawing for now
            //df_save = df[['TOTSQFT_C', 'YEARMADERANGE', 'ROOFTYPE', 'TREESHAD', 'MONEYPY', 'KWH_C']]
            std::vector<std::vector<double>> xs;
            //draw from the joint distribution
            for(auto i = 0 ; i < w->params_d[EParamTypes::N_HO]; ++i)
            {
                xs.push_back(tools::draw_joint_distribution(e_dist, w->rand));
            };
            
            
            
            //read other parameters
            serialize::deserialize(pt.get_child("params"), params_str);
            std::map<EParamTypes, std::vector<double>> param_values;
            
            for (auto& iter:params_str)
            {
                EParamTypes name = EnumFactory::ToEParamTypes(iter.first);
                //explicitly solve for values
                auto params = serialize::create_dist_from_formula(iter.second, w->rand);
                
                if (params.valid_dist == true)
                {
                    param_values[name] = std::vector<double>(w->params_d[EParamTypes::N_HO], 0.0);
                    //generate map of random number generators with the name for this parameter?
                    switch (params.type)
                    {
                        case ERandomParams::N_trunc:
                            throw std::runtime_error("unfinished thread, should not be here");
                            break;
                        case ERandomParams::custom:
                            //see what parameter it is - save values
                            if (name == EParamTypes::ElectricityBill)
                            {
                                for (auto i = 0; i < w->params_d[EParamTypes::N_HO]; ++i)
                                {
                                    param_values[name][i] = xs[i][5] * WorldSettings::instance().params_exog[EParamTypes::ElectricityPriceUCDemand]/constants::NUMBER_DAYS_IN_YEAR * constants::NUMBER_DAYS_IN_MONTH;
                                };
                            }
                            else if (name == EParamTypes::Income)
                            {
                                for (auto i = 0; i < w->params_d[EParamTypes::N_HO]; ++i)
                                {
                                    param_values[name][i] = xs[i][4];
                                };
                            }
                            else if (name == EParamTypes::ElectricityConsumption)
                            {
                                for (auto i = 0; i < w->params_d[EParamTypes::N_HO]; ++i)
                                {
                                    param_values[EParamTypes::ElectricityConsumption][i] = xs[i][5]/constants::NUMBER_DAYS_IN_YEAR * constants::NUMBER_DAYS_IN_MONTH;
                                };
                            }
                            else
                            {
                                throw std::runtime_error("unsupported ho specification rule");
                            
                            };
                            
                            break;
                        default:
                            break;
                    };
                }
                else
                {
                    //get value and store it as a value for all agents?
                    auto param = serialize::solve_str_formula<double>(iter.second, *(w->rand));
                    param_values[name] = std::vector<double>(w->params_d[EParamTypes::N_HO], param);
                    
                };
            };
            
            
            std::vector<double> THETA_params;
            serialize::deserialize(pt.get_child("THETA_params"),THETA_params);
            
            
            //create HO
            for (auto i = 0; i < w->params_d[EParamTypes::N_HO]; ++i)
            {
                //generate location
                pt.put("location_x", rng_location_x());
                pt.put("location_y", rng_location_y());
                
                
                //generate House
                //roof_age
                //roof_size
                pt.put("House.roof_size", std::max(0.0, roof_size(xs[i][0])));
                pt.put("House.roof_age", std::max(0.0, roof_age(xs[i][1])));
                pt.put("House.house_size", xs[i][0]);
                
                
                //read configuration file
                //replace parameters if necessary
                hos->push_back(new H(pt, w));
                
                
                //copy other parameters
                for (auto iter:param_values)
                {
                    hos->back()->params[iter.first] = param_values[iter.first][i];
                };
                
                hos->back()->THETA_params = THETA_params;
                
            };

        
            delete e_dist;
            return hos;
        }
        
        
        
        
        
    };

    

    
} // solar_core namespace



#endif /* defined(__ABMSolar__HelperW__) */
