#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
using namespace std;

string fileName;

void outputResult(int numHits, int numAccesses, ofstream &out) {
    out << numHits << "," << numAccesses << ";";
}

void directMapped(int cacheSize, ofstream &out){
    int lineSize = 32;
    int numIndices = cacheSize/32;
    int cache[numIndices][2];
    
    char action;
    unsigned long long target;

    ifstream infile(fileName);
    string line;
    cout << "before while" << endl;
    while (getline(infile, line)) {
        cout << "in while" << endl;
        istringstream iss(line);
        iss >> action >> std::hex >> target;
        out << action << " " << std::hex << target << endl;
    }
    cout << "after while" << endl;
}

int main(int argc, char *argv[]) {
    
    fileName = argv[1]; 
    ofstream outfile(argv[2], std::ofstream::out);
    
    
    directMapped(1000, outfile);
    //directMapped(4000, outfile);
    //directMapped(16000, outfile);
    //directMapped(32000, outfile);
    outfile << endl;


    outfile.close();

   return 0;
}