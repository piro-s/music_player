/**
 *******************************************************************************
 * @file           : vft_text.c
 * @author         : piro.tex
 * @brief          : Program to send text and symbols to VFD
 *******************************************************************************
*/


#include <iostream>
#include <fstream>
#include <sstream>
#include <locale>

#include <wiringPi.h>
#include <unistd.h>

///--- enum's
enum vfd 
{
    vfd_DI          = 7,
    vfd_CK          = 8,
    vfd_CS          = 9,

    vfd_place_1     = 0x20,
    vfd_symbol_1    = 0x60,
    vfd_segm_max    = 12, // Max segments on display
    vfd_symb_max    = 16, // Max symbols on display

    vfd_sym_prev    = 0x06,
    vfd_sym_play    = 0x07,
    vfd_sym_pause   = 0x08,
    vfd_sym_next    = 0x09,

    read_sym_prev   = 0,
    read_sym_play   = 1,
    read_sym_pause  = 2,
    read_sym_next   = 3
};


///--- functions
std::wstring getName(); // Return songs name
unsigned int getSymbol(); // Return symbol what need to set

//--- pin functions
void vfd_DI_low()   { digitalWrite(vfd_DI, LOW);    }
void vfd_DI_high()  { digitalWrite(vfd_DI, HIGH);   }
void vfd_CK_low()   { digitalWrite(vfd_CK, LOW);    }
void vfd_CK_high()  { digitalWrite(vfd_CK, HIGH);   }
void vfd_CS_low()   { digitalWrite(vfd_CS, LOW);    }
void vfd_CS_high()  { digitalWrite(vfd_CS, HIGH);   }

//--- vfd functions
void vfd_init(); // Init vfd
void vfd_tranmitByte(unsigned int byte); // Transmit byte
void vfd_setSymbol(unsigned int symbol); // Set symbol (Only one, another reset)
void vfd_resetSymbols(); // Reset all symbols
void vfd_sendChar(unsigned int place, wchar_t byte); // Send char
void vfd_sendString(std::wstring name); // Send wstring (12 symbols from vfd_place_1)
void vfd_sendShiftName(std::wstring name, int shift); // Send shift name

//--- vfd russian
unsigned int  vfd_checkRussian(unsigned int byte); // Check for russian symbols
void vfg_sendRussian(); // Send russian 'b' and 'v' in user memory

 
int main() 
{
    try
    {
        if(wiringPiSetup() == 0)
        {
            vfd_init(); // Init VFD
            vfd_resetSymbols(); // Reset all symbols
            vfg_sendRussian(); // Send russian 'b' and 'v' in user memory
            
            //--- Init variables

            std::wstring name, prev_name = getName();
            unsigned int symbol, prev_symbol = getSymbol();
            size_t shift = 0, shift_max = 0;


            while(1) // Infinite loop
            {
                //--- Name
                name = getName();
                shift_max = name.length() - vfd_segm_max;
                if(prev_name != name) // If song change
                {
                    prev_name = name;
                    shift = 0;
                } // if(prev_name != name)


                if(name.length() < vfd_segm_max) // If name place on display
                    vfd_sendString(name);
                else // Shift if doesn't
                    vfd_sendShiftName(name, shift++);


                if(shift > shift_max) // Check shift
                    shift = 0;


                //--- Symbol
                symbol = getSymbol();
                if(prev_symbol != symbol) // If symbol change
                {
                    prev_symbol = symbol;
                    vfd_setSymbol(symbol); // Send digit
                } // if(prev_symbol != symbol)
                else if((prev_symbol == read_sym_prev) |
                        (prev_symbol == read_sym_next))
                {
                    prev_symbol = read_sym_play;
                    symbol = read_sym_play;
                    vfd_setSymbol(symbol); // Send digit
                }


                usleep(500 * 1000); // Sleep 500 ms
            } // while(1)
        } // if(wiringPiSetup() == 0)
    } // try
    catch (std::exception& e)
    {
        std::cerr << e.what() << std::endl;
    }

    return 0;
} // int main() 


