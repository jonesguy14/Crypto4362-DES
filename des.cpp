#include "des.h"
#define MAX_KEY_SIZE 256
int hex_char_to_decimal(std::string c);
std::string permute(std::string input, const std::vector<std::string> & permute)
{
    std::string output = "";
    for (unsigned int i = 0; i < permute.size(); ++i)
    {
        output += input[ hex_char_to_decimal(permute[i]) - 1 ];
    }
    return output;
}

std::string XOR(std::string input1, std::string input2)
{
    std::string output = "";
    for (unsigned int i = 0; i < input1.size(); ++i)
    {
        output += (input1[i]^input2[i]) ? "1" : "0";
    }
    return output;
}

std::string cycleLeft(std::string input, std::string places)
{
    std::string output = "";
    int shift_amount = hex_char_to_decimal(places) % input.size();
    for (unsigned int i = shift_amount; i < input.size(); ++i)
    {
        output += input[i];
    }
    for (unsigned int i = 0; i < shift_amount; ++i)
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

std::string decToBin(int decimal_input, unsigned int size)
{
    std::bitset<MAX_KEY_SIZE> binary_output(decimal_input);
    std::string binary_string = binary_output.to_string();
    return binary_string.substr(binary_string.size() - size, std::string::npos);
}

int hex_char_to_decimal(std::string c)
{
    if (c == "A") return 10;
    if (c == "B") return 11;
    if (c == "C") return 12;
    if (c == "D") return 13;
    if (c == "E") return 14;
    if (c == "F") return 15;
    int numb;
    std::istringstream ( c ) >> numb;
    return numb;
}

unsigned long long binToDec(std::string binary_input)
{
    return std::bitset<64>(binary_input).to_ullong();
}

std::vector<std::string> inversePermute(std::vector<std::string> permutation)
{
    std::vector<std::string> inverse;
    for (unsigned int i = 1; i < permutation.size() + 1; ++i)
    {
        for (unsigned int j = 0; j < permutation.size(); ++j)
        {
            if (permutation[j] == std::to_string(i))
            {
                inverse.push_back(std::to_string(j+1));
            }
        }
    }
    return inverse;
}
