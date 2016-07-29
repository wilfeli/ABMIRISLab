//
//  W.h
//  ABMSolar
//
//  Created by Ekaterina Sinitskaya on 2/22/16.
//  Copyright (c) 2016 IRIS Lab. All rights reserved.
//

#ifndef ABMSolar_W_h
#define ABMSolar_W_h


#include <mutex>
#include <atomic>
#include "Tools/ExternalIncludes.h"
#include "Tools/IParameters.h"
#include "Tools/IRandom.h"


namespace solar_ui
{
    class UI;
}


namespace solar_core {
    
    
    
    class IAgent;
    class WorldMap;
    class G;
    class MarketingInst;
    class Utility;
    class SEI;
    class SEM;
    class Homeowner;
    class PVProject;
    class PVDesign;
    class HelperW;
    
    
    
    
    
    
    
    
    class W
    {
        friend class MarketingInst;
        friend class solar_ui::UI;
    public:
        //@{
        /**
         
         Section with initialization and serialization code.
         
         Multiple step initialization from the project file. Keep structure of a project file simple, all parameters in one file.
         
         @DevStage2: serialization: choose custom/binary/to database. For now think that cereal will be emough, with saving to .json. All agents of the same type will be saved in the same file. Loading also from .json with simple structure.
         
         */
        
        W() = default;
        W(std::string path_, HelperW* w_, std::string mode_ = "NEW"); /*!< */
        virtual void init();
        std::string base_path;
        
        
        
        //@}
        
        
        
        
        //@{
        /**
         
         Section with main loop
         
         */
        
        virtual void life(); /*!< general loop */
        
        virtual void life_hos(); /*!< life of Homeowners */
        void life_seis(); /*!< life of seis */
        void life_sems(); /*!< life of sems */
        void life_gs(); /*!< life of g */
        void life_markets(); /*!< life of marketing */
        
        virtual void ac_update_tick();
        
        
        
        
        
        
        //@}
        
        
        //@{
        /**
         
         Section with general world parameters and etc.
         
         */
        
        std::atomic<bool> FLAG_IS_STOPPED;
        std::atomic<bool> FLAG_IS_STARTED;
        std::atomic<bool> FLAG_IS_PAUSED;
        
        TimeUnit time;
        TimeUnit begin_time = 0;
        std::mutex lock_tick; /*!< lock for tick */
        std::condition_variable all_update; /*!< waits until all have updated */
        
        std::atomic<long> updated_counter;
        std::atomic<long> notified_counter;
        
        std::atomic<bool> FLAG_SEI_TICK;
        std::atomic<bool> FLAG_H_TICK;
        std::atomic<bool> FLAG_G_TICK;
        std::atomic<bool> FLAG_UTILITY_TICK;
        std::atomic<bool> FLAG_MARKET_TICK;
        std::atomic<bool> FLAG_SEM_TICK;
        
        
        IRandom* rand = nullptr; /*!< random number generator, same for everyone for now */
        
        //@}
        
        //@{
        /**
         
         Interactions on geography
         
         */
        
        double get_solar_irradiation(double location_x, double location_y) const; /*!< returns estimated amount of solar irradiation for the tile */
        double get_permit_difficulty(double location_x, double location_y) const; /*!< returns permit difficulty */
        
        WorldMap* world_map;
        
        //@}
        
        
        
        
        //@{
        /**
         
         Interactions with other agents
         
         */
        
        
        void get_state_inf(Homeowner* agent_, EParamTypes state_); /*!< gets information about state change from agent */
        virtual void get_state_inf_installed_project(std::shared_ptr<PVProject> project_); /*!< is called when project is finished to record it */
        void get_state_inf_interconnected_project(std::shared_ptr<PVProject> project_); /*!< is called when project is interconnected to record it */
        
        
        //@}
        
        
        //@{
        /**
         
         Institutions
         
         */
        
        G* g; /*!< government */
        MarketingInst* marketing;
        Utility* utility; 
        //@}
        
        
        
        
    protected:
        
        //@{
        /**
         
         Initialization part
         
         */
        
        
        virtual void create_world(boost::filesystem::path& path_to_model_file, boost::filesystem::path& path_to_dir, boost::filesystem::path& path_to_template, PropertyTree& pt, std::map<std::string, std::string>& params_str);
        
        
        
        //@}
        
        
        
        
        std::deque<IAgent*> get_inf_marketing_sei_agents; /*!< Agents that requested information, inform SEI that there is request for information for this agent */
        
        
        
        std::vector<Homeowner*> hos;/*!< all H agents */
        std::vector<SEI*> seis;/*!< all SEI agents */
        std::vector<SEM*> sems; /*!< all SEM */
        /*!< H agents that are active, @DevStage3 think about splitting more fine grained */
        
        std::map<std::string, std::string> params; /*!< have string values to parameters */
        std::map<EParamTypes, double> params_d; /*!< have numerical values to parameters */
        
        std::vector<std::shared_ptr<PVProject>> interconnected_projects; /*!< @DevStage2 think here, might change to weak_ptr, but will pay the cost of checking each time if it is still alive */
        
        
        
        
        
    };
    
    
    
    
    
    
}



#endif
