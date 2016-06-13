//
//  Simulation.cpp
//  ABMSolar
//
//  Created by Ekaterina Sinitskaya on 6/9/16.
//  Copyright (c) 2016 IRIS Lab. All rights reserved.
//

#include "Tools/Simulation.h"
#include "Tools/IParameters.h"
#include "Tools/Serialize.h"

using namespace solar_core;

void
tools::create_joint_distribution(std::string path_to_scheme, std::string path_to_data)
{
    //read file
    std::vector<std::vector<double>> parsed_file;
    std::string path_to_file;
    
    tools::parse_csv_file(path_to_data, parsed_file);
    
    
    //read file with specification
    
    
    //assume each high level has a name for the variable in joint distribution
    //iterate over them, get N_BINS, for each variable as a result create vector of values, and leave the last blank to add frequency for this combination
    
    PropertyTree pt;
    read_json(path_to_scheme, pt);
    
    std::vector<long> N_BINS;
    std::vector<std::vector<long>> BIN_VALUES;
    std::vector<int64_t> N_BINS_CUM;
    
    //total number of bins
    N_BINS_CUM.push_back(1);
    
    for (const auto& node: pt)
    {
        N_BINS.push_back(node.second.get<long>("N_BINS"));
        
        //get bin values if discrete, otherwise assume it will be just numbers
        std::vector<double> bin_ends;
        BIN_VALUES.push_back(std::vector<long>{});
        if (node.second.get<std::string>("type") == "continious")
        {
            //bin edges
            serialize::deserialize(node.second.get_child("BIN_ENDS"), bin_ends);
        };

        //read BIN_VALUES from the file
        serialize::deserialize(node.second.get_child("values"), BIN_VALUES.back());
    };
    
    for (auto i = 0; i < N_BINS.size(); ++i)
    {
        N_BINS_CUM[i + 1] = N_BINS_CUM[i] * N_BINS[N_BINS.size() - 1 - i];
    };
    
    
    
    //joint distribution table
    std::vector<std::vector<long>> bins(N_BINS_CUM.back(), std::vector<long>(N_BINS.size(), 0.0));
    
    
    for (int64_t i = 0; i < bins.size(); ++i)
    {
        for(int64_t j = 0; j < N_BINS.size(); ++j)
        {
            //
            auto N_TAILS = N_BINS_CUM[N_BINS_CUM.size() - 2 - j];
            auto i_sub = i % (N_TAILS * N_BINS[j]);
            long k = i_sub/N_TAILS;
            bins[i][j] = BIN_VALUES[j][k];
        };
    };
    
    
    //create vector of frequencies
    std::vector<long> freq_n(N_BINS_CUM.back(), 0);
    
    
    //calculate frequencies
    for (auto i = 0; i < bins.size(); ++i)
    {
        auto x_i = std::find_if(parsed_file.begin(), parsed_file.end(), [&](std::vector<double> &x_i_){
            //check that x_i_ is equal to x
            bool FLAG_EQ = true;
            for(auto j = 0; j < x_i_.size(); ++j)
            {
                if (x_i_[j] != bins[i][j])
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
