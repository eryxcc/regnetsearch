#ifndef _PARSER_H_
#define _PARSER_H_

namespace regnetsearch {

  struct parseerror {
    std::string msg;
    int at1, at2;
    parseerror(std::string _msg, int _a1, int _a2) : msg(_msg), at1(_a1), at2(_a2) {}
    };
    
  struct xnumtable {
    numtable t;
    double fixed;
    xnumtable(numtable _t) : t(_t) {}
    xnumtable(double _d=0) : fixed(_d) {}
    };  
  
  xnumtable apply(xnumtable x1, xnumtable x2, auto f) {
    if(x1.t && x2.t && unify(x1.t->ds, x2.t->ds)) {
      numtable res = make<numtable>(x1.t->ds);
      for(int i=0; i<x1.t->ds->qty; i++) res->val[i] = f(x1.t->val[i], x2.t->val[i]);
      return res;
      }
    else if(x1.t) {
      numtable res = make<numtable>(x1.t->ds);
      for(int i=0; i<x1.t->ds->qty; i++) res->val[i] = f(x1.t->val[i], x2.fixed);
      return res;
      }
    else if(x2.t) {
      numtable res = make<numtable>(x2.t->ds);
      for(int i=0; i<x2.t->ds->qty; i++) res->val[i] = f(x1.fixed, x2.t->val[i]);
      return res;
      }
    else return f(x1.fixed, x2.fixed);
    }
  
  xnumtable operator +(xnumtable x1, xnumtable x2) {
    return apply(x1, x2, [] (auto a, auto b) { return a+b; });
    }
  
  xnumtable operator *(xnumtable x1, xnumtable x2) {
    return apply(x1, x2, [] (auto a, auto b) { return a*b; });
    }
  
  xnumtable operator -(xnumtable x1, xnumtable x2) {
    return apply(x1, x2, [] (auto a, auto b) { return a-b; });
    }
  
  xnumtable operator /(xnumtable x1, xnumtable x2) {
    return apply(x1, x2, [] (auto a, auto b) { return a/b; });
    }
  
  struct parser {

    int parsepos = 0;
    std::string s;
    
    parser(std::string _s) : s(_s) {} 
    
    char peek() { if(parsepos == s.size()) return -1; return s[parsepos]; }
    
    char eat() { if(parsepos == s.size()) return -1; return s[parsepos++]; }
    
    void skipwhitespace() {
      while(among(peek(), ' ', '\n', '\r', '\t')) eat();
      }
    
    char skippeek() {
      skipwhitespace(); return peek();
      }
    
    bool peekletter() { return peek() >= 'a' && peek() <= 'z' || peek() >= 'A' && peek() <= 'Z'; }
    
    std::string readToken() {
      std::string res;
      while(peekletter())  res += eat();
      return res;
      }
    
    virtual relation getrelation(std::string s) { return nullptr; }
    virtual stringtable getstringtable(std::string s) { return nullptr; }
    virtual bool namedNumtable(std::string s, xnumtable& d) { return false; }

    std::string readStringConstant() {
      int p = parsepos;
      if(peek() != '"') throw parseerror("string constant expected", p, parsepos);
      eat();
      std::string s;
      while(!among(peek(), '"', -1, '\n', '\r')) s += eat();
      if(peek() == -1) throw parseerror("string constant not ended", p, parsepos);
      eat();
      return s;
      }
    
