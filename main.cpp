#include<iostream>
#include<string>
#include<vector>
#include<bitset>
#include<fstream>

using namespace std;

#define ADDU 1
#define SUBU 3
#define AND 4
#define OR  5
#define NOR 7

// Memory size.
// In reality, the memory size should be 2^32, but for this lab and space reasons,
// we keep it as this large number, but the memory is still 32-bit addressable.
#define MemSize 65536

bitset<32> signExtend(bitset<16> Imm){
    int j;
    bitset<32> temp;
    if(Imm[15] == 0){
    for (j = 0; j < 16; j++){
        temp[j] = Imm[j];
        temp[j+16] = 0;
    }
    }else {
        for (j = 0; j < 16; j++){
            temp[j] = Imm[j];
            temp[j+16] = 1;
        }
    }
    return temp;
}

class RF
{
  public:
    bitset<32> Data1, Data2;
    RF()
    {
      Registers.resize(32);
      Registers[0] = bitset<32> (0);
    }

    void ReadWrite(bitset<5> RdReg1, bitset<5> RdReg2, bitset<5> WrtReg, bitset<32> WrtData, bitset<1> WrtEnable)
    {
      // TODO: implement!
        Data1 = Registers[RdReg1.to_ulong()];
        Data2 = Registers[RdReg2.to_ulong()];
        if(WrtEnable.to_ulong() == 1) {
            Registers[WrtReg.to_ulong()] = WrtData;
        }
    }

    void OutputRF()
    {
      ofstream rfout;
      rfout.open("RFresult.txt",std::ios_base::app);
      if (rfout.is_open())
      {
        rfout<<"A state of RF:"<<endl;
        for (int j = 0; j<32; j++)
        {
          rfout << Registers[j] << endl;
        }
      }
      else cout<<"cannot open file";
      rfout.close();

    }
  private:
    vector<bitset<32> >Registers;
};


class ALU
{
  public:
    bitset<32> ALUresult;
    bitset<32> ALUOperation (bitset<3> ALUOP, bitset<32> oprand1, bitset<32> oprand2)
    {
      // TODO: implement!
        if(ALUOP.to_ulong() == ADDU)  {      //addition
            ALUresult = oprand1.to_ulong() + oprand2.to_ulong();
        }
        else if(ALUOP.to_ulong() == SUBU) {    //subtraction
        ALUresult = oprand1.to_ulong() - oprand2.to_ulong();
        }

        else if(ALUOP.to_ulong() == AND){      //and
        ALUresult = oprand1 & oprand2;
        }
        else if (ALUOP.to_ulong() == OR)   {    //or
        ALUresult = oprand1|oprand2;
        }
        else if (ALUOP.to_ulong() == NOR)  {    //nor
        ALUresult = ~(oprand1|oprand2);
        }
      return ALUresult;
    }
    
};


class INSMem
{
  public:
    bitset<32> Instruction;
    INSMem()
    {
      IMem.resize(MemSize);
      ifstream imem;
      string line;
      int i=0;
      imem.open("imem.txt");
      if (imem.is_open())
      {
        while (getline(imem,line))
        {
          IMem[i] = bitset<8>(line);
          i++;
        }

      }
      else cout<<"cannot open file";
      imem.close();
    }

    bitset<32> ReadMemory (bitset<32> ReadAddress)
    {
      // TODO: implement!
      // (Read the byte at the ReadAddress and the following three byte).
        Instruction = 0;
        for(int i = 0; i < 4; i++) {
            Instruction <<= 8;
            Instruction = Instruction.to_ulong() + IMem[i + ReadAddress.to_ulong()].to_ulong();
        }
      return Instruction;
    }

  private:
    vector<bitset<8> > IMem;
};

class DataMem
{
  public:
    bitset<32> readdata;
    DataMem()
    {
      DMem.resize(MemSize);
      ifstream dmem;
      string line;
      int i=0;
      dmem.open("dmem.txt");
      if (dmem.is_open())
      {
        while (getline(dmem,line))
        {
          DMem[i] = bitset<8>(line);
          i++;
        }
      }
      else cout<<"Unable to open file";
      dmem.close();
    }
    bitset<32> MemoryAccess (bitset<32> Address, bitset<32> WriteData, bitset<1> readmem, bitset<1> writemem)
    {
      // TODO: implement!
        int x=31;
        if (readmem.to_ulong()==1) {
            readdata = 0;
            for (int i = 0; i < 4; ++i) {
            readdata <<= 8;
            readdata = readdata.to_ulong() + DMem[Address.to_ulong() + i].to_ulong();
            }
        }
        if (writemem.to_ulong()==1) {
            for (int i = 0; i < 4; i++)
            for (int j = 7; j >= 0; j--)
            {
            DMem[Address.to_ulong() + i][j] = WriteData[x];
            x--;
            }
        }
      return readdata;
    }

    void OutputDataMem()
    {
      ofstream dmemout;
      dmemout.open("dmemresult.txt");
      if (dmemout.is_open())
      {
        for (int j = 0; j< 1000; j++)
        {
          dmemout << DMem[j]<<endl;
        }
      }
      else cout<<"Unable to open file";
      dmemout.close();
    }

  private:
    vector<bitset<8> > DMem;

};



