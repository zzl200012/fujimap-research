#include <iostream>
#include <string>
#include <fstream>
#include <cassert>
#include <map>
#include <cstring>
#include <cstdlib>
#include <thread>
#include "fujimap.hpp"

using namespace std;

#include <time.h>
#include <sys/time.h>
#include <stdio.h>

double gettimeofday_sec()
{
  struct timeval tv;
  gettimeofday(&tv, NULL);
  return tv.tv_sec + (double)tv.tv_usec*1e-6;
}

void run_fujimap(const uint64_t N){
  fujimap_tool::Fujimap fm;
  fm.initFP(0);
  fm.initTmpN(N);
  //fm.initEncodeType(fujimap_tool::GAMMA);

  char buf[100];
  double t1 = gettimeofday_sec();
  for (uint64_t i = 0; i < N; ++i){
    snprintf(buf, 100, "%llu", (unsigned long long int)i);
    fm.setInteger(buf, strlen(buf), i, true);
  }
  //fm.build();
  double t2 = gettimeofday_sec();
  fprintf(stderr, "fm  set   : %f (%f)\n", t2 - t1, (t2 - t1) / N);

  //int dummy = 0;
  for (uint64_t i = 0; i < N; ++i){
    snprintf(buf, 100, "%llu", (unsigned long long int)i);
    fm.getInteger(buf, strlen(buf));
    //dummy += fm.getInteger(buf, strlen(buf));
  }
  double t3 = gettimeofday_sec();
  fprintf(stderr, "fm  lookup: %f (%f)\n", t3 - t2, (t3 - t2) / N);
  cerr <<         "fm    size: " << fm.getWorkingSize() << endl;

  fm.printInfo();



  //if (dummy == 77777){
  //  fprintf(stderr, "you're lucky!");
  //}

}

void run_map(const uint64_t N){
  map<string, int> m;
  char buf[100];
  double t1 = gettimeofday_sec();
  for (uint64_t i = 0; i < N; ++i){
    snprintf(buf, 100, "%llu", (unsigned long long int)i);
    m[string(buf)] = i;
  }
  double t2 = gettimeofday_sec();
  fprintf(stderr, "map set   : %f (%f)\n", t2 - t1, (t2 - t1) / N);

  //int dummy = 0;
  for (uint64_t i = 0; i < N; ++i){
    snprintf(buf, 100, "%llu", (unsigned long long int)i);
    m[string(buf)];
    //dummy += m[string(buf)];
  }
  double t3 = gettimeofday_sec();
  //fprintf(stderr, "map lookup: %f (%f)\n", t3 - t2, (t3 - t2) / N);
  fprintf(stderr, "map lookup: %f (%f)\n", t3 - t2, (t3 - t2) / N);

  //if (dummy == 77777){
  //  fprintf(stderr, "you're lucky!");
  //}

  
}

void run_fm_concurrent() {
  int N = 10000000;
  fujimap_tool::Fujimap fm;
  fm.initFP(0);
  fm.initTmpN(N / 40);

  char buf[100];
  vector<string> keys;
  for (uint64_t i = 0; i < N; ++i){
    snprintf(buf, 100, "%llu", (unsigned long long int)i);
    keys.push_back(string(buf));
    //fm.setInteger(buf, strlen(buf), i, true);
  }
  double t1 = gettimeofday_sec();
  int tno = 1;
  thread ts[tno];
  for (uint64_t i = 0; i < tno; i++) {
      ts[i] = thread([&]{
          for (int j = 0; j < 1000000; j++) {
              fm.setInt(keys[i * 1000000 + j], i * 1000000 + j);
          }
      });
  }
  for (auto& t : ts) {
      t.join();
  }
  double t2 = gettimeofday_sec();
  fprintf(stderr, "fm  concurrent set   : %f (%f)\n", t2 - t1, (t2 - t1) / N);

  //int dummy = 0;
  for (uint64_t i = 0; i < N; ++i){
    //snprintf(buf, 100, "%llu", (unsigned long long int)i);
    //fm.getInteger(keys[i], keys[i].size());
    //dummy += fm.getInteger(buf, strlen(buf));
    fm.getInt(keys[i]);
  }
  double t3 = gettimeofday_sec();
  fprintf(stderr, "fm  lookup: %f (%f)\n", t3 - t2, (t3 - t2) / N);
  cerr <<         "fm    size: " << fm.getWorkingSize() << endl;
}

int main(int argc, char* argv[]){
  static uint64_t N = 100000000;
  if (argc == 2){
    N = strtoll(argv[1], NULL, 10);
  }


   run_fujimap(N);
   run_map(N);
  //run_fm_concurrent();
  return 0;
}