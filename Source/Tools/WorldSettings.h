//
//  WorldSettings.h
//  ABMSolar
//
//  Created by Ekaterina Sinitskaya on 3/7/16.
//  Copyright (c) 2016 IRIS Lab. All rights reserved.
//

#ifndef __ABMSolar__WorldSettings__
#define __ABMSolar__WorldSettings__


#include <unordered_set>
#include "Tools/IParameters.h"


namespace solar_core
{
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
    
    std::unordered_set<EParamTypes> params_to_copy_preliminary_quote{EParamTypes::CreditScore, EParamTypes::RoofSize, EParamTypes::ElectricityBill};
    
    //@}
    
private:
    WorldSettings();
    
    
    
};
    
} //end of namespace solar_core





#endif /* defined(__ABMSolar__WorldSettings__) */
