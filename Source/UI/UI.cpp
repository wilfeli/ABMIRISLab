//
//  UI.cpp
//  ABMSolar
//
//  Created by Ekaterina Sinitskaya on 5/7/16.
//  Copyright (c) 2016 IRIS Lab. All rights reserved.
//

#include "UI/UI.h"
#include "UI/W.h"
#include "Agents/Homeowner.h"
#include "Agents/SEI.h"
#include <boost/filesystem.hpp>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>


#include "Agents/SolarPanel.h"
#include "Institutions/IMessage.h"

using namespace solar_ui;


void
UI::init(solar_core::W *w_)
{
    w = w_;
};



void
UI::save(std::string path_to_save_file_)
{
    if (path_to_save_file_ == "")
    {
        path_to_save_file_ = w->params["path_to_save"];
    };
    
    //get length of a simulation from w
    auto simulation_length = w->time + 1;
    
    
    auto save_data_raw = new std::vector<std::deque<std::shared_ptr<solar_core::PVProject>>>(simulation_length, {});
    
    //go through all installed projects and sort them by the date they were installed
    for (auto& project:*(w->interconnected_projects))
    {
        (*save_data_raw)[project->ac_utility_time].push_back(project);
    };
    
    
    //go through each bucket and count number of projects [0], count total number of projects [1], average price for the project per time unit
    auto N_SERIES = 5;
    auto save_data = new std::vector<std::vector<double>>(simulation_length, std::vector<double>(N_SERIES, 0.0));
    
    
    std::size_t N_INSTALLED_PROJECTS = 0;
    std::size_t N_INSTALLED_PROJECTS_T_1 = 0.0;
    double total_watt = 0.0;
    double average_price_per_watt = 0.0;
    double accum_price_per_watt = 0.0;
    double price_per_watt = 0.0;
    for (long i = 0; i < simulation_length - 1; ++i)
    {
		(*save_data)[i][0] = i;
        N_INSTALLED_PROJECTS = (*save_data_raw)[i].size();
        (*save_data)[i][1] = N_INSTALLED_PROJECTS;
		(*save_data)[i][2] = N_INSTALLED_PROJECTS_T_1 + N_INSTALLED_PROJECTS;
        N_INSTALLED_PROJECTS_T_1 += N_INSTALLED_PROJECTS;
        
        accum_price_per_watt = 0.0;
        total_watt = 0.0;
        price_per_watt = 0.0;
        
        for (auto& project: (*save_data_raw)[i])
        {
            price_per_watt += project->design->design->total_costs/project->design->design->DC_size;
            accum_price_per_watt += project->design->design->total_costs;
            total_watt += project->design->design->DC_size;
            
        };
        
        average_price_per_watt = accum_price_per_watt/total_watt;
        
		(*save_data)[i][3] = average_price_per_watt;
		if ((*save_data_raw)[i].size() != 0.0)
		{
			(*save_data)[i][4] = price_per_watt / (*save_data_raw)[i].size();
		}
		else 
		{
			(*save_data)[i][4] = std::numeric_limits<double>::quiet_NaN();
		};
		

		(*save_data)[i].insert((*save_data)[i].end(), w->history_sei[i].begin(), w->history_sei[i].end());
        
    };
    
    //convert to path, get parent path
    boost::filesystem::path path(path_to_save_file_);
    boost::filesystem::path path_to_dir = path.parent_path();
    boost::uuids::uuid file_name_short = boost::uuids::random_generator()();
    std::string file_name = boost::uuids::to_string(file_name_short) + "_w.csv";
    boost::filesystem::path path_tmp = path_to_dir;
    path_tmp /= file_name;
	auto path_to_save_file = path_tmp;
    
    std::ofstream out_file(path_tmp.string());
    
    if (!out_file)
    {
		out_file.close();
		//create location for saves in the directory with setup files? 
		//set path_to_save_file
		path = boost::filesystem::path(w->base_path);
		//path to the directory with model file, save here in case there are save errors 
		path_to_save_file = boost::filesystem::path(path.parent_path().make_preferred());

		path_to_save_file /= "Saves";
		path_to_save_file /= file_name;


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
		for (auto i = 0; i < (*save_data).size(); ++i)
		{
			for (auto j = 0; j <(*save_data)[i].size(); ++j)
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


	out_file.close();


	//save file with information about simulation run
	//seed, type of a run 
	path_tmp = path_to_save_file.parent_path();
	file_name_short = boost::uuids::random_generator()();
	file_name = boost::uuids::to_string(file_name_short) + "_params.log";
	path_tmp /= file_name;

	out_file.open(path_tmp.string());
	if (out_file)
	{
		out_file << w->params["SEED"] << std::endl;
		out_file << w->params["ParametersCode"] << std::endl;
	};
	out_file.close();


	//go through all projects and save location of installation and price 
	//create new file

	path_tmp = path_to_save_file.parent_path();
	file_name_short = boost::uuids::random_generator()();
	file_name = boost::uuids::to_string(file_name_short) + "_pr.csv";
	path_tmp /= file_name;

	out_file.open(path_tmp.string());



	if (out_file)
	{
		auto price_per_watt = 0.0;
		for (auto& project : *(w->interconnected_projects))
		{
			//time, location_x, location_y, installer id, price_per_watt, size, type
			out_file << project->ac_utility_time << ",";
			out_file << project->agent->location_x << ",";
			out_file << project->agent->location_y << ",";
			out_file << project->sei->uid.get_string() << ",";
			price_per_watt = project->design->design->total_costs / project->design->design->DC_size;
			out_file << price_per_watt << ",";
			out_file << project->design->design->DC_size << ",";
			out_file << project->design->design->PV_module->origin << ",";
			out_file << project->design->design->PV_module->efficiency << ",";
			out_file << project->sei->params[solar_core::EParamTypes::SEIEquipmentType] << ",";
			out_file << project->sei->params[solar_core::EParamTypes::SEIInteractionType] << ",";
			out_file << project->is_direct_marketing << std::endl;

		};

	};

	out_file.close();

	path_tmp = path_to_save_file.parent_path();
	file_name_short = boost::uuids::random_generator()();
	file_name = boost::uuids::to_string(file_name_short) + "_loc_h.csv";
	path_tmp /= file_name;

	out_file.open(path_tmp.string());

	auto save_data_loc = new std::vector<std::vector<double>>(w->world_map->g_map.size(), std::vector<double>(w->world_map->g_map.back().size(), 0.0));

	//save h location
	if (out_file)
	{
		for (auto& row : w->world_map->h_map) 
		{
			for (auto& tile:row) 
			{
				out_file << tile.size() << ",";
			};
			out_file << std::endl;
		};
	}

	out_file.close();



	path_tmp = path_to_save_file.parent_path();
	file_name_short = boost::uuids::random_generator()();
	file_name = boost::uuids::to_string(file_name_short) + "_loc_sei.csv";
	path_tmp /= file_name;

	out_file.open(path_tmp.string());

	//save sei location
	for (auto& sei:(*w->seis)) 
	{
		(*save_data_loc)[(long)sei->location_x][(long)sei->location_y] += 1.0;
	};


	if (out_file)
	{
		for (auto i = 0; i < (*save_data_loc).size(); ++i)
		{
			for (auto j = 0; j <(*save_data_loc)[i].size(); ++j)
			{
				out_file << (*save_data_loc)[i][j];
				if (j != (*save_data_loc)[i].size() - 1)
				{
					out_file << ",";
				};
			};
			out_file << std::endl;
		};
	}

	delete save_data;
	delete save_data_loc;
	delete save_data_raw;
    
}