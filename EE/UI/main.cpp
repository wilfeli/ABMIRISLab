//
//  main.cpp
//  ABMSolar
//
//  Created by Ekaterina Sinitskaya on 7/29/16.
//  Copyright (c) 2016 IRIS Lab. All rights reserved.
//


/** /!\ HERE BE DRAGONS /!\
 *
 ^    ^
 / \  //\
 |\___/|      /   \//  .\
 /O  O  \__  /    //  | \ \
 /     /  \/_/    //   |  \  \
 @___@'    \/_   //    |   \   \
 |       \/_ //     |    \    \
 |        \///      |     \     \
 _|_ /   )  //       |      \     _\
 '/,_ _ _/  ( ; -.    |    _ _\.-~        .-~~~^-.
 ,-{        _      `-.|.-~-.           .~         `.
 '/\      /                 ~-. _ .-~      .-~^-.  \
 `.   {            }                   /      \  \
 .----~-.\        \-'                 .~         \  `. \^-.
 ///.----..>    c   \             _ -~             `.  ^-`   ^-_
 ///-._ _ _ _ _ _ _}^ - - - - ~                     ~--,   .-~
 
 */


#include <thread>
#include <iostream>
#include <boost/filesystem.hpp>
#include "UI/WEE.h"
#include "UI/UI.h"
#include "UI/UIBL.h"
#include "UI/HelperW.h"



#include "UI/model.h"



#ifdef _WIN64
#include  <io.h>
#endif

using namespace solar_core;


int main(int argc, const char * argv[])
{
    solar_ui::UIBL* ui = reinterpret_cast<solar_ui::UIBL*>(init_model(argc, argv));
    
    //introduce some delay
    for (auto i = 0; i <  10000; ++i)
    {};
    
    
    run_model(ui);
    
    return 0;
    
}


#ifdef __cplusplus
extern "C" {
#endif

    

void* init_model(int argc, const char** argv)
{
    //store starting directory
    boost::filesystem::path full_path(boost::filesystem::current_path());
    
    
    //in parameters - argv[1] - mode, argv[2] - path to model file (save/load)
    boost::filesystem::path path_to_model_file(argv[2]);
    
    
    std::string MODE = argv[1];
    
#ifdef __APPLE__
    //check that there is file, otherwise use default
    if (access(path_to_model_file.string().c_str(), F_OK) != -1)
#endif
    
#ifdef _WIN64
    if (_access(path_to_model_file.string().c_str(), 00) != -1)
#endif
    {
    }
    else
    {
        throw std::runtime_error("No model file");
    };
    
    WEE* w = nullptr;
    
    
    
    if (MODE == "NEW")
    {
        auto helper = new solar_core::HelperWSpecialization<solar_core::W, solar_core::BaselineModel>();
        
        //create world
        w = new WEE(path_to_model_file.string(), helper, "NEW");
        
        //initialize world
        w->init();
    }
    else
    {
        throw std::runtime_error("Unsupported mode");
    };
    
    
    w->FLAG_IS_STARTED = true;
    w->FLAG_IS_STOPPED = false;
    w->FLAG_IS_PAUSED = false;
    
    solar_ui::UIBL* ui = new solar_ui::UIBL();
    
    //initialize world
    ui->init(w);
    
    return ui;
}
    
    
int run_model(void* ui_)
{
    
    auto ui = reinterpret_cast<solar_ui::UIBL*>(ui_);
    auto w = ui->w;
    
    //start threads with fs, bs, main cycle, markets
    std::vector<std::thread> threads;
    
    void (WEE::*func)();
    
    func = &WEE::life;
    threads.push_back(std::thread(func, w));
    
//    func = &WEE::life_hos;
//    threads.push_back(std::thread(func, w));
    
    func = &WEE::life_seis;
    threads.push_back(std::thread(func, w));
    
    func = &WEE::life_sems;
    threads.push_back(std::thread(func, w));
    
    
    time_t rawtime;
    tm* timeinfo;
    
    time (&rawtime);
    timeinfo = localtime(&rawtime);
    
    char mbstr[100];
    std::strftime(mbstr, sizeof(mbstr), "%a %F %T ", timeinfo);
    std::cout << "time: " <<  mbstr << " "<< "INFO: " << "started running" << std::endl;
    
    // work for the workers to finish
    for(auto& t : threads)
    {
        t.join();
    };
    
    time (&rawtime);
    timeinfo = localtime(&rawtime);
    std::strftime(mbstr, sizeof(mbstr), "%a %F %T ", timeinfo);
    std::cout << "time: " <<  mbstr << " "<< "INFO: " << "stopped running" << std::endl;
    
    
    //save model here
    ui->save();

    return 0;
    
}


#ifdef __cplusplus
} //C API
#endif








