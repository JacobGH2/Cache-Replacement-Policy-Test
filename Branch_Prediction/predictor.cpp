#include<iostream>
#include<fstream>
#include<sstream>
#include<string>

using namespace std;

string fileName;
//ofstream outfile;

void outputResult(int correct, int branches, ofstream &out) {
  out << correct << "," << branches << ";";
}

void alwaysTaken(ofstream &out) {
  int numCorrect = 0, numBranches = 0;
  unsigned long long addr;
  string behavior;
  unsigned long long target;
  ifstream infile(fileName);
  string line;
  while (getline(infile, line)) {
    numBranches++;
    istringstream iss(line);
    iss >> std::hex >> addr >> behavior >> std::hex >> target;
  
    if (behavior == "T") {numCorrect++;} 
  }
  outputResult(numCorrect, numBranches, out);
}

void alwaysNotTaken(ofstream &out) {
  int numCorrect = 0, numBranches = 0;
  unsigned long long addr;
  string behavior;
  unsigned long long target;
  ifstream infile(fileName);
  string line;
  while (getline(infile, line)) {
    numBranches++;
    istringstream iss(line);
    iss >> std::hex >> addr >> behavior >> std::hex >> target;
    if (behavior == "NT") {numCorrect++;} 
  }
  outputResult(numCorrect, numBranches, out);
}

int getMask(int size) {
  int returnMask = 0;
  switch(size) {
    case 16:
      returnMask = 0xf;
      break;
    case 32:
      returnMask = 0x1f;
      break;
    case 128:
      returnMask = 0x7f;
      break;
    case 256:
      returnMask = 0xff;
      break;
    case 512:
      returnMask = 0x1ff;
      break;
    case 1024:
      returnMask = 0x3ff;
      break;
    case 2048:
      returnMask = 0x7ff;

  }
  return returnMask;
}

void oneBitBimodal(int tableSize, ofstream &out) {
  
  int numCorrect = 0, numBranches = 0;
  long long addr;
  string behavior;
  unsigned long long target;
  ifstream infile(fileName);
  string line;
  int table[tableSize];
  for (int i = 0; i < tableSize; i++) {
    table[i] = 1;
  }
  
  while (getline(infile, line)) {
    numBranches++;
    // getting index from address
    istringstream iss(line);
    iss >> std::hex >> addr >> behavior >> std::hex >> target;
    int mask = getMask(tableSize);
   
    int index = mask & addr;
    
    //setting behavior
    string predictedBehavior;
    if (table[index] == 1) {
      predictedBehavior = "T";
    } else {
      predictedBehavior = "NT";
    }
    if (predictedBehavior == behavior) {
      numCorrect++;
      //cout << "correct" << endl;
    } else {
      //cout << "incorrect" << endl;
      if (table[index] == 1) {
        table[index] = 0;
      } else {
        table[index] = 1;
      }
    }
  }
  outputResult(numCorrect, numBranches, out);
  if (tableSize != 2048) {
    out << " ";
  }
}

void twoBitBimodal(int tableSize, ofstream &out) {
  int numCorrect = 0, numBranches = 0;
  long long addr;
  string behavior;
  unsigned long long target;
  ifstream infile(fileName);
  string line;
  int table[tableSize];
  for (int i = 0; i < tableSize; i++) {
    table[i] = 4; // TT
  }
  
  while (getline(infile, line)) {
    numBranches++;
    // getting index from address
    istringstream iss(line);
    iss >> std::hex >> addr >> behavior >> std::hex >> target;
    int mask = getMask(tableSize);
   
    int index = mask & addr;
    
    //setting behavior
    string predictedBehavior;
    if (table[index] == 3 || table[index] == 4) {
      predictedBehavior = "T";
    } else {
      predictedBehavior = "NT";
    }
    if (predictedBehavior == behavior) {
      numCorrect++; 
      if (table[index] == 3) {table[index]++;}
      if (table[index] == 2) {table[index]--;}
      //cout << "correct" << endl;
    } else {
      //cout << "incorrect" << endl;
      if (table[index] == 1 || table[index] == 2) {
        table[index]++;;
      } else {
        table[index]--;
      }
    }
  }
  outputResult(numCorrect, numBranches, out);
  if (tableSize != 2048) {
    out << " ";
  }
}

int getGHRSizeMask(int size) {
  int returnMask = 0b1;
  for (int i = 1; i < size; i++) {
    returnMask = (returnMask << 1) | 0b1;
    
  }
  return returnMask;
}