int main()
{
  RF myRF;
  ALU myALU;
  INSMem myInsMem;
  DataMem myDataMem;
    int i;
    bool isEqual = false;
    bitset<32> pc = 0;
    bitset<32> tempc;
    bitset<6> opcode;
    bitset<6> funct;
    bitset<5> rs;
    bitset<5> rt;
    bitset<5> rd;
    bitset<5> shatamt;
    bitset<3> ALUop;
    bitset<32> curIns;
    bitset<32> haltIns = 0xFFFFFFFF;
    bitset<32> jAddr;
    bitset<16> imm;
    bitset<32> offset;
    bitset<32> result;
    bitset<32> readData;
    bitset<32> BranchAddr;
  while (1)
  {
    // Fetch
      curIns = myInsMem.ReadMemory(pc.to_ulong());
      cout<< "Instraction is :" << curIns <<endl;
    // If current insturciton is "11111111111111111111111111111111", then break;
      if(curIns == haltIns) {
          cout<<" halt" << endl;
          break;
      }
      int opIndex = 5;
      for(int i = 31; i >= 26; i--) {
          opcode[opIndex] = curIns[i];
          opIndex--;
      }
      cout<< "opcode:" << opcode <<endl;
    // decode(Read RF)
      if(opcode == 0x00) {                   //R type
          cout<< "R type instraction" <<endl;
          int rIndex = 4;
          for (int i = 25; i >= 21; i--){
              rs[rIndex] = curIns[i];
              rIndex--;
          }
          rIndex = 4;
          for (int i = 20; i >= 16; i--){
              rt[rIndex] = curIns[i];
              rIndex--;
          }
          rIndex = 4;
          for (int i = 15; i >= 11; i--){
              rd[rIndex] = curIns[i];
              rIndex--;
          }
          int functindex = 5;
          for(int i = 5; i >= 0; i--) {
              funct[functindex] = curIns[i];
              functindex--;
          }
          for (i = 0; i < 3; i++) {
              ALUop[i] = funct[i];
          }
          if(ALUop==1)
          cout<<"addu instruction"<<endl;
          else if(ALUop==3)
          cout<<"subu instruction"<<endl;             //output certain function
          else if(ALUop==4)
          cout<<"and instruction"<<endl;
          else if(ALUop==5)
          cout<<"or instruction"<<endl;
          else
          cout<<"nor instruction"<<endl;
          myRF.ReadWrite(rs, rt, rd, 0, 0);
      }else if (opcode == 0x02) {           // J type
          cout<< "J type instraction" <<endl;
          tempc = pc.to_ulong() + 4;
          for(i = 2; i < 28; i++) {
              jAddr[i] = curIns[i - 2];
          }
          for(i = 28; i < 32; i++) {
              jAddr[i] = tempc[i];
          }
          for(i = 0; i < 2; i++) {
              jAddr[i] = 0;
          }
          cout<<"PC jump to: "<<jAddr.to_ulong()<<endl;
          
      }else {                         //I type
          cout<< "I type instraction" <<endl;
          int rIndex = 4;
          for (int i = 25; i >= 21; i--){
              rs[rIndex] = curIns[i];
              rIndex--;
          }
          rIndex = 4;
          for (int i = 20; i >= 16; i--){
              rt[rIndex] = curIns[i];
              rIndex--;
          }
          for(i = 0; i < 16; i++){
              imm[i] = curIns[i];
          }
          offset = signExtend(imm);
          myRF.ReadWrite(rs, rt, rt, 0, 0);
      }
    // Execute
      if (opcode == 0x00) {
          result = myALU.ALUOperation(ALUop, myRF.Data1, myRF.Data2);
          cout << " result= " << result << endl ;
      }
      else if(opcode!=0x02){
          if (opcode == 0x04)           //beq
          {
          cout<<"beq instruction"<<endl;
          ALUop = 3;
          result = myALU.ALUOperation(ALUop, myRF.Data1, myRF.Data2);
          if (result == 0)           //beq judgement
          {
          isEqual = true;
          cout<<"R[rs]=R[rt]"<<endl;
          }
          else
          cout<<"R[rs]!=R[rt]"<<endl;
          }
          else
          {
          ALUop = 1;            //lw,sw,addiu
          result = myALU.ALUOperation(ALUop, myRF.Data1, offset);
          }
      }
    // Read/Write Mem
          if(opcode == 0x23) { //lw   update register data
              cout<<"lw instruction"<<endl;
              readData = myDataMem.MemoryAccess(result, myRF.Data2, 1, 0);
          }
          else if(opcode  == 0x2b) {
              cout<<"sw instruction"<<endl;
              readData = myDataMem.MemoryAccess(result,myRF.Data2, 0, 1);
          }
          else if (opcode == 0x09) {
              cout<<"addiu instruction"<<endl;
          }
    // Write back to RF
         if (opcode != 0x04 && opcode != 0x2b && opcode != 0x02) {
           if (opcode == 0x00)
            {
            myRF.ReadWrite(rs, rt, rd, result, 1);
            }
           else if(opcode==0x23)
           myRF.ReadWrite(rs, rt, rt, readData, 1);
           else
           myRF.ReadWrite(rs, rt, rt, result, 1);
         }
            if(!isEqual)
             cout << "PC: " << pc.to_ulong() << endl<<endl;
             if (opcode == 0x02)
             pc = jAddr;


             else if (opcode == 0x04 && isEqual)
             {
             for (i = 2; i < 18; i++) {
             BranchAddr[i] = imm[i-2];
                 }
                 for (i = 18; i < 32; i++) {
                     BranchAddr[i] = imm[15];
                 }
                 for (i = 0; i < 2; i++) {
                     BranchAddr[i] = 0;
                 }
                 
             bitset<32> pctemp;
             pctemp=pc;
             pc = pc.to_ulong() + 4 + BranchAddr.to_ulong()*4 ;            //beq jump
             cout<<"PC jump to: "<<pc.to_ulong()<<endl;
             cout << "PC: " << pctemp.to_ulong() <<endl<<endl;
             isEqual=false;
             }
             else{
                 pc = pc.to_ulong() + 4;}
           myRF.OutputRF(); // dump RF;
  }
          myDataMem.OutputDataMem();
          system("pause");
          return 0;
}
