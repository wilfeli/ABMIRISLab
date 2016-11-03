//
//  HelperW.h
//  ABMSolar
//
//  Created by Ekaterina Sinitskaya on 5/20/16.
//  Copyright (c) 2016 IRIS Lab. All rights reserved.
//

#ifndef __ABMSolar__HelperW__
#define __ABMSolar__HelperW__

#include "Tools/ParsingTools.h"
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

using boost::property_tree::read_json;

namespace solar_core {

    
    class W;

    class CoreModel {};
    class BaselineModel {};
    class UnitTestModel {};
    class ExploreExploit {};
    
    
    class HelperW
    {
    public:
        virtual void init(){};

    };
    

    template <class T, class Param>
    class HelperWSpecialization: public HelperW
    {
    public:
        
        

        template <class TA>
        std::vector<TA*>* create_hos(PropertyTree& pt, std::string mode_, boost::filesystem::path& path_to_dir, boost::variate_generator<boost::mt19937&, boost::uniform_int<uint64_t>>& rng_location_x, boost::variate_generator<boost::mt19937&, boost::uniform_int<uint64_t>>& rng_location_y, W* w_)
        {
            
            auto hos = new std::vector<TA*>();
            
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
            std::vector<std::vector<double>>* xs = new std::vector<std::vector<double>>();
            //draw from the joint distribution
            for(auto i = 0 ; i < w->params_d[EParamTypes::N_HO]; ++i)
            {
                xs->push_back(tools::draw_joint_distribution(e_dist, w->rand));
            };
            
            
            
            //read other parameters
            serialize::deserialize(pt.get_child("params"), params_str);
            std::map<EParamTypes, std::vector<double>>* param_values = new std::map<EParamTypes, std::vector<double>>();
            
            for (auto& iter:params_str)
            {
                EParamTypes name = EnumFactory::ToEParamTypes(iter.first);
                //explicitly solve for values
                auto params = serialize::create_dist_from_formula(iter.second, w->rand);
                
                if (params.valid_dist == true)
                {
                    (*param_values)[name] = std::vector<double>(w->params_d[EParamTypes::N_HO], 0.0);
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
                                    (*param_values)[name][i] = (*xs)[i][5] * WorldSettings::instance().params_exog[EParamTypes::ElectricityPriceUCDemand]/constants::NUMBER_DAYS_IN_YEAR * constants::NUMBER_DAYS_IN_MONTH;
                                };
                            }
                            else if (name == EParamTypes::Income)
                            {
                                for (auto i = 0; i < w->params_d[EParamTypes::N_HO]; ++i)
                                {
                                    (*param_values)[name][i] = (*xs)[i][4];
                                };
                            }
                            else if (name == EParamTypes::ElectricityConsumption)
                            {
                                for (auto i = 0; i < w->params_d[EParamTypes::N_HO]; ++i)
                                {
                                    (*param_values)[EParamTypes::ElectricityConsumption][i] = (*xs)[i][5]/constants::NUMBER_DAYS_IN_YEAR * constants::NUMBER_DAYS_IN_MONTH;
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
                    (*param_values)[name] = std::vector<double>(w->params_d[EParamTypes::N_HO], param);
                    
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
                pt.put("House.roof_size", std::max(0.0, roof_size((*xs)[i][0])));
                pt.put("House.roof_age", std::max(0.0, roof_age((*xs)[i][1])));
                pt.put("House.house_size", (*xs)[i][0]);
                
                
                //read configuration file
                //replace parameters if necessary
                hos->push_back(new TA(pt, w));
                
                
                //copy other parameters
                for (auto iter:(*param_values))
                {
                    hos->back()->params[iter.first] = (*param_values)[iter.first][i];
                };
                
                hos->back()->THETA_params = THETA_params;
                
            };
            
            
            delete e_dist;
            delete xs;
            delete param_values;
            return hos;

            
            
            
            
        }
        
        
        
        
    };
    
    
    
    template <class T>
    class HelperWSpecialization<T, BaselineModel>: public HelperW
    {
    public:

