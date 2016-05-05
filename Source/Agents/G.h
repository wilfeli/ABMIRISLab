//  Kelley reviewed 4/26/16 line 17, 22, 24, 49, 65, 67-69, 73-75, 97
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

//initially when I was reviewing this file, I didn't understand what G was; this may become clear once we add parameters but maybe changing name of class?

namespace solar_core
{
   
class PVProject;
//why do we call PVProject here? Why not call on agent SolarPanel?
    

class G 
{
public:
    //@{
    /**
     
     Section with actions in the world
     
     */
    
    
    virtual void act_tick() ;
    //@}
    
    //@{
    /**
     
     Section relevant for permitting process
     
     */
    virtual void request_permit(std::shared_ptr<PVProject> project_); /*!< request permit for the project */
    //@}
    
    //add virtual void request_inspection
    
    
    
    
    
protected:
    
    
    //@{
    /**
     
     Section relevant for permitting process
     
     */
    
    std::deque<std::shared_ptr<PVProject>> pending_projects; //projects are in pending state twice, during permitting and during inspection
    
    std::vector<std::vector<std::weak_ptr<PVProject>>> schedule_visits; /*!< schedule for visits for the permitting assessment, length is equal to the MaxLengthWaitPermitVisit */
    //because of lack of standardization, some jurisdictions require visits while others do not
	//should jurisdiction be considered a class, kind of like PVProject to account for wait times, online/in-person turn-in requirements, etc. or can this still be modeled as an agent?
    std::size_t i_schedule_visits; //this would most likely be for inspection
    

    void collect_inf_site_visit(std::shared_ptr<PVProject> project_); //what does this void mean?
    void grant_permit(std::shared_ptr<PVProject> project_); //I think better to rename approve_permit
	void pass_inspection(std::shared_ptr<PVProject> project_); //added this because this is one more area of interaction between SEI and G
    
    
    //@}
    
    
    //@{
    
    /**
     
     Section with internal parameters
     
     */
    
    
    std::map<EParamTypes, double> params; /** Parameters of government */
    
    void ac_update_tick();
    
    
    W* w; 
    
    TimeUnit a_time; //Is this initialized? Because it is a typedef, is there a special way to initialize? it is a piece of memory, right?
    //@}
    
    
};
} //end namespace solar_core





#endif /* defined(__ABMSolar__G__) */
