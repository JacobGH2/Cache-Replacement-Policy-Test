#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <cmath>
#include <algorithm>
#include <vector>
#include <list>
#include <bitset>
using namespace std;

string fileName;
int powers[] = {2,4,8,16,32,64,128,256,512,1024,2048};
int lineSize = 32;

void outputResult(int numHits, int numAccesses, ofstream &out) {
    out << numHits << "," << numAccesses << "; ";
}

void makePowerOf2(int &num) {
    while (1) {
        for (int i = 0; i < sizeof(powers)/sizeof(powers[0]); i++) {
            if (num == powers[i]) {
                return;
            }
        }
        num++;
    }
}

void directMapped(int cacheSize, ofstream &out){
    
    int numIndices = cacheSize/lineSize;
    makePowerOf2(numIndices);
    int cache[numIndices][2];

    for (int i = 0; i < numIndices; i++) {
        cache[i][0] = 0;
    }
    
    int hits = 0, accesses = 0;
   
    char action;
    unsigned long long target;

    ifstream infile(fileName);
    string line;
    
    int shiftAmt1 = log2(lineSize);
    int shiftAmt2 = log2(numIndices);

    while (getline(infile, line)) {
        accesses++;
        istringstream iss(line);
        iss >> action >> std::hex >> target;
        // discard byte offset
        target = target >> shiftAmt1;
        int index = target % numIndices;
        
        int targetTag = target >> shiftAmt2;

        int validBitInCache = cache[index][0];
        int tagInCache = cache[index][1];

        if (action == 'L') {
            // load
            if (validBitInCache == 1 && tagInCache == targetTag) {
                hits++;
                // hit
            } else if(validBitInCache == 0) {
                // miss because invalid
                cache[index][0] = 1;
                cache[index][1] = targetTag;
            } else if (tagInCache != targetTag) {
                // miss because occupied
                cache[index][1] = targetTag; 
            }
        } else {
            // store
            // miss if index invalid or wrong tag
            if (validBitInCache == 1 && tagInCache == targetTag) {
                hits++;
            } else {
                cache[index][0] = 1;
                cache[index][1] = targetTag;
            }
        } 
    }
    outputResult(hits, accesses, out);
}

int getIndexInVector(int data, vector<int> vec) {
    for (int i = 0; i < vec.size(); i++) {
        if (data == vec.at(i)) {
            return i;
        }
    }
    return -1;
}

void setAssociative(int cacheSize, int numSets, ofstream &out) {
    int hits = 0, accesses = 0;

    int numIndices = cacheSize/(lineSize * numSets);
    makePowerOf2(numIndices);

    int cache[numIndices][numSets][2];

    for (int i = 0; i < numIndices; i++) {
        for (int j = 0; j < numSets; j++) {
            cache[i][j][0] = 0;
        }
    }

    char action;
    unsigned long long target;

    ifstream infile(fileName);
    string line;

    int shiftAmt1 = log2(lineSize);
    int shiftAmt2 = log2(numIndices);

    // linked list LRU Buffer
    list<int> LRUBuf[numIndices];

    newLine:
    while (getline(infile, line)) {
        accesses++;
        istringstream iss(line);
        iss >> action >> std::hex >> target;
        // discard byte offset
        target = target >> shiftAmt1;
        int index = target % numIndices;

        int targetTag = target >> shiftAmt2;

        if (action == 'L') {
            // load
            // check for hit
            for (int i = 0; i < numSets; i++) {
                if (cache[index][i][0] == 1 && cache[index][i][1] == targetTag) {
                    hits++;
                    // if setIndex is in LRU, erase it, then add at the back (stores check the front)
                    LRUBuf[index].remove(i);
                    LRUBuf[index].push_back(i);
                    //cout << "LRU Index: " << index << " size: " << LRUBuf[index].size() << endl;
                    goto newLine;
                }
            }
            // check for open spot
            for (int i = 0; i < numSets; i++) {
                if (cache[index][i][0] == 0) {
                    // not hit
                    cache[index][i][0] = 1;
                    cache[index][i][1] = targetTag;

                    LRUBuf[index].remove(i);
                    LRUBuf[index].push_back(i);
                    
                    goto newLine;
                }
            }
            // no hit or open spot, must evict
            int leastRecentIndex = LRUBuf[index].front();
            cache[index][leastRecentIndex][1] = targetTag;
            LRUBuf[index].pop_front();
            LRUBuf[index].push_back(leastRecentIndex);
            
            goto newLine;
        } else {
            // store
            // in block that already has tag, hit
            for (int i = 0; i < numSets; i++) {
                if (cache[index][i][0] == 1 && cache[index][i][1] == targetTag) {
                    hits++;
                    
                    LRUBuf[index].remove(i);
                    LRUBuf[index].push_back(i);
                   
                    goto newLine;
                }
            }
            // check for open spots to store in, hit
            for (int i = 0; i < numSets; i++) {
                if (cache[index][i][0] == 0) {
                    
                    cache[index][i][0] = 1;
                    cache[index][i][1] = targetTag;
                    
                    LRUBuf[index].remove(i);
                    LRUBuf[index].push_back(i);

                    goto newLine;
                }
            }
            // no open spots, must evict
            int leastRecentIndex = LRUBuf[index].front();
            cache[index][leastRecentIndex][1] = targetTag;
            LRUBuf[index].pop_front();
            LRUBuf[index].push_back(leastRecentIndex);
            goto newLine;
        }     
    }
    outputResult(hits, accesses, out);
}

