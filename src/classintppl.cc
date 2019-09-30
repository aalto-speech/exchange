#include <cstdlib>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

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


void
evaluate(const LNNgram &ngram,
         const LNNgram &class_ngram,
         vector<int> &indexmap,
         const map<string, pair<int, flt_type> > &class_memberships,
         conf::Config &config,
         string infname,
         double iw=0.5)
{
    double word_iw = log(iw);
    double class_iw = log(1.0-iw);

    cerr << endl << "Evaluating sentences.." << endl;
    SimpleFileInput infile(infname);
    string line;
    long int num_words = 0;
    long int num_sents = 0;
    long int num_oovs = 0;
    double total_ll = 0.0;
    while (infile.getline(line)) {

        line = str::cleaned(line);
        if (line.length() == 0) continue;

        double sent_ll = 0.0;

        vector<string> words;
        string unk = "<unk>";
        preprocess_sent(line, ngram, class_memberships, unk, words, num_words, num_oovs);

        int curr_lm_node = ngram.sentence_start_node;
        int curr_class_lm_node = class_ngram.sentence_start_node;

        for (int i=0; i<(int)words.size(); i++) {
            if (words[i] == unk) {
                if (config["unk-root-node"].specified) {
                    curr_lm_node = ngram.root_node;
                    curr_class_lm_node = class_ngram.root_node;
                }
                else {
                    curr_lm_node = ngram.advance(curr_lm_node, ngram.unk_symbol_idx);
                    curr_class_lm_node = class_ngram.advance(curr_class_lm_node, class_ngram.unk_symbol_idx);
                }
            } else {
                double ngram_score = 0.0;
                curr_lm_node = ngram.score(curr_lm_node, ngram.vocabulary_lookup.at(words[i]), ngram_score);
                ngram_score += word_iw;

                pair<int, flt_type> word_class = class_memberships.at(words[i]);
                double class_score = 0.0;
                curr_class_lm_node = class_ngram.score(curr_class_lm_node, indexmap[word_class.first], class_score);
                class_score += word_class.second;
                class_score += class_iw;

                sent_ll += add_log_domain_probs(ngram_score, class_score);
            }
        }

        double ngram_score = 0.0;
        curr_lm_node = ngram.score(curr_lm_node, ngram.sentence_end_symbol_idx, ngram_score);
        ngram_score += word_iw;

        double class_score = 0.0;
        curr_class_lm_node = class_ngram.score(curr_class_lm_node, class_ngram.sentence_end_symbol_idx, class_score);
        class_score += class_iw;

        sent_ll += add_log_domain_probs(ngram_score, class_score);
        total_ll += sent_ll;
        num_sents++;
    }

    cerr << "Interpolation weight: " << iw << endl;
    cerr << "Number of sentences: " << num_sents << endl;
    cerr << "Number of in-vocabulary words excluding sentence ends: " << num_words-num_sents << endl;
    cerr << "Number of in-vocabulary words including sentence ends: " << num_words << endl;
    cerr << "Number of OOV words: " << num_oovs << endl;
    cerr << "Total log likelihood (ln): " << total_ll << endl;
    cerr << "Total log likelihood (log10): " << total_ll/2.302585092994046 << endl;

    double ppl = exp(-1.0/double(num_words) * total_ll);
    cerr << "Perplexity: " << ppl << endl;

    if (config["num-words"].specified) {
        double wnppl = exp(-1.0/double(config["num-words"].get_int()) * total_ll);
        cerr << "Word-normalized perplexity: " << wnppl << endl;
    }
}


int main(int argc, char* argv[])
{
    try {
        conf::Config config;
        config("usage: classintppl [OPTION...] ARPAFILE CLASS_ARPA CLASS_MEMBERSHIPS INPUT\n")
        ('i', "weights=FLOAT", "arg", "0.5", "Comma separated list of interpolation weights [0.0,1.0] for the word ARPA model")
        ('r', "unk-root-node", "", "", "Pass through root node in contexts with unks, DEFAULT: advance with unk symbol")
        ('w', "num-words=INT", "arg", "", "Number of words for computing word-normalized perplexity")
        ('h', "help", "", "", "display help");
        config.default_parse(argc, argv);
        if (config.arguments.size() != 4) config.print_help(stderr, 1);

        string arpafname = config.arguments[0];
        string classngramfname = config.arguments[1];
        string classmfname = config.arguments[2];
        string infname = config.arguments[3];

        vector<string> str_weights = str::split(config["weights"].get_str(), ",", false);
        vector<double> weights;
        for (int i=0; i<(int)str_weights.size(); i++) {
            bool weight_ok = true;
            double weight;
            try {
                weight = std::stof(str_weights[i]);
                if (weight < 0.0 || weight > 1.0) weight_ok = false;
            } catch (...) {
                weight_ok = false;
            }

            if (weight_ok) {
                weights.push_back(weight);
            } else {
                cerr << "Invalid interpolation weights: " << str_weights[i] << endl;
                exit(EXIT_FAILURE);
            }
        }

        LNNgram ngram;
        ngram.read_arpa(arpafname);

        map<string, pair<int, flt_type> > class_memberships;
        cerr << "Reading class memberships.." << endl;
        int num_classes = read_class_memberships(classmfname, class_memberships);

        cerr << "Reading class n-gram model.." << endl;
        LNNgram class_ngram;
        class_ngram.read_arpa(classngramfname);

        // The class indexes are stored as strings in the n-gram class
        vector<int> indexmap(num_classes);
        for (int i=0; i<(int)indexmap.size(); i++)
            if (class_ngram.vocabulary_lookup.find(int2str(i)) != class_ngram.vocabulary_lookup.end())
                indexmap[i] = class_ngram.vocabulary_lookup[int2str(i)];
            else indexmap[i] = -1;

        cerr << "evaluating " << weights.size() << " interpolation weights: ";
        for (int i=0; i<(int)weights.size(); i++)
            cerr << (i>0 ? ", " : "") << weights[i];
        cerr << endl;
        for (int i=0; i<(int)weights.size(); i++)
            evaluate(ngram, class_ngram, indexmap, class_memberships, config, infname, weights[i]);

        exit(EXIT_SUCCESS);

    } catch (string &e) {
        cerr << e << endl;
    }
}
