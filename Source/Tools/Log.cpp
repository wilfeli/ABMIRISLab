//
//  Log.cpp
//  ABMSolar
//
//  Created by Ekaterina Sinitskaya on 5/7/16.
//  Copyright (c) 2016 IRIS Lab. All rights reserved.
//

#include "Tools/ExternalIncludes.h"
#include "Tools/Log.h"


using namespace solar_core;



#ifdef ABMS_DEBUG_MODE
#if defined(_WIN64) || defined(_WIN32)
#define NOMINMAX
#include <windows.h>
#endif
#endif




/**

Static variable is initialized only once, compiler is taking care of it.

*/

Log&
Log::instance() {

	return Log::instance("");

};


Log&
Log::instance(std::string path_) {

	static Log* instance = new Log(path_);
	return *instance;

};


Log::Log(std::string path_) {
	boost::filesystem::path path_to_log_file_;

	if (path_ != "")
	{
		//set path_to_log_file
		boost::filesystem::path path(path_);
		path_to_log_file_ = boost::filesystem::path(path.parent_path().make_preferred());


		boost::uuids::uuid file_name_short = boost::uuids::random_generator()();
		std::string file_name = boost::uuids::to_string(file_name_short) + "_log.log";

		//TODO change to save location
		//Added versioning
		path_to_log_file_ /= "Logs";
		path_to_log_file_ /= file_name;
	}
	else
	{
		throw std::runtime_error("Expected path to log file");
	};

	//for appending to old log use std::ofstream::app    
	//overwrite old log
	log_file.open(path_to_log_file_.string(), std::ofstream::trunc);

	if (log_file)
	{
	}
	else
	{
		log_file.close();

		//create directory if needed
		boost::filesystem::path dir = path_to_log_file_.parent_path();

		if (!(boost::filesystem::exists(dir)))
		{
			boost::filesystem::create_directory(dir);
		};

		log_file.open(path_to_log_file_.string(), std::ofstream::trunc);

		if (!log_file)
		{
			throw std::runtime_error("Can not create log file");
		};

	};

	path_to_log_file = path_to_log_file_.make_preferred().string();

	char mbstr[100];

#if defined(__APPLE__)
	time_t rawtime;
	tm* timeinfo;

	time(&rawtime);
	timeinfo = localtime(&rawtime);

	std::strftime(mbstr, sizeof(mbstr), "%a %F %T ", timeinfo);
#endif   

#if defined(_WIN64) || defined(_WIN32)


	time_t rawtime;
	::time(&rawtime);
	tm timeinfo;
	localtime_s(&timeinfo, &rawtime);

	std::strftime(mbstr, sizeof(mbstr), "%a %F %T ", &timeinfo);
#endif
	log_file << "time: " << mbstr << " " << "INFO: " << "created log" << std::endl;




}

Log::~Log()
{
	if (log_file)
	{
		log_file.close();
	};
}
;



void
Log::log(std::string mes_, std::string mes_type_) {

	//    std::ofstream log_file(path_to_log_file, std::ofstream::app);

	char mbstr[100];

#if defined(__APPLE__)
	time_t rawtime;
	tm* timeinfo;

	time(&rawtime);
	timeinfo = localtime(&rawtime);
	std::strftime(mbstr, sizeof(mbstr), "%a %F %T ", timeinfo);

#endif   

#if defined(_WIN64) || defined(_WIN32)
	time_t rawtime;
	::time(&rawtime);
	tm timeinfo;
	auto err = localtime_s(&timeinfo, &rawtime);

#ifdef ABMS_DEBUG_MODE
#if defined(_WIN64) || defined(_WIN32)
	if (err)
	{
		OutputDebugString("Invalid argument to localtime_s.");
	};
#endif
#endif

	if (!err)
	{
		std::strftime(mbstr, sizeof(mbstr), "%a %F %T ", &timeinfo);
	};
#endif


	log_file << "time: " << mbstr << " " << mes_type_ << mes_ << std::endl;





}
