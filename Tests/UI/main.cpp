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


#include "gtest/gtest.h"


#include <thread>
#include <iostream>
#include "ExternalTools/boost_1_57_0/boost/filesystem.hpp"
#include "UI/W.h"





using namespace solar_core;




namespace{
    
    
    // The fixture for testing payment system
    class WTest : public ::testing::Test
    {
    protected:
        WTest()
        {
        }
        
        
        virtual ~WTest()
        {
        }
        
        
        static void SetUpTestCase()
        {
            boost::filesystem::path path_to_template("");
            boost::filesystem::path path_to_save("");
            
            
            //create world
#ifdef __linux__
            path_to_template = "/home/cat/Dropbox/aimacro/gaim/gaim/Examples/simple2/world_base_template.json";
#endif
            
#ifdef  __APPLE__
            path_to_template = "/Users/wilfeli/XCodeProjects/gaim/gaim/Examples/simple2/world_base_template.json";
#endif
            
#ifdef _WIN64
            
            
            
#endif
            
            
            
#ifdef  __APPLE__
            path_to_save = "/Users/wilfeli/XCodeProjects/gaim/gaim/Saves/simple2/world_base_template.json";
#endif
            
            
            //create world
            w = new W(path_to_template.string(), "NEW");
            
            
            //replace MarketGeneralFX with its Mock variant
            for (auto iter = w->markets.begin(); iter != w->markets.end(); ++iter)
            {
                if (dynamic_cast<MarketGeneralFX*>(*iter))
                {
                    delete (*iter);
                    (*iter) = new MarketGeneralFXMock();
                };
            };
            
            
            //initialize world
            w->init();
            
            
            //create ui
            ui = new gaim_ui::UIW(w);
            
            
            w->FLAG_IS_STARTED = true;
            w->FLAG_IS_STOPPED = false;
            
            //run world for 1 tick to allow for the delayed initialization to take place
            
            
            //start threads with fs, bs, main cycle, markets
            std::vector<std::thread> threads;
            
            
            void (*func)();
            
            func = &PSTest::WActTickH;
            threads.push_back(std::thread(func));
            
            func = &PSTest::WActTickF;
            threads.push_back(std::thread(func));
            
            func = &PSTest::WActTickB;
            threads.push_back(std::thread(func));
            
            func = &PSTest::WActTickCB;
            threads.push_back(std::thread(func));
            
            func = &PSTest::WActTickG;
            threads.push_back(std::thread(func));
            
            // work for the workers to finish
            for(auto& t : threads)
            {
                t.join();
            };
            
            
            
        }
        
        
        static void WActTickH(){
            
            for (auto agent:w->hs)
            {
                //get tick
                agent->act_tick(w->time_tick);
            };
            
            
        };
        
        
        static void WActTickB(){
            
            for (auto agent:w->bs)
            {
                //get tick
                agent->act_tick(w->time_tick);
            };
            
            
        };
        
        
        static void WActTickF(){
            
            for (auto agent:w->fs)
            {
                //get tick
                agent->act_tick(w->time_tick);
            };
            
            
        };
        
        static void WActTickG(){
            
            for (auto agent:w->gs)
            {
                //get tick
                agent->act_tick(w->time_tick);
            };
            
            
        };
        
        
        static void WActTickCB(){
            
            for (auto agent:w->cbs)
            {
                //get tick
                agent->act_tick(w->time_tick);
            };
            
            
        };
        
        
        static void TearDownTestCase() {
            
            //deletes world
            //now DevStage1 does nothing, later will clean agents
            
            delete w;
            w = nullptr;
            
            delete ui;
            ui = nullptr;
        }
        
        
        
        // If the constructor and destructor are not enough for setting up
        // and cleaning up each test, you can define the following methods:
        
        virtual void SetUp() {
            // Code here will be called immediately after the constructor (right
            // before each test).
        }
        
        virtual void TearDown() {
            // Code here will be called immediately after each test (right
            // before the destructor).
        }
        
        
        virtual bank::B* get_b(std::size_t i){
            return w->bs[i];
        };
        
        
        virtual g::G* get_g(std::size_t i){
            return w->gs[i];
        };
        
        
        virtual cb::CB* get_cb(std::size_t i){
            return w->cbs[i];
        };
        
        
        virtual human::H* get_h(std::size_t i){
            return w->hs[i];
        };
        
        
        virtual firm::F* get_f(std::size_t i){
            return w->fs[i];
        };
        
        
        static W* w;
        static gaim_ui::UIW* ui;
        
        
        virtual std::vector<human::H*> get_hs(){
            return w->hs;
        };
        
        virtual std::vector<firm::F*> get_fs(){
            return w->fs;
        };
        
    };
    
    W* PSTest::w = nullptr;
    gaim_ui::UIW* PSTest::ui = nullptr;

}




int main(int argc, char * argv[])
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
















