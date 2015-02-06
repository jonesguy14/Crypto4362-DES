#ifndef des_h
#define des_h
/*
 * Known Limitations:
 * 		Maximum Key Size: 256 bits (binary), due to hexToBin bitset<MAX_KEY_SIZE>. Can be changed.
 * Brittleness:
 *		Requires users to have cohesive inputs; e.g., permutations need to select bits that exist,
 *      SBox bits need to select permutations that exist 
 *      Requires SBox Specifications to be in Decimal
 *      Permutations support hex from 0-F but any numbers larger than 0xF need to be in decimal.
 * Deviations from Specs:
 * 		Only accept ASCII-encoded binary and ASCII-encoded hex, not pure binary.
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
