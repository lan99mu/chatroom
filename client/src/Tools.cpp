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

//将字符流转换成比特流
void Tools::string_to_bits(std::string &s, std::string &bits)
{
    bits = "";
    for(char ch : s)
    {
        for(int i = 7; i >= 0; i --)
        {
            if(ch & (1 << i))

                bits += "1";
            else 
                bits += "0";
        }
    }
}

//将比特流转换成字符流
void Tools::bits_to_string(std::string &bits, std::string &s)
{
    s = "";
    int size = bits.size();
    for(int i = 0; i < size; i += 8)
    {
        char ch = 0;
        for(int j = 0; j < 8; j ++)
        {
            ch = ch * 2 + bits[i + j] - '0';
        }
        s += ch;
    }
}
