//
//  VectorMath.h
//  ABMSolar
//
//  Created by Ekaterina Sinitskaya on 10/17/16.
//  Copyright (c) 2016 IRIS Lab. All rights reserved.
//

#ifndef __ABMSolar__VectorMath__
#define __ABMSolar__VectorMath__


#include "ExternalIncludes.h"


namespace tools {
    
    
    template <class T>
    class Vector2D
    {
    public:
        
        Vector2D()
        {
            data = new std::vector<std::vector<T>>();
        
        }
        
        ~Vector2D()
        {
            delete data;
        }
        
        
    protected:
        std::vector<std::vector<T>>* data;
    };
    
    
} //end namespace tools




#endif /* defined(__ABMSolar__VectorMath__) */
