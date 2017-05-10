#include <fstream>
#include <iostream>
#include <sstream>

#include "str.hh"
#include "defs.hh"
#include "conf.hh"
#include "Ngram.hh"

using namespace std;


int main(int argc, char* argv[]) {

    conf::Config config;
    config("usage: ngramppl [OPTION...] ARPAFILE INPUT\n")
    ('r', "use-root-node", "", "", "Pass through root node in contexts with unks, DEFAULT: advance with unk symbol")
    ('w', "num-words=INT", "arg", "", "Number of words for computing word-normalized perplexity")
    ('h', "help", "", "", "display help");
    config.default_parse(argc, argv);
    if (config.arguments.size() != 2) config.print_help(stderr, 1);

    string arpafname = config.arguments[0];
    string infname = config.arguments[1];

    string unk = "<unk>";
    bool root_unk_states = config["use-root-node"].specified;

    LNNgram lm;
    lm.read_arpa(arpafname);

    SimpleFileInput infile(infname);
    string line;
    long int num_words = 0;
    long int num_sents = 0;
    long int num_oov = 0;
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
            words.push_back(word);
        }
        words.push_back(lm.sentence_end_symbol);

        double sent_ll = 0.0;
        int node_id = lm.sentence_start_node;
        for (auto wit=words.begin(); wit != words.end(); ++wit) {
            double score = 0.0;
            if (lm.vocabulary_lookup.find(*wit) != lm.vocabulary_lookup.end()
                && lm.vocabulary_lookup.at(*wit) != lm.unk_symbol_idx)
            {
                int sym = lm.vocabulary_lookup[*wit];
                node_id = lm.score(node_id, sym, score);
                sent_ll += score;
                num_words++;
            }
            else {
                if (root_unk_states) node_id = lm.root_node; // SRILM
                else node_id = lm.advance(node_id, lm.unk_symbol_idx); // VariKN style UNKs
                num_oov++;
            }
        }

        total_ll += sent_ll;
        num_sents++;
    }

    cerr << endl;
    cerr << "Number of sentences: " << num_sents << endl;
    cerr << "Number of in-vocabulary words exluding sentence ends: " << num_words-num_sents << endl;
    cerr << "Number of in-vocabulary words including sentence ends: " << num_words << endl;
    cerr << "Number of OOV words: " << num_oov << endl;
    cerr << "Total log likelihood (ln): " << total_ll << endl;
    cerr << "Total log likelihood (log10): " << total_ll/2.302585092994046 << endl;

    if (config["num-words"].specified)
        num_words = config["num-words"].get_int();
    double ppl = exp(-1.0/double(num_words) * total_ll);
    cerr << "Perplexity: " << ppl << endl;

    exit(0);
}
