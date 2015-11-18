#include <string>
#include <iostream>
#include <ctime>

#include "conf.hh"
#include "ExchangeAlgorithm.hh"


using namespace std;


int main(int argc, char* argv[])
{
    try {
        conf::Config config;
        config("usage: exchange [OPTION...] CORPUS MODEL\n")
        ('c', "num-classes=INT", "arg", "1000", "Number of classes, default: 1000")
        ('m', "max-time=INT", "arg", "100000", "Optimization time limit, default: 100000 (seconds)")
        ('t', "num-threads=INT", "arg", "1", "Number of threads, default: 1")
        ('o', "top-words=INT", "arg", "0", "Own class in initialization for most common words, default: 0")
        ('p', "ll-print-interval=INT", "arg", "100000", "Likelihood print interval, default: 100000 (words)")
        ('w', "model-write-interval=INT", "arg", "3600", "Model write interval, default: 3600 (seconds)")
        ('v', "vocabulary=FILE", "arg", "", "Vocabulary, one word per line")
        ('i', "class-init=FILE", "arg", "", "Class initialization, same format as in model classes file")
        ('b', "word-boundary", "", "", "Keep word boundary symbol in the vocabulary, ignore in training")
        ('h', "help", "", "", "display help");
        config.default_parse(argc, argv);
        if (config.arguments.size() != 2) config.print_help(stderr, 1);

        string corpus_fname = config.arguments[0];
        string model_fname = config.arguments[1];

        int num_classes = config["num-classes"].get_int();
        int max_seconds = config["max-time"].get_int();
        int ll_print_interval = config["ll-print-interval"].get_int();
        int num_threads = config["num-threads"].get_int();
        int top_words = config["top-words"].get_int();
        int model_write_interval = config["model-write-interval"].get_int();
        string vocab_fname = config["vocabulary"].get_str();
        string class_fname = config["class-init"].get_str();
        bool word_boundary = config["word-boundary"].specified;

        Exchange e(num_classes, corpus_fname, vocab_fname,
                   class_fname, top_words, word_boundary);

        time_t t1,t2;
        t1=time(0);
        cerr << "log likelihood: " << e.log_likelihood() << endl;
        e.iterate(0, max_seconds, ll_print_interval,
                  model_write_interval, model_fname, num_threads);
        t2=time(0);
        cerr << "Train run time: " << t2-t1 << " seconds" << endl;

        e.write_word_classes(model_fname + ".cgenprobs.gz");
        e.write_class_mem_probs(model_fname + ".cmemprobs.gz");
        e.write_classes(model_fname + ".classes.gz");

    } catch (string &e) {
        cerr << e << endl;
    }
}
