#include <cstdint>
#include "machineinfo.h"

bool MachineIsLittleEndian()
{
	uint32_t t = 0;
	char* p = reinterpret_cast<char*>(&t);
	*p = 1;
	return t == 1;
}
