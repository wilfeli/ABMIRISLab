//
//  IParameters.h
//  ABMSolar
//
//  Created by Ekaterina Sinitskaya on 2/24/16.
//  Copyright (c) 2016 IRIS Lab. All rights reserved.
//

#ifndef ABMSolar_IParameters_h
#define ABMSolar_IParameters_h

#include <cstdint>


namespace solar_core
{
    /**
     
     @DevStage1 add factories enum<->std::string
     
     */
    enum class EParamTypes: int64_t
    {
        /** Average yearly income for hh */
        Income,
        

        /**  Number of humans in a household */
        N_H,
        
        
        /** Credit score */
        CreditScore,
        
        
        /** Electric bill */
        ElectricalBill,
        
        
        /** Roof size */
        RoofSize,
        
        
        /** Estimated price of installation */
        PreliminaryQuote,
        
        
        /** Industry standard price per watt that is used in estimating installation cost */
        EstimatedPricePerWatt,
        
        
        /** Industry standard PV */
        AveragePVPrice,
        
        
        /** Empty enum for completeness */
        None
        
    };

} //end of namespace solar_core
#endif
