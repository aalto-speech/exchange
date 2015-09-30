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
        ('c', "num-classes=INT", "arg", "1000", "Number of classes, default: 1000")
        ('h', "help", "", "", "display help");
        config.default_parse(argc, argv);
        if (config.arguments.size() != 2) config.print_help(stderr, 1);

        int num_classes = config["num-classes"].get_int();

        string corpus_fname = config.arguments[0];
        string model_fname = config.arguments[1];

        Exchange e(num_classes, corpus_fname);
        e.initialize_classes();
        e.set_class_counts();

    } catch (string &e) {
        cerr << e << endl;
    }
}
