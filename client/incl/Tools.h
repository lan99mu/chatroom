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
        //将字符流转换成比特流
        static void string_to_bits(std::string &s, std::string &bits);
        //将比特流转换成字符流
        static void bits_to_string(std::string &bits, std::string &s);
};
#endif
