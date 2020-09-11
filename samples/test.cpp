/**
 * @file test.cpp
 * @brief 
 * @author arthur
 * @version 1.0.0
 * @date 2020-08-01
 */

#include "kconfig.hpp"
using namespace kconfig; 

int main()
{

    // KConfig aConfig  ; 
    // aConfig.read("samples/test.cfg"); 

    // std::cout << "port is " << aConfig.get_int("port")<< std::endl; 
    // std::cout << "start is " << aConfig.get_bool("start")<< std::endl; 

    KConfig segConfig (true); 
    segConfig.read("samples/test.cfg"); 

    std::cout << "port is " << segConfig.get_int("database.port")<< std::endl; 
    std::cout << "start is " << segConfig.get_bool("database.start")<< std::endl; 
    std::cout << "name is " << segConfig.get_string("database.name")<< std::endl; 


//    std::vector<int> keys =  segConfig.get_array<int>("database.items") ; 
//    for(auto it:keys)
//    {
//        std::cout << "item is " << it << std::endl; 
//    }
//
//
//    std::cout << "template get " <<  segConfig.get<int>("database.port") << std::endl; 
//
//
//
//    segConfig.get<int>("database.port",[](const int &rst ){
//
//            std::cout << " result is "<< rst << std::endl; 
//
//            } ) ; 
//
//
//    std::cout << "file is " << segConfig.get_string("database.file",[](const std::string & result) {
//
//            std::cout << " get string is " <<  result << std::endl; 
//
//            } )<< std::endl; 

    return 0; 
}; 

