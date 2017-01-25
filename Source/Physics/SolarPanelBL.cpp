#include "Agents/SolarPanel.h"
#include "Agents/SEMBL.h"
#include "Physics/SolarPanelBL.h"


using namespace solar_core;

SolarModuleBL::SolarModuleBL(const PropertyTree& pt_) :SolarModule(pt_)
{
	warranty_length = pt_.get<double>("Warranty length");


}

void SolarModuleBL::init()
{
	//
	THETA_reliability = std::vector<double>(1, 0.0);
	THETA_complexity = std::vector<double>(2, 0.0);

	//get parameters from manufacturer
	auto sem = dynamic_cast<SEMBL*>(manufacturer);

	//update efficiency to the efficiency of the panel from the .json specification
	sem->THETA_dist_efficiency[2] = efficiency;
	//update parameters for the distribution
	THETA_reliability[0] = sem->THETA_dist_reliability[2];
	THETA_complexity[0] = sem->THETA_dist_complexity[SEMBL::N_complexity_params * 3];
	THETA_complexity[1] = sem->THETA_dist_complexity[SEMBL::N_complexity_params * 3 + 1];

}