///--- functions
std::wstring getName() // Return songs name
{
    std::wifstream wif("mpc_now");
    wif.imbue(std::locale("en_US.UTF-8"));
    std::wstringstream wss;
    wss << wif.rdbuf();

    std::wstring w_name;
    std::getline(wss, w_name);

    return w_name;
} // std::wstring getName()

unsigned int getSymbol() // Return symbol what need to set
{
    std::wifstream wif("vfd_set");
    wif.imbue(std::locale("en_US.UTF-8"));
    std::wstringstream wss;
    wss << wif.rdbuf();

    std::wstring w_symbol;
    std::getline(wss, w_symbol);

    return ((unsigned int)w_symbol[0] - 0x30); // - 0x30 because of hex
} // unsigned int getSymbol()

//--- vfd functions
void vfd_init() // Init vfd
{
    //--- Set pins as output
    pinMode(vfd_DI, OUTPUT);
    pinMode(vfd_CK, OUTPUT);
    pinMode(vfd_CS, OUTPUT);

    vfd_CS_low();
    vfd_tranmitByte(0xE0); // Command - number of digits set
    vfd_tranmitByte(0x0B); // Number of characters displayed on the display
    vfd_CS_high();

    vfd_CS_low();
    vfd_tranmitByte(0xE4); // Command - dimming set
    vfd_tranmitByte(0xFF); // Brightness
    vfd_CS_high();

    vfd_CS_low();
    vfd_tranmitByte(0xE8); // Command - display lights - normal mode
    vfd_CS_high();
} // void vfd_init()

void vfd_tranmitByte(unsigned int byte) // Transmit byte
{
    for(size_t it = 0; it < 8; it++)
    {
        vfd_CK_low();

        if((byte >> it) & 0x01)
            vfd_DI_high();
        else
            vfd_DI_low();

        vfd_CK_high();
    } // for(byte it = 0; it < 8; it++)
} // void vfd_tranmitByte(unsigned int byte)

void vfd_setSymbol(unsigned int symbol) // Set symbol (Only one, another reset)
{
    unsigned int symbol_position = 0;
    switch(symbol)
    {
        case read_sym_prev: // Prev
            symbol = vfd_sym_prev;
            symbol_position = vfd_symbol_1 + vfd_sym_prev;
            break;
        case read_sym_play: // Play
            symbol = vfd_sym_play;
            symbol_position = vfd_symbol_1 + vfd_sym_play;
            break;
        case read_sym_pause: // Pause
            symbol = vfd_sym_pause;
            symbol_position = vfd_symbol_1 + vfd_sym_pause;
            break;
        case read_sym_next: // Next
            symbol = vfd_sym_next;
            symbol_position = vfd_symbol_1 + vfd_sym_next;
            break;
    } // switch(symbol)

    vfd_resetSymbols(); // Reset all symbols

    vfd_CS_low(); // Set need symbol

    vfd_tranmitByte(symbol_position);
    vfd_tranmitByte(0x0E); // First bit - colons

    vfd_CS_high();
} // void vfd_setSymbol(unsigned int symbol)

void vfd_resetSymbols() // Reset all symbols
{    
    vfd_CS_low(); // Reset 4 use symbols

    vfd_tranmitByte(vfd_symbol_1); // Start with prev (position 7G)
    for(size_t it = 0; it < vfd_symb_max; it++)
        vfd_tranmitByte(0x00);

    vfd_CS_high();
} // void vfd_resetSymbols()

void vfd_sendChar(unsigned int place, wchar_t byte) // Send char
{
    vfd_CS_low();

    vfd_tranmitByte(place + vfd_place_1); // Place on VFD
    vfd_tranmitByte(byte);

    vfd_CS_high();
} // void vfd_sendChar(unsigned int place, wchar_t byte)

void vfd_sendString(std::wstring name) // Send wstring (12 symbols from vfd_place_1)
{
    vfd_CS_low();

    vfd_tranmitByte(vfd_place_1); // First place on VFD

    for(size_t it = 0; it < vfd_segm_max; it++)
    {
        if(it < name.length())
            vfd_tranmitByte(name[it]);
        else
            vfd_tranmitByte(' ');
    } // for(int it = 0; it < vfd_segm_max; it++)

    vfd_CS_high();
} // void vfd_sendString(std::wstring name)

