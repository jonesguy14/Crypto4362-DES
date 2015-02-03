#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <iterator>
#include <algorithm>
#include <vector>

using namespace std;

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

    bool using_stdin  = false; //if using stdin instead of input file
    bool using_stdout = false; //if using stdout instead of output file

    if ( argc > 7 ) {
        cout << "Too many flags detected." << endl;
        return 1;
    }

    for (int i = 0; i < argc; ++i) { //run through each flag detected
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

    cout << "Done with flags" << endl;

    int block_size, key_size, eff_key_size, rnd_key_size, num_rounds, num_sboxes;
    vector<string> permute_choice_pc1_vec;
    vector<string> permute_choice_pc2_vec;
    vector<int>    rotation_schedule_vec;
    vector<int>    init_permute_vec;
    vector<int>    expan_permute_vec;
    vector<int>    pbox_trans_perm_vec;
    vector<int>    row_selection_vec;
    vector<int>    col_selection_vec;

    //vector to store all the sboxes, will be vector of vector of vectors :^)
    //to retrieve: sboxes[sbox number][sbox row][sbox column]
    vector< vector< vector<int> > > sboxes;
    int sbox_width, sbox_height, line_start_sbox, sbox_count, sbox_row = 0;

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
                        copy( istream_iterator<int>(iss7),
                              istream_iterator<int>(),
                              back_inserter(rotation_schedule_vec) ); }
                        // this should split by spaces and insert items into rotate schedule vector
                        break;
                    case 8: //initial permutation
                        { istringstream iss8( line );
                        copy( istream_iterator<int>(iss8),
                              istream_iterator<int>(),
                              back_inserter(init_permute_vec) ); }
                        // this should split by spaces and insert items into initial permutation vector
                        break;
                    case 9: //expansion permutation
                        { istringstream iss9( line );
                        copy( istream_iterator<int>(iss9),
                              istream_iterator<int>(),
                              back_inserter(expan_permute_vec) ); }
                        // this should split by spaces and insert items into expansion permutation vector
                        break;
                    case 10: //p-box transposition permutation
                        { istringstream iss10( line );
                        copy( istream_iterator<int>(iss10),
                              istream_iterator<int>(),
                              back_inserter(pbox_trans_perm_vec) ); }
                        // this should split by spaces and insert items into pbox vector
                        break;
                    case 11: //number of sboxes
                        num_sboxes = atoi( line.c_str() );
                        break;
                    case 12: //row selection bits
                        { istringstream iss12( line );
                        copy( istream_iterator<int>(iss12),
                              istream_iterator<int>(),
                              back_inserter(row_selection_vec) ); }
                        // this should split by spaces and insert items into row select vector
                        break;
                    case 13: //column selection bits
                        { istringstream iss13( line );
                        copy( istream_iterator<int>(iss13),
                              istream_iterator<int>(),
                              back_inserter(col_selection_vec) ); }
                        // this should split by spaces and insert items into column select vector
                        break;
                } //end switch

                if ( line_counter > 13 ) {
                    //sbox territory
                    sbox_height = 2 ^ (rnd_key_size/num_sboxes - block_size/(2 * num_sboxes)); //num rows
                    sbox_width  = 2 ^ (block_size/(2 * num_sboxes)); //num columns
                    line_start_sbox = 14; //sboxes start on the 14th relevant line in param file

                    sbox_count = (line_counter - line_start_sbox) / sbox_height; //which # sbox we are on
                    sbox_row = sbox_count * sbox_height + (line_counter - line_start_sbox); //which row in that sbox

                    istringstream issbox( line );
                    copy( istream_iterator<int>(issbox),
                          istream_iterator<int>(),
                          back_inserter(sboxes[sbox_count][sbox_row]) );
                    //this should insert the row from the file as a row in the correct sbox

                }

                line_counter++; //increment line counter so we know what is next, will only increment if has data

            }
        }
    }

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


    return 0;
}









