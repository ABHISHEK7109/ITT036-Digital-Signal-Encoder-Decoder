#include<iostream>
#include<string>
#include<vector>
#include<algorithm>
#include<random>
#include<cstdlib>
#include<ctime>
using namespace std;

string makeRandomBits(int length){
    string bits=" ";

    for(int i =0;i<length;i++){
        int bit=rand()% 2;
        if(bit==0)
        bits +="0 ";
        else
        bits +="1 ";
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