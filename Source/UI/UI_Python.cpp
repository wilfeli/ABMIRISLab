//
//  UI_Python.c
//  ABMSolar
//
//  Created by Ekaterina Sinitskaya on 8/6/16.
//  Copyright (c) 2016 IRIS Lab. All rights reserved.
//

#include "UI/WEE.h"
#include "UI/UI_Python.h"
#include "Agents/SolarPanel.h"
#include "Agents/SEIBL.h"


int run_model_test(int argc, const char ** argv)
{
    return 5;
};

#ifdef __cplusplus




extern "C" {
#endif
    
    
C_API_Vector_PVProjectFlat C_API_get_pvprojects(HUIBL * ui_)
{
    solar_ui::UIBL* ui = reinterpret_cast<solar_ui::UIBL*>(ui_);
    
    //
    C_API_Vector_PVProjectFlat ret;
    
    ret.data = static_cast<C_API_PVProjectFlat*>(malloc((ui->w->i_pool_projects + 1) * sizeof(*ret.data)));
    
    for (auto i = 0; i < ui->w->i_pool_projects; ++i)
    {
        ret.data[i].DC_size = ui->w->pool_projects[i]->DC_size;
    };
    
    
    ret.size = ui->w->i_pool_projects;
    
    
    return ret;
}
    
    
 
double C_API_estimate_profit(HUIBL* ui_, int sei_i, double p)
{
    
    solar_ui::UIBL* ui = reinterpret_cast<solar_ui::UIBL*>(ui_);
    
    //pick agent by number
    auto sei = (*ui->w->seis)[sei_i];
    
    //design
    auto dec_design_hat = sei->dec_design;
    
    //allocate PVProjectFlat
    auto average_project = solar_core::PVProjectFlat();
    
    //setup project
    sei->init_average_pvproject(dec_design_hat, &average_project, p);
    
    //estimate profit
    auto profit = sei->est_profit(dec_design_hat, &average_project, p);
    
    return profit;
    
}

    
double C_API_estimate_irr(HUIBL* ui_, int sei_i, double p)
{
    
    solar_ui::UIBL* ui = reinterpret_cast<solar_ui::UIBL*>(ui_);
    
    //pick agent by number
    auto sei = (*ui->w->seis)[sei_i];
    
    //design
    auto dec_design_hat = sei->dec_design;
    
    //allocate PVProjectFlat
    auto average_project = solar_core::PVProjectFlat();
    
    //setup project
    sei->init_average_pvproject(dec_design_hat, &average_project, p);
    
    //estimate irr
    auto irr = sei->est_irr_from_params(dec_design_hat, &average_project, p);
    
    return irr;
    
}
    
    
double C_API_estimate_demand(HUIBL* ui_, int sei_i, double p, int size_THETA, const double* THETA)
{
    solar_ui::UIBL* ui = reinterpret_cast<solar_ui::UIBL*>(ui_);
    //pick agent by number
    auto sei = (*ui->w->seis)[sei_i];

    
    //set THETA for demand estimation
    for (auto i = 0; i < size_THETA; ++i)
    {
        //iterate over the array
        sei->THETA_demand[i] = THETA[i];
    };
    
    
    //design
    auto dec_design_hat = sei->dec_design;
    
    //allocate PVProjectFlat
    auto average_project = solar_core::PVProjectFlat();
    
    //setup project
    sei->init_average_pvproject(dec_design_hat, &average_project, p);
    
    //estimate irr
    auto demand = sei->est_demand_from_params(dec_design_hat, &average_project, p);
    
    
    return demand;
}
    
    
    

    

    

#ifdef __cplusplus
} //C API
#endif