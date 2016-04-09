//
//  WorldSettings.h
//  ABMSolar
//
//  Created by Ekaterina Sinitskaya on 3/7/16.
//  Copyright (c) 2016 IRIS Lab. All rights reserved.
//

#ifndef __ABMSolar__WorldSettings__
#define __ABMSolar__WorldSettings__


#include "Tools/ExternalIncludes.h"

#include "Tools/IParameters.h"


namespace solar_core
{
    
class SolarModule;
    
/**
 
 
 Settings for the simulations
 
 */
class WorldSettings
{
public:
    
    static WorldSettings& instance();
    
    //@{
    /**
     
     Parameteres, setting up, etc. section
     
     
     
     
     */
    
    struct EHash
    {
        template <typename T>
        std::size_t operator()(T t) const
        {
            return static_cast<std::size_t>(t);
        }
    };
    
    std::unordered_set<EParamTypes, EHash> params_to_copy_preliminary_quote{EParamTypes::CreditScore, EParamTypes::RoofSize, EParamTypes::ElectricityBill}; /*!< @wp small installers priorities based on the source of information. During online quote stage they do not ask about credit score it is moved to the preliminary quote. */
    
    std::map<EConstraintParams, double> constraints; /*!< parameters for decision making and etc in a form of imposed exogenous constraints, such as number of ticks to collect quotes, for now have general setting for all agents, later might do individual settings. */
    
    std::map<EParamTypes, double> params_exog; /*!< exogenous parameters, such as price per watt that utility compary is asking, or price per watt that is paid to the connected to the grid solar project */
    
    std::map<std::string, std::shared_ptr<SolarModule>> solar_modules; /*!< list of available solar modules */
    
    
    
    //@}
    
private:
    WorldSettings();
    
    
    
};
    
} //end of namespace solar_core





#endif /* defined(__ABMSolar__WorldSettings__) */
