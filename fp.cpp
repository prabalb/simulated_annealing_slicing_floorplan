/*
 * Simulated Annealing Algorithm for Slicing Floorplan
 * AUTHOR: Prabal Basu (A02049867)
 */


#include "./fp.h"

// create vector of Tile from an NPE
std::vector<Tile *> NPE::createNpe(std::vector<std::string>& npeVec)
{
  std::vector<Tile *> npeElems;
  std::vector<std::string>::const_iterator npeIter = npeVec.begin();
  for ( ; npeIter != npeVec.end(); npeIter++) {
    std::string index = *npeIter;
    if(index != H_OPER && index != V_OPER) { // operands
      std::vector<std::pair<float, float> > dims;
      std::pair<float, float> dim1 = modules[index];
      dims.push_back(dim1);
      float width  = dim1.first;
      float height = dim1.second;
      std::pair<float, float> dim2;
      if(width != height) { // rotation of a hard block
        dim2 = std::make_pair(height, width);
        dims.push_back(dim2);
      }
      Tile *tile = new Tile(index, dims);
      npeElems.push_back(tile);     
    } else { // operators
      std::vector<std::pair<float, float> > dims; // an empty placeholder as of now
      Tile *operTile = new Tile(index, dims);
      npeElems.push_back(operTile);
    }
  }
  return npeElems;
}

// use stack operation to calcuate cost/area of subtrees
void NPE::processStack(Tile *i, Tile *j, Tile *oper)
{
  std::vector<std::pair<float, float> >::iterator dimIter1;
  std::vector<std::pair<float, float> >::iterator dimIter2;
  for (dimIter1 = (i->dims).begin(); dimIter1 != (i->dims).end(); dimIter1++) {
    float width1 = (*dimIter1).first;
    float height1 = (*dimIter1).second;
    for (dimIter2 = (j->dims).begin(); dimIter2 != (j->dims).end(); dimIter2++) {
      float width2 = (*dimIter2).first;
      float height2 = (*dimIter2).second;
      float operWidth = 0.0;
      float operHeight = 0.0;
      if(oper->index == H_OPER) {
        operWidth = std::max(width1, width2);       
        operHeight = height1 + height2;
      } else if(oper->index == V_OPER) {
        operWidth = width1 + width2;
        operHeight = std::max(height1, height2);
      } else {
        assert(0); // sanity check
      }
      assert(operWidth * operHeight); // sanity check (assuming area cannot be '0')
      std::pair<float, float> operDim = std::make_pair(operWidth, operHeight);
      (oper->dims).push_back(operDim);
    }
  }

#ifdef COMB_OPT
  // Remove the width-height combinations (of an operator-Tile) which will certainly not result in optimal area of the floorplan.
  // This optimization gives ~600X speed-up on my system (Intel(R) Core(TM) i5-2410M CPU @ 2.30GHz) in terms of both cpu time and wall time!!
  // But this is only for small NPEs. It will be interesting to see the speed-up for fairly large size NPEs. Also I believe that the
  // execution time of the cost() function will improve further if the combination pairs are stored in a 'list' container since erase
  // operation is faster in 'list' as compared to 'vector'.
  std::vector<std::pair<float, float> >::iterator dIter1;
  std::vector<std::pair<float, float> >::iterator dIter2;
  for(dIter1 = (oper->dims).begin(); dIter1 != (oper->dims).end(); dIter1++) {
    float w1 = (*dIter1).first;
    float h1 = (*dIter1).second;
    for(dIter2 = dIter1 + 1; dIter2 != (oper->dims).end(); dIter2++) {
      assert(dIter1 != dIter2); // sanity check
      float w2 = (*dIter2).first;
      float h2 = (*dIter2).second;
      if(w1 >= w2 && h1 >= h2) {
        dIter1 = (oper->dims).erase(dIter1);
        dIter1--;
        break;
      }
      if(w2 >= w1 && h2 >= h1) {
        dIter2 = (oper->dims).erase(dIter2);
        dIter2--;
      }
    }
  }
#endif
}

