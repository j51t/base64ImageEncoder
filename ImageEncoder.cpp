#include <iostream>
#include <fstream>
#include <string>
#include <bitset>
#include <istream>
#include "LisnrImageEncoder.h"

using namespace std;

int main(int argc, char *argv[]) {
    if (argc != 3) {
        cerr << "Error, number of arguments should be 2 but was " << argc - 1 << endl;
        return -1;
    }

    cout << "Beginning encode, this may take a moment" << endl;
    LisnrImageEncoder::encode(argv[1],argv[2]);
    cout << "Done!" << endl;
}