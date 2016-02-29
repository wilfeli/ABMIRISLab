//
//  IParameters.h
//  ABMSolar
//
//  Created by Ekaterina Sinitskaya on 2/24/16.
//  Copyright (c) 2016 IRIS Lab. All rights reserved.
//

#ifndef ABMSolar_IParameters_h
#define ABMSolar_IParameters_h




namespace solar_core {
    
    enum class EParamTypes: int64_t
    {
        /** Average yearly income for hh */
        Income,

        /**  Number of humans in a household */
        N_H,
        
        /** Credit score */
        CreditScore,
        
        
        /** Electric bill */
        ElectricBill,
        
        
        /** Roof size */
        RoofSize,
        
        
        
        
        
        /** Empty enum for completeness */
        None
        
    };

}
#endif