void fullyAssociativeLRU(int cacheSize, ofstream &out) {
    int hits = 0, accesses = 0;                     // SETUP
    int numIndices = cacheSize/lineSize;
    makePowerOf2(numIndices);
    int cache[numIndices][2];
    char action;
    unsigned long long target;
    ifstream infile(fileName);
    string line;
    int shiftAmt1 = log2(lineSize);
    list<int> LRUBuf = list<int>();             // SINGE LINKED LIST BUFFER
    
    newLine:
    while (getline(infile, line)) {
        accesses++;
        istringstream iss(line);
        iss >> action >> std::hex >> target;
        int targetTag = target >> shiftAmt1;
        
        if (action == 'L') {                        // LOAD
            for (int i = 0; i < numIndices; i++) {
                if (cache[i][0] == 1 && cache[i][1] == targetTag) {
                    hits++;
                    LRUBuf.remove(i);
                    LRUBuf.push_back(i);
                    goto newLine;
                }
            }
            for (int i = 0; i < numIndices; i++) {
                if (cache[i][0] == 0) {
                    cache[i][0] = 1;
                    cache[i][1] = targetTag;
                    LRUBuf.remove(i);
                    LRUBuf.push_back(i);
                    goto newLine;
                }
            }
            int leastRecentIndex = LRUBuf.front();          // EVICTION STEP, ONLY REACHED IF NO HITS OR EMPTY SPOTS
            cache[leastRecentIndex][0] = 1;
            cache[leastRecentIndex][1] = targetTag;
            LRUBuf.pop_front();
            LRUBuf.push_back(leastRecentIndex);
            goto newLine;
        } else {
            for (int i = 0; i < numIndices; i++) {                  // STORE
                if (cache[i][0] == 1 && cache[i][1] == targetTag) {
                    hits++;
                    LRUBuf.remove(i);
                    LRUBuf.push_back(i);
                    goto newLine;
                }
            } 
            for (int i = 0; i < numIndices; i++) {
                if (cache[i][0] == 0) {
                    hits++;
                    cache[i][0] = 1;
                    cache[i][1] = targetTag;
                    LRUBuf.remove(i);
                    LRUBuf.push_back(i);
                    goto newLine;
                }
            }
            // evict
            int leastRecentIndex = LRUBuf.front();
            cache[leastRecentIndex][0] = 1;
            cache[leastRecentIndex][1] = targetTag;
            LRUBuf.pop_front();
            LRUBuf.push_back(leastRecentIndex);
        }
    }
    outputResult(hits, accesses, out);
}