void gshare(int tableSize, int GHRSize, ofstream &out) {
  
  // declaring temporaries
  int numCorrect = 0, numBranches = 0;
  long long addr;
  string behavior;
  unsigned long long target;
  ifstream infile(fileName);
  string line;
  
  // initialize table
  int table[tableSize];
  for (int i = 0; i < tableSize; i++) {
    table[i] = 4; // TT
  }
  // ghr info
  unsigned int GHR = 0;
  unsigned int GHRSizeMask = getGHRSizeMask(GHRSize);
  unsigned int mask = getMask(tableSize);
  
  
  
  while (getline(infile, line)) {
    numBranches++;
    
    // getting index from address
    istringstream iss(line);
    iss >> std::hex >> addr >> behavior >> std::hex >> target;
    
    unsigned int indexWithoutGHR = mask & addr;
    //cout << "before: " << indexWithoutGHR << endl;
    //cout << "GHR: " << GHR << endl;
    unsigned int index = indexWithoutGHR ^ GHR;
    
    //cout << "after: " << index << endl;
    unsigned int behaviorMask = 0;
    //setting behavior
    string predictedBehavior;
    if (table[index] == 3 || table[index] == 4) {
      predictedBehavior = "T";
      
    } else {
      predictedBehavior = "NT";
    }
    if (predictedBehavior == behavior) {
      numCorrect++; 
      if (table[index] == 3) {table[index]++;}
      if (table[index] == 2) {table[index]--;}
      //cout << "correct" << endl;
    } else {
      //cout << "incorrect" << endl;
      if (table[index] == 1 || table[index] == 2) {
        table[index]++;
      } else {
        table[index]--;
      }
    }
    
    if (behavior == "T") {
      behaviorMask = 0b1;
    }
    
    GHR = (GHRSizeMask & (GHR << 1)) ^ behaviorMask;
    // cout << GHR << endl;
    // keep experimenting with thiszx
  }
  outputResult(numCorrect, numBranches, out);
  if (GHRSize != 11) {
    out << " ";
  }
}

void tournament(int tournamentSize, int bimodalSize, int gshareSize, int GHRSize, ofstream &out) {
  int numCorrect = 0, numBranches = 0;
  long long addr;
  string behavior;
  unsigned long long target;
  ifstream infile(fileName);
  string line;
  
  int tournamentTable[tournamentSize];
  for (int i = 0; i < tournamentSize; i++) {
    tournamentTable[i] = 1; // PG
  }

  int bimodalTable[bimodalSize];
  for (int i = 0; i < bimodalSize; i++) {
    bimodalTable[i] = 4; // TT
  }
  
  int gshareTable[gshareSize];
  for (int i = 0; i < gshareSize; i++) {
    gshareTable[i] = 4; // TT
  }
  int GHR = 0b000;
  int GHRSizeMask = getGHRSizeMask(GHRSize);

  while (getline(infile, line)) {
    numBranches++;
    istringstream iss(line);
    iss >> std::hex >> addr >> behavior >> std::hex >> target;
    int mask = getMask(bimodalSize);
    int bimodalIndex = mask & addr;
    int tournamentIndex = bimodalIndex;
    int gshareIndex = bimodalIndex ^ GHR;


    // set behaviors
    string predictedBehavior, gshareBehavior, bimodalBehavior;
    if (gshareTable[gshareIndex] == 1 || gshareTable[gshareIndex] == 2) {
        gshareBehavior = "NT";
      } else {
        gshareBehavior = "T";
      }
    
    if (bimodalTable[bimodalIndex] == 1 || bimodalTable[bimodalIndex] == 2) {
        bimodalBehavior = "NT";
      } else {
        bimodalBehavior = "T";
      }
    

    // select behavior
    if (tournamentTable[tournamentIndex] == 1 || tournamentTable[tournamentIndex] == 2) {
      // gshare
      predictedBehavior = gshareBehavior;
    } else {
      // bimodal
      predictedBehavior = bimodalBehavior;
    }

    if (predictedBehavior == behavior) {
      numCorrect++;
    }
    
    // updating gshare
    if (gshareBehavior == behavior) {
      if (gshareTable[gshareIndex] == 3) {gshareTable[gshareIndex]++;}
      if (gshareTable[gshareIndex] == 2) {gshareTable[gshareIndex]--;}
      //cout << "correct" << endl;
    } else {
      //cout << "incorrect" << endl;
      if (gshareTable[gshareIndex] == 1 || gshareTable[gshareIndex] == 2) {
        gshareTable[gshareIndex]++;;
      } else {
        gshareTable[gshareIndex]--;
      }
    }
    int behaviorMask = 0;
    if (behavior == "T") {
      behaviorMask = 0b001;
    }
    GHR = (GHRSizeMask & (GHR << 1)) ^ behaviorMask;  


    // updating bimodal
    if (bimodalBehavior == behavior) {
      if (bimodalTable[bimodalIndex] == 3) {bimodalTable[bimodalIndex]++;}
      if (bimodalTable[bimodalIndex] == 2) {bimodalTable[bimodalIndex]--;}
      //cout << "correct" << endl;
    } else {
      //cout << "incorrect" << endl;
      if (bimodalTable[bimodalIndex] == 1 || bimodalTable[bimodalIndex] == 2) {
        bimodalTable[bimodalIndex]++;;
      } else {
        bimodalTable[bimodalIndex]--;
      }
    }

    // updating tournament
    if (bimodalBehavior != gshareBehavior) {
      if (bimodalBehavior == behavior) {
        if (tournamentTable[tournamentIndex] == 1 || tournamentTable[tournamentIndex] == 2 || tournamentTable[tournamentIndex] == 3) {
          tournamentTable[tournamentIndex]++;
        }
      } else if (gshareBehavior == behavior) {
        if (tournamentTable[tournamentIndex] == 2 || tournamentTable[tournamentIndex] == 3 || tournamentTable[tournamentIndex] == 4) {
          tournamentTable[tournamentIndex]--;
        }
      }
    }
  }
  outputResult(numCorrect, numBranches, out);
}

