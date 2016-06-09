#ifndef PROJECT_DEFS
#define PROJECT_DEFS

#include <cmath>
#include <cstdlib>
#include <cstdio>
#include <map>
#include <sstream>

#include "io.hh"

typedef float flt_type;

#define START_CLASS 0
#define UNK_CLASS 1


// Return log(X+Y) where a=log(X) b=log(Y)
static flt_type add_log_domain_probs(flt_type a, flt_type b) {
    flt_type delta = a - b;
    if (delta > 0) {
      b = a;
      delta = -delta;
    }
    return b + log1p(exp(delta));
}

// Return log(X-Y) where a=log(X) b=log(Y)
static flt_type sub_log_domain_probs(flt_type a, flt_type b) {
    flt_type delta = b - a;
    if (delta > 0) {
      fprintf(stderr, "invalid call to sub_log_domain_probs, a should be bigger than b (a=%f,b=%f)\n",a,b);
      exit(1);
    }
    return a + log1p(-exp(delta));
}

static int str2int(std::string str) {
    int val;
    std::istringstream numstr(str);
    numstr >> val;
    return val;
}

static std::string int2str(int a)
{
    std::ostringstream temp;
    temp<<a;
    return temp.str();
}

static int
read_class_memberships(std::string fname,
                       std::map<std::string, std::pair<int, flt_type> > &class_memberships)
{
    SimpleFileInput wcf(fname);

    std::string line;
    int max_class = 0;
    while (wcf.getline(line)) {
        std::stringstream ss(line);
        std::string word;
        int clss;
        flt_type prob;
        ss >> word >> clss >> prob;
        class_memberships[word] = std::make_pair(clss, prob);
        max_class = std::max(max_class, clss);
    }
    return max_class+1;
}

#endif /* PROJECT_DEFS */
