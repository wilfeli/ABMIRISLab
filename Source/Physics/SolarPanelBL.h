#pragma once


#include "Tools/ExternalIncludes.h"
#include "Tools/ID.h"
#include "Tools/IParameters.h"
#include "Agents/SolarPanel.h"

namespace solar_core
{
	class SolarModuleBL;

	class PVProjectFlat
	{
	public:
		std::shared_ptr<SolarModuleBL> PV_module;
		double N_PANELS = 0.0;
		double DC_size = 0.0;
		double AC_size = 0.0;
		TimeUnit begin_time;
		SEIBL* sei; /*!< installer of a project */
		H* agent;/*!< for whom this project is created */
		double production_time = 0.0; /*!< realized production for t period */
		double production_time_1 = 0.0; /*!< realized production for t-1 period */
		double irr_e = 0.0; /*!< expected irr of a project */
		double irr_a = 0.0; /*!< advertized by installer irr */
		TimeUnit maintenance_time;
		TimeUnit maintenance_time_1;
		double maintenance_complexity;
		double p; /*!< total price for this project */

	};

	class SolarModuleBL : public SolarModule
	{
	public:
		SolarModuleBL(const PropertyTree& pt_);
		/*!< uses manufacturer priors to specialize itself */
		void init();
		/** parameters of a true reliability distribution (exponential) */
		std::vector<double> THETA_reliability;
		/** parameters of a true reliability distribution (normal) */
		std::vector<double> THETA_complexity;
	protected:
	};


} //end namespace solar_core