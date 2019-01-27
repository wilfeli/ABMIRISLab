#pragma once
#include "Tools/ExternalIncludes.h"
#include "Tools/IParameters.h"
#include "Tools/SerializeRJ.h"


namespace solar_core
{
	namespace tools
	{
		void read_json_rj(std::string path_, DocumentRJ& pt_);

		extern decltype (&serialize::GetNodeValue<double>::get_value) get_double; 
		extern decltype (&serialize::GetNodeValue<long>::get_value) get_long;

		//auto get_double = &serialize::GetNodeValue<double>::get_value;
		//auto get_long = &serialize::GetNodeValue<long>::get_value;

		extern std::function<std::string (const DocumentNode&)> get_string;


		//auto get_string = [](const DocumentNode& node_) -> std::string {
		//	if (node_.IsString())
		//	{
		//		return std::string(node_.GetString());
		//	}
		//	else
		//	{
		//		if (node_.IsNumber())
		//		{
		//			return std::to_string(node_.GetDouble());
		//		}
		//		else
		//		{
		//			//return empty string
		//			return std::string();
		//		};
		//	};
		//};






	}
}