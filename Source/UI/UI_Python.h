//
//  UI_Python.h
//  ABMSolar
//
//  Created by Ekaterina Sinitskaya on 8/6/16.
//  Copyright (c) 2016 IRIS Lab. All rights reserved.
//

#ifndef __ABMSolar__UI_Python__
#define __ABMSolar__UI_Python__

#ifdef __cplusplus
#include "UI/UIBL.h"



extern "C" {
#endif

    #include <stddef.h>

    int run_model_test(int argc, const char ** argv);


    struct HPVProjectFlat; // An opaque type that we'll use as a handle
    typedef struct HPVProjectFlat HPVProjectFlat;
    
    
    struct HUIBL; // An opaque type that we'll use as a handle
    typedef struct HUIBL HUIBL;
    
    
    typedef struct C_API_Vector
    {
        double *data;
        size_t size;
    } C_API_Vector;
    
    
    
    typedef struct C_API_PVProjectFlat
    {
        double DC_size;
        
    } C_API_PVProjectFlat;
    
    
    typedef struct C_API_Vector_PVProjectFlat
    {
        C_API_PVProjectFlat* data;
        size_t size;
    } C_API_Vector_PVProjectFlat;
    
    
    
    C_API_Vector_PVProjectFlat C_API_get_pvprojects(HUIBL * ui);
    
    
    double C_API_estimate_profit(HUIBL *ui_, int sei_i, double p);
    double C_API_estimate_irr(HUIBL *ui_, int sei_i, double p);
    double C_API_estimate_demand(HUIBL* ui_, int sei_i, double p, int size_THETA, const double* THETA);
    
    

#ifdef __cplusplus
} //C API
#endif







#endif /* defined(__ABMSolar__UI_Python__) */
