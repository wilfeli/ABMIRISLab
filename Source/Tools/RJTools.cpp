#include "Tools/RJTools.h"


using namespace rapidjson;

void solar_core::tools::read_json_rj(std::string path_, DocumentRJ & pt_)
{
	std::ifstream ifs(path_);
	IStreamWrapper isw(ifs);

	pt_.ParseStream(isw);

}



decltype (&serialize::GetNodeValue<double>::get_value) solar_core::tools::get_double = &serialize::GetNodeValue<double>::get_value;
decltype (&serialize::GetNodeValue<long>::get_value) solar_core::tools::get_long = &serialize::GetNodeValue<long>::get_value;



std::function<std::string (const solar_core::DocumentNode&)> solar_core::tools::get_string = [](const solar_core::DocumentNode& node_) -> std::string {
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