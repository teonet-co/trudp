/* 
 * File:   main_cpp_t
 * Author: Kirill Scherba <kirill@scherba.ru>
 *
 * Created on Mar 25, 2018, 10:27:27 PM
 */

#include <CppUTest/CommandLineTestRunner.h>
#include <vector>

int main(int argc, char** argv) {
  
  std::vector<const char*> args(argv, argv + argc);
  args.push_back("-v"); // Verbose output (mandatory!)
  args.push_back("-c"); // Colored output (optional)

  return RUN_ALL_TESTS(args.size(), &args[0]);
}