int getIndexFromBitset(bitset<511> &set) {
    // helper for pseudoLRU
    int currBit = 0;
    int index = 0;
    while (index < 512 && currBit < 512) {
        currBit = currBit*2 + set[index]; // keep adding to currBit to get LRU
        index = index*2 + 1 + set[index]; // needed for traversal through the tree
    }
    return currBit;
}

void updateLRUBitset(bitset<511> &set, int MRUIndex) {
    // helper for pseudoLRU
    bitset<9> numBit(MRUIndex);
    int numIndex = 8;
    int bitsetIndex = 0;
    while (numIndex >= 0) {
        if (numBit[numIndex] == 0) {
            //cout << "bitset index: " << bitsetIndex << endl;
            set[bitsetIndex] = 1;
            bitsetIndex = bitsetIndex*2 + 2;
        } else {
            set[bitsetIndex] = 0;
            bitsetIndex = bitsetIndex*2 +1;
        }
        numIndex--;
    }
}

void fullyAssociativeHotCold(int cacheSize, ofstream &out) {
    int hits = 0, accesses = 0;

    int numIndices = cacheSize/lineSize;
    makePowerOf2(numIndices);
    //cout << "Num indices: " << numIndices << endl;
    int cache[numIndices][2];

    std::bitset<511> LRUBit;

    char action;
    unsigned long long target;
    ifstream infile(fileName);
    string line;
    int shiftAmt1 = log2(lineSize);
    //list<int> LRUBuf = list<int>();
    //cout << "lrubuf initial size: " << LRUBuf.size();

    newLine:
    while (getline(infile, line)) {
        
        accesses++;
        istringstream iss(line);
        iss >> action >> std::hex >> target;

        int targetTag = target >> shiftAmt1;

        if (action == 'L') {
            for (int i = 0; i < numIndices; i++) {
                if (cache[i][0] == 1 && cache[i][1] == targetTag) {
                    hits++;
                    //cout << "lru size after hit: " << LRUBuf.size() << endl;
                    updateLRUBitset(LRUBit, i);
                    goto newLine;
                }
            }
            // no matches, insert into cache using pLRU

            
            int leastRecentIndex = getIndexFromBitset(LRUBit);
            cout << "LRU Load: " << leastRecentIndex << endl;
            cache[leastRecentIndex][0] = 1;
            cache[leastRecentIndex][1] = targetTag;
            
            updateLRUBitset(LRUBit, leastRecentIndex);
            goto newLine;
        } else {
            for (int i = 0; i < numIndices; i++) {
                if (cache[i][0] == 1 && cache[i][1] == targetTag) {
                    hits++;
                    updateLRUBitset(LRUBit, i);
                    goto newLine;
                }
            } 
            
            // no hit, use pLRU from bitset

            int leastRecentIndex = getIndexFromBitset(LRUBit);
            cout << "LRU Store: " << leastRecentIndex << endl;
            cache[leastRecentIndex][0] = 1;
            cache[leastRecentIndex][1] = targetTag;
            updateLRUBitset(LRUBit, leastRecentIndex);
        }
    }
    //cout << "final lru size: " << LRUBuf.size() << endl;
    outputResult(hits, accesses, out);
}

