/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

#include <iostream>
#include "../src/queue.hpp"

int main(int argc, char** argv) {
  
  teo::Queue que;
  
  que.add(7);
  que.add(3.14);
  que.add("Hello");
  
  auto *req = que.addTop(79);
  que.addTop(6.28);
  que.addTop("Hello world!");
  
  auto *req2 = que.addAfter(332, req);
  que.addAfter(12.44, req);
  que.addAfter("Hello after!", req);
  
  que.remove(req); // reg not free
  que.del(req2);   // reg2 undefined
  
  que.delFirst();
  que.delLast();
  
  // move to top
  // move to end
  // put
  
  std::cout << "size() = " << que.size() << "\n";
}
