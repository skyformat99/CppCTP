#include "safequeue.h"

#include <iostream>

using namespace std;

int main()
{
  SafeQueue <int> my_queue;
 
  my_queue.enqueue(1);
  my_queue.enqueue(2);
  my_queue.enqueue(3);
  
  std::cout << my_queue.dequeue() << std::endl;
  std::cout << my_queue.dequeue() << std::endl;
  std::cout << my_queue.dequeue() << std::endl;
 
  return 0;
}