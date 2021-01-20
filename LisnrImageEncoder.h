#pragma once

#include <iostream>
#include <fstream>
#include <string>
#include <bitset>
#include <istream>

using namespace std;

/* Library to turn an image URL into a base64 file

*/
namespace LisnrImageEncoder {
    
    /*  downloads an image from the given url to a temp file

    */
    void downloadImage(string url) {
        string command = "curl ";
        command += url;
        command += " -o temp --silent";
        cout << "downloading image" << endl;
        system(command.c_str());
        cout << "download complete" << endl;
    }
    
    /* Given 2 bitsets, concatenates them such that the returned bitset = b1 concatenate b2
        
    */
    template <size_t N1, size_t N2 > bitset <N1 + N2> concatTwo( const bitset <N1> & b1, const bitset <N2> & b2 ) {
        string s1 = b1.to_string();
        string s2 = b2.to_string();
        return bitset <N1 + N2>( s1 + s2 );
    }


    /* Given 3 bitsets, concatenates them such that the returned bitset = b1 concat b2 concatenate b3
        
    */
    template <size_t N1, size_t N2, size_t N3> bitset <N1 + N2 + N3> concatThree( const bitset <N1> & b1, const bitset <N2> & b2, const bitset <N3> & b3 ) {
        string s1 = b1.to_string();
        string s2 = b2.to_string();
        string s3 = b3.to_string();
        return bitset <N1 + N2 + N3>( s1 + s2 + s3);
    }


    /*

    */
    int encode(char *fIn, char *fOut) {
        // Variable Declaration
        ifstream fileIn; // input image file
        fstream fileOut; // output text file
        size_t fileSize = 0; // size of input file
        int iterations = 0; // number of "clean" iterations that can be complete before end edge case, since 3 Bytes == 4 Base64 characters
        const char BASE64[64] = {'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z', 'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z', '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '+', '/'}; // reference arry of the base64 table
        char const *fileOutName = fOut; // the file to output data to
        char block[3]; // buffer to hold 3 characters (bytes) from the input image 
        double threshhold = 0.05; // percentage threshhold for update messages, so the user knows if it is frozen

        // Download Image
        string url = fIn;
        downloadImage(url);

        // Begin
        // designate and open file streams
        fileIn.open("temp", ios::binary);
        fileOut.open(fileOutName, fstream::out);

        // get length of file
        fileIn.seekg(0,ios::end);
        fileSize = fileIn.tellg();
        fileIn.seekg(0, ios::beg);

        // get number of clean iterations
        iterations = fileSize / 3;
        cout << "encoding" << endl;
        // read in 3 bytes, convert to 4 base64 characters, the write the base64
        for (int i = 0; i < iterations; i++) {
            // go to next 3 unread bytes, read in
            fileIn.seekg(3*i);
            fileIn.read(block,3);

            // convert the 3 bytes into a 24 bit string
            bitset<24> bytes = concatThree(bitset<8>(int(block[0])),bitset<8>(int(block[1])),bitset<8>(int(block[2]))); // 24 bit string to hold the 3 bytes

            // make masks of (W)(X)(Y)(Z) where 1 of W,X,Y,Z is (111111) and the other 3 are (000000)
            bitset<24> maskFirst(16515072); // 111111 000000 000000 000000
            bitset<24> maskSecond(258048); // 000000 111111 000000 000000
            bitset<24> maskThird(4032); // 000000 000000 111111 000000
            bitset<24> maskFourth(63); // 000000 000000 000000 111111

            // logical AND with byte representation, then shift out such that only the 6 LSB can be 1's and the 18 MSB are 0
            maskFirst &= bytes;
            maskFirst >>= 18;

            maskSecond &= bytes;
            maskSecond >>= 12;

            maskThird &= bytes;
            maskThird >>= 6;

            // no need to shift, mask has 1's only in 6LSB
            maskFourth &= bytes;

            // output to file
            fileOut.put(BASE64[maskFirst.to_ullong()]);
            fileOut.put(BASE64[maskSecond.to_ullong()]);
            fileOut.put(BASE64[maskThird.to_ullong()]);
            fileOut.put(BASE64[maskFourth.to_ullong()]);

            if (double(i) / iterations > threshhold) {
                cout << threshhold * 100 << "% complete" << endl;
                threshhold += 0.05;
            }
        }

        // edge case for 1 or 2 remaining bytes, explained on https://en.wikipedia.org/wiki/Base64#Examples

        if ((fileSize % 3) == 2) { // 2 remaining bytes => pad with "00", makes 3 base64 characters

            // read in two bytes
            fileIn.read(block,2);
            bitset<18> bytes = concatThree(bitset<8>(int(block[0])), bitset<8>(int(block[1])), bitset<2>(0));

            // make masks like from before
            bitset<18> maskFirst(258048); // 111111 000000 000000
            bitset<18> maskSecond(4032); // 000000 111111 000000
            bitset<18> maskThird(63); // 000000 000000 111111

            // shift like before
            maskFirst &= bytes;
            maskFirst >>= 12;

            maskSecond &= bytes;
            maskSecond >>= 6;

            maskThird &= bytes;

            // output to file
            fileOut.put(BASE64[maskFirst.to_ullong()]);
            fileOut.put(BASE64[maskSecond.to_ullong()]);
            fileOut.put(BASE64[maskThird.to_ullong()]);
            fileOut.put('=');

        } else if ((fileSize % 3) == 1) { // 1 remaining bytes => pad with "0000", makes 2 base64 characters

            // read in 1 byte
            fileIn.read(block,1);
            bitset<12> bytes = concatTwo(bitset<8>(int(block[0])),bitset<4>(0));

            // make masks like from before
            bitset<12> maskFirst(4032); // 111111 000000
            bitset<12> maskSecond(63); // 000000 111111

            // shift like before
            maskFirst &= bytes;
            maskFirst >>= 6;

            maskSecond &= bytes;

            // output to file
            fileOut.put(BASE64[maskFirst.to_ullong()]);
            fileOut.put(BASE64[maskSecond.to_ullong()]);
            fileOut.put('=');
        } // else there are 0 remaining bytes, so no edge case

        // close file streams
        fileIn.close();
        fileOut.close();

        return 0;
    }
}