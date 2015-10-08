#include <sstream>
#include <cmath>

#include "Exchange.hh"
#include "io.hh"

using namespace std;


Exchange::Exchange(int num_classes, std::string fname)
    : m_num_classes(num_classes+2)
{
    read_corpus(fname);
    initialize_classes();
    set_class_counts();
}


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

    m_vocabulary.push_back("<s>");
    m_vocabulary_lookup["<s>"] = m_vocabulary.size() - 1;
    m_vocabulary.push_back("</s>");
    m_vocabulary_lookup["</s>"] = m_vocabulary.size() - 1;
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

        sent.push_back(m_vocabulary_lookup["<s>"]);
        while (ss >> token) sent.push_back(m_vocabulary_lookup[token]);
        sent.push_back(m_vocabulary_lookup["</s>"]);

        for (unsigned int i=0; i<sent.size(); i++)
            m_word_counts[sent[i]]++;
        for (unsigned int i=0; i<sent.size()-1; i++) {
            m_word_bigram_counts[sent[i]][sent[i+1]]++;
            m_word_rev_bigram_counts[sent[i+1]][sent[i]]++;
        }
        num_tokens += sent.size()-2;
    }
    cerr << " " << num_tokens << " tokens" << endl;
}


void
Exchange::initialize_classes()
{
    multimap<int, int> sorted_words;
    for (unsigned int i=0; i<m_word_counts.size(); ++i) {
        if (m_vocabulary[i].find("<") != string::npos) continue;
        sorted_words.insert(make_pair(m_word_counts[i], i));
    }

    m_classes.resize(m_num_classes);
    m_word_classes.resize(m_vocabulary.size());
    unsigned int class_idx_helper = 2;
    for (auto swit=sorted_words.rbegin(); swit != sorted_words.rend(); ++swit) {
        unsigned int class_idx = class_idx_helper % m_num_classes;
        m_word_classes[swit->second] = class_idx;
        m_classes[class_idx].insert(swit->second);

        class_idx_helper++;
        while (class_idx_helper % m_num_classes == START_CLASS ||
               class_idx_helper % m_num_classes == UNK_CLASS)
            class_idx_helper++;

    }
    m_word_classes[m_vocabulary_lookup["<s>"]] = START_CLASS;
    m_word_classes[m_vocabulary_lookup["</s>"]] = START_CLASS;
}


void
Exchange::set_class_counts()
{
    m_class_counts.resize(m_num_classes, 0);
    m_class_bigram_counts.resize(m_num_classes);
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
}


double
Exchange::log_likelihood() const
{
    double ll = 0.0;
    for (auto cbg1=m_class_bigram_counts.cbegin(); cbg1 != m_class_bigram_counts.cend(); ++cbg1)
        for (auto cbg2=cbg1->cbegin(); cbg2 != cbg1->cend(); ++cbg2)
            ll += cbg2->second * log(cbg2->second);
    for (auto wit=m_word_counts.begin(); wit != m_word_counts.end(); ++wit)
        ll += (*wit) * log(*wit);
    for (auto cit=m_class_counts.begin(); cit != m_class_counts.end(); ++cit)
        if (*cit != 0) ll -= 2* (*cit) * log(*cit);

    return ll;
}


double
Exchange::evaluate_exchange(int word,
                            int curr_class,
                            int tentative_class) const
{
    double ll_diff = 0.0;
    int wc = m_word_counts[word];

    ll_diff += 2 * (m_class_counts[curr_class]) * log(m_class_counts[curr_class]);
    ll_diff -= 2 * (m_class_counts[curr_class]-wc) * log(m_class_counts[curr_class]-wc);
    ll_diff += 2 * (m_class_counts[tentative_class]) * log(m_class_counts[tentative_class]);
    ll_diff -= 2 * (m_class_counts[tentative_class]+wc) * log(m_class_counts[tentative_class]+wc);

    map<pair<int, int>, int> count_diffs;
    const map<int, int> &bctxt = m_word_bigram_counts.at(word);
    for (auto wit = bctxt.begin(); wit != bctxt.end(); ++wit) {
        if (wit->first == word) continue;
        int tgt_class = m_word_classes[wit->first];
        count_diffs[make_pair(curr_class, tgt_class)] -= wit->second;
        count_diffs[make_pair(tentative_class, tgt_class)] += wit->second;
    }

    const map<int, int> &rbctxt = m_word_rev_bigram_counts.at(word);
    for (auto wit = rbctxt.begin(); wit != rbctxt.end(); ++wit) {
        if (wit->first == word) continue;
        int src_class = m_word_classes[wit->first];
        count_diffs[make_pair(src_class, curr_class)] -= wit->second;
        count_diffs[make_pair(src_class, tentative_class)] += wit->second;
    }

    auto wit = bctxt.find(word);
    if (wit != bctxt.end()) {
        count_diffs[make_pair(curr_class,curr_class)] -= wit->second;
        count_diffs[make_pair(tentative_class,tentative_class)] += wit->second;
    }

    for (auto cdit=count_diffs.begin(); cdit != count_diffs.end(); ++cdit) {
        int src_class = cdit->first.first;
        int tgt_class = cdit->first.second;
        int curr_count = m_class_bigram_counts[src_class].at(tgt_class);
        int new_count = curr_count + cdit->second;
        ll_diff -= curr_count * log(curr_count);
        ll_diff += new_count * log(new_count);
    }

    return ll_diff;
}


void
Exchange::do_exchange(int word,
                      int prev_class,
                      int new_class)
{
    int wc = m_word_counts[word];
    m_class_counts[prev_class] -= wc;
    m_class_counts[new_class] += wc;

    map<int, int> &bctxt = m_word_bigram_counts[word];
    for (auto wit = bctxt.begin(); wit != bctxt.end(); ++wit) {
        if (wit->first == word) continue;
        int tgt_class = m_word_classes[wit->first];
        m_class_bigram_counts[prev_class][tgt_class] -= wit->second;
        m_class_bigram_counts[new_class][tgt_class] += wit->second;
    }

    map<int, int> &rbctxt = m_word_rev_bigram_counts[word];
    for (auto wit = rbctxt.begin(); wit != rbctxt.end(); ++wit) {
        if (wit->first == word) continue;
        int src_class = m_word_classes[wit->first];
        m_class_bigram_counts[src_class][prev_class] -= wit->second;
        m_class_bigram_counts[src_class][new_class] += wit->second;
    }

    auto wit = bctxt.find(word);
    if (wit != bctxt.end()) {
        m_class_bigram_counts[prev_class][prev_class] -= wit->second;
        m_class_bigram_counts[new_class][new_class] += wit->second;
    }

    m_classes[prev_class].erase(word);
    m_classes[new_class].insert(word);
    m_word_classes[word] = new_class;
}


double
Exchange::iterate()
{
    for (int widx=0; widx < (int)m_vocabulary.size(); widx++) {
        if (m_word_classes[widx] == START_CLASS || m_word_classes[widx == UNK_CLASS]) continue;
        vector<double> ll_diffs(m_classes.size(), -1e20);
        int curr_class = m_word_classes[widx];
        for (int cidx=2; cidx<(int)m_classes.size() && cidx != curr_class; cidx++)
        {
            double ll_diff = evaluate_exchange(widx, curr_class, cidx);
            ll_diffs[widx] = ll_diff;
        }
    }

    return log_likelihood();
}

