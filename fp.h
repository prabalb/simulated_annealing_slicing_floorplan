/*
 * Simulated Annealing Algorithm for Slicing Floorplan
 * AUTHOR: Prabal Basu (A02049867)
 */

#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <stack>
#include <algorithm>
#include <cassert>

#include <cmath>
#include <stdlib.h>

#define H_OPER "H"
#define V_OPER "V"

#define COMB_OPT
#define TIME

#define nmoves    10
#define ratio     0.85
#define P         0.99
#define epsilon   0.001
#define iseed     9
#define t0        -1
#define lambdatf  0.005

#define numrandmoves 40 // a 'sweetspot' for calculating initial temperature

typedef std::map<std::string, std::pair<float, float> > Mod;

using namespace std;

class Tile {

  friend class NPE; // NPE object needs to access the private members of Tile class

  private:
    std::string index; // name of a block
    std::vector<std::pair<float, float> > dims; // width-height combinations of a block

  public:
    Tile(std::string i, std::vector<std::pair<float, float> > d) : index(i), dims(d) {}
};

class NPE {

  private:
    Mod modules;

  public:
    NPE(Mod mod) : modules(mod) {}

  public:
    // to create vector of Tiles from NPE string
    std::vector<Tile *> createNpe(std::vector<std::string>& npeVec);

    // stack operation invoked inside the cost routine
    void processStack(Tile *i, Tile *j, Tile *oper);

    // to calculate the cost(area) of an NPE/Floorplan
    float cost(std::vector<std::string>& npeStr);

    // various move routines to change an NPE
    void doMoveM1(std::vector<std::string>& npeVec);
    void doMoveM2(std::vector<std::string>& npeVec);
    void doMoveM3(std::vector<std::string>& npeVec);

    // to check if an NPE is skewed
    bool isSkewed(std::vector<std::string>& npeVec);

    // to check balloting property of an NPE
    bool checkBallotingProperty(std::vector<std::string>& npeVec);

    // to calculate the initial temperature for simulated annealing
    float calculateInitialTemperature(std::vector<std::string>& npeVec);

    // top level method to perform simulated annealing
    void doSimulatedAnnealing(std::vector<std::string>& npeVec);

    // to print an NPE
    void printNpe(std::vector<std::string>& npe);
};
