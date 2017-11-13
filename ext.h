// extensions of the standard library

#ifndef _EXT_H_
#define _EXT_H_

#include <memory>
#include <cctype>
#include <sys/time.h>
#include <thread>

namespace ext {

template<class T> struct maker {  };

  template<class T> struct maker<std::shared_ptr<T>> { 
    template<class... U> static std::shared_ptr<T> make(U... u) { return std::make_shared<T> (u...); } };
  
  template<class T, class... U> auto make(U... u) { return maker<T>::make(u...); }
  
  int scanerr;
  
  template<class T> void binwrite(FILE *f, const T& x) {
    fwrite(&x, sizeof(x), 1, f);
    }
  
  template<class T> void binread(FILE *f, T& x) {
    scanerr=fread(&x, sizeof(x), 1, f);
    }
  
  template<class T> void binwrite(FILE *f, const std::vector<T>& x) {
    int siz = x.size();
    binwrite(f, siz);
    fwrite(x.data(), sizeof(x[0]), siz, f);
    }
  
  template<class T> void binread(FILE *f, std::vector<T>& x) {
    int siz;
    binread(f, siz);
    x.resize(siz);
    scanerr=fread(x.data(), sizeof(x[0]), siz, f);
    }
  
  template<class T> void binwrite(FILE *f, const std::shared_ptr<T>& what) {
    binwrite(f, *what);
    }

  template<class T, class... U> void binread(FILE *f, std::shared_ptr<T>& what, U... params) {
    what = make<std::shared_ptr<T>> (params...);
    binread(f, *what);
    }

  template<class T, class... U> void binwrite(const char *fname, const T& what, U... params) {
    FILE *f = fopen(fname, "wb");
    binwrite(f, what, params...);
    fclose(f);
    }
  
  template<class T, class... U> void binread(const char *fname, T& what, U... params) {
    FILE *f = fopen(fname, "rb");
    binread(f, what, params...);
    fclose(f);
    }
  
  bool isearch(const char * haystack, const std::string &needle) {
    auto hayend = haystack; while(*hayend) hayend++;
    return std::search(haystack, hayend, needle.begin(), needle.end(),
      [] (char c1, char c2) { return toupper(c1) == toupper(c2); }) != hayend;
    }
  
  bool among(char c) { return false; }
  template<class... T> bool among(char c, char x, T... t) { return (c==x) || among(c,t...); }    

  long long getVa() {
    struct timeval tval;
    gettimeofday(&tval, NULL);
    return tval.tv_sec * 1000000 + tval.tv_usec;
    }

#ifdef NO_THREADS
  constexpr int threads = 1;
#else
  int threads;
#endif

  void init_threads() {
#ifndef NO_THREADS
    char* t = getenv("THREADCOUNT");
    if(!t) threads = 1;
    else {
      threads = atoi(t);
      if(threads < 1 || threads > 128) 
        threads = 1;
      }
#endif
    }
  
  template<class T> auto parallelize(long long N, T action) {
#ifdef NO_THREADS
    return action(0,N);
#else
    if(!threads) init_threads();
    if(threads == 1) return action(0,N);
    std::vector<std::thread> v;
    typedef decltype(action(0,0)) Res;
    std::vector<Res> results(threads);
#ifdef THREADSTATS
    std::vector<long long> times(threads);
#endif
    for(int k=0; k<threads; k++)
      v.emplace_back([&,k] () { 
        long long t = getVa();
        results[k] = action(N*k/threads, N*(k+1)/threads); 
#ifdef THREADSTATS
        times[k] = getVa()-t; 
#endif
        });
    for(std::thread& t:v) t.join();
#ifdef THREADSTATS
    long long maxt = 0;
    for(int k=0; k<threads; k++) maxt = std::max(times[k], maxt);
    for(int k=0; k<threads; k++) printf("%4d", times[k] * 100 / maxt); printf(" :: %dus\n", int(maxt));
#endif
    Res res = 0;
    for(Res r: results) res += r;
    return res;
#endif
    }

  // for objects that can be read/constructed later
  
  struct lazyboy {
    int state = 0; // 0 - lazy not specified, 1 - reader specified, 2 - already read
    std::function<void()> actual;
    
    void done() {
      state = 2; actual = std::function<void()> ();
      }
    
    lazyboy(int i = 0) { state = i; }
    
    template<class T> void set(const T& t) { actual = t; state = 1; }
    void set(const lazyboy& r) { *this = r; }
    template<class T> void setNew(const T& t) { if(state == 0) set(t); }
    
    bool isUnknown() { return state = 0; }

    void operator() () {
      if(state == 0) {
        fprintf(stderr, "unknown lazy called\n");
        }
      else if(state == 1) {
        // we make a copy, to prevent problems when done() is called
        auto a = actual;
        a();
        done();
        }
      }
    };
  }

#endif