// routine to calculate cost of the floorplan pertaining to a given NPE
float NPE::cost(std::vector<std::string>& npeStr)
{
  std::vector<Tile *> npeElems = createNpe(npeStr);
  std::stack<Tile *> npeStack;
  std::vector<Tile *>::iterator tileIter = npeElems.begin();
  for ( ; tileIter != npeElems.end(); tileIter++) {
    Tile *tile = *tileIter;
    if(tile->index != H_OPER && tile->index != V_OPER) {
      npeStack.push(tile);
    } else {
      assert(!npeStack.empty()); // sanity check
      Tile *i = npeStack.top();
      npeStack.pop();
      assert(!npeStack.empty()); // sanity check
      Tile *j = npeStack.top();
      npeStack.pop();
      processStack(i, j, tile);
      npeStack.push(tile);
    }
  }
  assert(npeStack.size() == 1); // sanity check
  Tile *stackTop = npeStack.top();

  std::vector<std::pair<float, float> >::iterator dimIter = (stackTop->dims).begin();
  float minArea = 100000000000.00;
  for ( ; dimIter != (stackTop->dims).end(); dimIter++) {
    float area = (*dimIter).first * (*dimIter).second;
    if(area < minArea) {
      minArea = area;
    }
  }
  return minArea;
}

// swap adjacent operands ignoring operator-chain
void NPE::doMoveM1(std::vector<std::string>& npeStrVec)
{
  size_t posOperand[100];
  size_t i = 0;
  // detect positions of operands in an NPE
  for (size_t tPos = 0; tPos < npeStrVec.size(); tPos++) {
    if(npeStrVec[tPos] != H_OPER && npeStrVec[tPos] != V_OPER) { // operand
      posOperand[i] = tPos;
      i++;
    }
  }
  // now randomly swap two adjacent operands
  int randPos = rand() % (i - 1);
  std::iter_swap(npeStrVec.begin() + posOperand[randPos], npeStrVec.begin() + posOperand[randPos + 1]);
  //std::cout << "NPE after move M1: ";
  //printNpe(npeStrVec);
}

// complement an operator-chain
void NPE::doMoveM2(std::vector<std::string>& npeStrVec)
{
  size_t posOperatorChain[100];
  size_t i = 0;
  // detect positions of operator-chains in an NPE
  for (size_t tPos = 0; tPos + 1 < npeStrVec.size(); tPos++) {
    if((npeStrVec[tPos] != H_OPER && npeStrVec[tPos] != V_OPER)
       && (npeStrVec[tPos + 1] == H_OPER || npeStrVec[tPos + 1] == V_OPER)) { 
      posOperatorChain[i] = tPos + 1;
      i++;
    }
  }

  // now randomly complement one operator-chain
  int randPos = rand() % i;
  int tPos = posOperatorChain[randPos];
  while(tPos < int(npeStrVec.size())) {
    if(npeStrVec[tPos] == H_OPER || npeStrVec[tPos] == V_OPER) {
      if(npeStrVec[tPos] == H_OPER) {
        npeStrVec[tPos] = V_OPER;
      } else {
        npeStrVec[tPos] = H_OPER;
      }
      tPos++;
    } else {
      break;
    }
  }
  //std::cout << "NPE after move M2: ";
  //NPE::printNpe(npeStrVec);
}

// swap two adjacent operator and operand
void NPE::doMoveM3(std::vector<std::string>& npeStrVec)
{
  std::vector<std::string> npeCopy;
  for(size_t tPos = 0; tPos < npeStrVec.size(); tPos++) {
    npeCopy.push_back(npeStrVec[tPos]);
  }

  int posPair[100];
  size_t i = 0;
  // detect positions of adjacent operand-operator pair in an NPE
  for (size_t tPos = 0; tPos < (npeStrVec.size() - 1); tPos++) { 
    if((npeStrVec[tPos] != H_OPER && npeStrVec[tPos] != V_OPER)
       && (npeStrVec[tPos + 1] == H_OPER || npeStrVec[tPos + 1] == V_OPER)) {
      posPair[i] = tPos;
      i++;
    }
  }

  int numTry = 0;
  // now randomly swap two adjacent operand and operator
  while(numTry < 10) { // make at most 10 attempts to get a valid NPE out of an M3 move
    int randPos = rand() % (i - 1);
    int tPos = posPair[randPos];
    std::iter_swap(npeStrVec.begin() + tPos - 1, npeStrVec.begin() + tPos);
    // reject the move if the new NPE violates balloting property or becomes non-skewed
    if(checkBallotingProperty(npeStrVec) && isSkewed(npeStrVec)) {
      break;
    }
    else {
      npeStrVec = npeCopy;
      numTry++;
    }
  }
  //std::cout << "NPE after move M3: ";
  //NPE::printNpe(npeStrVec);
}

// check balloting property of an NPE
bool NPE::checkBallotingProperty(std::vector<std::string>& npeStrVec)
{
  int numOperands = 0;
  int numOperators = 0;
  for(size_t tPos = 0; tPos < npeStrVec.size(); tPos++) {
    if(npeStrVec[tPos] == H_OPER || npeStrVec[tPos] == V_OPER) {
      numOperators++;
    }
    if(npeStrVec[tPos] != H_OPER && npeStrVec[tPos] != V_OPER) {
      numOperands++;
    }
    if(numOperators >= numOperands) return false;
  }
  return true;
}

