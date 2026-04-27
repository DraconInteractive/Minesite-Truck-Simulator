#pragma once

namespace Utilities
{
    template<typename T>                                            
    static T Lerp(T a, T b, float t)                                
    {                                                               
        return a + (b - a) * t;                                     
    }
    
};