        std::vector<SEI*>* create_seis(PropertyTree& pt, std::string mode_, boost::filesystem::path& path_to_dir, boost::variate_generator<boost::mt19937&, boost::uniform_int<uint64_t>>& rng_location_x, boost::variate_generator<boost::mt19937&, boost::uniform_int<uint64_t>>& rng_location_y, W* w_)
        {
            std::vector<SEI*>* seis;
            
            T* w = static_cast<T*>(w_);
            
            //create random number generators for locations
            //is created here to speed up generation, otherwise rng is created for each agent, so location formula is not used directly.
            //check that it is uniform distribution
            if (pt.get<std::string>("location").find("FORMULA::p.d.f.::u_int(0, size)") == std::string::npos)
            {
                throw std::runtime_error("unsupported sei specification rule");
            };
            
            
            
            //parameters
            //read file from params for seis - create sei from it
            
            
            //read from file parameters,
            //convert into enums for types
            //depending on the type of techonology assign efficiency from database
            //draw random until efficiency is in the range required for this type of installer
            //for inverters - assume that all use both types of inverters for now - connect randomly to inverters?
            
            
            
            auto path_to_data = path_to_dir;
            path_to_data /= "df_installers.csv";
            
            //read file
            std::vector<std::vector<std::string>>* parsed_file = new std::vector<std::vector<std::string>>();
            tools::parse_csv_file(path_to_data.string(), parsed_file);

            
        
            
            
            std::map<std::string, int64_t> column_names  {{"Name", 0},
                                                            {"Module_Name_1", 1},
                                                            {"Efficiency", 2},
                                                            {"Inverter_Name_1", 3},
                                                            {"Inverter_Type_1", 4},
                                                            {"Inverter_Name_2", 5},
                                                            {"Inverter_Type_2", 6},
                                                            {"Size", 7},
                                                            {"Rating", 8},
                                                            {"Interaction", 9},
                                                            {"Technology", 10},
                                                            {"LeadInProjectTime", 11},
                                                            {"Warranty", 12}};
            
            std::map<std::string, int64_t> technology_types {{"Traditional" , 0},
                {"Standard", 1}, {"CuttingEdge", 2}};
            
            std::map<std::string, int64_t> interaction_types {{"Independent", 0}, {"Moderate", 1}, {"Collaborative", 2}};
            
            
            //list of precreated partial SEI, will add other parameters from json
            //similar to h creation algorithm
            

            
            //read bins for techonology types
            std::vector<double> technology_efficiency_bins;
            serialize::deserialize(pt.get_child("technology_efficiency_bins"), technology_efficiency_bins);
            
            std::map<std::string, std::vector<double>> technology_bins;
            
            technology_bins["Traditional"] = std::vector<double>{0,technology_efficiency_bins[0]};
            technology_bins["Standard"] = std::vector<double>{technology_efficiency_bins[0], technology_efficiency_bins[1]};
            technology_bins["CuttingEdge"] = std::vector<double>{technology_efficiency_bins[1], constants::SOLAR_INFINITY()};
            

            
            
            //SEM connections PV module and SEM connections Inverter
            auto max_ = w->sems->size() - 1;
            auto pdf_i = boost::uniform_int<uint64_t>(0, max_);
            auto rng_i = boost::variate_generator<boost::mt19937&, boost::uniform_int<uint64_t>>(w->rand_sei->rng, pdf_i);
            int64_t j_sem = 0;
            
            
            
            
            for (auto i = 0; i < w->params_d[EParamTypes::N_SEI]; ++i)
            {
                //put sei_type
                pt.put("sei_type", EnumFactory::FromEParamTypes(EParamTypes::None));
                
                //generate location
                pt.put("location_x", rng_location_x());
                pt.put("location_y", rng_location_y());
                
                
                seis->push_back(new SEI(pt, w));

                
                
                //set parameters
                //rating
                seis->back()->params[EParamTypes::SEIRating] = std::stod((*parsed_file)[i][column_names["Rating"]]);
                
                
                //Warranty
                seis->back()->params[EParamTypes::SEIWarranty] = std::stod((*parsed_file)[i][column_names["Warranty"]]);
                
                
                //LeadInProjectTime
                seis->back()->params[EParamTypes::SEILeadInProjectTime] = std::stod((*parsed_file)[i][column_names["LeadInProjectTime"]]);
                
                
                //interaction type
                seis->back()->params[EParamTypes::SEIInteractionType] = interaction_types[(*parsed_file)[i][column_names["Interaction"]]];
                
                
                //number specifies type
                //type of technology
                seis->back()->params[EParamTypes::SEIEquipmentType] = technology_types[(*parsed_file)[i][column_names["Technology"]]];
                

                
                //set manufacturer connection
                if (pt.get<std::string>("dec_design") == "FORMULA::RANDOM")
                {
                    
                    double technology_to_set = 2.0;
                    double pv_module_to_set = 1.0;
                    
                    while ((pv_module_to_set > 0.0) || (technology_to_set > 0.0))
                    {
                        j_sem = rng_i();
                        
                        
                        if ((*w->sems)[j_sem]->sem_type == EParamTypes::SEMPVProducer)
                        {
                            
                            auto cut_off_min = technology_bins[(*parsed_file)[i][column_names["Technology"]]][0];
                            auto cut_off_max = technology_bins[(*parsed_file)[i][column_names["Technology"]]][1];
                            
                            //check efficiency of the first template
                            if (((*w->sems)[j_sem]->solar_panel_templates[0]->efficiency >= cut_off_min) && ((*w->sems)[j_sem]->solar_panel_templates[0]->efficiency <= cut_off_max))
                            {
                                //connect with first template
                                //connect to random PV manufacturer,
                                seis->back()->dec_solar_modules[EParamTypes::SEIMidEfficiencyDesign] = (*w->sems)[j_sem]->solar_panel_templates[0];
                                --pv_module_to_set;
                                
                            };
                        }
                        else
                        {
                            //connect to random Inverter manufacturer
                            //check inverter type
                            //keep searching until required type is found
                            auto inverter_type = (*w->sems)[j_sem]->inverter_templates[0]->technology;

                            while (technology_to_set > 0.0)
                            {
                                if ((*w->sems)[j_sem]->sem_type == EParamTypes::SEMInverterProducer)
                                {
                                    if (inverter_type == ESEIInverterType::Central)
                                    {
                                        if (seis->back()->dec_inverters[EParamTypes::TechnologyInverterCentral] == nullptr)
                                        {
                                            seis->back()->dec_inverters[EParamTypes::TechnologyInverterCentral] = (*w->sems)[j_sem]->inverter_templates[0];
                                            
                                            --technology_to_set;
                                        };
                                    };
                                    
                                    if (inverter_type == ESEIInverterType::Micro)
                                    {
                                        if (seis->back()->dec_inverters[EParamTypes::TechnologyInverterMicro] == nullptr)
                                        {
                                            seis->back()->dec_inverters[EParamTypes::TechnologyInverterMicro] = (*w->sems)[j_sem]->inverter_templates[0];
                                            --technology_to_set;
                                        };
                                    };
                                    
                                    j_sem = rng_i();
                                }
                                else
                                {
                                    j_sem = rng_i();
                                };
                                
                            };
                            
                        };

                        
                        
                        
                    };
                    
                    
                }
                else
                {
                    throw std::runtime_error("unsupported specification");
                };
            };
            

            
            
            
            return seis;

            
            
            
            
            
        }
        