// check if the NPE is skewed
bool NPE::isSkewed(std::vector<std::string>& npeStrVec)
{
  for(size_t tPos = 1; tPos < npeStrVec.size(); tPos++) {
    if(npeStrVec[tPos] == H_OPER || npeStrVec[tPos] == V_OPER) {
      std::string oper = npeStrVec[tPos];
      if(npeStrVec[tPos - 1] == oper) return false;
    }
  }
  return true;
}

// calculate initial temperature for simulated annealing
float NPE::calculateInitialTemperature(std::vector<std::string>& npeVec)
{
  int numMoves = 0;
  float totalCostChange = 0.0;

  float oldCost = cost(npeVec);
  
  do {
    int choice = rand() % 3 + 1;
    
    switch(choice) {
      case 1: {
        doMoveM1(npeVec);
        break;
      }
      case 2: {
        doMoveM2(npeVec);
        break;
      }
      case 3: {
        doMoveM3(npeVec);
        break;
      }
    }

    float newCost = cost(npeVec);
    float deltaCost = newCost - oldCost;
    oldCost = newCost;

    // Mimic the effect of an uphill move by taking the absolute value of the 
    // cost change for all the moves. This will result in faster execution.
    totalCostChange += abs(deltaCost);
    numMoves++;
  } while (numMoves < numrandmoves);

  float avgCostChange = totalCostChange / numMoves;
  float initialTemperature = P < 1 ? (t0 * avgCostChange) / log(P) : avgCostChange / log(P);
  return initialTemperature;
}

// simulated annealing routine
void NPE::doSimulatedAnnealing(std::vector<std::string>& npeVec)
{
  const int N = nmoves * int(modules.size());
  
  std::vector<std::string> bestFp = npeVec;
  std::vector<std::string> tempFp = npeVec;

  // calculate initial temperature
  float T0 = calculateInitialTemperature(tempFp);
  //std::cout << "Initial temperature: " << T0 << " ...\n\n";

  int numMoves; // total number of moves at a given temperature
  int numUphillMoves; // total number of uphill moves made at a given temperature
  int numRejectedMoves; // total number of rejected moves
  float T = T0;

  int iteration = 0;
  //float bestCost = 0.0;
  do {
    numMoves = 0;
    numUphillMoves = 0;
    numRejectedMoves = 0;

    //std::cout << "   === Starting Iteration " << iteration << " ===\n";
    //std::cout << "temperature: " << T << std::endl;

    do {
      float oldCost = cost(tempFp);
      int choice = rand() % 3 + 1;
      switch(choice) {
        case 1: {
          doMoveM1(tempFp);
          break;
        }
        case 2: {
          doMoveM2(tempFp);
          break;
        }
        case 3: {
          doMoveM3(tempFp);
          break;
        }
      }
      numMoves++;
      float newCost = cost(tempFp);
      float deltaCost = newCost - oldCost;
      double randNum = ((double) rand() / (RAND_MAX)); // 0 < randNum < 1
      if(deltaCost < 0 || (randNum < exp(-1 * deltaCost / T))) {
        if(deltaCost > 0) {
          numUphillMoves++;
        }
        npeVec = tempFp; // accept the new floorplan
        if(cost(npeVec) < cost(bestFp)) {
          bestFp = npeVec;
          //bestCost = cost(bestFp);
        }
      } else {
        numRejectedMoves++;
      }
    } while(numUphillMoves < N && numMoves <= 2 * N);
    T = T < lambdatf * T0 ? 0.1 * T : ratio * T;
    //std::cout << "number of moves performed: " << numMoves << std::endl;
    //std::cout << "number of uphill moves: " << numUphillMoves << std::endl;
    //std::cout << "number of rejected moves: " << numRejectedMoves << std::endl;
    //std::cout << "best cost: " << bestCost << std::endl;

    //std::cout << "   === End of Iteration " << iteration << " ===\n\n";
    iteration++;
  } while (numRejectedMoves / numMoves <= 0.95 && T >= epsilon);

  npeVec = bestFp;
}

void NPE::printNpe(std::vector<std::string>& npeVec)
{
  std::vector<std::string>::const_iterator npeIter = npeVec.begin();
  for( ; npeIter != npeVec.end(); npeIter++) {
    std::cout << *npeIter;
  }
  std::cout << "\n";
}
