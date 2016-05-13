//
//  G.h
//  ABMSolar
//
//  Created by Ekaterina Sinitskaya on 3/20/16.
//  Copyright (c) 2016 IRIS Lab. All rights reserved.
//

#ifndef __ABMSolar__G__
#define __ABMSolar__G__

#include <memory>
#include <deque>

#include "Tools/IParameters.h"


namespace solar_core
{
    
    class PVProject;
    
    /**
     
     @DevStage1 Two separate permit process, one for general permit one for interconnection
     
     @DevStage1 having the probability of passing the inspection 
     
     
     1. permit to install 2. install 3. request for inspection 4. inspection - pass or fail 5. apply for interconnection if pass 6. pass or not interconnection - variable time depending on  7.
     
     
     @DevStage1 make Utility - param - already connected amount of energy, param - potential amount of energy that could be connected (also add inflation to that), chances of connecting will depend on the difference between the two. 
     
     Degradation of PV - 0.5% per year
     
     Check that payment is before or after installation
     
     
     */
    class G
    {
    public:
        //@{
        /**
         
         Section with actions in the world
         
         */
        
        G(const PropertyTree& pt_, W* w_);
        void init(W* w_);
        virtual void act_tick() ;
        //@}
        
        //@{
        /**
         
         Section relevant for permitting process
         
         */
        virtual void request_permit_for_installation(std::shared_ptr<PVProject> project_); /*!< request permit for the project */
        void request_inspection(std::shared_ptr<PVProject> project_); /*!< request inspection after installation */
        
        //@}
        
        
        
        
        
        
        
        
    protected:
        
        
        //@{
        /**
         
         Section relevant for permitting process
         
         */
        
        std::deque<std::shared_ptr<PVProject>> pending_pvprojects;
        std::deque<std::shared_ptr<PVProject>> pending_pvprojects_to_add;
        std::mutex pending_pvprojects_lock;
        
        std::vector<std::vector<std::weak_ptr<PVProject>>> schedule_visits; /*!< schedule for visits for the permitting accessment, length is equal to the MaxLengthWaitPermitVisit. @DevStage2 think about it, not every jurisdiction will need it */
        
        std::size_t i_schedule_visits;
        
        void collect_inf_site_visit(std::shared_ptr<PVProject> project_);
        void approve_permit(std::shared_ptr<PVProject> project_);
        //void pass_inspection(std::shared_ptr<PVProject> project_); //added this because this is one more area of interaction between SEI and G
 

        std::set<EParamTypes> project_states_to_delete;
        
        //@}
        
        
        //@{
        
        /**
         
         Section with internal parameters
         
         */
        
        
        std::map<EParamTypes, double> params; /** Parameters of government */
        
        void ac_update_tick();
        
        
        W* w;
        
        TimeUnit a_time;
        //@}
        
        
    };
} //end namespace solar_core





#endif /* defined(__ABMSolar__G__) */
