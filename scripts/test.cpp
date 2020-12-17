#include <fstream>
#include <iostream>
#include <locale>
#include <sstream>


std::wstring getName() // Read name from file
{
    std::wifstream wif("/home/piro_tex/test_string");
    wif.imbue(std::locale("en_US.UTF-8"));
    std::wstringstream wss;
    wss << wif.rdbuf();

    std::wstring w_name;
    std::getline(wss, w_name);

    return w_name;
}


int main()
{
    std::wstring w_name = getName();
    std::wcout << w_name << std::endl;

    for(size_t it = 0; it < w_name.length(); it++)
    {
        std::wcout << "symbol: " << w_name[it];
        std::wcout << ", hex: " << std::hex << (unsigned int)w_name[it] << std::endl;
        
        unsigned int byte = (unsigned int)w_name[it];
        unsigned char c_byte = 0;

        switch(byte) // Unicode - russian
        {
            case 0x410:
            case 0x430:
                c_byte = 0x61;
        } // switch(byte)


        std::wcout << "byte: " << byte << ", c_byte: " << c_byte << std::endl;


    } // for(size_t it = 0; it < w_name.length(); it++)

    return 0;
} // int main()