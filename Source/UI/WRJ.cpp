//
//  WRJ.cpp
//  ABMSolar
//
//  Created by Ekaterina Sinitskaya on 6/16/17.
//  Copyright (c) 2017 IRIS Lab. All rights reserved.
//

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wc++11-compat"




#include "Tools/Serialize.h"
#include "Tools/SerializeRJ.h"
#include "UI/W.h"
#include "Institutions/IMessage.h"
#include "Tools/WorldSettings.h"
#include "Tools/ParsingTools.h"
#include "Tools/Simulation.h"
#include "Tools/Log.h"
#include "Tools/RJTools.h"
#include "Geography/Geography.h"
#include "Institutions/MarketingSystem.h"
#include "Agents/IAgent.h"
#include "Agents/SEI.h"
#include "Agents/SEM.h"
#include "Agents/Utility.h"
#include "Agents/G.h"
#include "Agents/Homeowner.h"
#include "UI/HelperW.h"
#include "UI/WRJ.h"


using namespace solar_core;


WRJ::WRJ(std::string path_, HelperW * helper_, std::string mode_)
{


	//set ui flags
	FLAG_IS_STOPPED = true;


	//saves path to basic template
	base_path = path_;
	boost::filesystem::path path_to_model_file(path_);
	boost::filesystem::path path_to_dir = path_to_model_file.parent_path();
	boost::filesystem::path path_to_template;
	std::string path;

#ifdef ABMS_DEBUG_MODE
	Log::instance(path_);
#endif

	//preallocate stuff
	DocumentRJ pt;
	std::map<std::string, std::string> params_str;

	auto get_double = &serialize::GetNodeValue<double>::get_value;
	auto get_long = &serialize::GetNodeValue<long>::get_value;
	auto get_string = [](const DocumentNode& node_) -> std::string {
		if (node_.IsString())
		{
			return std::string(node_.GetString());
		}
		else
		{
			if (node_.IsNumber())
			{
				return std::to_string(node_.GetDouble());
			}
			else
			{
				//return empty string
				return std::string();
			};
		};
	};


	if (mode_ == "NEW")
	{

		create_world(path_to_model_file, path_to_dir, path_to_template, pt, params_str);



		//ho.json
		path_to_template = path_to_dir;
		path_to_template /= "ho.json";
		path = path_to_template.string();
		tools::read_json_rj(path, pt);




		//left as an old version as need to check that formulas are consistent here with the geography definition
		//create random number generators for locations
		//is created here to speed up generation, otherwise rng is created for each agent, so location formula is not used directly.
		//check that it is uniform distribution
		if (get_string(pt["location"]).find("FORMULA::p.d.f.::u_int(0, size)") == std::string::npos)
		{
			throw std::runtime_error("unsupported ho specification rule");
		};
		auto max_ = world_map->g_map[0].size() - 1;
		auto pdf_location_x = boost::uniform_int<uint64_t>(0, max_);
		auto rng_location_x = boost::variate_generator<boost::mt19937&, boost::uniform_int<uint64_t>>(rand->rng, pdf_location_x);
		max_ = world_map->g_map.size() - 1;
		auto pdf_location_y = boost::uniform_int<uint64_t>(0, max_);
		auto rng_location_y = boost::variate_generator<boost::mt19937&, boost::uniform_int<uint64_t>>(rand->rng, pdf_location_y);


		hos = dynamic_cast<HelperWSpecialization<W, BaselineModelRJ>*>(helper_)->create_hos(pt, mode_, path_to_dir, rng_location_x, rng_location_y, this);




		//sem.json
		///@DevStage2 each sem will pick initial templates by name? - could make it base creation mode
		path_to_template = path_to_dir;
		path_to_template /= "sem.json";
		path = path_to_template.string();
		tools::read_json_rj(path, pt);

		sems = dynamic_cast<HelperWSpecialization<W, BaselineModelRJ>*>(helper_)->create_sems(pt, mode_, path_to_dir, rng_location_x, rng_location_y, this);




		//sei.json
		path_to_template = path_to_dir;
		path_to_template /= "sei.json";
		path = path_to_template.string();
		tools::read_json_rj(path, pt);

		seis = dynamic_cast<HelperWSpecialization<W, BaselineModelRJ>*>(helper_)->create_seis(pt, mode_, path_to_dir, rng_location_x, rng_location_y, this);





		//create G
		//g.json
		path_to_template = path_to_dir;
		path_to_template /= "g.json";
		path = path_to_template.string();
		tools::read_json_rj(path, pt);
		g = new G(pt, this);


		//create Utility
		//utility.json
		path_to_template = path_to_dir;
		path_to_template /= "utility.json";
		path = path_to_template.string();
		tools::read_json_rj(path, pt);
		utility = new Utility(pt, this);


		//create marketing
		//market.json
		path_to_template = path_to_dir;
		path_to_template /= "market.json";
		path = path_to_template.string();
		tools::read_json_rj(path, pt);
		marketing = new MarketingInst(pt, this);



		//set flags
		FLAG_H_TICK = true;
		FLAG_G_TICK = true;
		FLAG_UTILITY_TICK = true;
		FLAG_SEI_TICK = true;
		FLAG_SEM_TICK = true;
		FLAG_MARKET_TICK = true;
		updated_counter = 0;
		notified_counter = 0;

	};



}




