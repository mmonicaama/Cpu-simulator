#include <iostream>
#include <string>
#include <fstream>
#include "Cpu.h"

int main()
{
	std::string path = "C:\\Users\\Monica\\OneDrive\\Documents\\Picsart\\Cpu\\Cpu\\myCode.txt";
	Cpu myCpu;
	myCpu.execute(path);
	myCpu.dump_memory();
	
	return 0;
}