#include "wscript.h"
#include <iostream>

int main(int argc, char* argv[])
{
	wscript script;
	std::cout << script.get_cpu_vendor_string().c_str() << std::endl;
}