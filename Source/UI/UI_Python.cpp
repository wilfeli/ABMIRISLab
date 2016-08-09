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
};

#ifdef __cplusplus
} //C API
#endif