#include "HelperFunctions.h"


bool HelperFunctions::numericalSort(const std::string a, const std::string b)
{
	return (a.length() < b.length() || (a.length() == b.length() && (a < b)));
}
