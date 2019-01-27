#pragma once
#include "Tools/ExternalIncludes.h"
#include "W.h"


namespace solar_core {
	class WRJ : public W
	{
		template <class T1, class T2> friend class HelperWSpecialization;
	public:
		WRJ(std::string path_, HelperW* helper_, std::string mode_ = "NEW"); /*!< */

		void create_world(boost::filesystem::path & path_to_model_file, boost::filesystem::path & path_to_dir, boost::filesystem::path & path_to_template, DocumentRJ & pt, std::map<std::string, std::string>& params_str);

	};
}