//
//  H.h
//  ABMSolar
//
//  Created by Ekaterina Sinitskaya on 7/9/16.
//  Copyright (c) 2016 IRIS Lab. All rights reserved.
//

#ifndef __ABMSolar__H__
#define __ABMSolar__H__

#include "Agents/Homeowner.h"


namespace solar_core
{
    
    class WEE;
    class PVProjectFlat;

    
    
    /**
     
     Simple H that could only decide on ROI
     
     */
    class H
    {
        template <class T1, class T2> friend class HelperWSpecialization;
    public:
        //@{
        
        /**
         
        Initialization section
        
        */
        
        H(const PropertyTree& pt_, WEE* w_);
        void init(WEE* w_);
        
         
        //@}
        
        
        
        //@{
        /**
         
         Section with geographical parameters
         
         */
        
        double location_x; /** Location of an agent, x coordinate. @DevStage2 think about decreasing size for this field, use uint64_t or smaller for it */
        double location_y; /** Location of an agent, y coordinate. @DevStage2 think about decreasing size for this field, use uint64_t or smaller for it */
        
        //@}

        
        /**
         
         
         Section with parameters for solar projects, mainly House for now
         
         
         
         
         */
        House* house; /** Simplification: assume only 1 house per h*/
        bool FLAG_INSTALLED_SYSTEM = false;
        
        //@}
        
        
        //@{
        /**
         
         Decision section
         */
        
        std::vector<double> THETA_decision;/*!< irr threshold for decision to switch - \f$\mu\f$ in Logistic distribution */
        bool ac_dec_design(std::shared_ptr<PVProjectFlat> project_, WEE* w_); /*!< decides to accept project or not */
        std::vector<double> THETA_params; /*!< parameters for defining decision parameters */
        
        
        //@}
        
        
        
        //@{
        /**
         
         Section with general parameters that describe hh
         
         */
        
        
        
        std::map<EParamTypes, double> params; /** Parameters of a Homeowner, such as income, number of humans, etc. */
        
        
        //@}
        
        
        
    };
    

    
    
} //end namespace solar_core




#endif /* defined(__ABMSolar__H__) */
