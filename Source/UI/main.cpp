//
//  main.cpp
//  ABMSolar
//
// Hi there
//  Created by Ekaterina Sinitskaya on 2/22/16.
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
#include "ExternalTools/boost_1_57_0/boost/filesystem.hpp"
#include "UI/W.h"



using namespace solar_core;


int main(int argc, const char * argv[])
{
    
    //store starting directory
    boost::filesystem::path full_path(boost::filesystem::current_path());
    
    
    //in parameters - argv[1] - mode, argv[2] - path to model file (save/load)
    boost::filesystem::path path_to_model_file(argv[2]);
    
    
    std::string MODE = argv[1];
    
    
    //check that there is file, otherwise use default
    if (access(path_to_model_file.string().c_str(), F_OK) != -1)
    {
    }
    else
    {
        throw std::runtime_error("No model file");
    };
    
    W* w = nullptr;
    
    
    
    if (MODE == "NEW")
    {
        //create world
        w = new W(path_to_model_file.string(), "NEW");
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

    
    
    //start threads with fs, bs, main cycle, markets
    std::vector<std::thread> threads;
    
    void (W::*func)();
    
    func = &W::life;
    threads.push_back(std::thread(func, w));
    
    func = &W::life_hhs;
    threads.push_back(std::thread(func, w));
    
    func = &W::life_seis;
    threads.push_back(std::thread(func, w));
    
    func = &W::life_sems;
    threads.push_back(std::thread(func, w));

    func = &W::life_gs;
    threads.push_back(std::thread(func, w));
    
    func = &W::life_markets;
    threads.push_back(std::thread(func, w));
    
    
    
    // work for the workers to finish
    for(auto& t : threads)
    {
        t.join();
    };
    
    
    
    return 0;

    
    


}

















