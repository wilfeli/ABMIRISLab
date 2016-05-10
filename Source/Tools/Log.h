//
//  Log.h
//  ABMSolar
//
//  Created by Ekaterina Sinitskaya on 5/7/16.
//  Copyright (c) 2016 IRIS Lab. All rights reserved.
//

#ifndef __ABMSolar__Log__
#define __ABMSolar__Log__

#include <string>

namespace solar_core
{
    
    
    /**
     
     
     Registration of classes
     
     */
    
    class Log
    {
    public:
        
        static Log& instance();
        static Log& instance(std::string path_);
        
        void log(std::string mes_, std::string mes_type_ = "ERROR: ");
        
        //@{
        /**
         *
         * Parameteres, setting up, etc. section
         *
         *
         *
         *
         */
        void log();
        
        //@}
        
    private:
        Log(std::string path_);
        
        std::string path_to_log_file = "../Logs/world_log.log";/*!< is created during run, has current time stamp */
        
    };

        
        
    
    
} //end namespace solar_core



#endif /* defined(__ABMSolar__Log__) */
