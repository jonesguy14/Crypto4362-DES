#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <iterator>
#include <algorithm>
#include <vector>
#include <math.h>
#include "des.h"

using namespace std;

vector< vector< vector<int> > > make_sboxes(vector<string> lines, int num_sboxes, int sbox_height, int sbox_width) {
    vector< vector< vector<int> > > result;
    vector< vector<int> > new_box;
    vector<int> new_row;
    int counter = 0;
    for (int i = 0; i < num_sboxes; i++) {
        new_box.clear();
        for (int j = 0; j < sbox_height; j++) {
            new_row.clear();

            string buf; // Have a buffer string
            stringstream ss( lines[counter] ); // Insert the string into a stream
            while (ss >> buf) {
                //cout << "bout to print ";
                //cout << atoi( buf.c_str() ) << endl;
                new_row.push_back( atoi( buf.c_str() ) ); //sboxes[sbox_count][sbox_row]
            }

            new_box.push_back( new_row );
            counter ++;
        }
        result.push_back( new_box );
    }
    return result;
}

int main( int argc, char *argv[] )
{
    bool encrypt = false; //if want to encrypt input
    bool decrypt = false; //if want to decrypt input
    bool showstp = false; //if want to print intermediate steps
    bool hex_rep = false; //if want hexadecimal instead of binary

    string input_file_str  = "input.txt";
    string output_file_str = "output.txt";
    string param_file_str  = "params.txt";
    string key_str_hex     = "key.txt";
    
    //ofstream desr_out;
    

    bool using_stdin  = false; //if using stdin instead of input file
    bool using_stdout = false; //if using stdout instead of output file

    if ( argc > 8 ) {
        cout << "Too many flags detected." << endl;
        return 1;
    }

    for (int i = 1; i < argc; ++i) { //run through each flag detected
        string arg_str = argv[i]; //set arg to string for easy comparison
        if ( arg_str == "-e" ) {
            encrypt = true;
        }
        else if ( arg_str == "-d" ) {
            decrypt = true;
        }
        else if ( arg_str == "-s" ) {
            showstp = true;
        }
        else if ( arg_str == "-x" ) {
            hex_rep = true;
        }
        else if ( arg_str.substr(0, 2) == "-i" ) {
            //want different input file name
            input_file_str = arg_str.substr(2, arg_str.length() - 2); //get str after '-i'
            if ( input_file_str == "-" ) {
                //use stdin
                using_stdin = true;
            }
        }
        else if ( arg_str.substr(0, 2) == "-o" ) {
            //want different output file name
            output_file_str = arg_str.substr(2, arg_str.length() - 2); //get str after '-o'
            if ( output_file_str == "-" ) {
                //use stdout
                using_stdout = true;
            } 
        }
        else if ( arg_str.substr(0, 2) == "-k" ) {
            //want to use key (in hex) from command line
            key_str_hex = arg_str.substr(2, arg_str.length() - 2); //get str after '-k'
        }
        else if ( arg_str.substr(0, 2) == "-p" ) {
            //want different param file name
            param_file_str = arg_str.substr(2, arg_str.length() - 2); //get str after '-p'
        }
    }
    int block_size, key_size, eff_key_size, rnd_key_size, num_rounds, num_sboxes;
    vector<string>    permute_choice_pc1_vec;
    vector<string>    permute_choice_pc2_vec;
    vector<string>    rotation_schedule_vec;
    vector<string>    init_permute_vec;
    vector<string>    expan_permute_vec;
    vector<string>    pbox_trans_perm_vec;
    vector<string>    row_selection_vec;
    vector<string>    col_selection_vec;

    //vector to store all the sboxes, will be vector of vector of vectors :^)
    //to retrieve: sboxes[sbox number][sbox row][sbox column]
    vector< vector< vector<int> > > sboxes;
    int sbox_width, sbox_height, line_start_sbox, sbox_count, sbox_row = 0;
    bool initSBoxvec = false;
    vector<string> sbox_lines;

    //NEXT, GET PARAMETERS FROM FILES
    ifstream param_file;
    param_file.open( param_file_str.c_str() ); //open parameter file for reading
    string line; //where each line will be saved while parsing file
    int comment_start; //integer to store where comments are in each line
    int line_counter = 0; //keep track of lines and which parameter is which

    if ( param_file.is_open() ) {
        while ( getline(param_file, line) ) { //get each line from the text file
            comment_start = line.find("//"); //find if there are comments to remove for parsing
            if ( comment_start != line.length() ) {
                //found comment
                line = line.substr(0, comment_start); //remove comment from line
                if ( line_counter < 5 || line_counter == 11 ) { //will remove whitespace from entries which aren't vectors
                    line.erase( remove_if( line.begin(), line.end(), ::isspace ), line.end() ); //should remove whitespace
                }

                if ( line.length() > 0 ) {
                    //whats left should be the final string
                    switch ( line_counter ) {
                        case 0: //block size
                            block_size = atoi( line.c_str() );
                            break;
                        case 1: //key size
                            key_size = atoi( line.c_str() );
                            break;
                        case 2: //effective key size
                            eff_key_size = atoi( line.c_str() );
                            break;
                        case 3: //round key size
                            rnd_key_size = atoi( line.c_str() );
                            break;
                        case 4: //number of rounds
                            num_rounds = atoi( line.c_str() );
                            break;
                        case 5: //initial permuted choice (pc1)
                            { istringstream iss5( line );
                            copy( istream_iterator<string>(iss5),
                                  istream_iterator<string>(),
                                  back_inserter(permute_choice_pc1_vec) ); }
                            // this should split by spaces and insert items into permute choice pc1 vector
                            break;
                        case 6: //permute choice 2
                            { istringstream iss6( line );
                            copy( istream_iterator<string>(iss6),
                                  istream_iterator<string>(),
                                  back_inserter(permute_choice_pc2_vec) ); }
                            // this should split by spaces and insert items into permute choice pc2 vector
                            break;
                        case 7: //rotation schedule
                            { istringstream iss7( line );
                            copy( istream_iterator<string>(iss7),
                                  istream_iterator<string>(),
                                  back_inserter(rotation_schedule_vec) ); }
                            // this should split by spaces and insert items into rotate schedule vector
                            break;
                        case 8: //initial permutation
                            { istringstream iss8( line );
                            copy( istream_iterator<string>(iss8),
                                  istream_iterator<string>(),
                                  back_inserter(init_permute_vec) ); }
                            // this should split by spaces and insert items into initial permutation vector
                            break;
                        case 9: //expansion permutation
                            { istringstream iss9( line );
                            copy( istream_iterator<string>(iss9),
                                  istream_iterator<string>(),
                                  back_inserter(expan_permute_vec) ); }
                            // this should split by spaces and insert items into expansion permutation vector
                            break;
                        case 10: //p-box transposition permutation
                            { istringstream iss10( line );
                            copy( istream_iterator<string>(iss10),
                                  istream_iterator<string>(),
                                  back_inserter(pbox_trans_perm_vec) ); }
                            // this should split by spaces and insert items into pbox vector
                            break;
                        case 11: //number of sboxes
                            num_sboxes = atoi( line.c_str() );
                            break;
                        case 12: //row selection bits
                            { istringstream iss12( line );
                            copy( istream_iterator<string>(iss12),
                                  istream_iterator<string>(),
                                  back_inserter(row_selection_vec) ); }
                            // this should split by spaces and insert items into row select vector
                            break;
                        case 13: //column selection bits
                            { istringstream iss13( line );
                            copy( istream_iterator<string>(iss13),
                                  istream_iterator<string>(),
                                  back_inserter(col_selection_vec) ); }
                            // this should split by spaces and insert items into column select vector
                            line_counter ++;
                            break;
                    } //end switch

                    if ( line_counter > 15 ) {
                        //cout << "Parsing sboxes" << endl;
                        if (line[0] != ' ' && line[0] != '\t') {sbox_lines.push_back(line);}

                    }

                    line_counter++; //increment line counter so we know what is next, will only increment if has data
                }

            }
        }

        sbox_height = pow( 2 , (rnd_key_size/num_sboxes - block_size/(2 * num_sboxes)) ); //num rows
        sbox_width  = pow( 2 , (block_size/(2 * num_sboxes)) ); //num columns
        sboxes = make_sboxes( sbox_lines, num_sboxes, sbox_height, sbox_width );
    }

	if ( showstp ) {
		//Printing to make sure parsing was correct
		cout << "Block Size: " << block_size << endl;
		cout << "Key Size: " << key_size << endl;
		cout << "Eff Key Size: " << eff_key_size << endl;
		cout << "Round Key Size: " << rnd_key_size << endl;
		cout << "Num Rounds: " << num_rounds << endl;
		cout << "Perm1: ";
		for (int i = 0; i < permute_choice_pc1_vec.size(); ++i) {
			cout << permute_choice_pc1_vec[i] << " ";
		}
		cout << "\nPerm2: ";
		for (int i = 0; i < permute_choice_pc2_vec.size(); ++i) {
			cout << permute_choice_pc2_vec[i] << " ";
		}
		cout << "\nRotation: ";
		for (int i = 0; i < rotation_schedule_vec.size(); ++i) {
			cout << rotation_schedule_vec[i] << " ";
		}
		cout << "\nInit permutation: ";
		for (int i = 0; i < init_permute_vec.size(); ++i) {
			cout << init_permute_vec[i] << " ";
		}
		cout << "\nExpand permutation: ";
		for (int i = 0; i < expan_permute_vec.size(); ++i) {
			cout << expan_permute_vec[i] << " ";
		}
		cout << "\nP-Box transposition: ";
		for (int i = 0; i < pbox_trans_perm_vec.size(); ++i) {
			cout << pbox_trans_perm_vec[i] << " ";
		}
		cout << "\nNum SBoxes: " << num_sboxes << endl;
		cout << "Row Select Bits: ";
		for (int i = 0; i < row_selection_vec.size(); ++i) {
			cout << row_selection_vec[i] << " ";
		}
		cout << "\nColumn Select Bits: ";
		for (int i = 0; i < col_selection_vec.size(); ++i) {
			cout << col_selection_vec[i] << " ";
		}

		cout << "\nSboxes" << endl;
		for (int i = 0; i < num_sboxes; ++i) {
			for (int j = 0; j < sbox_height; ++j) {
				for (int k = 0; k < sbox_width; ++k) {
					cout << sboxes[i][j][k];
				}
				cout << endl;
			}
			cout << "---------" << endl;
		}
	}
	
    vector<string> init_permute_vec_inverse = inversePermute(init_permute_vec);
		if ( showstp ) {
			cout << "\nInit permutation inverse: ";
		  for (int i = 0; i < init_permute_vec_inverse.size(); ++i) {
		      cout << init_permute_vec_inverse[i] << " ";
		  }
		}

    string left, right;
    string key = hexToBin("357", key_size); //the master key
    string plain = "100101";
    //Initial Permutation, Split
    string init_permutation = permute(plain, init_permute_vec);
    left = splitLeft(init_permutation); //L0
    right = splitRight(init_permutation); //R0
    
    if ( showstp ) {
		cout << "Plain: " + plain << endl;
		cout << "Key: " + key << endl;
		cout << "Initital Permutation: " + init_permutation << endl;
		cout << "Left Split: " + left << endl;
		cout << "Right Split: " + right << endl;
	}
	
    for (unsigned int i = 0; i < num_rounds; ++i)
    {
        //Generating the Key
        //Apply Permutation 1
        string round_key = permute(key, permute_choice_pc1_vec);
        string left_round = splitLeft(round_key);
        string right_round = splitRight(round_key);
        if ( showstp ) {
			cout << "Round: " << i + 1 << endl;
			cout << "Round Key: " + round_key << endl;
			cout << "Left Round: " + left_round << endl;
			cout << "Right Round: " + right_round << endl;
		}
        //Apply rotation schedule to both sides, cumulatively
        for (unsigned int j = 0; j < i + 1; ++j)
        {
            left_round = cycleLeft(left_round, rotation_schedule_vec[j]);
            right_round = cycleLeft(right_round, rotation_schedule_vec[j]);
        }
        
        if ( showstp ) {
			cout << "Cycled Left Round: " + left_round << endl;
			cout << "Cycled Right Round: " + right_round << endl;
		}
		
        round_key = left_round + right_round;
        if ( showstp ) {
			cout << "New Round Key: " + round_key << endl;
		}
        //Apply permutation 2
        round_key = permute(round_key, permute_choice_pc2_vec);
        if ( showstp ) {
			cout << "Final Round Key: " + round_key << endl;
		}
        //Round Key is now fully generated

        //Starting on F:
        //Apply Expansion Permutation to R0
        string right_exp = permute(right, expan_permute_vec);
        string xi = XOR(right_exp, round_key);
        if ( showstp ) {
			cout << "Expansion Permutation: " + right_exp << endl;
			cout << "XOR with Round Key: " + xi << endl;
		}
        //xi is split into (number of s boxes) equal pieces, each with Round key size/number of S boxes bits
        //concat outputs from s-boxes
        string yi;
        for ( int i = 0; i < num_sboxes; ++i) { //Go through sboxes to find results
			string xi_subset = xi.substr( i*xi.length()/num_sboxes, xi.length()/num_sboxes );
			
			//find row selection bits
			string rowbitstr = "";
			for ( int r = 0; r < row_selection_vec.size(); ++r) { //find each bit in the row select vector
				rowbitstr += xi_subset[ atoi( row_selection_vec[r].c_str() ) - 1 ];
			}
			int rowbitdec = binToDec( rowbitstr );
			
			//find column selection bits
			string colbitstr = "";
			for ( int c = 0; c < col_selection_vec.size(); ++c) { //find each bit in the col select vector
				colbitstr += xi_subset[ atoi( col_selection_vec[c].c_str() ) - 1 ];
			}
			int colbitdec = binToDec( colbitstr );
			
			//size of output of sbox is the number of bits needed to represent all the entries
			int output_size = log( sbox_height * sbox_width ) / log( 2 );

			yi += decToBin( sboxes[i][rowbitdec][colbitdec], output_size );
		}
		
		if ( showstp ) {
			cout << "Sbox result: " << yi << endl;
		}
        //The concatenated output from the T S-boxes, Yi, is then transposed using the P-box permutation
        string ui = permute(yi, pbox_trans_perm_vec);
        // Ui, which is then XORed with L0 to form R1.
        string temp = right; //save value of R0
        right = XOR(left, ui);
        //L1 = R0
        left = temp;
        if ( showstp ) {
			cout << "Left: " + left << endl;
			cout << "Right: " + right << endl;
		}
    } // end of rounds loop
    //After the final round, the left and right halves are swapped and the inverse initial permutation is applied to form the ciphertext C
    string temp = right;
    right = left;
    left = temp;
    string final = permute(left + right, init_permute_vec_inverse);
		cout << "Result: " + final << endl;
    return 0;
}
