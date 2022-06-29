#include "MathFunc.h"

vector<bool> IntToBinVec(int num, int len)
{
	vector<bool> ret; //vector to be returned
	while(ret.size()<len)
	{
		ret.insert(ret.begin(), num % 2);
		num /= 2;
	}
	return ret;
}