void setAssociativeNoAlloc(int cacheSize, int numSets, ofstream &out) {
    int hits = 0, accesses = 0;

    int numIndices = cacheSize/(lineSize * numSets);
    makePowerOf2(numIndices);

    int cache[numIndices][numSets][2];

    for (int i = 0; i < numIndices; i++) {
        for (int j = 0; j < numSets; j++) {
            cache[i][j][0] = 0;
        }
    }

    char action;
    unsigned long long target;

    ifstream infile(fileName);
    string line;

    int shiftAmt1 = log2(lineSize);
    int shiftAmt2 = log2(numIndices);

    // linked list LRU Buffer
    list<int> LRUBuf[numIndices];

    newLine:
    while (getline(infile, line)) {
        accesses++;
        istringstream iss(line);
        iss >> action >> std::hex >> target;
        // discard byte offset
        target = target >> shiftAmt1;
        int index = target % numIndices;

        int targetTag = target >> shiftAmt2;

        if (action == 'L') {
            // load
            // check for hit
            for (int i = 0; i < numSets; i++) {
                if (cache[index][i][0] == 1 && cache[index][i][1] == targetTag) {
                    hits++;
                    // if setIndex is in LRU, erase it, then add at the back (stores check the front)
                    LRUBuf[index].remove(i);
                    LRUBuf[index].push_back(i);
                    //cout << "LRU Index: " << index << " size: " << LRUBuf[index].size() << endl;
                    goto newLine;
                }
            }
            // check for open spot
            for (int i = 0; i < numSets; i++) {
                if (cache[index][i][0] == 0) {
                    // not hit
                    cache[index][i][0] = 1;
                    cache[index][i][1] = targetTag;

                    LRUBuf[index].remove(i);
                    LRUBuf[index].push_back(i);
                    
                    goto newLine;
                }
            }
            // no hit or open spot, must evict
            int leastRecentIndex = LRUBuf[index].front();
            cache[index][leastRecentIndex][1] = targetTag;
            LRUBuf[index].pop_front();
            LRUBuf[index].push_back(leastRecentIndex);
            
            goto newLine;
        } else {
            // store
            // in block that already has tag, hit
            for (int i = 0; i < numSets; i++) {
                if (cache[index][i][0] == 1 && cache[index][i][1] == targetTag) {
                    hits++;
                    
                    LRUBuf[index].remove(i);
                    LRUBuf[index].push_back(i);
                   
                    goto newLine;
                }
            }
        }     
    }
    outputResult(hits, accesses, out);
}

void setAssociativeNLP(int cacheSize, int numSets, ofstream &out) {
    int hits = 0, accesses = 0;

    int numIndices = cacheSize/(lineSize * numSets);
    makePowerOf2(numIndices);

    int cache[numIndices][numSets][2];

    for (int i = 0; i < numIndices; i++) {
        for (int j = 0; j < numSets; j++) {
            cache[i][j][0] = 0;
        }
    }

    char action;
    unsigned long long target;

    ifstream infile(fileName);
    string line;

    int shiftAmt1 = log2(lineSize);
    int shiftAmt2 = log2(numIndices);

    // linked list LRU Buffer
    list<int> LRUBuf[numIndices];

    int missesDebug = 0;

    newLine:
    while (getline(infile, line)) {
        accesses++;
        istringstream iss(line);
        iss >> action >> std::hex >> target;
        // discard byte offset
        target = target >> shiftAmt1;
        int index = target % numIndices;
        int index2 = (target + 1) % numIndices;
        
        int targetTag = target >> shiftAmt2;

        if (action == 'L') {
            // load
            // check for hit
            for (int i = 0; i < numSets; i++) {
                if (cache[index][i][0] == 1 && cache[index][i][1] == targetTag) {
                    hits++;
                    // if setIndex is in LRU, erase it, then add at the back (stores check the front)
                    LRUBuf[index].remove(i);
                    LRUBuf[index].push_back(i);
                    //cout << "LRU Index: " << index << " size: " << LRUBuf[index].size() << endl;
                    goto prefetch;
                }
            }
            // check for open spot
            for (int i = 0; i < numSets; i++) {
                if (cache[index][i][0] == 0) {
                    // not hit
                    cache[index][i][0] = 1;
                    cache[index][i][1] = targetTag;

                    LRUBuf[index].remove(i);
                    LRUBuf[index].push_back(i);
                    
                    goto prefetch;
                }
            }
            // no hit or open spot, must evict
            int leastRecentIndex = LRUBuf[index].front();
            cache[index][leastRecentIndex][0] = 1;
            cache[index][leastRecentIndex][1] = targetTag;
            //cout << "index " << index << endl;
            
            LRUBuf[index].remove(leastRecentIndex);
            LRUBuf[index].push_back(leastRecentIndex);
            // error would not occur before, because there was always 
            
            goto prefetch;
        } else {
            // store
            // in block that already has tag, hit
            for (int i = 0; i < numSets; i++) {
                if (cache[index][i][0] == 1 && cache[index][i][1] == targetTag) {
                    hits++;
                    
                    LRUBuf[index].remove(i);
                    LRUBuf[index].push_back(i);
                   
                    goto prefetch;
                    
                }
            }
            // check for open spots to store in, hit
            for (int i = 0; i < numSets; i++) {
                if (cache[index][i][0] == 0) {
                    hits++;
                    cache[index][i][0] = 1;
                    cache[index][i][1] = targetTag;
                    
                    LRUBuf[index].remove(i);
                    LRUBuf[index].push_back(i);

                    goto prefetch;
                   
                }
            }
            // no open spots, must evict
            int leastRecentIndex = LRUBuf[index].front();
            cache[index][leastRecentIndex][1] = targetTag;
         
            LRUBuf[index].remove(leastRecentIndex);
            LRUBuf[index].push_back(leastRecentIndex);
            
            goto prefetch;
            
        }
        prefetch:           // PREFETCH STEP
        for (int i = 0; i < numSets; i++) {
                if (cache[index2][i][0] == 1 && cache[index2][i][1] == targetTag) {
                    
                    // if setIndex is in LRU, erase it, then add at the back (stores check the front)
                    
                    LRUBuf[index2].remove(i);
                    LRUBuf[index2].push_back(i);
                    
                    //cout << "LRU Index: " << index << " size: " << LRUBuf[index].size() << endl;
                    goto newLine;
                }
        }
        // check for open spot
        for (int i = 0; i < numSets; i++) {
            if (cache[index2][i][0] == 0) {
                // not hit
                cache[index2][i][0] = 1;
                cache[index2][i][1] = targetTag;

                
                LRUBuf[index2].remove(i);
                LRUBuf[index2].push_back(i);
                goto newLine;
            }
        }
        // no hit or open spot, must evict
        
        
        
        int leastRecentIndex = LRUBuf[index2].front();
        cache[index2][leastRecentIndex][0] = 1;
        cache[index2][leastRecentIndex][1] = targetTag;
        LRUBuf[index2].remove(leastRecentIndex);
        LRUBuf[index2].push_back(leastRecentIndex);
        
        goto newLine;
             
    }
    outputResult(hits, accesses, out);

}

