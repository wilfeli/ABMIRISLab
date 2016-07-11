//
//  SEIBL.h
//  ABMSolar
//
//  Created by Ekaterina Sinitskaya on 7/9/16.
//  Copyright (c) 2016 IRIS Lab. All rights reserved.
//

#ifndef __ABMSolar__SEIBL__
#define __ABMSolar__SEIBL__

#include "Agents/SEI.h"
#include "Tools/ID.h"


namespace solar_core {
    class H;
}




namespace solar_core
{
    
    class TDesign
    {
    public:
        std::shared_ptr<SolarModule> module; /** is used to pull ID, efficiency, degradation */
        std::vector<double> THETA_dists; /** learned parameters for distributions */
    };
    
    
    
    class SEIBL: public SEI
    {
    public:
        
    protected:
        
        std::map<UID, std::shared_ptr<TDesign>>  designs;  /** have current estimates on system design parameters for different learning distributions */
        
        std::shared_ptr<TDesign> dec_design;
    };
    
    
    
    
} //end namespace solar_core



#endif /* defined(__ABMSolar__SEIBL__) */
