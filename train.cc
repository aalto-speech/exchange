#include <string>
#include <iostream>

#include "conf.hh"
#include "Exchange.hh"


using namespace std;


int main(int argc, char* argv[])
{
    try {
        conf::Config config;
        config("usage: train [OPTION...] CORPUS MODEL\n")
          ('h', "help", "", "", "display help");
        config.default_parse(argc, argv);
        if (config.arguments.size() != 2) config.print_help(stderr, 1);

        string corpus_fname = config.arguments[0];
        string model_fname = config.arguments[1];

    } catch (string &e) {
        cerr << e << endl;
    }
}
