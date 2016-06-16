//
//  Simulation.cpp
//  ABMSolar
//
//  Created by Ekaterina Sinitskaya on 6/9/16.
//  Copyright (c) 2016 IRIS Lab. All rights reserved.
//

#include "Tools/Simulation.h"
#include "Tools/Serialize.h"
#include "Tools/ParsingTools.h"
#include "Tools/IRandom.h"

using namespace solar_core;

tools::EmpiricalMVD
tools::create_joint_distribution(std::string path_to_scheme, std::string path_to_data)
{
    //read file
    std::vector<std::vector<double>> parsed_file;
    std::string path_to_file;
    
    tools::parse_csv_file(path_to_data, parsed_file);
    
    
    //read file with specification
    //assume each high level has a name for the variable in joint distribution
    //iterate over them, get N_BINS,
    
    PropertyTree pt;
    read_json(path_to_scheme, pt);
    
    
    EmpiricalMVD e_dist;
    
    
    std::vector<long> N_BINS;
    std::vector<int64_t> N_BINS_CUM;
    
    //total number of bins
    N_BINS_CUM.push_back(1);
    
    for (const auto& node: pt)
    {
        e_dist.mvd.push_back(EmpiricalUVD());
        N_BINS.push_back(node.second.get<long>("N_BINS"));
        
        //get bin values if discrete, otherwise assume it will be just numbers
        if (node.second.get<std::string>("type") == "continious")
        {
            e_dist.mvd.back().type = ERandomParams::ContiniousDiscretized;
            //bin edges
            serialize::deserialize(node.second.get_child("BIN_ENDS"), e_dist.mvd.back().bin_ends);
        }
        else
        {
            e_dist.mvd.back().type = ERandomParams::Discrete;
        };

        //read BIN_VALUES from the file
        serialize::deserialize(node.second.get_child("values"), e_dist.mvd.back().bin_values);
        
        //read name from the file
        e_dist.mvd.back().name = node.second.get<std::string>("name");
    };
    
    for (auto i = 0; i < N_BINS.size(); ++i)
    {
        N_BINS_CUM[i + 1] = N_BINS_CUM[i] * N_BINS[N_BINS.size() - 1 - i];
    };
    
    
    
    //joint distribution table
    e_dist.values = std::vector<std::vector<long>>(N_BINS_CUM.back(), std::vector<long>(N_BINS.size(), 0.0));
    
    
    for (int64_t i = 0; i < e_dist.values.size(); ++i)
    {
        for(int64_t j = 0; j < N_BINS.size(); ++j)
        {
            //
            auto N_TAILS = N_BINS_CUM[N_BINS_CUM.size() - 2 - j];
            auto i_sub = i % (N_TAILS * N_BINS[j]);
            long k = i_sub/N_TAILS;
            e_dist.values[i][j] = k; //from what bin to take values    //BIN_VALUES[j][k];
        };
    };
    
    
    //create vector of frequencies
    e_dist.freq = std::vector<long>(N_BINS_CUM.back(), 0);
    
    
    //calculate frequencies
    for (auto i = 0; i < e_dist.values.size(); ++i)
    {
        auto x_i = std::find_if(parsed_file.begin(), parsed_file.end(), [&](std::vector<double> &x_i_){
            //check that x_i_ is equal to x
            bool FLAG_EQ = true;
            for(auto j = 0; j < x_i_.size(); ++j)
            {
                //check what value it should be
                if (x_i_[j] != e_dist.mvd[j].bin_values[e_dist.values[i][j]])
                {
                    FLAG_EQ = false;
                    break;
                };
            };
            
            return FLAG_EQ;});
        
        //count number
        for (; x_i != parsed_file.end(); ++x_i)
        {
            e_dist.freq[i] += 1;
        };
    };
    
    //store index into actual bin values
    for (auto value_add = 0; value_add < e_dist.mvd[0].bin_values.size(); ++value_add)
    {
        e_dist.mvd[0].cond_values.push_back(std::vector<long>{});
        e_dist.mvd[0].cond_values.back().push_back(value_add);
    };
    
    
    //take table from the previous one and multiply by own values
    for (auto i = 1; i < e_dist.mvd.size(); ++i)
    {
        //take previous distribution and multiply by own bin values
        for (auto value: e_dist.mvd[i-1].cond_values)
        {
            for (auto value_add = 0 ; value_add< e_dist.mvd[i].bin_values.size(); ++value_add)
            {
                e_dist.mvd[i].cond_values.push_back(value);
                e_dist.mvd[i].cond_values.back().push_back(value_add);
            };
        };
        
        //calculate conditional pmf
        calculate_pmf(e_dist.mvd[i].cond_values, parsed_file, e_dist.mvd[i].cond_freq, e_dist);
        
    };

    return e_dist;
    
    
}



void
tools::calculate_pmf(std::vector<std::vector<long>>& bins, std::vector<std::vector<double>>& parsed_file, std::vector<long>& freq_n, EmpiricalMVD& e_dist)
{
    //calculate frequencies
    for (auto i = 0; i < bins.size(); ++i)
    {
        auto x_i = std::find_if(parsed_file.begin(), parsed_file.end(), [&](std::vector<double> &x_i_){
            //check that x_i_ is equal to x
            bool FLAG_EQ = true;
            for(auto j = 0; j < bins[i].size(); ++j)
            {
                if (x_i_[j] != e_dist.mvd[j].bin_values[bins[i][j]])
                {
                    FLAG_EQ = false;
                    break;
                };
            };
            
            return FLAG_EQ;});
        
        //count number
        for (; x_i != parsed_file.end(); ++x_i)
        {
            freq_n[i] += 1;
        };
    };



}





