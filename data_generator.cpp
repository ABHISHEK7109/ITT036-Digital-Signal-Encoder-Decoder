#include<iostream>
#include<string>
#include<vector>
#include<algorithm>
#include<random>
#include<cstdlib>
#include<ctime>
using namespace std;
string makeRandomBits(int length) {
    string bits = "";

    for (int i = 0; i < length; i++) {
        bits += to_string(rand() % 2);
        bits += " ";
    }

    return bits;
}

int main(){
     cout<<"Data Generator & Palindrome Finder"<<endl;
     srand(time(0));
     string myBits = makeRandomBits(16);
     cout<<"Generated Bitstream:"<< myBits<<endl;

    return 0;

}