    xnumtable parseval(int prio = 0) {
      int p = parsepos;
      xnumtable d;
      skipwhitespace();
      if(peek() == '(') { 
        eat(); d = parseval(); 
        if(peek() != ')') throw parseerror("expected ')' in numtable", p, parsepos);
        eat(); 
        }
      else if(peek() >= '0' && peek() <= '9' || peek() == '.') {
        std::string s = "";
        while(true) {
          char c = peek();
          if(peek() >= '0' && peek() <= '9' || peek() == '.') s += c, eat();
          else break;
          }
        d = atof(s.c_str());
        }
      else if(peekletter()) {
        std::string s = readToken();
        if(s == "search") {
          skipwhitespace();
          stringtable st = getstringtable(readToken());
          if(!st) throw parseerror("expected a string table", p, parsepos);
          skipwhitespace();
          std::string s1 = readStringConstant();
          printf("Searching for %s...\n", s1.c_str());
          return icasesearch(st, s1);
          }
        else if(s == "regexsearch") {
          skipwhitespace();
          stringtable st = getstringtable(readToken());
          if(!st) throw parseerror("expected a string table", p, parsepos);
          skipwhitespace();
          std::string s1 = readStringConstant();
          using namespace std::regex_constants;
          printf("Searching for %s...\n", s1.c_str());
          return regexsearch(st, std::regex(s1, icase | extended));
          }
        else if(namedNumtable(s, d)) ;
        else throw parseerror("unexpected keyword while parsing a numtable", p, parsepos);
        }
      else throw parseerror("numtable expected", p, parsepos);
      skipwhitespace();
      while(prio <= 1 && among(peek(), '*', '/', '&')) { 
        char c = eat(); 
        xnumtable t = parseval(1);
        if(c == '*' || c == '&') d = d * t; 
        else d = d / t;
        skipwhitespace(); 
        }
      while(prio <= 0 && among(peek(), '+', '-', '<', '>', '=', '|', '^', '/')) { 
        int c = eat(); 
        xnumtable t = parseval(0);
        static const int LEQ = 300;
        static const int GEQ = 301;
        static const int NEQ = 302;
        if(c == '<' && peek() == '=') { c = LEQ; eat(); }
        else if(c == '>' && peek() == '=') { c = GEQ; eat(); }
        else if(c == '/' && peek() == '=') { c = NEQ; eat(); }
        if(c == '+') d = d + t; 
        else if(c == '-') d = d - t;
        else if(c == '|') d = apply(d, t, [] (auto a, auto b) { return a+b-a*b; });
        else if(c == '^') d = apply(d, t, [] (auto a, auto b) { return std::min(a,b); });
        else if(c == '>') d = apply(d, t, [] (auto a, auto b) { return a>b; });
        else if(c == '<') d = apply(d, t, [] (auto a, auto b) { return a<b; });
        else if(c == '=') d = apply(d, t, [] (auto a, auto b) { return a==b; });
        else if(c == LEQ) d = apply(d, t, [] (auto a, auto b) { return a<=b; });
        else if(c == GEQ) d = apply(d, t, [] (auto a, auto b) { return a>=b; });
        else if(c == NEQ) d = apply(d, t, [] (auto a, auto b) { return a!=b; });
        skipwhitespace(); 
        }
      return d;
      }
    
    expression parseexp(int prio = 0) {
      int p = parsepos;
      expression ex;
      skipwhitespace();
      if(peek() == '(') { 
        eat(); ex = parseexp(); 
        if(peek() != ')') throw parseerror("expected ')' in expression", p, parsepos);
        eat(); 
        }
      else if(peekletter()) {
        std::string s = readToken();
        relation r;
        if(s == "while") {
          xnumtable x = parseval();
          expression ex1 = parseexp(1);
          if(x.t) ex = std::make_shared<xWhile> (ex1, x.t);
          else ex = std::make_shared<xWhileF> (ex1, x.fixed);
          }
        else if(s == "if") {
          xnumtable x = parseval();
          skipwhitespace(); 
          if(readToken() != "then") throw parseerror("'then' expected", p, parsepos);
          expression e1 = parseexp(0);
          skipwhitespace(); 
          if(readToken() != "else") throw parseerror("'else' expected", p, parsepos);
          expression e2 = parseexp(1);
          if(x.t)
            ex = cond(x.t, e1, e2);
          else 
            ex = std::make_shared<xRandom>(x.fixed, e1, e2);
          }
        else if(s == "accept") 
          ex = accept;
        else if(s == "reject") 
          ex = reject;
        else if(s == "skip") 
          ex = skip;
        else if(s == "start") {
          auto x = parseval();
          if(!x.t) throw parseerror("cannot start on a fixed constant", p, parsepos);
          ex = std::make_shared<xStart> (x.t);
          }
        else if(r = getrelation(s)) {
          ex = r;
          }
        else throw parseerror("unexpected keyword while parsing an expression", p, parsepos);
        }
      else throw parseerror("expression expected", p, parsepos);
      if(!ex) 
        throw parseerror("internal error", p, parsepos);
      
      skipwhitespace();
      while(prio <= 0 && peek() == ';') {
        eat();
        ex = ex >> parseexp(1);
        }
    
      return ex;
      }

    void checkend() {
      if(peek() != -1) throw parseerror("not whole program parsed", 0, parsepos);
      }

    std::string display(parseerror error) {
      return "Error: " + error.msg + "\n" + s.substr(0, error.at1) + "<*>" + s.substr(error.at1, error.at2-error.at1) + "<*>" + s.substr(error.at2);
      }
    };

  }
  
#endif