void WRJ::create_world(boost::filesystem::path& path_to_model_file,
	boost::filesystem::path& path_to_dir,
	boost::filesystem::path& path_to_template,
	DocumentRJ& pt, 
	std::map<std::string,std::string>& params_str)
{

	auto get_double = &serialize::GetNodeValue<double>::get_value;
	auto get_long = &serialize::GetNodeValue<long>::get_value;
	auto get_string = [](const DocumentNode& node_) -> std::string {
		if (node_.IsString()) 
		{
			return std::string(node_.GetString());
		}
		else 
		{
			if (node_.IsNumber()) 
			{
				return std::to_string(node_.GetDouble());
			}
			else 
			{
				//return empty string
				return std::string();
			};
		};
		};

	std::map<std::string, std::string> parsed_model;

	tools::parse_model_file(path_to_model_file.string(), parsed_model);

	std::string w_file_name = "";
	if (parsed_model.count("path_to_save") > 0)
	{
		//here convert into usable format
		//set path_to_log_file
		boost::filesystem::path path_to_save(parsed_model["path_to_save"]);
		params["path_to_save"] = path_to_save.make_preferred().string();
	}
	else
	{
		throw std::runtime_error("Wrong configuration file");
	};


	path_to_template = path_to_dir;
	path_to_template /= "w.json";

	std::string path = path_to_template.string();

	//baseline model
	tools::read_json_rj(path, pt);

	params_d[EParamTypes::N_SEI] = get_double(pt["N_SEI"]);
	params_d[EParamTypes::N_SEILarge] = get_double(pt["N_SEILarge"]);
	params_d[EParamTypes::N_SEM] = get_double(pt["N_SEM"]);
	params_d[EParamTypes::N_SEMPVProducer] = get_double(pt["N_SEMPVProducer"]);
	params_d[EParamTypes::N_SEMInverterProducer] = get_double(pt["N_SEMInverterProducer"]);
	params_d[EParamTypes::N_HO] = get_double(pt["N_HO"]);
	params_d[EParamTypes::N_HOMarketingStateHighlyInterested] = get_double(pt["N_HOMarketingStateHighlyInterested"]);

	//create RNG
	rand_sei = new IRandom(get_double(pt["SEED"]));
	rand = new IRandom(get_double(pt["SEED"]));
	rand_sem = new IRandom(get_double(pt["SEED"]));
	rand_ho = new IRandom(get_double(pt["SEED"]));
	rand_g = new IRandom(get_double(pt["SEED"]));
	rand_market = new IRandom(get_double(pt["SEED"]));
	rand_utility = new IRandom(get_double(pt["SEED"]));

	//save seed
	params["SEED"] = get_string(pt["SEED"]);



	//save type of a simulation run
	params["ParametersCode"] = get_string(pt["ParametersCode"]);

	//create parameters
	serialize::deserialize(pt["WorldSettings"]["params_exog"], params_str);

	for (auto& iter : params_str)
	{
		WorldSettings::instance().params_exog[EnumFactory::ToEParamTypes(iter.first)] = serialize::solve_str_formula<double>(iter.second, *rand);
	};



	params_str.clear();
	serialize::deserialize(pt["WorldSettings"]["constraints"], params_str);
	for (auto& iter : params_str)
	{
		WorldSettings::instance().constraints[EnumFactory::ToEConstraintParams(iter.first)] = serialize::solve_str_formula<double>(iter.second, *rand);
	};

	serialize::deserialize(pt["params"], params);



	//add parameters for roof updating
	serialize::deserialize(pt["WorldSettings"]["THETA_roof_update"], WorldSettings::instance().THETA_roof_update);





	//set internals
	time = 0;


	//solar_module.json
	path_to_template = path_to_dir;
	path_to_template /= "solar_equipment.json";
	path = path_to_template.string();
	tools::read_json_rj(path, pt);

	//create existing solar modules
	serialize::deserialize(pt["solar_modules"], WorldSettings::instance().solar_modules);


	//create existing inverters
	serialize::deserialize(pt["inverters"], WorldSettings::instance().inverters);


	//create grid
	path_to_template = path_to_dir;
	path_to_template /= "geography.json";
	path = path_to_template.string();
	tools::read_json_rj(path, pt);
	world_map = new WorldMap(pt, this);


	//update permitting difficulty
	//average permit difficulty is connected to processing time required by g
	//




	//allocate interconnected projects 
	interconnected_projects = new std::set<std::shared_ptr<PVProject>>();

	//allocate active agents
	active_hos = new std::vector<Homeowner*>(params_d[EParamTypes::N_HO] / 2, nullptr);



}