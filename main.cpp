#include <iostream>
#include "stdio.h"


using namespace std;

void readFile();
void procedure();
void midcodeToFile();
void genAsm();
void optimize();

int main(){
	readFile();
	procedure();
    midcodeToFile();
    genAsm();
    optimize();
	return 0;
}

