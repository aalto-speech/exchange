#include <sstream>
#include <cmath>

#include "Exchange.hh"
#include "io.hh"

using namespace std;


void
Exchange::read_corpus(string fname)
{
    string line;

    cerr << "Reading vocabulary..";
    SimpleFileInput corpusf(fname);
    set<string> word_types;
    while (corpusf.getline(line)) {
        stringstream ss(line);
        string token;
        while (ss >> token) word_types.insert(token);
    }
    cerr << " " << word_types.size() << " words" << endl;

    for (auto wit=word_types.begin(); wit != word_types.end(); ++wit) {
        m_vocabulary.push_back(*wit);
        m_vocabulary_lookup[*wit] = m_vocabulary.size() - 1;
    }
    word_types.clear();

    cerr << "Reading word counts..";
    m_word_counts.resize(m_vocabulary.size());
    m_word_bigram_counts.resize(m_vocabulary.size());
    m_word_rev_bigram_counts.resize(m_vocabulary.size());
    SimpleFileInput corpusf2(fname);
    int num_tokens = 0;
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
        num_tokens += sent.size();
    }
    cerr << " " << num_tokens << " tokens" << endl;
}


void
Exchange::initialize_classes()
{
    multimap<int, int> sorted_words;
    for (unsigned int i=0; i<m_word_counts.size(); ++i)
        sorted_words.insert(make_pair(m_word_counts[i], i));

    m_classes.resize(m_num_classes);
    m_word_classes.resize(m_vocabulary.size());
    unsigned int class_idx = 0;
    for (auto swit=sorted_words.rbegin(); swit != sorted_words.rend(); ++swit) {
        m_word_classes[swit->second] = class_idx;
        m_classes[class_idx].insert(swit->second);
        class_idx = (class_idx+1) % m_num_classes;
    }
}


void
Exchange::set_class_counts()
{
    m_class_counts.resize(m_num_classes, 0);
    m_class_bigram_counts.resize(m_num_classes);
    m_class_rev_bigram_counts.resize(m_num_classes);
    for (unsigned int i=0; i<m_word_counts.size(); i++)
        m_class_counts[m_word_classes[i]] += m_word_counts[i];
    for (unsigned int i=0; i<m_word_bigram_counts.size(); i++) {
        int src_class = m_word_classes[i];
        map<int, int> &curr_bigram_ctxt = m_word_bigram_counts[i];
        for (auto bgit = curr_bigram_ctxt.begin(); bgit != curr_bigram_ctxt.end(); ++bgit) {
            int tgt_class = m_word_classes[bgit->first];
            m_class_bigram_counts[src_class][tgt_class] += bgit->second;
        }
    }
    for (unsigned int i=0; i<m_word_rev_bigram_counts.size(); i++) {
        int src_class = m_word_classes[i];
        map<int, int> &curr_rev_bigram_ctxt = m_word_rev_bigram_counts[i];
        for (auto rbgit = curr_rev_bigram_ctxt.begin(); rbgit != curr_rev_bigram_ctxt.end(); ++rbgit) {
            int tgt_class = m_word_classes[rbgit->first];
            m_class_rev_bigram_counts[src_class][tgt_class] += rbgit->second;
        }
    }
}


double
Exchange::log_likelihood() {
    double ll = 0.0;
    for (auto cbg1=m_class_bigram_counts.begin(); cbg1 != m_class_bigram_counts.end(); ++cbg1)
        for (auto cbg2=cbg1->begin(); cbg2 != cbg1->end(); ++cbg2)
            ll += cbg2->second * log(cbg2->second);
    for (auto wit=m_word_counts.begin(); wit != m_word_counts.end(); ++wit)
        ll += (*wit) * log(*wit);
    for (auto cit=m_class_counts.begin(); cit != m_class_counts.end(); ++cit)
        ll -= 2* (*cit) * log(*cit);

    return ll;
}

