//
//  model.h
//  ABMSolar
//
//  Created by Ekaterina Sinitskaya on 8/6/16.
//  Copyright (c) 2016 IRIS Lab. All rights reserved.
//

#ifndef ABMSolar_model_h
#define ABMSolar_model_h

#ifdef __cplusplus
extern "C" {
#endif

void* init_model(int argc, const char** argv);
    
int run_model(void* ui_);

#ifdef __cplusplus
} //C API
#endif

#endif
