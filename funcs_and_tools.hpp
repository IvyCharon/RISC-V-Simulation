namespace myspa
{
    enum Types{LUI,AUIPC,JAL,JALR,BEQ,BNE,BLT,BGE,BLTU,BGEU,LB,LH,LW,
    LBU,LHU,SB,SH,SW,ADDI,SLTI,SLTIU,XORI,ORI,ANDI,SLLI,SRLI,SRAI,
    ADD,SUB,SLL,SLT,SLTU,XOR,SRL,SRA,OR,AND};

    enum Overall_Type{I,S,B,U,J,R};

    #define END 0x0ff00513

    #define op_LUI 0b0110111 
    #define op_AUIPC 0b0010111
    #define op_JAL 0b1101111
    #define op_JALR 0b1100111

    #define op_B 0b1100011
    #define op_BEQ 0b000
    #define op_BNE 0b001
    #define op_BLT 0b100
    #define op_BGE 0b101
    #define op_BLTU 0b110
    #define op_BGEU 0b111

    #define op_L 0b0000011
    #define op_LB 0b000
    #define op_LH 0b001
    #define op_LW 0b010
    #define op_LBU 0b100
    #define op_LHU 0b101

    #define op_S 0b0100011
    #define op_SB 0b000
    #define op_SH 0b001
    #define op_SW 0b010

    #define op_I 0b0010011
    #define op_ADDI 0b000
    #define op_SLTI 0b010
    #define op_SLTIU 0b011
    #define op_XORI 0b100
    #define op_ORI 0b110
    #define op_ANDI 0b111
    #define op_SLLI 0b001
    #define op_SRI 0b101
    #define op_SRLI 0b0000000
    #define op_SRAI 0b0100000

    #define op_R 0b0110011
    #define op_ADDorSUB 0b000
    #define op_ADD 0b0000000
    #define op_SUB 0b0100000
    #define op_SLL 0b001
    #define op_SLT 0b010
    #define op_SLTU 0b011
    #define op_XOR 0b100
    #define op_SR 0b101
    #define op_SRL 0b0000000
    #define op_SRA 0b0100000
    #define op_OR 0b110
    #define op_AND 0b111

    int get_imm(int code, Overall_Type T)//R-type 没有立即数
    {
        int out = 0;
        int i31 = (code & 0x80000000) >> 31;
        switch (T)
        {
        case (Overall_Type)I:
            for(int i = 31;i >= 11;-- i)
            {
                if(i != 31) out <<= 1;
                out += i31;
            }
            out <<= 6;
            out += (code & 0x7e000000) >> 25;
            out <<= 4;
            out += (code & 0x1e00000) >> 21;
            out <<= 1;
            out += (code & 0x100000) >> 20;
            return out;
        case (Overall_Type)S:
            for(int i = 31;i >= 11;-- i)
            {
                if(i != 31) out <<= 1;
                out += i31;
            }
            out <<= 6;
            out += (code & 0x7e000000) >> 25;
            out <<= 4;
            out += (code & 0xf00) >> 8;
            out <<= 1;
            out += (code & 0x80) >> 7;
            return out;
        case (Overall_Type)B:
            for(int i = 31;i >= 12;-- i)
            {
                if(i != 31) out <<= 1;
                out += i31;
            }
            out <<= 1;
            out += (code & 0x80) >> 7;
            out <<= 6;
            out += (code & 0x7e000000) >> 25;
            out <<= 4;
            out += (code & 0xf00) >> 8;
            out <<= 1;
            return out;
        case (Overall_Type)U:
            out = (code & 0xfffff000);
            return out;
        case (Overall_Type)J:
            for(int i = 31;i >= 20;-- i)
            {
                if(i != 31) out <<= 1;
                out += i31;
            }
            out <<= 8;
            out += (code & 0xff000) >> 12;
            out <<= 1;
            out += (code & 0x100000) >> 20;
            out <<= 6;
            out += (code & 0x7e000000) >> 25;
            out <<= 4;
            out += (code & 0x1e00000) >> 21;
            out <<= 1;
            return out;
        case (Overall_Type) R:
            return 0;
        }
    }

    int hex_to_dec(char t)
    {
        if(t >= '0' && t <= '9')
            return t - '0';
        else 
            return t - 'A' + 10;
    }

}

