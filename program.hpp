//with 5 stage pipnlining
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
    int pc;

    int cycle;

    struct branch_pre_cache
    {
        bool a[2] = {1,1};
        int pos_c = 0;

        void push(bool x)
        {
            if(a[0] == 1 && a[1] == 1)
            {
                if(x == 0) a[1] = 0;
                return;
            }
            if(a[0] == 1 && a[1] == 0)
            {
                if(x == 1) a[1] = 1;
                else a[0] = 0;
                return;
            }
            if(a[0] == 0 && a[1] == 1)
            {
                if(x == 1) a[0] = 1;
                else a[1] = 0;
                return;
            }
            if(a[0] == 0 && a[1] == 0)
            {
                if(x == 1) a[1] = 1;
                return;
            }
        }

        bool jud()
        {
            return a[0];
        }
    }bpc;

    struct ID_Reg
    {
        int now_code = 0;
        int pc = 0;
    }dReg;

    struct EXE_Reg
    {
        int imm = 0;
        int data1 = 0, data2 = 0;
        int shamt = 0;
        Types type = (Types)NOType;
        int rs1 = 0, rs2 = 0, rd = 0;
        int pc = 0;
    }eReg;

    struct MEM_Reg
    {
        int pos = 0;
        int data = 0;
        int rs1 = 0, rs2 = 0, rd = 0;
        bool isChanged = 0;
        Types type = (Types)NOType;
    }mReg;

    struct WB_Reg
    {
        bool isChanged = 0;
        int rs1 = 0, rs2 = 0, rd = 0;
        int data = 0;
        int pos = 0;
        Types type = (Types)NOType;
    }wReg;

    bool pc_changed(Types type)
    {
        return (type == AUIPC) || (type == JAL) || (type == JALR);
    }

    bool branch(Types t)
    {
        return (t == BEQ) || (t == BNE) || (t == BGEU) || (t == BLT) || (t == BLTU) || (t == BGE);
    }

    bool get_in_mem(Types t)
    {
        return (t == SB) || (t == SH) || (t == SW) || (t == LB) || (t == LH) || (t == LW) || (t == LBU) || (t == LHU);
    }

    bool mem_changed(Types T)
    {
        return (T == SB) || (T == SW) || (T == SH);
    }

    void get_r(Overall_Type T)
    {
        eReg.rs1 = eReg.rs2 = eReg.rd = 0;
        switch (T)
        {
        case R:
            eReg.rs2 = (dReg.now_code & 0x1f00000) >> 20;
            eReg.rs1 = (dReg.now_code & 0xf8000) >> 15;
            eReg.rd = (dReg.now_code & 0xf80) >> 7;
            break;
        case I:
            eReg.rs1 = (dReg.now_code & 0xf8000) >> 15;
            eReg.rd = (dReg.now_code & 0xf80) >> 7;
            break;
        case S:
        case B:
            eReg.rs2 = (dReg.now_code & 0x1f00000) >> 20;
            eReg.rs1 = (dReg.now_code & 0xf8000) >> 15;
            break;
        case U:
        case J:
            eReg.rd = (dReg.now_code & 0xf80) >> 7;
            break;
        }
    }

