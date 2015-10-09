#include <string>
#include <iostream>
#include <ctime>

#include "conf.hh"
#include "Exchange.hh"


using namespace std;


int main(int argc, char* argv[])
{
    try {
        conf::Config config;
        config("usage: train [OPTION...] CORPUS MODEL\n")
        ('c', "num-classes=INT", "arg", "1000", "Number of classes, default: 1000")
        ('t', "max-time=INT", "arg", "100000", "Optimization time limit, default: 100000 (seconds)")
        ('p', "ll-print-interval=INT", "arg", "100000", "Likelihood print interval, default: 100000 (words)")
        ('h', "help", "", "", "display help");
        config.default_parse(argc, argv);
        if (config.arguments.size() != 2) config.print_help(stderr, 1);

        int num_classes = config["num-classes"].get_int();
        int max_seconds = config["max-time"].get_int();
        int ll_print_interval = config["ll-print-interval"].get_int();

        string corpus_fname = config.arguments[0];
        string model_fname = config.arguments[1];

        Exchange e(num_classes, corpus_fname);

        cout << "log likelihood: " << e.log_likelihood() << endl;

        e.iterate(0, max_seconds, ll_print_interval);
        e.write_word_classes(model_fname + ".cgenprobs.gz");
        e.write_classes(model_fname + ".classes.gz");

    } catch (string &e) {
        cerr << e << endl;
    }
}