std::vector<double>
tools::draw_joint_distribution(EmpiricalMVD& pmf, IRandom* rand)
{
    
    std::vector<double> x;
    std::vector<long> i_x; //indices for bin values for each drawn discrete distribution, and into bins for continious - the same because used for collapsing cond distribution
    //initialize uniform distribution
    double u_i = 0.0;
    
    
    //go through the list of univariate distributions
    for (auto dist:pmf.mvd)
    {
        //in indexes last value will be bin index
        //collapse conditional for already drawn values
        //returns list of indices into main list for conditional distribution
        auto cond_dist = collapse_pmf(i_x, dist);
        
        //calculate scale factor
        auto scale_factor = calculate_scale_factor(cond_dist, dist);
        
        //draw first from conditional distribution
        //create cmf - vector of end cumulative values from cond_dist
        auto cmf = create_cmf(cond_dist, scale_factor, dist);
        
        //if it is discrete - just get inverse index, cmf has value and index; and bin value from that
        //draw next uniform
        u_i = rand->ru();
        auto i = get_inverse_index(cmf, u_i);
        i_x.push_back(dist.cond_values[cond_dist[i]].back());
        if (dist.type == ERandomParams::Discrete)
        {
            x.push_back(dist.bin_values[i_x.back()]);
        }
        else if (dist.type == ERandomParams::ContiniousDiscretized)
        {
            //if it is continious - get inverse index and inverse value from the corresponding description
            //will point to the bin to use - simple linear inverse
            //check if it is last bin and if it will include inf right end
            
            if (( i == dist.theta_bins.size() - 1) && (dist.bin_ends.back() == constants::SOLAR_INFINITY()))
            {
                x.push_back(get_inverse_value_exp(dist.theta_bins[i], u_i));
            }
            else
            {
                x.push_back(get_inverse_value(dist.theta_bins[i], u_i));
            };
        };
        
    };
    return x;
}




std::vector<long>
tools::collapse_pmf(std::vector<long>& i_x, EmpiricalUVD& dist)
{
    std::vector<long> cond_dist;
    bool FLAG_EQ = true;
    //pick only rows with values from there
    for (auto i = 0; i < dist.cond_values.size(); ++i)
    {
        //should still push all as will never go into the cycle in the first place
        FLAG_EQ = true;
        for (auto j = 0; j < i_x.size(); ++j)
        {
            if (dist.cond_values[i][j] != i_x[j])
            {
                FLAG_EQ = false;
                break;
            };
        };
        if (FLAG_EQ)
        {
            cond_dist.push_back(i);
        };
    };
    
    
    return cond_dist;
    
}



double
tools::calculate_scale_factor(std::vector<long>& cond_dist, EmpiricalUVD& dist)
{
    double scale_factor = 0;
    
    for (auto i:cond_dist)
    {
        scale_factor += dist.cond_freq[i];
    };
    
    
    return scale_factor;
}



std::vector<double>
tools::create_cmf(std::vector<long>& cond_dist, double scale_factor, EmpiricalUVD& dist)
{
    std::vector<double> cmf{0};
    for (auto i = 0; i < cond_dist.size(); ++i)
    {
        cmf.push_back(cmf[i] + dist.cond_freq[cond_dist[i]] / scale_factor);
    };
    
    
    dist.theta_bins.clear();
    //update theta_bins
    for (auto i = 1; i < cmf.size(); ++i)
    {
        dist.theta_bins.push_back(std::vector<double>{});
        if (( i != cmf.size() - 1) || ((i == cmf.size() - 1) && (dist.bin_ends.back() != constants::SOLAR_INFINITY())))
        {
            dist.theta_bins.back().push_back(cmf[i-1]);
            dist.theta_bins.back().push_back(dist.bin_ends[i] - dist.bin_ends[i-1]);
            dist.theta_bins.back().push_back(cmf[i] - cmf[i-1]);
            dist.theta_bins.back().push_back(dist.bin_ends[i-1]);
        }
        else
        {
            dist.theta_bins.back().push_back(std::exp(dist.bin_ends[i-1]) * (1 - cmf[i-1]));
        };
    };
    
    
    return cmf;
    
}


long
tools::get_inverse_index(std::vector<double>& cmf, double u_i)
{
    long i = 0;
    for (; i < cmf.size(); ++i)
    {
        if (u_i > cmf[i])
        {
            break;
        };
    };
    
    //breaks when crossed over, so returns step back
    return i - 1;
}



double
tools::get_inverse_value(std::vector<double>& theta_bin, double u_i)
{
    return (u_i - theta_bin[0])*(theta_bin[1]/theta_bin[2])+theta_bin[3];
    
}


double
tools::get_inverse_value_exp(std::vector<double>& theta_bin, double u_i)
{
    //uses exponent as a proxy
    
    return (-std::log((1 - u_i)/theta_bin[0]));
    
    
}



