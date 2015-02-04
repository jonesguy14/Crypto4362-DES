#include "des.h"
#define MAX_KEY_SIZE 256;
std::string permute(std::string input, const std::vector<int> & permute)
{
	std::string output = "";
	for (unsigned int i = 0; i < permute.size(); ++i)
	{
		output += input[ permute[i]-1 ];
	}
	return output;
}

std::string XOR(std::string input1, std::string input2)
{
	std::string output = "";
	for (unsigned int i = 0; i < input1.size(); ++i)
	{
		output += (input[i]^key[i]) ? "1" : "0";
	}
	return output;
}

std::string cycleLeft(std::string input, int places)
{
	std::string output = "";
	places = places % input.size();
	for (unsigned int i = places; i < input.size(); ++i)
	{
		output += input[i];
	}
	for (unsigned int i = 0; i < places; ++i)
	{
		output += input[i];
	}
	return output;
}

std::string splitLeft(std::string input)
{
	return input.substr(0, input.size()/2);
}

std::string splitRight(std::string input)
{
	return input.substr(input.size()/2, std::string::npos);
}

std::string binToHex(std::string binary_input)
{
	std::bitset<MAX_KEY_SIZE> bs(binary_input);
    unsigned n = bs.to_ulong();
    std::stringstream ss;
    ss << std::hex << n;
    return ss.str();
}

std::string hexToBin(std::string hex_input, unsigned int size)
{
    std::stringstream ss;
    ss << std::hex << hex_input;
    unsigned n;
    ss >> n;
    std::bitset<MAX_KEY_SIZE> binary_output(n);
    std::string binary_string = binary_output.to_string();
    return binary_string.substr(binary_string.size() - size, std::string::npos);
}
