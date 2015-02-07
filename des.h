#ifndef des_h
#define des_h
/*
 * Known Limitations:
 * 		Maximum Key Size: 256 bits (binary), due to hexToBin bitset<MAX_KEY_SIZE>. Can be changed.
 * Brittleness:
 *		Requires users to have cohesive inputs; e.g., permutations need to select bits that exist,
 *      											  SBox bits need to select permutations that exist 
 *      Requires SBox Specifications to be in Decimal
 *      Params are assumed to be the same type as given in the project description
 *      stdin is assumed ASCII represented hex or binary. stdout is in ASCII represented hex or binary.
  *     Files are assumed standard binary and outputted in standard binary.
 * Deviations from Specs:
 */
#include <vector>
#include <string>
#include <sstream>
#include <bitset>
#include <iostream>

std::string permute(std::string input, const std::vector<std::string> & permute);
std::string XOR(std::string input1, std::string input2);
std::string cycleLeft(std::string input, std::string places);
std::string splitLeft(std::string input);
std::string splitRight(std::string input);
std::string binToHex(std::string binary_input);
std::string hexToBin(std::string hex_input, unsigned int size);
std::string decToBin(int decimal_input, unsigned int size);
int hex_char_to_decimal(std::string c);
unsigned long long binToDec(std::string binary_input);
std::vector<std::string> inversePermute(std::vector<std::string> permutation);

#endif
