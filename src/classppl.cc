#include <fstream>
#include <iostream>
#include <sstream>

#include "str.hh"
#include "defs.hh"
#include "io.hh"
#include "conf.hh"
#include "Ngram.hh"

using namespace std;


int main(int argc, char* argv[]) {

    conf::Config config;
    config("usage: classppl [OPTION...] CLASS_ARPA CLASS_MEMBERSHIPS INPUT\n")
    ('r', "use-root-node", "", "", "Pass through root node in contexts with unks, DEFAULT: advance with unk symbol")
    ('w', "num-words=INT", "arg", "", "Number of words for computing word-normalized perplexity")
    ('h', "help", "", "", "display help");
    config.default_parse(argc, argv);
    if (config.arguments.size() != 3) config.print_help(stderr, 1);

    string ngramfname = config.arguments[0];
    string classmfname = config.arguments[1];
    string infname = config.arguments[2];

    string unk = "<unk>";
    bool root_unk_states = config["use-root-node"].specified;

    map<string, pair<int, flt_type> > class_memberships;
    cerr << "Reading class memberships.." << endl;
    int num_classes = read_class_memberships(classmfname, class_memberships);

    cerr << "Reading class n-gram model.." << endl;
    LNNgram ng;
    ng.read_arpa(ngramfname);

    // The class indexes are stored as strings in the n-gram class
    vector<int> indexmap(num_classes);
    for (int i=0; i<(int)indexmap.size(); i++)
        if (ng.vocabulary_lookup.find(int2str(i)) != ng.vocabulary_lookup.end())
            indexmap[i] = ng.vocabulary_lookup[int2str(i)];

    cerr << "Scoring sentences.." << endl;
    SimpleFileInput infile(infname);
    string line;
    long int num_words = 0;
    long int num_sents = 0;
    long int num_oovs = 0;
    double total_ll = 0.0;
    int linei = 0;
    while (infile.getline(line)) {

        line = str::cleaned(line);
        if (line.length() == 0) continue;
        if (++linei % 10000 == 0) cerr << "sentence " << linei << endl;

        stringstream ss(line);
        vector<string> words;
        string word;
        while (ss >> word) {
            if (word == "<s>") continue;
            if (word == "</s>") continue;
            if (class_memberships.find(word) == class_memberships.end()
                || word == "<unk>"  || word == "<UNK>")
            {
                words.push_back(unk);
                num_oovs++;
            }
            else {
                words.push_back(word);
                num_words++;
            }
        }
        num_words++;

        double sent_ll = 0.0;

        int curr_node = ng.sentence_start_node;
        for (int i=0; i<(int)words.size(); i++) {
            if (words[i] == unk) {
                if (root_unk_states) curr_node = ng.root_node;
                else curr_node = ng.advance(curr_node, ng.unk_symbol_idx);
                continue;
            }

            pair<int, flt_type> word_class = class_memberships.at(words[i]);
            sent_ll += word_class.second;
            double ngram_score = 0.0;
            curr_node = ng.score(curr_node, indexmap[word_class.first], ngram_score);
            sent_ll += ngram_score;
        }

        double ngram_score = 0.0;
        curr_node = ng.score(curr_node, ng.sentence_end_symbol_idx, ngram_score);
        sent_ll += ngram_score;

        total_ll += sent_ll;
        num_sents++;
    }

    cerr << endl;
    cerr << "Number of sentences: " << num_sents << endl;
    cerr << "Number of in-vocabulary words excluding sentence ends: " << num_words-num_sents << endl;
    cerr << "Number of in-vocabulary words including sentence ends: " << num_words << endl;
    cerr << "Number of OOV words: " << num_oovs << endl;
    cerr << "Total log likelihood: " << total_ll << endl;
    cerr << "Total log likelihood (log10): " << total_ll/log(10.0) << endl;

    double ppl = exp(-1.0/double(num_words) * total_ll);
    cerr << "Perplexity: " << ppl << endl;

    if (config["num-words"].specified) {
        double wnppl = exp(-1.0/double(config["num-words"].get_int()) * total_ll);
        cerr << "Word-normalized perplexity: " << wnppl << endl;
    }

    exit(0);
}