void setAssociativeNLPOnMiss(int cacheSize, int numSets, ofstream &out) {
    int hits = 0, accesses = 0;

    int numIndices = cacheSize/(lineSize * numSets);
    makePowerOf2(numIndices);

    int cache[numIndices][numSets][2];

    for (int i = 0; i < numIndices; i++) {
        for (int j = 0; j < numSets; j++) {
            cache[i][j][0] = 0;
        }
    }

    char action;
    unsigned long long target;

    ifstream infile(fileName);
    string line;

    int shiftAmt1 = log2(lineSize);
    int shiftAmt2 = log2(numIndices);

    // linked list LRU Buffer
    list<int> LRUBuf[numIndices];

    newLine:
    while (getline(infile, line)) {
        accesses++;
        istringstream iss(line);
        iss >> action >> std::hex >> target;
        // discard byte offset
        target = target >> shiftAmt1;
        int index = target % numIndices;
        int index2 = (target + 1) % numIndices;
        
        int targetTag = target >> shiftAmt2;

        if (action == 'L') {
            // load
            // check for hit
            for (int i = 0; i < numSets; i++) {
                if (cache[index][i][0] == 1 && cache[index][i][1] == targetTag) {
                    hits++;
                    // if setIndex is in LRU, erase it, then add at the back (stores check the front)
                    LRUBuf[index].remove(i);
                    LRUBuf[index].push_back(i);
                    //cout << "LRU Index: " << index << " size: " << LRUBuf[index].size() << endl;
                    goto newLine;
                }
            }
            // check for open spot
            for (int i = 0; i < numSets; i++) {
                if (cache[index][i][0] == 0) {
                    // not hit
                    cache[index][i][0] = 1;
                    cache[index][i][1] = targetTag;

                    LRUBuf[index].remove(i);
                    LRUBuf[index].push_back(i);
                    
                    goto prefetch;
                }
            }
            // no hit or open spot, must evict
            int leastRecentIndex = LRUBuf[index].front();
            cache[index][leastRecentIndex][0] = 1;
            cache[index][leastRecentIndex][1] = targetTag;
            //cout << "index " << index << endl;
            
            LRUBuf[index].remove(leastRecentIndex);
            LRUBuf[index].push_back(leastRecentIndex);
            // error would not occur before, because there was always 
            
            goto prefetch;
        } else {
            // store
            // in block that already has tag, hit
            for (int i = 0; i < numSets; i++) {
                if (cache[index][i][0] == 1 && cache[index][i][1] == targetTag) {
                    hits++;
                    
                    LRUBuf[index].remove(i);
                    LRUBuf[index].push_back(i);
                   
                    goto newLine;
                    
                }
            }
            // check for open spots to store in, hit
            for (int i = 0; i < numSets; i++) {
                if (cache[index][i][0] == 0) {
                    hits++;
                    cache[index][i][0] = 1;
                    cache[index][i][1] = targetTag;
                    
                    LRUBuf[index].remove(i);
                    LRUBuf[index].push_back(i);

                    goto newLine;
                   
                }
            }
            // no open spots, must evict
            int leastRecentIndex = LRUBuf[index].front();
            cache[index][leastRecentIndex][1] = targetTag;
         
            LRUBuf[index].remove(leastRecentIndex);
            LRUBuf[index].push_back(leastRecentIndex);
            
            goto prefetch;
            
        }
        prefetch:           // PREFETCH STEP
        for (int i = 0; i < numSets; i++) {
                if (cache[index2][i][0] == 1 && cache[index2][i][1] == targetTag) {
                    
                    // if setIndex is in LRU, erase it, then add at the back (stores check the front)
                    
                    LRUBuf[index2].remove(i);
                    LRUBuf[index2].push_back(i);
                    
                    //cout << "LRU Index: " << index << " size: " << LRUBuf[index].size() << endl;
                    goto newLine;
                }
        }
        // check for open spot
        for (int i = 0; i < numSets; i++) {
            if (cache[index2][i][0] == 0) {
                // not hit
                cache[index2][i][0] = 1;
                cache[index2][i][1] = targetTag;

                
                LRUBuf[index2].remove(i);
                LRUBuf[index2].push_back(i);
                goto newLine;
            }
        }
        // no hit or open spot, must evict
        
        
        
        int leastRecentIndex = LRUBuf[index2].front();
        cache[index2][leastRecentIndex][0] = 1;
        cache[index2][leastRecentIndex][1] = targetTag;
        LRUBuf[index2].remove(leastRecentIndex);
        LRUBuf[index2].push_back(leastRecentIndex);
        
        goto newLine;
             
    }
    outputResult(hits, accesses, out);

}

