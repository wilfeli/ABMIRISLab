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
    
    class IRandom;
    
    
    
    namespace tools
    {
        
        
        
        
        /**
         
         
         Empirical univariate distribution
         
        
         */
        class EmpiricalUVD
        {
        public:
            EmpiricalUVD() = default;
            std::string name;
            ERandomParams type;
            std::vector<double> bin_ends;
            std::vector<long> bin_values;
            std::map<long, long> bin_values_map;
            
            std::vector<std::vector<long>> cond_values;
            std::vector<long> cond_freq;
            
            std::vector<std::vector<double>> theta_bins; /*! linear proxy to get internal values from inverse, if continuous */ 
            
            
        };
        
        
        /**
         
         
         Empirical multi-variate distribution
         
         */
        class EmpiricalMVD
        {
        public:
            std::vector<EmpiricalUVD> mvd;
            std::vector<std::vector<long>> values;
            std::vector<long> freq;
            std::vector<int64_t> n_bins_cum;
            
        };
        
        
        EmpiricalMVD create_joint_distribution(std::string path_to_scheme, std::string path_to_data);
        
        void calculate_pmf(std::vector<std::vector<long>>& bins, std::vector<std::vector<double>>& parsed_file, std::vector<long>& freq_n, EmpiricalMVD& e_dist, std::vector<int64_t>& N_BINS_CUM);
        
        std::vector<long> collapse_pmf(std::vector<long>& i_x, EmpiricalUVD& dist, EmpiricalMVD& pmf);
        
        double calculate_scale_factor(std::vector<long>& cond_dist, EmpiricalUVD& dist);
        
        std::vector<double> create_cmf(std::vector<long>& cond_dist, double scale_factor, EmpiricalUVD& dist);
        
        
        long get_inverse_index(std::vector<double>& cmf, double u_i);
        
        double get_inverse_value(std::vector<double>& theta_bin, double u_i);
        
        double get_inverse_value_exp(std::vector<double>& theta_bin, double u_i);
        
        std::vector<double> draw_joint_distribution(EmpiricalMVD& pmf, IRandom* rand);
        
        
    } //tools
    
} //solar_core


#endif /* defined(__ABMSolar__Simulation__) */
