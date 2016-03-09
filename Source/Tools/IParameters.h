//
//  IParameters.h
//  ABMSolar
//
//  Created by Ekaterina Sinitskaya on 2/24/16.
//  Copyright (c) 2016 IRIS Lab. All rights reserved.
//

#ifndef ABMSolar_IParameters_h
#define ABMSolar_IParameters_h

#include "Tools/ExternalIncludes.h"


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
        
        
        /** Electricity Bill */
        ElectricityBill,
        
        
        /** Roof size */
        RoofSize,
        
        
        /** Estimated price of installation */
        PreliminaryQuote,
        
        
        /** Industry standard price per watt that is used in estimating installation cost */
        EstimatedPricePerWatt,
        
        
        /** Industry standard PV */
        AveragePVPrice,
        
        
        /** Small SEI agent - such as mom and pop shop */
        SEISmall,
        
        
        /** Large SEI */
        SEILarge,
        
        
        
        /** States of a Project */
        RequestedOnlineQuote,
        
        
        RequestedPreliminaryQuote,
        
        
        ProvidedOnlineQuote,
        
        
        ProvidedPreliminaryQuote,
        
        
        /**  MARK: cont. with other project states */
        
        /** Empty enum for completeness */
        None
        
    };
    
    
    
    
    typedef int64_t TimeUnit;
    

} //end of namespace solar_core
#endif
