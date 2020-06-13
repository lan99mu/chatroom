/*工具类*/
#ifndef TOOLS_H
#define TOOLS_H
#include <iostream>
#include <string>
#include <vector>
class Tools
{
    public:
        Tools();
        ~Tools();
        //分割字符串
        static void split(std::string & s, const std::string & division, std::vector <std::string> & vs);
};
#endif