void BTB(int tableSize, int BTBSize, ofstream &out) {
  
  int numCorrect = 0, numPredictions = 0;
  
  long long addr;
  string behavior;
  unsigned long long target;
  ifstream infile(fileName);
  string line;
  
  long table[tableSize];
  for (int i = 0; i < tableSize; i++) {
    table[i] = 1;
  }
  
  unsigned long long BTBtable[BTBSize];
  
  while (getline(infile, line)) {
    
    // getting index from address
    istringstream iss(line);
    iss >> std::hex >> addr >> behavior >> std::hex >> target;
    int mask = getMask(tableSize);
    int index = mask & addr;

    
    
    
    //setting behavior
    string predictedBehavior;
    if (table[index] == 1) {
      predictedBehavior = "T";
      numPredictions++;
      if (BTBtable[index] == target) {
        numCorrect++;
      }
    } else {
      predictedBehavior = "NT";
    }

    if (behavior == "T"){
      BTBtable[index] = target;
    }

    if (predictedBehavior == behavior) {
    } else {
      if (table[index] == 1) {
        table[index] = 0;
      } else {
        table[index] = 1;
      }
    }
  }
  outputResult(numCorrect, numPredictions, out);
}

int main(int argc, char *argv[]) {
  fileName = argv[1];
  ofstream outfile(argv[2], std::ofstream::out);
  

  alwaysTaken(outfile);
  outfile << endl;
 
  alwaysNotTaken(outfile);
  outfile << endl;
  
  
  oneBitBimodal(16, outfile);
  oneBitBimodal(32, outfile);
  oneBitBimodal(128, outfile);
  oneBitBimodal(256, outfile);
  oneBitBimodal(512, outfile);
  oneBitBimodal(1024, outfile);
  oneBitBimodal(2048, outfile);
  outfile << endl;
  
  twoBitBimodal(16, outfile);
  twoBitBimodal(32, outfile);
  twoBitBimodal(128, outfile);
  twoBitBimodal(256, outfile);
  twoBitBimodal(512, outfile);
  twoBitBimodal(1024, outfile);
  twoBitBimodal(2048, outfile);
  outfile << endl;
  
  gshare(2048,3, outfile);
  gshare(2048,4, outfile);
  gshare(2048,5, outfile);
  gshare(2048,6, outfile);
  gshare(2048,7, outfile);
  gshare(2048,8, outfile);
  gshare(2048,9, outfile);
  gshare(2048,10, outfile);
  gshare(2048,11, outfile);
  outfile << endl;


  // unsure if gshare is working, no change in outputs (could be expected)
  // tournament depends on gshare working correctly

  tournament(2048, 2048, 2048, 11, outfile);
  outfile << endl;
 

  BTB(512, 512, outfile);
  outfile << endl;


  outfile.close();
  return 0;
}