        std::vector<SEM*>* create_sems(PropertyTree& pt, std::string mode_, boost::filesystem::path& path_to_dir, boost::variate_generator<boost::mt19937&, boost::uniform_int<uint64_t>>& rng_location_x, boost::variate_generator<boost::mt19937&, boost::uniform_int<uint64_t>>& rng_location_y, W* w_)
        {
            
            
            T* w = static_cast<T*>(w_);
            auto sems = new std::vector<SEM*>();
            
            for (auto i = 0; i < w->params_d[EParamTypes::N_SEM]; ++i)
            {
                sems->push_back(new SEM(pt, w_));
            };
            
            
            
            //create connections
            auto max_ = WorldSettings::instance().solar_modules.size() - 1;
            auto pdf_i = boost::uniform_int<uint64_t>(0, max_);
            auto rng_i = boost::variate_generator<boost::mt19937&, boost::uniform_int<uint64_t>>(w->rand_sem->rng, pdf_i);
            
            //create PV module producer
            //Every SEM has one solar_module
            for (auto i = 0; i < w->params_d[EParamTypes::N_SEMPVProducer]; ++i)
            {
                auto iter = (*sems)[i];
                
                //set PV module
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
                        
                        iter->solar_panel_templates.push_back(module);
                        iter->init_world_connections();
                        
                        //no initialization for the module itself in this simple version
                        
                        break;
                    };
                };
                
