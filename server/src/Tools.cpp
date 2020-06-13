#include "Tools.h"
Tools::Tools()
{

}

Tools::~Tools()
{

}

//分割字符串
void Tools::split(std::string &s, const std::string & divison, std::vector <std::string> & vs)
{
    size_t last = 0;
    //返回divison中任一字符在s中第一次出现的位置，在last之后
    size_t index = s.find_first_of(divison, last);
    while(index != std::string::npos)
    {
        vs.push_back(s.substr(last, index - last));
        last = index + 1;
        index = s.find_first_of(divison, last);
    }
    if(index - last > 0)
        vs.push_back(s.substr(last, index - last));
}
