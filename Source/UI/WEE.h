//
//  WEE.h
//  ABMSolar
//
//  Created by Ekaterina Sinitskaya on 7/9/16.
//  Copyright (c) 2016 IRIS Lab. All rights reserved.
//

#ifndef __ABMSolar__WEE__
#define __ABMSolar__WEE__

#include "UI/UI_Python.h"
#include "Tools/ID.h"
#include "UI/W.h"

namespace solar_ui
{
    class UIBL;
}
namespace solar_core {

    
    class PVProjectFlat;
    class SEIBL;
    class SEMBL;
    class H;
    class ExploreExploit;
    template<class Param> class WorldMapSpecialization;
    

class WEE: public W
{
    template <class T1, class T2> friend class HelperWSpecialization;
    friend class solar_ui::UIBL;
    friend int ::C_API_run_model_steps(void* ui_, int N);
public:
    WEE(std::string path_, HelperW* w_, std::string mode_ = "NEW");
    virtual void init() override;
    
    
    //@{
    /**
     
     Interactions with other agents
     
     */
    
    
    
    virtual void get_state_inf_installed_project(std::shared_ptr<PVProjectFlat> project_);
    double get_inf(EDecParams type_, SEIBL* agent_);
    //@}
    
    
    
    //@{
    /**
     
     Interactions on geography
     
     */
    
    WorldMapSpecialization<WEE>* world_map;
    
    
    //@}
    
    
    //@{
    /**
     
    Agents 
     
     */
    
	

    
    std::vector<SEMBL*>* sems; /*!< all SEM */
    std::vector<H*>* hos;/*!< all H agents */
    std::vector<SEIBL*>* seis;/*!< all SEI agents */

	std::vector<SEIBL*>* const get_seis() const { return seis; };
    //@}
    
    
    //@{
    /**
     
     Main loop
     
     */
    
    virtual void life_hos() override; /*!< life of Homeowners */
    virtual void life_seis() override;
    virtual void life_sems() override;


    virtual void life() override;
    virtual void ac_update_tick() override;
    void ac_update_wm();
    
    
    //@}
    
    
    
    
    
    //@{
    /**
     
     Results of simulations
     
     */
    std::vector<std::shared_ptr<PVProjectFlat>> pool_projects;
    std::size_t i_pool_projects;
    //@}
    
protected:
    
    //@{
    /**
     
     Initialization part
     
     */
    
    
    virtual void create_world(boost::filesystem::path& path_to_model_file, boost::filesystem::path& path_to_dir, boost::filesystem::path& path_to_template, PropertyTree& pt, std::map<std::string, std::string>& params_str) override;
    
    
    
    //@}

    
    

    typedef W Super;

    std::map<UID, std::vector<std::shared_ptr<PVProjectFlat>>> installed_projects_time;
    std::vector<std::map<UID, std::vector<std::shared_ptr<PVProjectFlat>>>> installed_projects_history;
    std::map<UID, double> market_share_seis;
    std::vector<UID> sorted_by_market_share_seis;
    
    int64_t N_installed_projects_time = 0;
    
    
    std::vector<std::vector<double>> history_data;
    void save_end_data(); /*!< saves data from the last tick */
    
    
};

} //end namespace solar_core


#endif /* defined(__ABMSolar__WEE__) */
