#include <iostream>
#include <stdio.h>
#include <fstream>
#include <string.h>
#include "funcs_and_tools.hpp"

using std::cin;
using std::cout;
using namespace myspa;

class Memory_simu
{
    int mem[0x20000];

public:
    Memory_simu()
    {
        memset(mem, 0, sizeof(mem));
        initialize_memory();
    }

    void initialize_memory()
    {
        char tmp; int now_pos = 0;
        while(cin>>tmp)
        {
            if(tmp == '@')
            {
                now_pos = 0;
                for(int i = 0;i < 8;++ i)
                {
                    cin>>tmp;
                    now_pos <<= 4;
                    now_pos += hex_to_dec(tmp);
                }
                //cout<<now_pos<<"\n";
            }
            else
            {
                int k = hex_to_dec(tmp);
                cin>>tmp;
                k <<= 4;
                k += hex_to_dec(tmp);
                mem[now_pos ++] = k;
                //cout<<now_pos - 1<<" "<<mem[now_pos - 1]<<"\n";
            }
        }
    }

    int get_instruction(int pos)
    {
        return mem[pos] + (mem[pos + 1] << 8) + (mem[pos + 2] << 16) + (mem[pos + 3] << 24);
    }

    void StoreByte(int pos, int val)
    {
        mem[pos] = val;
    }

    void StoreHalfWord(int pos, int val)//存进2个里
    {
        int tmp1 = (val & 0xff00) >> 8;
        int tmp2 = (val & 0xff);
        mem[pos] = tmp2; mem[pos + 1] = tmp1;
    }

    void StoreWord(int pos, int val)//存进4个里
    {
        int tmp1 = (val & 0xff000000) >> 24;
        int tmp2 = (val & 0xff0000) >> 16;
        int tmp3 = (val & 0xff00) >> 8;
        int tmp4 = (val & 0xff);
        mem[pos] = tmp4; mem[pos + 1] = tmp3; mem[pos + 2] = tmp2; mem[pos + 3] = tmp1;
    }

    int LoadByte(int pos)
    {
        return mem[pos];
    }

    int LoadHalfWord(int pos)
    {
        return mem[pos] + (mem[pos + 1] << 8);
    }

    int LoadWord(int pos)
    {
        int out = mem[pos] + (mem[pos + 1] << 8) + (mem[pos + 2] << 16) + (mem[pos +3] << 24);
        return out;
    }
};