                iter->sem_type = EParamTypes::SEMPVProducer;

            };
            
            //create inverter producers
            max_ = WorldSettings::instance().inverters.size() - 1;
            pdf_i = boost::uniform_int<uint64_t>(0, max_);
            auto rng_i_inv = boost::variate_generator<boost::mt19937&, boost::uniform_int<uint64_t>>(w->rand_sem->rng, pdf_i);
            
            
            for (auto i = w->params_d[EParamTypes::N_SEMPVProducer]; i < w->params_d[EParamTypes::N_SEMInverterProducer] + w->params_d[EParamTypes::N_SEMPVProducer]; ++i)
            {
                auto iter = (*sems)[i];
                
                //set inverter
                while (true)
                {
                    auto it = WorldSettings::instance().inverters.begin();
                    std::advance(it, rng_i_inv());
                    auto inverter = it->second;
                    if (inverter->manufacturer_id == "FORMULA::RANDOM")
                    {
                        //could use this inverter
                        inverter->manufacturer = iter;
                        inverter->manufacturer_id = inverter->manufacturer->uid.get_string();
                        
                        iter->inverter_templates.push_back(inverter);
                        iter->init_world_connections();
                        
                        //no initialization for the inverter itself in this simple version
                        
                        break;
                    };
                };
                
                iter->sem_type = EParamTypes::SEMInverterProducer;
                
            };

            
            int64_t N_Micro = 0;
            int64_t N_Central = 0;
            //check that there is at least 1 of each type?
            for (auto i = 0; i < (*sems).size(); ++i)
            {
                switch ((*sems)[i]->inverter_templates[0]->technology)
                {
                    case ESEIInverterType::Central:
                        ++N_Central;
                        break;
                    case ESEIInverterType::Micro:
                        ++N_Micro;
                        break;
                    default:
                        break;
                };
            };

            auto type_to_reset = ESEIInverterType::Micro;
            
            if (N_Central <= 0.0)
            {
                type_to_reset = ESEIInverterType::Central;
            };
            

            //draw last sem, check that it is inverter producer and convert into micro
            auto iter = (*sems).back();

            if (iter->sem_type != EParamTypes::SEMInverterProducer){ throw std::runtime_error("unsupported specification"); };
            
            //check that it is not the required type - maybe later, because now it is binary - so no need to check

            
            //set inverter
            while (true)
            {
                auto it = WorldSettings::instance().inverters.begin();
                std::advance(it, rng_i_inv());
                auto inverter = it->second;
                if ((inverter->manufacturer_id == "FORMULA::RANDOM") && (inverter->technology == type_to_reset))
                {
                    //could use this inverter
                    inverter->manufacturer = iter;
                    inverter->manufacturer_id = inverter->manufacturer->uid.get_string();
                    
                    
                    //reset old template
                    iter->inverter_templates[0]->manufacturer = nullptr;
                    iter->inverter_templates[0]->manufacturer_id = "FORMULA::RANDOM";
                    
                    
                    //set new template
                    //assume only 1 per manufacturer
                    iter->inverter_templates[0] = inverter;
                    iter->init_world_connections();
                    
                    //no initialization for the inverter itself in this simple version
                    
                    break;
                };
            };

            return sems;
            
        }
        
        
        
