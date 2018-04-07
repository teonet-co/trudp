/* 
 * File:   queue_cpp_t
 * Author: Kirill Scherba <kirill@scherba.ru>
 *
 * Created on Mar 25, 2018, 10:51:53 PM
 */

#include <iostream>
#include <CppUTest/TestHarness.h>
#include "../src/queue.hpp"

TEST_GROUP(QueueSuite) {
  teo::Queue que;
};

TEST(QueueSuite, queueGetExample) {
  que.add(7);
  que.add(3.14);
  que.add("Hello");
  CHECK(que.size() == 3);
  
  que.foreach([](teoQueue *q, int idx, teoQueueData *data, void* user_data) {
    
    switch(idx) {
      
      case 0: {
        auto d = teo::Queue::getData<int*>(data->data);   
        CHECK_EQUAL(*d, 7);
        
        auto &d2 = *teo::Queue::getData<int*>(data->data);   
        CHECK_EQUAL(d2, 7);
        
      } break;
        
      case 1: {
        auto d = teo::Queue::getData<double*>(data->data);   
        CHECK_EQUAL(*d, 3.14);
        
        auto &d2 = *teo::Queue::getData<double*>(data->data);   
        CHECK_EQUAL(d2, 3.14);
      } break;
      
      case 2: {
        auto d = teo::Queue::getData<const char*>(data->data);   
        STRCMP_EQUAL(d, "Hello");
      } break;
    }
    return 0;    
  });
  
  // free test queue ----
  que.freeAll();
  CHECK(que.size() == 0);
}

TEST(QueueSuite, queueForeachExample) {
  
  const std::string str[] = {
    "String 1",
    "String 2",
    "String 3",
    "String 4",
    "String 5"
  };  
  const int str_length = (int)(sizeof(str) / sizeof(str[0]));
  
  for(int i =0; i < str_length; i++)
    que.add(str[i]);

  que.foreach([](teoQueue *q, int idx, teoQueueData *data, void* user_data) {
    auto str = (const std::string *)user_data;
    STRCMP_EQUAL(data->data, str[idx].c_str());
    return 0;
  }, (void*)str);
}

TEST(QueueSuite, queueDeleteExample) {
  que.add(7);
  auto qd = que.add(3.14);
  que.add("Hello");
  CHECK(que.size() == 3);
  
  que.delFirst();
  CHECK(que.size() == 2);

  que.delLast();
  CHECK(que.size() == 1);
  
  que.del(qd);
  CHECK(que.size() == 0);
  
  // free test queue ----
  que.freeAll();
  CHECK(que.size() == 0);
}

TEST(QueueSuite, queueAddExample) {
  que.add(7);
  que.add(3.14);
  que.add("Hello");
  CHECK(que.size() == 3);
  
  // free test queue ----
  que.freeAll();
  CHECK(que.size() == 0);
}

TEST(QueueSuite, queueInitExample) {
  teo::Queue *q = new teo::Queue();
  delete q;
}
