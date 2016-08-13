#include <iostream>
#include <fstream>
#include <memory>
#include <unordered_map>
#include "string_pool.h"

#include "lib/mem_usage.h"

using namespace LLSP;

int main(int argc, char** argv)
{
  if(argc < 2)
  {
    std::cout << "Usage: " << argv[0] << " FILE [...]\n";
    return 0;
  }

  StringPool pool;
  for(int i = 1; i < argc; i++)
  {
    std::cout << "Reading file " << argv[i] << "...\n";
    std::ifstream in(argv[i]);
    std::string next;
    int counter = 0;
    while(in >> next)
    {
      if((++counter % 1000000) == 0)
        std::cout << counter << "... " << get_vm_info() << '\n';

      pool.add(next);
    }
    std::cout << counter << "... " << get_vm_info() << '\n';
    std::cout << pool.get_debug_info() << "\n\n";
  }

  pool.destroy_all_references();
  return 0;
}
