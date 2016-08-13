/*
 * Simulated Annealing Algorithm for Slicing Floorplan
 * AUTHOR: Prabal Basu (A02049867)
 */


#include "./fp.h"
#include <fstream>
#include <cstring>

#ifdef TIME
#include <sys/time.h>
double get_wall_time()
{
  struct timeval time;
  if (gettimeofday(&time,NULL)) {
    // Handle error
    return 0;
  }
  return (double)time.tv_sec + (double)time.tv_usec * .000001;
}

double get_cpu_time()
{
  return (double)clock() / CLOCKS_PER_SEC;
}
#endif

int main() {

  Mod modules;

  std::string fileName = "";
  ifstream file;

  std::cout << "Please specify full path of the input file: ";
  cin >> fileName;

  file.open(fileName.c_str());

  if (file.is_open()) {
    while (!file.eof())
    {
      char buf[100];
      file.getline(buf, 100);
      
      const char* token[4] = {};
      token[0] = strtok(buf, " "); // name of a block
      if (token[0]) // checking for blank line
      {
        token[1] = strtok(0, " "); 
        float area = atof(token[1]); // area of a block
        token[2] = strtok(0, " ");
        float artio = atof(token[2]); // aspect ratio of a block
        modules[token[0]] = std::make_pair(sqrt(area * artio), sqrt(area / artio));
      }
    }
    file.close();
  } else {
    std::cout << "Could not open the input file specified, please try again ..." << "\n";
    exit(1);
  }

  NPE *npeObj = new NPE(modules);

  // Initial floorplan: 12V3V4V...nV
  std::vector<std::string> npeVec;
  Mod::const_iterator modIter = modules.begin();
  npeVec.push_back(modIter->first);
  modIter++;
  npeVec.push_back(modIter->first);
  modIter++;

  for ( ; modIter != modules.end(); modIter++) {
    npeVec.push_back(V_OPER);
    npeVec.push_back(modIter->first);
  }
  npeVec.push_back(V_OPER);
   
  std::cout << "\nInitial topology: ";
  npeObj->printNpe(npeVec);

  std::cout << "Cost of the initial topology: " << npeObj->cost(npeVec) << "\n";

  std::cout << "\nStarting simulated annealing ...\n\n";

  // initialize random number generator
  srand(iseed);

  double wall0 = get_wall_time();
  double cpu0  = get_cpu_time();
  
  // perform simulated annealing
  npeObj->doSimulatedAnnealing(npeVec);

  double wall1 = get_wall_time();
  double cpu1  = get_cpu_time();

  std::cout << "Finished simulated annealing ..." << std::endl;

  std::cout << "\nTime(s) : Simulated Annealing : Wall Time : " << wall1 - wall0 << ", CPU Time : " << cpu1  - cpu0 << std::endl;
  
  std::cout << "\nFinal optimized topology: ";
  npeObj->printNpe(npeVec);

  std::cout << "Cost of the optimized topology: " << npeObj->cost(npeVec) << "\n\n";

  return 0;
}
