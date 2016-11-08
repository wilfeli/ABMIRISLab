//
//  Utility.h
//  ABMSolar
//
//  Created by Ekaterina Sinitskaya on 5/10/16.
//  Copyright (c) 2016 IRIS Lab. All rights reserved.
//

#ifndef __ABMSolar__Utility__
#define __ABMSolar__Utility__

#include <mutex>
#include "Tools/ExternalIncludes.h"
#include "Tools/IParameters.h"

namespace solar_core
{

    
    class PVProject;
    class W;

    class Utility
    {
        friend class W;
    public:
        Utility(const PropertyTree& pt_, W* w_);
        void init(W* w_);
        void act_tick();
        
        void request_permit_for_interconnection(std::shared_ptr<PVProject> project_);
        
    protected:
        
        void ac_update_tick();
        W* w;
        TimeUnit a_time;
        std::map<EParamTypes, double> params;
        
        
        std::vector<std::shared_ptr<PVProject>> pending_pvprojects; /*!< list PV projects to consider */
        std::vector<std::shared_ptr<PVProject>> pending_pvprojects_to_add; /*!< list PV projects to consider */
        std::mutex pending_pvprojects_lock;
      
        std::vector<double> THETA_dec; /*!< parameters for decisions, here THETA[0] - parameter for exponential distribution */
        
        
    };

} // end namespace solar_core




#endif /* defined(__ABMSolar__Utility__) */
