#include "ThreadPool.h"

#include <iostream>
#include <future>
#include <vector>

int main(int argc, char* argv[])
{
  constexpr int numTasks = 100;
  ThreadPool pool;

  auto task1 = [](int x, int y) {
    return x * y;
  };

  auto task2 = [](int x, int y, int z)
  {
    return x * y + z % 10;
  };

  std::vector<std::future<int>> results;
  for (int i = 0; i < numTasks; i++)
  {
    if (i % 2)
      results.emplace_back(std::move(pool.enqueueTask(task1, i, i + 1)));
    else
      results.emplace_back(std::move(pool.enqueueTask(task2, i, i + 1, i + 2)));
  }

  for (int i = 0; i < numTasks; i++)
  {
    std::cout << i << ": " << results[i].get() << std::endl;
  }

  return 0;
}