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
                //*desr_out << "b*desr_out to print ";
                //*desr_out << atoi( buf.c_str() ) << endl;
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
    bool encrypt = true; //if want to encrypt input. The default!
    bool decrypt = false; //if want to decrypt input
    bool showstp = false; //if want to print intermediate steps
    bool hex_rep = false; //if want hexadecimal instead of binary

    string input_file_str  = "ptxt";
    string output_file_str = "ctxt";
    string param_file_str  = "params.txt";
    string key_str_hex     = "key.txt";
    
    string key_str_from_file;
    
    ofstream fout;
	ostream* desr_out;

    bool using_stdin  = false; //if using stdin instead of input file
    bool using_stdout = false; //if using stdout instead of output file
    bool setNewIn = false; //if set new input file name
    bool setNewOut = false; //if using std*desr_out instead of *desr_output file
    bool setNewKey = false; //set true if not using the key.txt file

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
			encrypt = false;
        }
        else if ( arg_str == "-s" ) {
            showstp = true;
        }
        else if ( arg_str == "-x" ) {
            hex_rep = true;
        }
        else if ( arg_str.substr(0, 2) == "-i" ) {
            //want different input file name
            setNewIn = true;
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
                setNewOut = true;
                using_stdout = true;
                desr_out = &cout;
            } else {
				setNewOut = true;
				fout.open( output_file_str.c_str() );
				desr_out = &fout;
			}
        }
        else if ( arg_str.substr(0, 2) == "-k" ) {
            //want to use key (in hex) from command line
            key_str_hex = arg_str.substr(2, arg_str.length() - 2); //get str after '-k'
            setNewKey = true;
            key_str_from_file = key_str_hex;
        }
        else if ( arg_str.substr(0, 2) == "-p" ) {
            //want different param file name
            param_file_str = arg_str.substr(2, arg_str.length() - 2); //get str after '-p'
        }
    }
    
    if ( setNewOut == false ) { //no -o flag
        if ( encrypt ) output_file_str = "ctxt";
        if ( decrypt ) output_file_str = "ptxt";
		fout.open( output_file_str.c_str() );
		desr_out = &fout;
	}

    if ( setNewIn == false ) { //no -i flag
        if ( encrypt ) input_file_str = "ptxt";
        if ( decrypt ) input_file_str = "ctxt";
    }
	
	if ( setNewKey == false ) { //use default key.txt
		string keyline;
		ifstream key_file ( key_str_hex.c_str() );
		if ( key_file.is_open() ) {
			while ( key_file.good() ) {
				getline( key_file, keyline );
				key_str_from_file += keyline;
			}
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
                        //*desr_out << "Parsing sboxes" << endl;
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
		*desr_out << "Block Size: " << block_size << endl;
		*desr_out << "Key Size: " << key_size << endl;
		*desr_out << "Eff Key Size: " << eff_key_size << endl;
		*desr_out << "Round Key Size: " << rnd_key_size << endl;
		*desr_out << "Num Rounds: " << num_rounds << endl;
		*desr_out << "Perm1: ";
		for (int i = 0; i < permute_choice_pc1_vec.size(); ++i) {
			*desr_out << permute_choice_pc1_vec[i] << " ";
		}
		*desr_out << "\nPerm2: ";
		for (int i = 0; i < permute_choice_pc2_vec.size(); ++i) {
			*desr_out << permute_choice_pc2_vec[i] << " ";
		}
		*desr_out << "\nRotation: ";
		for (int i = 0; i < rotation_schedule_vec.size(); ++i) {
			*desr_out << rotation_schedule_vec[i] << " ";
		}
		*desr_out << "\nInit permutation: ";
		for (int i = 0; i < init_permute_vec.size(); ++i) {
			*desr_out << init_permute_vec[i] << " ";
		}
		*desr_out << "\nExpand permutation: ";
		for (int i = 0; i < expan_permute_vec.size(); ++i) {
			*desr_out << expan_permute_vec[i] << " ";
		}
		*desr_out << "\nP-Box transposition: ";
		for (int i = 0; i < pbox_trans_perm_vec.size(); ++i) {
			*desr_out << pbox_trans_perm_vec[i] << " ";
		}
		*desr_out << "\nNum SBoxes: " << num_sboxes << endl;
		*desr_out << "Row Select Bits: ";
		for (int i = 0; i < row_selection_vec.size(); ++i) {
			*desr_out << row_selection_vec[i] << " ";
		}
		*desr_out << "\nColumn Select Bits: ";
		for (int i = 0; i < col_selection_vec.size(); ++i) {
			*desr_out << col_selection_vec[i] << " ";
		}

		*desr_out << "\nSboxes" << endl;
		for (int i = 0; i < num_sboxes; ++i) {
			for (int j = 0; j < sbox_height; ++j) {
				for (int k = 0; k < sbox_width; ++k) {
					*desr_out << sboxes[i][j][k];
				}
				*desr_out << endl;
			}
			*desr_out << "---------" << endl;
		}
	}
	
    vector<string> init_permute_vec_inverse = inversePermute(init_permute_vec);
	if ( showstp ) {
		*desr_out << "\nInit permutation inverse: ";
	  for (int i = 0; i < init_permute_vec_inverse.size(); ++i) {
	      *desr_out << init_permute_vec_inverse[i] << " ";
	  }
	}

    string left, right;
    string key = hexToBin( key_str_from_file , key_size ); //the master key
    //Generating the Keys
    vector<string> key_vector;
    //Apply Permutation 1 and split.
    string key_permute = permute(key, permute_choice_pc1_vec);
    string left_round = splitLeft(key_permute);
    string right_round = splitRight(key_permute);
    //Generate for each round
    for (unsigned int i = 0; i < num_rounds; ++i)
    {
    	//Apply rotation schedule to both sides, cumulatively
    	left_round = cycleLeft(left_round, rotation_schedule_vec[i]);
    	right_round = cycleLeft(right_round, rotation_schedule_vec[i]);

    	string round_key = left_round + right_round;
    	//Apply permutation 2
    	round_key = permute(round_key, permute_choice_pc2_vec);
    	//Final round key for that round is generated
    	key_vector.push_back(round_key);
    }
    //Round Keys are now fully generated
    //Get data from files
    string data;
    if ( !using_stdin )
    {
        streampos size;
        char * memblock;
        ifstream file (input_file_str, ios::in|ios::binary|ios::ate);
        if (file.is_open())
        {
            size = file.tellg();
            memblock = new char [size];
            file.seekg (0, ios::beg);
            file.read (memblock, size);
            file.close();
            string temp = string(memblock);
            for (unsigned int i = 0; i < temp.size(); ++i)
            {
                data += decToBin((int)temp[i], 8); //read binary information
            }
            delete[] memblock;
        }
    }
    else
    {
        string line;
        while ( getline(cin, line) )
        {
            data += line;
        }
    }

    data.erase( remove_if( data.begin(), data.end(), ::isspace ), data.end() ); //remove whitespace from input

    unsigned int chars_retrieved = 0;
    string combined_blocks_result = ""; //will store all the blocks in one string
    while( chars_retrieved < data.size() ) {
        string plain;
        if ( !hex_rep )
        {
            plain = data.substr(chars_retrieved, block_size);
            while ( plain.size() < block_size )
            {
                plain += "0";
            }
            chars_retrieved += block_size;
        }
        else
        {
            plain = data.substr(chars_retrieved, block_size/4);
            while ( plain.size() < block_size/4 )
            {
                plain += "0";
            }
            plain = hexToBin(plain, block_size);
            chars_retrieved += block_size/4;
        }
        string init_permutation = permute(plain, init_permute_vec);
        left = splitLeft(init_permutation); //L0
        right = splitRight(init_permutation); //R0
        if ( showstp ) {
            *desr_out << "\n\n-----NEW BLOCK-----" << endl;
    		if ( hex_rep ) {
    			*desr_out << "Initial Block: " + binToHex(plain) << endl;
    			*desr_out << "Key: " + binToHex(key) << endl;
    			*desr_out << "Initital Permutation: " + binToHex(init_permutation) << endl;
    			*desr_out << "Left Split: " + binToHex(left) << endl;
    			*desr_out << "Right Split: " + binToHex(right) << endl;
    		} else {
    			*desr_out << "Initial Block: " + plain << endl;
    			*desr_out << "Key: " + key << endl;
    			*desr_out << "Initital Permutation: " + init_permutation << endl;
    			*desr_out << "Left Split: " + left << endl;
    			*desr_out << "Right Split: " + right << endl;
    		}
    	}

        for (unsigned int i = 0; i < num_rounds; ++i) //begin the rounds
        {
    		if ( showstp ) {
    			*desr_out << "---Round: " << i + 1 << "---" << endl;
    		}
    		string round_key = "";
    		if ( decrypt ) round_key = key_vector[num_rounds - i - 1]; //Decrypt uses keys in reverse order
    		if ( encrypt ) round_key = key_vector[i];
    		if ( showstp ) {
                if ( hex_rep ) {
                    *desr_out << "Round Key: " << binToHex(round_key) << endl;
                } else {
                    *desr_out << "Round Key: " << round_key << endl;
                }
    		}

            //Starting on F:
            //Apply Expansion Permutation to R0
            string right_exp = permute(right, expan_permute_vec);
            string xi = XOR(right_exp, round_key);
            if ( showstp ) {
    			if ( hex_rep ) {
    				*desr_out << "Expansion Permutation: " + binToHex(right_exp) << endl;
    				*desr_out << "XOR with Round Key: " + binToHex(xi) << endl;
    			} else {
    				*desr_out << "Expansion Permutation: " + right_exp << endl;
    				*desr_out << "XOR with Round Key: " + xi << endl;
    			}
    		}
            //xi is split into (number of s boxes) equal pieces, each with Round key size/number of S boxes bits
            //concat *desr_outputs from s-boxes
            string yi = "";
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
    			
    			//size of *desr_output of sbox is the number of bits needed to represent all the entries
    			int output_size = log( sbox_width ) / log( 2 );

    			yi += decToBin( sboxes[i][rowbitdec][colbitdec], output_size );
    		}
    		
    		if ( showstp ) {
    			if ( hex_rep ) {
    				*desr_out << "Sbox result: " << binToHex(yi) << endl;
    			} else {
    				*desr_out << "Sbox result: " << yi << endl;
    			}
    		}
            //The concatenated *desr_output from the T S-boxes, Yi, is then transposed using the P-box permutation
            string ui = permute(yi, pbox_trans_perm_vec);
            // Ui, which is then XORed with L0 to form R1.
            string temp = right; //save value of R0
            right = XOR(left, ui);
            //L1 = R0
            left = temp;
            if ( showstp ) {
    			if ( hex_rep ) {
    				*desr_out << "Left: " + binToHex(left) << endl;
    				*desr_out << "Right: " + binToHex(right) << endl;
    			} else {
    				*desr_out << "Left: " + left << endl;
    				*desr_out << "Right: " + right << endl;
    			}
    		}
        } // end of rounds loop
        //After the final round, the left and right halves are swapped and the inverse initial permutation is applied to form the ciphertext C
        string temp = right;
        right = left;
        left = temp;
        string final = permute(left + right, init_permute_vec_inverse);
        if ( hex_rep ) {
    		*desr_out << binToHex(final);
            combined_blocks_result += binToHex(final);
    	} else {
    		*desr_out << final;
            combined_blocks_result += final;
    	}
    }//end get blocks
    if ( showstp ) {
        *desr_out << "\n\nFinal result through all blocks: " << combined_blocks_result << endl;
    }
    if ( using_stdout ) *desr_out << endl;
    if (fout.is_open()) fout.close();
    if ( !using_stdout && !hex_rep && !showstp ) //convert outputted ASCII binary to regular binary
    {
        string temp;
        ifstream file (output_file_str);
        if(file.is_open()){
            string line;
            while ( getline(file, line) )
            {
                temp += line;
            }
            file.close();
        }
        ofstream outfile(output_file_str, ios::binary);
        if (outfile)
        {
            for (unsigned int i = 0; i < temp.size(); i += 8)
            {
                string temp2 = temp.substr(i, 8);
                char byte = (char)binToDec(temp2);
                outfile.write(&byte,1);
            }
            outfile.close();
        }
    }
    return 0;
}
