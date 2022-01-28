#include <iostream>
#include <string>
#include <fstream>
#include <cassert>
#include <map>
#include <cstring>
#include <cstdlib>
#include <thread>
#include <vector>
#include <tuple>
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
  fm.initTmpN(N / 40);
  fm.initKeyBlockN(128);
  //fm.initEncodeType(fujimap_tool::GAMMA);

  char buf[100];
  double t1 = gettimeofday_sec();
  for (uint64_t i = 0; i < N; ++i){
    snprintf(buf, 100, "%llu", (unsigned long long int)i);
    fm.setInteger(buf, strlen(buf), i, true);
  }
  fm.build();
  double t2 = gettimeofday_sec();
  fprintf(stderr, "fm  set   : %f (%f)\n", t2 - t1, (t2 - t1) / N);

  //int dummy = 0;
  for (uint64_t i = 0; i < N; ++i){
    snprintf(buf, 100, "%llu", (unsigned long long int)i);
    auto val = fm.getInteger(buf, strlen(buf));
    assert(val == i);
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

/* void run_fm_concurrent() {
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
*/

void gen_data(const uint64_t N, vector<string>& data) {
  for (uint64_t i = 0; i < N; i++) {
    data.push_back(to_string(i));
  }
}

tuple<double, double, double, size_t> bench_fm(const uint64_t N, const uint64_t fp = 0, const uint64_t tmpN = 1000000, const uint64_t blockN = 128) {
  fujimap_tool::Fujimap fm;
  fm.initFP(fp);
  fm.initKeyBlockN(blockN);
  fm.initTmpN(tmpN);

  vector<string> keys;
  gen_data(N, keys);

  double t1 = gettimeofday_sec();

  for (uint64_t i = 0; i < N; i++) {
    fm.setInteger(keys[i], i);
  }
  fm.build();
  
  double hit = 0;
  double total = N;
  double t2 = gettimeofday_sec();
  
  for (uint64_t i = 0; i < N; i++) {
    auto val = fm.getInteger(keys[i]);
    if (val == i) {
      hit++;
    }
  }

  double t3 = gettimeofday_sec();

  // cout << "fujimap bench -- fpLen: " << fp << " bits -- TmpN: " << tmpN << " -- BlockN: " << blockN << endl;
  // cout << "fujimap -- set " << N << " entries, time elapsed: " << t2 - t1 << " sec" << endl;
  // cout << "fujimap -- get " << N << " entries, time elapsed: " << t3 - t2 << " sec" << endl;
  // cout << "space: " << fm.getWorkingSize() / 8 << " bytes" << endl;

  return tuple<double, double, double, size_t>(t2 - t1, t3 - t2, hit / total, fm.getWorkingSize() / 8);
}

void bench_map(const uint64_t N) {
  map<string, uint64_t> mp;
  vector<string> keys;
  gen_data(N, keys);

  double t1 = gettimeofday_sec();

  for (uint64_t i = 0; i < N; i++) {
    mp[keys[i]] = i;
  }
  
  double t2 = gettimeofday_sec();
  
  for (uint64_t i = 0; i < N; i++) {
    auto val = mp[keys[i]];
    assert(val == i);
  }

  double t3 = gettimeofday_sec();

  cout << "std::map -- set " << N << " entries, time elapsed: " << t2 - t1 << " sec" << endl;
  cout << "std::map -- get " << N << " entries, time elapsed: " << t3 - t2 << " sec" << endl;
  cout << "space: " << sizeof(mp) + mp.size() * (sizeof(decltype(mp)::key_type) + sizeof(decltype(mp)::mapped_type)) << " bytes" << endl;
}

void gen_config(vector<tuple<uint64_t, uint64_t, uint64_t, uint64_t> >& configs) {
  vector<uint64_t> ns;
  vector<uint64_t> fps;
  vector<uint64_t> tmpns;
  vector<uint64_t> blkns;

  for (uint64_t i = 1000000; i <= 1000000000; i *= 10) {
    ns.push_back(i);
  }

  fps.push_back(0);
  for (uint64_t i = 2; i <= 32; i *= 2) {
    fps.push_back(i);
  }

  for (uint64_t i = 1000000; i <= 100000000; i *= 10) {
    tmpns.push_back(i);
  }

  for (uint64_t i = 8; i <= 1024; i *= 2) {
    blkns.push_back(i);
  }

  for (auto a = 0; a < ns.size(); a++) {
    for (auto b = 0; b < fps.size(); b++) {
      for (auto c = 0; c < tmpns.size(); c++) {
        for (auto d = 0; d < blkns.size(); d++) {
          if (ns[a] == 100000000 /*&& tmpns[c] == 10000000*/ && fps[b] == 16 && blkns[d] == 128 && ns[a] >= tmpns[c]) {
            configs.push_back(tuple<uint64_t, uint64_t, uint64_t, uint64_t>(ns[a], fps[b], tmpns[c], blkns[d]));
          }
        }
      }
    }
  }

}

int main(int argc, char* argv[]){
  vector<tuple<uint64_t, uint64_t, uint64_t, uint64_t>> configs;
  gen_config(configs);
  // cout << configs.size() << endl;
  // cout << "-----------------" << endl;
  // // N, fpLen, TmpN, BlockN
  // bench_fm(N);
  // cout << "-----------------" << endl;
  //bench_map(10000000);

  auto now = gettimeofday_sec();
  auto fname = "/tmp/" + to_string(now);
  ofstream f(fname);
  int limit = 0;
  cout << "======= start bench =======" << endl;
  for (auto& conf : configs) {
    // if (limit++ == 1) {
    //   break;
    // }
    auto [a, b, c, d] = conf;
    //cout << a << " " << b << " " << c << " " << d << endl;
    f << a << " " << b << " " << c << " " << d << " ";
    auto [t1, t2, hit, size] = bench_fm(a, b, c, d);
    f << t1 << " " << t2 << " " << hit << " " << size << endl;
  }
  return 0;
}