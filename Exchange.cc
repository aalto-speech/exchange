#include <sstream>

#include "Exchange.hh"
#include "io.hh"

using namespace std;


void
Exchange::read_corpus(string fname) {

    string line;

    SimpleFileInput corpusf(fname);
    set<string> word_types;
    while (corpusf.getline(line)) {
        stringstream ss(line);
        string token;
        while (ss >> token) word_types.insert(token);
    }

    for (auto wit=word_types.begin(); wit != word_types.end(); ++wit) {
        m_vocabulary.push_back(*wit);
        m_vocabulary_lookup[*wit] = m_vocabulary.size() - 1;
    }
    word_types.clear();

    SimpleFileInput corpusf2(fname);
    while (corpusf2.getline(line)) {
        vector<int> sent;
        stringstream ss(line);
        string token;
        while (ss >> token) sent.push_back(m_vocabulary_lookup[token]);

        for (unsigned int i=0; i<sent.size(); i++)
            m_word_counts[sent[i]]++;
        for (unsigned int i=0; i<sent.size()-1; i++) {
            m_word_bigram_counts[sent[i]][sent[i+1]]++;
            m_word_rev_bigram_counts[sent[i+1]][sent[i]]++;
        }
    }

}
