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
    virtual void request_permit(std::shared_ptr<PVProject> project_); /*!< request permit for the project */
    
    //@}
    
    
    
    
    
    
    
    
protected:
    
    
    //@{
    /**
     
     Section relevant for permitting process
     
     */
    
    std::deque<std::shared_ptr<PVProject>> pending_projects;
    
    std::vector<std::vector<std::weak_ptr<PVProject>>> schedule_visits; /*!< schedule for visits for the permitting accessment, length is equal to the MaxLengthWaitPermitVisit
                                                                         */
    
    std::size_t i_schedule_visits;
    
    void collect_inf_site_visit(std::shared_ptr<PVProject> project_);
    void grant_permit(std::shared_ptr<PVProject> project_);
    
    
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