int main(int argc, char *argv[]) {
    
    fileName = argv[1]; 
    ofstream outfile(argv[2], std::ofstream::out);
    
    /*
    directMapped(1000, outfile);
    directMapped(4000, outfile);
    directMapped(16000, outfile);
    directMapped(32000, outfile);
    outfile << endl;
    

    
    setAssociative(16000, 2, outfile);
    setAssociative(16000, 4, outfile);
    setAssociative(16000, 8, outfile);
    setAssociative(16000, 16, outfile);
    outfile << endl;
    
    
    */
    fullyAssociativeLRU(16000, outfile);
    outfile << endl;
    /*

   //fullyAssociativeHotCold(16000, outfile);

    outfile << endl;


    setAssociativeNoAlloc(16000, 2, outfile);
    setAssociativeNoAlloc(16000, 4, outfile);
    setAssociativeNoAlloc(16000, 8, outfile);
    setAssociativeNoAlloc(16000, 16, outfile);
    
    outfile << endl;
   
   setAssociativeNLP(16000, 2, outfile);
   setAssociativeNLP(16000, 4, outfile);
   setAssociativeNLP(16000, 8, outfile);
   setAssociativeNLP(16000, 16, outfile);
    
   outfile << endl;
   setAssociativeNLPOnMiss(16000, 2, outfile);
   setAssociativeNLPOnMiss(16000, 4, outfile);
   setAssociativeNLPOnMiss(16000, 8, outfile);
   setAssociativeNLPOnMiss(16000, 16, outfile);
    */
    outfile.close();

   return 0;
}