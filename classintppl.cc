#include <fstream>
#include <iostream>
#include <sstream>
#include <cassert>

#include "str.hh"
#include "defs.hh"
#include "conf.hh"
#include "Ngram.hh"

using namespace std;


void preprocess_sent(string line,
                     const Ngram &lm,
                     const map<string, pair<int, flt_type> > &class_memberships,
                     string unk_symbol,
                     vector<string> &words,
                     long int &num_words,
                     long int &num_oovs)
{
    stringstream ss(line);
    words.clear();
    string word;
    while (ss >> word) {
        if (word == "<s>") continue;
        if (word == "</s>") continue;
        if (lm.vocabulary_lookup.find(word) == lm.vocabulary_lookup.end()
            || class_memberships.find(word) == class_memberships.end()
            || word == "<unk>"  || word == "<UNK>")
        {
            words.push_back(unk_symbol);
            num_oovs++;
        }
        else {
            words.push_back(word);
            num_words++;
        }
    }
    num_words++;
}


int main(int argc, char* argv[]) {

    conf::Config config;
    config("usage: classintppl [OPTION...] ARPAFILE CLASS_ARPA CLASS_MEMBERSHIPS INPUT\n")
    ('i', "weight=FLOAT", "arg", "0.5", "Interpolation weight [0.0,1,0] for the word ARPA model")
    ('r', "use-root-node", "", "", "Pass through root node in contexts with unks, DEFAULT: advance with unk symbol")
    ('w', "num-words=INT", "arg", "", "Number of words for computing word-normalized perplexity")
    ('h', "help", "", "", "display help");
    config.default_parse(argc, argv);
    if (config.arguments.size() != 4) config.print_help(stderr, 1);

    string arpafname = config.arguments[0];
    string classngramfname = config.arguments[1];
    string classmfname = config.arguments[2];
    string infname = config.arguments[3];

    string unk = "<unk>";
    bool root_unk_states = config["use-root-node"].specified;

    double iw = config["weight"].get_float();
    if (iw < 0.0 || iw > 1.0) {
        cerr << "Invalid interpolation weight: " << iw << endl;
        exit(1);
    }
    cerr << "Interpolation weight: " << iw << endl;
    double word_iw = log(iw);
    double class_iw = log(1.0-iw);

    Ngram lm;
    lm.read_arpa(arpafname);
    int lm_start_node = lm.advance(lm.root_node, lm.vocabulary_lookup.at("<s>"));

    map<string, pair<int, flt_type> > class_memberships;
    cerr << "Reading class memberships.." << endl;
    int num_classes = read_class_memberships(classmfname, class_memberships);

    cerr << "Reading class n-gram model.." << endl;
    Ngram class_ng;
    class_ng.read_arpa(classngramfname);
    int class_lm_start_node = class_ng.advance(class_ng.root_node, class_ng.vocabulary_lookup.at("<s>"));

    // The class indexes are stored as strings in the n-gram class
    vector<int> indexmap(num_classes);
    for (int i=0; i<(int)indexmap.size(); i++)
        if (class_ng.vocabulary_lookup.find(int2str(i)) != class_ng.vocabulary_lookup.end())
            indexmap[i] = class_ng.vocabulary_lookup[int2str(i)];
        else indexmap[i] = -1;

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

        double sent_ll = 0.0;

        vector<string> words;
        preprocess_sent(line, lm, class_memberships, unk, words, num_words, num_oovs);

        int curr_class_lm_node = class_lm_start_node;
        int curr_lm_node = lm_start_node;

        for (int i=0; i<(int)words.size(); i++) {

            if (words[i] == unk) {
                if (root_unk_states) {
                    curr_lm_node = lm.root_node;
                    curr_class_lm_node = class_ng.root_node;
                }
                else {
                    curr_lm_node = lm.advance(curr_lm_node, lm.unk_symbol_idx);
                    curr_class_lm_node = class_ng.advance(curr_class_lm_node, class_ng.unk_symbol_idx);
                }
                continue;
            }

            double ngram_score = 0.0;
            curr_lm_node = lm.score(curr_lm_node, lm.vocabulary_lookup.at(words[i]), ngram_score);
            ngram_score *= log(10.0);
            ngram_score += word_iw;

            pair<int, flt_type> word_class = class_memberships.at(words[i]);
            double class_score = 0.0;
            curr_class_lm_node = class_ng.score(curr_class_lm_node, indexmap[word_class.first], class_score);
            class_score *= log(10.0);
            class_score += word_class.second;
            class_score += class_iw;

            sent_ll += add_log_domain_probs(ngram_score, class_score);
        }

        double ngram_score = 0.0;
        curr_lm_node = lm.score(curr_lm_node, lm.sentence_end_symbol_idx, ngram_score);
        ngram_score *= log(10.0);
        ngram_score += word_iw;

        double class_score = 0.0;
        curr_class_lm_node = class_ng.score(curr_class_lm_node, class_ng.sentence_end_symbol_idx, class_score);
        class_score *= log(10.0);
        class_score += class_iw;

        sent_ll += add_log_domain_probs(ngram_score, class_score);

        total_ll += sent_ll;
        num_sents++;
    }

    cerr << endl;
    cerr << "Number of sentences: " << num_sents << endl;
    cerr << "Number of in-vocabulary words excluding sentence ends: " << num_words-num_sents << endl;
    cerr << "Number of in-vocabulary words including sentence ends: " << num_words << endl;
    cerr << "Number of OOV words: " << num_oovs << endl;
    cerr << "Total log likelihood (ln): " << total_ll << endl;
    cerr << "Total log likelihood (log10): " << total_ll/2.302585092994046 << endl;

    if (config["num-words"].specified)
        num_words = config["num-words"].get_int();
    double ppl = exp(-1.0/double(num_words) * total_ll);
    cerr << "Perplexity: " << ppl << endl;

    exit(0);
}

