#ifndef des_h
#define des_h
/*
 * Known Limitations:
 * 		Maximum Key Size: 256 bits (binary), due to hexToBin bitset<MAX_KEY_SIZE>. Can be changed.
 * Brittleness:
 *		Requires users to have valid inputs
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

#endif
