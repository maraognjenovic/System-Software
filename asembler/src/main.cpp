#include <iostream>
#include "../inc/Assembler.h"
using namespace std;
int main(int argc, const char *argv[])
{
    string object = argv[1];
    if(argc>4) 
        cout<<"Broj parametara mora biti 4"<<endl;
    if (object == "-o"){
        string input = argv[3];
        string output = argv[2];

        Assembler as(input, output);

        if(as.errors())
            return -1;
    }else{
        cout<<"Drugi argument mora biti -o";
        return -1;
    }
    return 0;
}