public:
    program() 
    {
        memset(Reg, 0, sizeof(Reg));
        pc = 0;
        cycle = 0;
    }
    void IF()//根据PC寄存器访问内存得到指令
    {
        if(cycle)
        {
            dReg.now_code = -1;
            cycle --;
            return;
        }
        if(pc_changed(eReg.type))
        {
            dReg.now_code = -1;
            return;
        }
        if(mem_changed(mReg.type) || mem_changed(eReg.type))
        {
            dReg.now_code = -1;
            return;
        }
        if(branch(eReg.type))
        {
            if(bpc.jud())//跳转
            {
                bpc.pos_c = pc - 4 + eReg.imm;
                dReg.now_code = memo.get_instruction(bpc.pos_c);
                dReg.pc = bpc.pos_c;
                pc += 4;
            }
            else//不跳转
            {
                bpc.pos_c = pc;
                dReg.now_code = memo.get_instruction(pc);
                dReg.pc = pc;
                pc += 4;
            }
            if(dReg.now_code == END)
            {
                bpc.pos_c = -100;
                dReg.now_code = -1;
            }
            return;
        }
        dReg.now_code = memo.get_instruction(pc);
        dReg.pc = pc;
        pc += 4;
    }
    void ID()//根据指令类型找到作为右值的寄存器并读值、并且解析立即数的值
    {
        if(dReg.now_code == -1)
        {
            eReg.type = NOType;
            return;
        }
        eReg.pc = dReg.pc;
        int op = dReg.now_code & 0b1111111;
        switch (op)
        {
        case op_LUI:
            eReg.type = (Types)LUI;
            eReg.imm = get_imm(dReg.now_code, (Overall_Type)U);
            get_r((Overall_Type)U);
            break;
        case op_AUIPC:
            eReg.type = (Types)AUIPC;
            eReg.imm = get_imm(dReg.now_code, (Overall_Type)U);
            get_r((Overall_Type)U);
            break;
        case op_JAL:
            eReg.type = (Types)JAL;
            eReg.imm = get_imm(dReg.now_code, (Overall_Type)J);
            get_r((Overall_Type)J);
            break;
        case op_JALR:
            eReg.type = (Types)JALR;
            eReg.imm = get_imm(dReg.now_code, (Overall_Type)I);
            get_r((Overall_Type)I);
            break;
        case op_B:
            {
                int jud = (dReg.now_code & 0x7000) >> 12;
                switch (jud)
                {
                case op_BEQ:
                    eReg.type = (Types)BEQ; break;
                case op_BNE:
                    eReg.type = (Types)BNE; break;
                case op_BLT:
                    eReg.type = (Types)BLT; break;
                case op_BGE:
                    eReg.type = (Types)BGE; break;
                case op_BLTU:
                    eReg.type = (Types)BLTU; break;
                case op_BGEU:
                    eReg.type = (Types)BGEU; break;
                }
            }
            eReg.imm = get_imm(dReg.now_code, (Overall_Type)B);
            get_r((Overall_Type)B);
            break;
        case op_L:
            {
                int jud = (dReg.now_code & 0x7000) >> 12;
                switch (jud)
                {
                case op_LB:
                    eReg.type = (Types)LB; break;
                case op_LH:
                    eReg.type = (Types)LH; break;
                case op_LW:
                    eReg.type = (Types)LW; break;
                case op_LBU:
                    eReg.type = (Types)LBU; break;
                case op_LHU:
                    eReg.type = (Types)LHU; break;
                }
            }
            eReg.imm = get_imm(dReg.now_code, (Overall_Type)I);
            get_r((Overall_Type)I);
            break; 
        case op_S:
            {
                int jud = (dReg.now_code & 0x7000) >> 12;
                switch (jud)
                {
                case op_SB:
                    eReg.type = (Types)SB; break;
                case op_SH:
                    eReg.type = (Types)SH; break;
                case op_SW:
                    eReg.type = (Types)SW; break;
                }
            }
            eReg.imm = get_imm(dReg.now_code, (Overall_Type)S);
            get_r((Overall_Type)S);
            break;
        case op_I:
            {
                eReg.imm = get_imm(dReg.now_code, (Overall_Type)I);
                get_r((Overall_Type)I);
                int jud = (dReg.now_code & 0x7000) >> 12;
                switch (jud)
                {
                case op_ADDI:
                    eReg.type = (Types)ADDI; break;
                case op_SLTI:
                    eReg.type = (Types)SLTI; break;
                case op_SLTIU:
                    eReg.type = (Types)SLTIU; break;
                case op_XORI:
                    eReg.type = (Types)XORI; break;
                case op_ANDI:
                    eReg.type = (Types)ANDI; break;
                case op_SLLI:
                    eReg.type = (Types)SLLI;
                    eReg.shamt = (dReg.now_code & 0x1f00000) >> 20;
                    break;
                case op_SRI:
                    {
                        int jud2 = (dReg.now_code & 0xfe000000) >> 25;
                        switch (jud2)
                        {
                        case op_SRLI:
                            eReg.type = (Types)SRLI;
                            eReg.shamt = (dReg.now_code & 0x1f00000) >> 20;
                            break;
                        case op_SRAI:
                            eReg.type = (Types)SRAI;
                            eReg.shamt = (dReg.now_code & 0x1f00000) >> 20;
                            break;
                        }
                    }
                    break;
                case op_ORI:
                    eReg.type = (Types)ORI; 
                    eReg.imm = get_imm(dReg.now_code, (Overall_Type)R);
                    get_r((Overall_Type)R);
                    break;
                }
            }
            break;
        case op_R:
            {
                int jud = (dReg.now_code & 0x7000) >> 12;
                switch (jud)
                {
                case op_ADDorSUB:
                    {
                        int jud2 = (dReg.now_code & 0xfe000000) >> 25;
                        switch (jud2)
                        {
                        case op_ADD:
                            eReg.type = (Types)ADD; break;
                        case op_SUB:
                            eReg.type = (Types)SUB; break;
                        }
                    }
                    break;
                case op_SLL:
                    eReg.type = (Types)SLL; break;
                case op_SLT:
                    eReg.type = (Types)SLT; break;
                case op_SLTU:
                    eReg.type = (Types)SLTU; break;
                case op_XOR:
                    eReg.type = (Types)XOR; break;
                case op_SR:
                    {
                        int jud2 = (dReg.now_code & 0xfe000000) >> 25;
                        switch (jud2)
                        {
                        case op_SRL:
                            eReg.type = (Types)SRL; break;
                        case op_SRA:
                            eReg.type = (Types)SRA; break;
                        }
                    }
                    break;
                case op_OR:
                    eReg.type = (Types)OR; break;
                case op_AND:
                    eReg.type = (Types)AND; break;
                }   
            }
            get_r((Overall_Type)R);
            break;
        }
        if(mReg.type != NOType && mReg.isChanged && mReg.rd != 0)
        {
            switch (eReg.type)
            {
            case LUI:
            case AUIPC:
            case JAL:
                break;
            case JALR:
            case LB:
            case LH:
            case LW:
            case LBU:
            case LHU:
            case ADDI:
            case SLTI:
            case SLTIU:
            case XORI:
            case ORI:
            case ANDI:
            case SLLI:
            case SRLI:
            case SRAI:
                if(eReg.rs1 == mReg.rd)
                {
                    eReg.type = NOType;
                    pc -= 4;
                }
                break;
            case BEQ:
            case BNE:
            case BLT:
            case BGE:
            case BLTU:
            case BGEU:
            case SB:
            case SH:
            case SW:
            case ADD:
            case SUB:
            case SLL:
            case SLT:
            case SLTU:
            case XOR:
            case SRL:
            case SRA:
            case OR:
            case AND:
                if(eReg.rs1 == mReg.rd || eReg.rs2 == mReg.rd)
                {
                    eReg.type = NOType;
                    pc -= 4;
                }
                break;
            default:
                break;
            }
        }
        if(get_in_mem(eReg.type))
        {
            cycle = 3;
            return;
        }
    }
    void EXE()//对解析好的值按指令要求进行计算
    {
        mReg.type = eReg.type;
        if(eReg.type == NOType) return;
        mReg.rd = eReg.rd; mReg.rs1 = eReg.rs1; mReg.rs2 = eReg.rs2;
        eReg.data1 = Reg[eReg.rs1]; eReg.data2 = Reg[eReg.rs2];
        switch (eReg.type)
        {
        case LUI:
            mReg.isChanged = 1;
            mReg.data = eReg.imm;
            break;
        case AUIPC:
            pc -= 4;
            pc += eReg.imm;
            mReg.isChanged = 1;
            mReg.data = pc;
            break;
        case JAL:
            pc -= 4;
            mReg.isChanged = 1;
            mReg.data = pc + 4;
            pc += eReg.imm;
            break;
        case JALR:
            pc -= 4;
            mReg.isChanged = 1;
            mReg.data = pc + 4;
            pc = (eReg.data1 + eReg.imm) & 0xfffffffe;
            break;
        case BEQ:
            if(eReg.data1 == eReg.data2)
            {
                pc -= 4;
                pc += eReg.imm;
                bpc.push(1);
            }   
            else
                bpc.push(0);
            break;
        case BNE:
            if(eReg.data1 != eReg.data2)
            {
                pc -= 4;
                pc += eReg.imm;
                bpc.push(1);
            }   
            else
                bpc.push(0);
            break;
        case BLT:
            if(eReg.data1 < eReg.data2)
            {
                pc -= 4;
                pc += eReg.imm;
                bpc.push(1);
            }   
            else
                bpc.push(0);
            break;
        case BGE:
            if(eReg.data1 >= eReg.data2)
            {
                pc -= 4;
                pc += eReg.imm;
                bpc.push(1);
            }   
            else
                bpc.push(0);
            break;
        case BLTU:
            if(((unsigned int)eReg.data1) < ((unsigned int)eReg.data2))
            {
                pc -= 4;
                pc += eReg.imm;
                bpc.push(1);
            }   
            else
                bpc.push(0);
            break;
        case BGEU:
            if(((unsigned int)eReg.data1) >= ((unsigned int)eReg.data2))
            {
                pc -= 4;
                pc += eReg.imm;
                bpc.push(1);
            }   
            else
                bpc.push(0);
            break;
        case LB:
        case LH:
        case LW:
        case LBU:
        case LHU:
            mReg.pos = eReg.data1 + eReg.imm;
            break;
        case SB:
            mReg.data = (eReg.data2 & 0xff);
            mReg.pos = eReg.data1 + eReg.imm;
            break;
        case SH:
            mReg.data = (eReg.data2 & 0xffff);
            mReg.pos = eReg.data1 + eReg.imm;
            break;
        case SW:
            mReg.data = (eReg.data2 & 0xffffffff);
            mReg.pos = eReg.data1 + eReg.imm;
            break;
        case ADDI:
            mReg.isChanged = 1;
            mReg.data = eReg.data1 + eReg.imm;
            break;
        case SLTI:
            mReg.isChanged = 1;
            mReg.data = (int)(eReg.data1 < eReg.imm);
            break;
        case SLTIU:
            mReg.isChanged = 1;
            mReg.data = (int)(((unsigned int)eReg.data1) < ((unsigned int)eReg.imm));
            break;
        case XORI:
            mReg.isChanged = 1;
            mReg.data = eReg.data1 ^ eReg.imm;
            break;
        case ORI:
            mReg.isChanged = 1;
            mReg.data = eReg.data1 | eReg.imm;
            break;
        case ANDI:
            mReg.isChanged = 1;
            mReg.data = eReg.data1 & eReg.imm;
            break;
        case SLLI:
            mReg.isChanged = 1;
            mReg.data = eReg.data1 << eReg.shamt;
            break;
        case SRLI:
            mReg.isChanged = 1;
            mReg.data = ((unsigned int)eReg.data1) >> eReg.shamt;
            break;
        case SRAI:
            mReg.isChanged = 1;
            mReg.data = eReg.data1 >> eReg.shamt;
            break;
        case ADD:
            mReg.isChanged = 1;
            mReg.data = eReg.data1 + eReg.data2;
            break;
        case SUB:
            mReg.isChanged = 1;
            mReg.data = eReg.data1 - eReg.data2;
            break;
        case SLL:
            mReg.isChanged = 1;
            mReg.data = eReg.data1 << (eReg.data2 & 0b11111);
            break;
        case SLT:
            mReg.isChanged = 1;
            mReg.data = (int)(eReg.data1 < eReg.data2);
            break;
        case SLTU:
            mReg.isChanged = 1;
            mReg.data = (int)(((unsigned int)eReg.data1) < ((unsigned int)eReg.data2));
            break;
        case XOR:
            mReg.isChanged = 1;
            mReg.data = eReg.data1 ^ eReg.data2;
            break;
        case SRL:
            mReg.isChanged = 1;
            mReg.data = ((unsigned int)eReg.data1) >> (((unsigned int)eReg.data2) && 0b11111);
            break;
        case SRA:
            mReg.isChanged = 1;
            mReg.data = eReg.data1 >> (eReg.data2 & 0b11111);
            break;
        case OR:
            mReg.isChanged = 1;
            mReg.data = eReg.data1 | eReg.data2;
            break;
        case AND:
            mReg.isChanged = 1;
            mReg.data = eReg.data1 & eReg.data2;
            break;
        }
        if(branch(eReg.type))
        {
            if(pc - 4 == bpc.pos_c) return;
            else
            {
                dReg.now_code = memo.get_instruction(pc - 4);
                dReg.pc = pc - 4;
            }
        }
    }
    void MEM()//根据计算出的地址从内存读出数据值，或将已准备好的数据值写入内存
    {
        wReg.type = mReg.type;
        if(mReg.type == NOType) return;
        wReg.rd = mReg.rd; wReg.rs1 = mReg.rs1; wReg.rs2 = mReg.rs2;
        wReg.isChanged = mReg.isChanged; wReg.data = mReg.data;
        Types T = mReg.type;
        switch (T)
        {
        case SB:
            memo.StoreByte(mReg.pos, mReg.data);
            break;
        case SH:
            memo.StoreHalfWord(mReg.pos, mReg.data);
            break;
        case SW:
            memo.StoreWord(mReg.pos, mReg.data);
            break;
        case LB:
            wReg.isChanged = 1;
            wReg.data = (int8_t)memo.LoadByte(mReg.pos);
            break;
        case LH:
            wReg.isChanged = 1;
            wReg.data = (int16_t)memo.LoadHalfWord(mReg.pos);
            break;
        case LW:
            wReg.isChanged = 1;
            wReg.data = memo.LoadWord(mReg.pos);
            break;
        case LBU:
            wReg.isChanged = 1;
            wReg.data = (uint8_t)memo.LoadByte(mReg.pos);
            break;
        case LHU:
            wReg.isChanged = 1;
            wReg.data = (uint16_t)memo.LoadHalfWord(mReg.pos);
            break;
        default:
            break;
        }
    }
    void WB()//完成对于左值寄存器的赋值，即写回寄存器
    {
        if(! wReg.isChanged) return;
        if(wReg.type == NOType) return;
        if(wReg.rd != 0)
            Reg[wReg.rd] = wReg.data;
        wReg.isChanged = 0;
    }

    int FSPrun()
    {
        while(1)
        {
            WB();
            MEM();
            EXE();
            ID();
            IF();
            if(dReg.now_code == END)
                break;
        }
        WB();
        MEM();
        EXE();
        WB();
        MEM();
        WB();
        return ((unsigned int)Reg[10]) & 255u;
    }
};
