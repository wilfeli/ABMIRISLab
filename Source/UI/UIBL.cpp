//
//  UIBL.cpp
//  ABMSolar
//
//  Created by Ekaterina Sinitskaya on 7/29/16.
//  Copyright (c) 2016 IRIS Lab. All rights reserved.
//


#include "UI/UIBL.h"
#include "UI/WEE.h"

#include <boost/filesystem.hpp>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>

#include "Agents/SEIBL.h"
#include "Agents/H.h"
#include "Agents/SolarPanel.h"
#include "Institutions/IMessage.h"

using namespace solar_ui;


void
UIBL::init(solar_core::WEE *w_)
{
    w = w_;
};

/**


Get soft costs 

each project - check that price stays the same during simulation run 
have installer - link by uid?


price is either per watt or cost based 


get hard costs for each project = module price * N_modules

average for period for installer? 
save all raw info?




write to them about memory corruption

save everything about costs of the project 


record seed 
time stamps

check return value


*/
void
UIBL::save(std::string path_to_save_file_)
{

	std::vector<std::string> save_file_names;


    if (path_to_save_file_ == "")
    {
        path_to_save_file_ = w->params["path_to_save"];
    };
    
    //get length of a simulation from w
    auto simulation_length = w->time + 1;
    
    
    auto save_data_raw = std::vector<std::deque<solar_core::PVProjectFlat*>>(simulation_length, {});
    
    
    
    //save data from last step
    w->save_end_data();
    
    
    
    
    
    //go through all installed projects and sort them by the date they were installed
    for (auto sei:*w->seis)
    {
        for (auto& project:sei->pvprojects)
        {
            save_data_raw[project->begin_time].push_back(project.get());
        };
    }
    
    
    //go through each bucket and count number of projects [0], count total number of projects [1], average price for the project per time unit
    auto N_SERIES = 5;
    auto save_data = new std::vector<std::vector<double>>(simulation_length, std::vector<double>(N_SERIES, 0.0));
    
    
    std::size_t N_INSTALLED_PROJECTS = 0;
    std::size_t N_INSTALLED_PROJECTS_T_1 = 0.0;
    double total_watt = 0.0;
    double average_price_per_watt = 0.0;
    double accum_price_per_watt = 0.0;
    double price_per_watt = 0.0;
    for (auto i = 0; i < save_data_raw.size(); ++i)
    {
        (*save_data)[i][0] = i;
        N_INSTALLED_PROJECTS = save_data_raw[i].size();
        (*save_data)[i][1] = N_INSTALLED_PROJECTS;
        (*save_data)[i][2] = N_INSTALLED_PROJECTS_T_1 + N_INSTALLED_PROJECTS;
        N_INSTALLED_PROJECTS_T_1 += N_INSTALLED_PROJECTS;
        
        accum_price_per_watt = 0.0;
        total_watt = 0.0;
        price_per_watt = 0.0;
        
        for (auto& project:save_data_raw[i])
        {
            price_per_watt += project->p/project->DC_size;
            accum_price_per_watt += project->p;
            total_watt += project->DC_size;
            
        };
        
        average_price_per_watt = accum_price_per_watt/total_watt;
        
        (*save_data)[i][3] = average_price_per_watt;
        (*save_data)[i][4] = price_per_watt/save_data_raw[i].size();
        
        
        //save efficiencies of the panels
		//save reliabilities of the panels
        (*save_data)[i].insert((*save_data)[i].end(), w->history_data[i].begin(), w->history_data[i].end());
        

		//save reliabilities of the panels
        





    };
    
    //convert to path, get parent path
    boost::filesystem::path path(path_to_save_file_);
    boost::filesystem::path path_to_dir = path.parent_path();
    boost::uuids::uuid file_name_short = boost::uuids::random_generator()();
    std::string file_name = boost::uuids::to_string(file_name_short) + "_w.csv";
    boost::filesystem::path path_tmp = path_to_dir;
    path_tmp /= file_name;
	save_file_names.push_back(file_name);
    std::ofstream out_file(path_tmp.string());


	if (!out_file)
	{
		out_file.close();
		//create location for saves in the directory with setup files? 
		//set path_to_save_file
		path = boost::filesystem::path(w->base_path);
		//path to the directory with model file, save here in case there are save errors 
		auto path_to_save_file = boost::filesystem::path(path.parent_path().make_preferred());

		path_to_save_file /= "Saves";
		path_to_save_file /= file_name;


		path_to_dir = path_to_save_file.parent_path();

		//for appending to old file use std::ofstream::app    
		//overwrite old file
		out_file.open(path_to_save_file.string(), std::ofstream::trunc);

		if (out_file)
		{
		}
		else
		{
			out_file.close();

			//create directory if needed
			boost::filesystem::path dir = path_to_save_file.parent_path();

			if (!(boost::filesystem::exists(dir)))
			{
				boost::filesystem::create_directory(dir);
			};

			out_file.open(path_to_save_file.string(), std::ofstream::trunc);

			if (!out_file)
			{
				throw std::runtime_error("Can not create save file");
			};
		};


	};



    if (out_file)
    {
        
        //MARK: cont. write column names
        for (auto i = 0; i < (*save_data).size(); ++i)
        {
            for (auto j = 0; j < (*save_data)[i].size(); ++j)
            {
                out_file << (*save_data)[i][j];
                if (j != (*save_data)[i].size() - 1)
                {
                    out_file << ",";
                };
            };
            out_file << std::endl;
        };
    }
    else
    {
        throw std::runtime_error("wrong path to save");
    };
    
    out_file.close();
    
    
    //save incomes, house sizes and electricity bills to double check
    file_name_short = boost::uuids::random_generator()();
    file_name = boost::uuids::to_string(file_name_short) + "_hos.csv";
    path_tmp = path_to_dir;
    path_tmp /= file_name;
    out_file.open(path_tmp.string());
	save_file_names.push_back(file_name);

    if (out_file)
    {
        for (auto h:*(w->hos))
        {
            
            out_file << ((h->params[solar_core::EParamTypes::Income] > 10000000)? 10000000 : h->params[solar_core::EParamTypes::Income]) << ",";
            out_file << h->params[solar_core::EParamTypes::ElectricityBill] << ",";
            out_file << h->house->house_size << ",";
            out_file << (h->time_installed) << ",";
            out_file << ((h->FLAG_INSTALLED_SYSTEM)? 1: 0);
            out_file << std::endl;
        };
    };
    
	out_file.close();

	file_name_short = boost::uuids::random_generator()();
	file_name = boost::uuids::to_string(file_name_short) + "_pv.csv";
	path_tmp = path_to_dir;
	path_tmp /= file_name;
	out_file.open(path_tmp.string());
	save_file_names.push_back(file_name);

	if (out_file)
	{


		for (auto i = 0; i < save_data_raw.size(); ++i)
		{
			for (auto& project : save_data_raw[i])
			{
				//hard costs if installed 
				//soft costs if installed
				//price if installed 
				//installer uid 
				out_file << project->p << ",";
				out_file << project->hard_costs << ",";
				out_file << project->soft_costs << ",";
				out_file << project->begin_time << ",";
				//as sei are created in a fixed order lowest uid will mean sei[0] and so on
				out_file << project->sei->uid.get_string();
				out_file << std::endl;
			};
		};
	};



	out_file.close();



	



	//save conf data about simulation results
	file_name_short = boost::uuids::random_generator()();
	file_name = boost::uuids::to_string(file_name_short) + "_conf.log";
	path_tmp = path_to_dir;
	path_tmp /= file_name;
	out_file.open(path_tmp.string());
	save_file_names.push_back(file_name);

	if (out_file)
	{
		for (auto name:save_file_names) 
		{
			out_file << name << std::endl;
		};
		out_file << "SEED " << w->params["SEED"] << " ";
		out_file << "Parameters Code: " << w->params["ParametersCode"];
	};

	out_file.close();
    
}