#include "memory.hpp"
#include <iostream>
#include <string.h>
#include <bitset>

using std::cin;
using std::cout;
using namespace myspa;

class program
{
    Memory_simu memo;
    int Reg[32];
    int now_code;
    int pc;

    struct write_Reg
    {
        bool isChanged = 0;
        int number = 0;
        int data = 0;
        int pos = 0;
    }wReg;

    struct read_Reg
    {
        int data1, data2;
        int imm;
        int shamt;
    }rReg;

    struct Instruction
    {
        Types type;
        int rs1 = 0, rs2 = 0, rd = 0;

        bool pc_changed()
        {
            return (type == AUIPC) || (type == BEQ) || (type == BNE) ||
            (type == BLT) || (type == BGE) || (type == BLTU) || (type == BGEU) ||
            (type == JAL) || (type == JALR);
        }
    }instr;

    void get_r(Overall_Type T)
    {
        instr.rs1 = instr.rs2 = instr.rd = 0;
        switch (T)
        {
        case R:
            instr.rs2 = (now_code & 0x1f00000) >> 20;
            instr.rs1 = (now_code & 0xf8000) >> 15;
            instr.rd = (now_code & 0xf80) >> 7;
            break;
        case I:
            instr.rs1 = (now_code & 0xf8000) >> 15;
            instr.rd = (now_code & 0xf80) >> 7;
            break;
        case S:
        case B:
            instr.rs2 = (now_code & 0x1f00000) >> 20;
            instr.rs1 = (now_code & 0xf8000) >> 15;
            break;
        case U:
        case J:
            instr.rd = (now_code & 0xf80) >> 7;
            break;
        }
        rReg.data1 = Reg[instr.rs1]; rReg.data2 = Reg[instr.rs2];
        wReg.number = instr.rd;
    }

public:
    program() 
    {
        memset(Reg, 0, sizeof(Reg));
        now_code = 0;
        pc = 0;
    }
    void IF()//根据PC寄存器访问内存得到指令
    {
        now_code = memo.get_instruction(pc);
    }
    void ID()//根据指令类型找到作为右值的寄存器并读值、并且解析立即数的值
    {
        int op = now_code & 0b1111111;
        switch (op)
        {
        case op_LUI:
            instr.type = (Types)LUI;
            rReg.imm = get_imm(now_code, (Overall_Type)U);
            get_r((Overall_Type)U);
            break;
        case op_AUIPC:
            instr.type = (Types)AUIPC;
            rReg.imm = get_imm(now_code, (Overall_Type)U);
            get_r((Overall_Type)U);
            break;
        case op_JAL:
            instr.type = (Types)JAL;
            rReg.imm = get_imm(now_code, (Overall_Type)J);
            get_r((Overall_Type)J);
            break;
        case op_JALR:
            instr.type = (Types)JALR;
            rReg.imm = get_imm(now_code, (Overall_Type)I);
            get_r((Overall_Type)I);
            break;
        case op_B:
            {
                int jud = (now_code & 0x7000) >> 12;
                switch (jud)
                {
                case op_BEQ:
                    instr.type = (Types)BEQ; break;
                case op_BNE:
                    instr.type = (Types)BNE; break;
                case op_BLT:
                    instr.type = (Types)BLT; break;
                case op_BGE:
                    instr.type = (Types)BGE; break;
                case op_BLTU:
                    instr.type = (Types)BLTU; break;
                case op_BGEU:
                    instr.type = (Types)BGEU; break;
                }
            }
            rReg.imm = get_imm(now_code, (Overall_Type)B);
            get_r((Overall_Type)B);
            break;
        case op_L:
            {
                int jud = (now_code & 0x7000) >> 12;
                switch (jud)
                {
                case op_LB:
                    instr.type = (Types)LB; break;
                case op_LH:
                    instr.type = (Types)LH; break;
                case op_LW:
                    instr.type = (Types)LW; break;
                case op_LBU:
                    instr.type = (Types)LBU; break;
                case op_LHU:
                    instr.type = (Types)LHU; break;
                }
            }
            rReg.imm = get_imm(now_code, (Overall_Type)I);
            get_r((Overall_Type)I);
            break; 
        case op_S:
            {
                int jud = (now_code & 0x7000) >> 12;
                switch (jud)
                {
                case op_SB:
                    instr.type = (Types)SB; break;
                case op_SH:
                    instr.type = (Types)SH; break;
                case op_SW:
                    instr.type = (Types)SW; break;
                }
            }
            rReg.imm = get_imm(now_code, (Overall_Type)S);
            get_r((Overall_Type)S);
            break;
        case op_I:
            {
                rReg.imm = get_imm(now_code, (Overall_Type)I);
                get_r((Overall_Type)I);
                int jud = (now_code & 0x7000) >> 12;
                switch (jud)
                {
                case op_ADDI:
                    instr.type = (Types)ADDI; break;
                case op_SLTI:
                    instr.type = (Types)SLTI; break;
                case op_SLTIU:
                    instr.type = (Types)SLTIU; break;
                case op_XORI:
                    instr.type = (Types)XORI; break;
                case op_ANDI:
                    instr.type = (Types)ANDI; break;
                case op_SLLI:
                    instr.type = (Types)SLLI;
                    rReg.shamt = (now_code & 0x1f00000) >> 20;
                    break;
                case op_SRI:
                    {
                        int jud2 = (now_code & 0xfe000000) >> 25;
                        switch (jud2)
                        {
                        case op_SRLI:
                            instr.type = (Types)SRLI;
                            rReg.shamt = (now_code & 0x1f00000) >> 20;
                            break;
                        case op_SRAI:
                            instr.type = (Types)SRAI;
                            rReg.shamt = (now_code & 0x1f00000) >> 20;
                            break;
                        }
                    }
                    break;
                case op_ORI:
                    instr.type = (Types)ORI; 
                    rReg.imm = get_imm(now_code, (Overall_Type)R);
                    get_r((Overall_Type)R);
                    break;
                }
            }
            break;
        case op_R:
            {
                int jud = (now_code & 0x7000) >> 12;
                switch (jud)
                {
                case op_ADDorSUB:
                    {
                        int jud2 = (now_code & 0xfe000000) >> 25;
                        switch (jud2)
                        {
                        case op_ADD:
                            instr.type = (Types)ADD; break;
                        case op_SUB:
                            instr.type = (Types)SUB; break;
                        }
                    }
                    break;
                case op_SLL:
                    instr.type = (Types)SLL; break;
                case op_SLT:
                    instr.type = (Types)SLT; break;
                case op_SLTU:
                    instr.type = (Types)SLTU; break;
                case op_XOR:
                    instr.type = (Types)XOR; break;
                case op_SR:
                    {
                        int jud2 = (now_code & 0xfe000000) >> 25;
                        switch (jud2)
                        {
                        case op_SRL:
                            instr.type = (Types)SRL; break;
                        case op_SRA:
                            instr.type = (Types)SRA; break;
                        }
                    }
                    break;
                case op_OR:
                    instr.type = (Types)OR; break;
                case op_AND:
                    instr.type = (Types)AND; break;
                }   
            }
            get_r((Overall_Type)R);
            break;
        }
    }
    void EXE()//对解析好的值按指令要求进行计算
    {
        switch (instr.type)
        {
        case LUI:
            wReg.isChanged = 1;
            wReg.data = rReg.imm;
            break;
        case AUIPC:
            pc += rReg.imm;
            wReg.isChanged = 1;
            wReg.data = pc;
            break;
        case JAL:
            wReg.isChanged = 1;
            wReg.data = pc + 4;
            pc += rReg.imm;
            break;
        case JALR:
            wReg.isChanged = 1;
            wReg.data = pc + 4;
            pc = (rReg.data1 + rReg.imm) & 0xfffffffe;
            break;
        case BEQ:
            if(rReg.data1 == rReg.data2)
                pc += rReg.imm;
            else pc += 4;
            break;
        case BNE:
            if(rReg.data1 != rReg.data2)
                pc += rReg.imm;
            else pc += 4;
            break;
        case BLT:
            if(rReg.data1 < rReg.data2)
                pc += rReg.imm;
            else pc += 4;
            break;
        case BGE:
            if(rReg.data1 >= rReg.data2)
                pc += rReg.imm;
            else pc += 4;
            break;
        case BLTU:
            if(((unsigned int)rReg.data1) < ((unsigned int)rReg.data2))
                pc += rReg.imm;
            else pc += 4;
            break;
        case BGEU:
            if(((unsigned int)rReg.data1) >= ((unsigned int)rReg.data2))
                pc += rReg.imm;
            else pc += 4;
            break;
        case LB:
        case LH:
        case LW:
        case LBU:
        case LHU:
            wReg.isChanged = 1;
            wReg.pos = rReg.data1 + rReg.imm;
            break;
        case SB:
            wReg.data = (rReg.data2 & 0xff);
            wReg.pos = rReg.data1 + rReg.imm;
            break;
        case SH:
            wReg.data = (rReg.data2 & 0xffff);
            wReg.pos = rReg.data1 + rReg.imm;
            break;
        case SW:
            wReg.data = (rReg.data2 & 0xffffffff);
            wReg.pos = rReg.data1 + rReg.imm;
            break;
        case ADDI:
            wReg.isChanged = 1;
            wReg.data = rReg.data1 + rReg.imm;
            break;
        case SLTI:
            wReg.isChanged = 1;
            wReg.data = (int)(rReg.data1 < rReg.imm);
            break;
        case SLTIU:
            wReg.isChanged = 1;
            wReg.data = (int)(((unsigned int)rReg.data1) < ((unsigned int)rReg.imm));
            break;
        case XORI:
            wReg.isChanged = 1;
            wReg.data = rReg.data1 ^ rReg.imm;
            break;
        case ORI:
            wReg.isChanged = 1;
            wReg.data = rReg.data1 | rReg.imm;
            break;
        case ANDI:
            wReg.isChanged = 1;
            wReg.data = rReg.data1 & rReg.imm;
            break;
        case SLLI:
            wReg.isChanged = 1;
            wReg.data = rReg.data1 << rReg.shamt;
            break;
        case SRLI:
            wReg.isChanged = 1;
            wReg.data = ((unsigned int)rReg.data1) >> rReg.shamt;
            break;
        case SRAI:
            wReg.isChanged = 1;
            wReg.data = rReg.data1 >> rReg.shamt;
            break;
        case ADD:
            wReg.isChanged = 1;
            wReg.data = rReg.data1 + rReg.data2;
            break;
        case SUB:
            wReg.isChanged = 1;
            wReg.data = rReg.data1 - rReg.data2;
            break;
        case SLL:
            wReg.isChanged = 1;
            wReg.data = rReg.data1 << (rReg.data2 & 0b11111);
            break;
        case SLT:
            wReg.isChanged = 1;
            wReg.data = (int)(rReg.data1 < rReg.data2);
            break;
        case SLTU:
            wReg.isChanged = 1;
            wReg.data = (int)(((unsigned int)rReg.data1) < ((unsigned int)rReg.data2));
            break;
        case XOR:
            wReg.isChanged = 1;
            wReg.data = rReg.data1 ^ rReg.data2;
            break;
        case SRL:
            wReg.isChanged = 1;
            wReg.data = ((unsigned int)rReg.data1) >> (((unsigned int)rReg.data2) && 0b11111);
            break;
        case SRA:
            wReg.isChanged = 1;
            wReg.data = rReg.data1 >> (rReg.data2 & 0b11111);
            break;
        case OR:
            wReg.isChanged = 1;
            wReg.data = rReg.data1 | rReg.data2;
            break;
        case AND:
            wReg.isChanged = 1;
            wReg.data = rReg.data1 & rReg.data2;
            break;
        }
        if(! instr.pc_changed()) pc += 4;
    }
    void MEM()//根据计算出的地址从内存读出数据值，或将已准备好的数据值写入内存
    {
        Types T = instr.type;
        switch (T)
        {
        case SB:
            memo.StoreByte(wReg.pos, wReg.data);
            break;
        case SH:
            memo.StoreHalfWord(wReg.pos, wReg.data);
            break;
        case SW:
            memo.StoreWord(wReg.pos, wReg.data);
            break;
        case LB:
            wReg.data = (int8_t)memo.LoadByte(wReg.pos);
            break;
        case LH:
            wReg.data = (int16_t)memo.LoadHalfWord(wReg.pos);
            break;
        case LW:
            wReg.data = memo.LoadWord(wReg.pos);
            break;
        case LBU:
            wReg.data = (uint8_t)memo.LoadByte(wReg.pos);
            break;
        case LHU:
            wReg.data = (uint16_t)memo.LoadHalfWord(wReg.pos);
            break;
        default:
            break;
        }
    }
    void WB()//完成对于左值寄存器的赋值，即写回寄存器
    {
        if(! wReg.isChanged) return;
        if(wReg.number != 0)
            Reg[wReg.number] = wReg.data;
        wReg.isChanged = 0;
    }

    int run()
    {
        while (1)
        {
            IF();
            if(now_code == END)
                return ((unsigned int)Reg[10]) & 255u;
            ID();
            EXE();
            MEM();
            WB();
        }
    }
};