void vfd_sendShiftName(std::wstring name, int shift) // Send shift name
{
    vfd_CS_low();

    vfd_tranmitByte(vfd_place_1); // First place on VFD

    for(size_t it = 0; it < vfd_segm_max; it++)
        vfd_tranmitByte(vfd_checkRussian((unsigned int)name[it + shift]));

    vfd_CS_high();
} // void vfd_sendShiftName(std::wstring name, int shift)

//--- vfd russian
unsigned int vfd_checkRussian(unsigned int byte) // Check for russian symbols
{
    switch(byte)
    {
        case 0x410: // А
        case 0x430: // а
            return 0x61;
        case 0x411: // Б
        case 0x431: // б
            return 0x01;
        case 0x412: // В
        case 0x432: // в
            return 0x02;
        case 0x413: // Г
        case 0x433: // г
            return 0x72;
        case 0x414: // Д
        case 0x434: // д
            return 0x80;
        case 0x415: // Е
        case 0x435: // е
            return 0x65;
        case 0x401: // Ё
        case 0x451: // ё
            return 0x65;
        case 0x416: // Ж
        case 0x436: // ж
            return 0x81;
        case 0x417: // З
        case 0x437: // з
            return 0x82;
        case 0x418: // И
        case 0x438: // и
            return 0x83;
        case 0x419: // Й
        case 0x439: // й
            return 0x84;
        case 0x41A: // К
        case 0x43A: // к
            return 0x85;
        case 0x41B: // Л
        case 0x43B: // л
            return 0x86;
        case 0x41C: // М
        case 0x43C: // м
            return 0x87;
        case 0x41D: // Н
        case 0x43D: // н
            return 0x88;
        case 0x41E: // О
        case 0x43E: // о
            return 0x6F;
        case 0x41F: // П
        case 0x43F: // п
            return 0x89;
        case 0x420: // Р
        case 0x440: // р
            return 0x70;
        case 0x421: // С
        case 0x441: // с
            return 0x8A;
        case 0x422: // Т
        case 0x442: // т
            return 0x8B;
        case 0x423: // У
        case 0x443: // у
            return 0x79;
        case 0x424: // Ф
        case 0x444: // ф
            return 0x8D;
        case 0x425: // Х
        case 0x445: // х
            return 0x78;
        case 0x426: // Ц
        case 0x446: // ц
            return 0x8E;
        case 0x427: // Ч
        case 0x447: // ч
            return 0x8F;
        case 0x428: // Ш
        case 0x448: // ш
            return 0x90;
        case 0x429: // Щ
        case 0x449: // щ
            return 0x91;
        case 0x42A: // Ъ
        case 0x44A: // ъ
            return 0x92;
        case 0x42B: // Ы
        case 0x44B: // ы
            return 0x93;
        case 0x42C: // Ь
        case 0x44C: // ь
            return 0x94;
        case 0x42D: // Э
        case 0x44D: // э
            return 0x95;
        case 0x42E: // Ю
        case 0x44E: // ю
            return 0x96;
        case 0x42F: // Я
        case 0x44F: // я
            return 0x97;
            
        default:
            return byte;
    } // switch(byte)
} // unsigned int vfd_checkRussian(unsigned int byte)

void vfg_sendRussian() // Send russian 'b' and 'v' in user memory
{
    vfd_CS_low();

    vfd_tranmitByte(0x41); // CGRAM - 0x01 in Font table - symbol 'b'
    vfd_tranmitByte(0x7C);
    vfd_tranmitByte(0x54);
    vfd_tranmitByte(0x54);
    vfd_tranmitByte(0x54);
    vfd_tranmitByte(0x24);

    vfd_tranmitByte(0x7C); // CGRAM - 0x02 in Font table - symbol 'v'
    vfd_tranmitByte(0x54);
    vfd_tranmitByte(0x54);
    vfd_tranmitByte(0x54);
    vfd_tranmitByte(0x28);

    vfd_CS_high();
} // void vfg_sendRussian()