        std::vector<Homeowner*>* create_hos(PropertyTree& pt, std::string mode_, boost::filesystem::path& path_to_dir, boost::variate_generator<boost::mt19937&, boost::uniform_int<uint64_t>>& rng_location_x, boost::variate_generator<boost::mt19937&, boost::uniform_int<uint64_t>>& rng_location_y, W* w_)
        {
            //create raw Hs
            auto helper = solar_core::HelperWSpecialization<T, solar_core::CoreModel>();
            
            auto hos = helper.template create_hos<Homeowner>(pt, mode_, path_to_dir, rng_location_x, rng_location_y, w_);
            
            
            std::map<std::string, std::string> params_str;
            T* w = static_cast<T*>(w_);
            
            
            //create H accroding to distribution of types
            //read in information about types
            //read json with distribution parameters
            auto path_to_scheme = path_to_dir;
            PropertyTree pt_decisions;
         
            std::map<EParamTypes, std::string> decision {{EParamTypes::HONCDecision, "ho-ncdecisions.json"},{EParamTypes::HOSEIDecision, "ho-installerdecisions.json"},{EParamTypes::HODesignDecision, "ho-designdecisions.json"}};
            
            
            EParamTypes attribute;
            for (auto iter:decision)
            {
                //create distribution
                w->ho_decisions[iter.first] = new tools::EmpiricalHUVD();
                
                path_to_scheme = path_to_dir;
                path_to_scheme /= iter.second;
                
                //read json with specification
                read_json(path_to_scheme.string(), pt_decisions);
                
                
                for (const auto& node: pt)
                {
                    //read distribution of classes
                    for (auto& node_dist:node.second.get_child("scheme"))
                    {
                        attribute = EnumFactory::ToEParamTypes(node_dist.first);
                        serialize::deserialize(node_dist.second, w->ho_decisions[iter.first]->HOD_distribution_scheme[node.first][attribute]);
                    };
                    
                    
                    //read class preferences
                    w->ho_decisions[iter.first]->HOD_distribution[node.first] = node.second.get<double>("frequency");
                    //create labels and cmf to generate class labels
                    w->ho_decisions[iter.first]->labels.push_back(node.first);
                    w->ho_decisions[iter.first]->cmf.push_back(w->ho_decisions[iter.first]->cmf.back() + w->ho_decisions[iter.first]->HOD_distribution[node.first]);
                };
            };
            
            
            
            //read other parameters and save them to proper thetas
            
            //create HO
            std::string label;
            double u_i;
            
            for (auto i = 0; i < w->params_d[EParamTypes::N_HO]; ++i)
            {
                //draw next uniform
                u_i = w->rand_ho->ru();
                //generate class label given frequencies
                label = w->ho_decisions[EParamTypes::HONCDecision]->labels[tools::get_inverse_index(w->ho_decisions[EParamTypes::HONCDecision]->cmf, u_i)];
                
                //assume the same parameters for each class
                (*hos)[i]->THETA_NCDecisions = w->ho_decisions[EParamTypes::HONCDecision]->HOD_distribution_scheme[label];
                
                
                //generate class label given frequencies
                label = w->ho_decisions[EParamTypes::HOSEIDecision]->labels[tools::get_inverse_index(w->ho_decisions[EParamTypes::HOSEIDecision]->cmf, u_i)];
                
                //assume the same parameters for each class
                (*hos)[i]->THETA_SEIDecisions = w->ho_decisions[EParamTypes::HOSEIDecision]->HOD_distribution_scheme[label];

                
                //generate class label given frequencies
                label = w->ho_decisions[EParamTypes::HODesignDecision]->labels[tools::get_inverse_index(w->ho_decisions[EParamTypes::HODesignDecision]->cmf, u_i)];
                
                //assume the same parameters for each class
                (*hos)[i]->THETA_DesignDecisions = w->ho_decisions[EParamTypes::HODesignDecision]->HOD_distribution_scheme[label];
                
                
                if (i < w->params_d[EParamTypes::N_HOMarketingStateHighlyInterested])
                {
                    //create few highly interested agents
                    //set marketing state
                    (*hos)[i]->marketing_state = EParamTypes::HOMarketingStateHighlyInterested;
                }
                else
                {
                    (*hos)[i]->marketing_state = EParamTypes::HOMarketingStateNotInterested;
                };

                
                
               

                
            };
            

            //@DevStage3 if decide to have random utility  - when generating H assign utility from the description of a probability distribution
            
            return hos;

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
            //in the last position have adjustment parameter for maximum ratio in case of different sign of profit estimation
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
                        
                        iter->init_world_connections();
                        
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
            
            //create raw Hs
            //uses core version directly, so careful when changing it
            auto helper = solar_core::HelperWSpecialization<T, solar_core::CoreModel>();
            
            auto hos = helper.template create_hos<H>(pt, mode_, path_to_dir, rng_location_x, rng_location_y, w_);
        
            return hos;
        }
        
        
        
        
        
    };

    

    
} // solar_core namespace



#endif /* defined(__ABMSolar__HelperW__) */
