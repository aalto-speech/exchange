#include <sstream>
#include <cmath>
#include <ctime>
#include <thread>
#include <functional>
#include <iterator>
#include <algorithm>

#include "Exchange.hh"
#include "io.hh"
#include "defs.hh"

using namespace std;


Exchange::Exchange(int num_classes,
                   string fname,
                   string vocab_fname,
                   string class_fname,
                   unsigned int top_word_classes)
    : m_num_classes(num_classes+2)
{
    read_corpus(fname, vocab_fname);
    if (class_fname.length())
        read_class_initialization(class_fname);
    else
        initialize_classes_by_random(top_word_classes);
    set_class_counts();
}


void
Exchange::read_corpus(string fname,
                      string vocab_fname)
{
    string line;

    cerr << "Reading vocabulary..";
    set<string> word_types;
    SimpleFileInput corpusf(fname);
    while (corpusf.getline(line)) {
        stringstream ss(line);
        string token;
        while (ss >> token) word_types.insert(token);
    }

    if (vocab_fname.length()) {
        set<string> constrained_vocab;
        SimpleFileInput vocabf(vocab_fname);
        while (vocabf.getline(line)) {
            stringstream ss(line);
            string token;
            while (ss >> token) constrained_vocab.insert(token);
        }

        set<string> intersection;
        set_intersection(word_types.begin(), word_types.end(),
                         constrained_vocab.begin(), constrained_vocab.end(),
                         inserter(intersection, intersection.begin()));
        word_types = intersection;
    }
    cerr << " " << word_types.size() << " words" << endl;

    m_vocabulary.push_back("<s>");
    m_vocabulary_lookup["<s>"] = m_vocabulary.size() - 1;
    m_vocabulary.push_back("</s>");
    m_vocabulary_lookup["</s>"] = m_vocabulary.size() - 1;
    m_vocabulary.push_back("<unk>");
    m_vocabulary_lookup["<unk>"] = m_vocabulary.size() - 1;
    for (auto wit=word_types.begin(); wit != word_types.end(); ++wit) {
        if (wit->find("<") != string::npos && *wit != "<w>") continue;
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

    int ss_idx = m_vocabulary_lookup["<s>"];
    int se_idx = m_vocabulary_lookup["</s>"];
    int unk_idx = m_vocabulary_lookup["<unk>"];

    while (corpusf2.getline(line)) {
        vector<int> sent;
        stringstream ss(line);
        string token;

        sent.push_back(ss_idx);
        while (ss >> token) {
            auto vlit = m_vocabulary_lookup.find(token);
            if (vlit != m_vocabulary_lookup.end())
                sent.push_back(vlit->second);
            else sent.push_back(unk_idx);
        }
        sent.push_back(se_idx);

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
Exchange::write_word_classes(string fname) const
{
    SimpleFileOutput mfo(fname);
    for (unsigned int widx = 0; widx < m_vocabulary.size(); widx++)
        mfo << m_vocabulary[widx] << "\t" << m_word_classes[widx] << " 0.000000\n";
    mfo.close();
}


void
Exchange::write_class_mem_probs(string fname) const
{
    SimpleFileOutput mfo(fname);
    mfo << "<s>\t" << START_CLASS << " " << "0.000000" << "\n";
    mfo << "<unk>\t" << UNK_CLASS << " " << "0.000000" << "\n";
    for (unsigned int widx = 0; widx < m_vocabulary.size(); widx++) {
        string word = m_vocabulary[widx];
        if (word.find("<") != string::npos && word != "<w>") continue;
        double lp = log(m_word_counts[widx]);
        lp -= log(m_class_counts[m_word_classes[widx]]);
        mfo << word << "\t" << m_word_classes[widx] << " " << lp << "\n";
    }
    mfo.close();
}


void
Exchange::write_classes(string fname) const
{
    SimpleFileOutput mfo(fname);
    for (int cidx = 0; cidx < m_num_classes; cidx++) {
        mfo << cidx << ": ";
        const set<int> &words = m_classes[cidx];
        for (auto wit=words.begin(); wit != words.end(); ++wit) {
            if (wit != words.begin()) mfo << ",";
            mfo << m_vocabulary[*wit];
        }
        mfo << "\n";
    }
    mfo.close();
}


void
Exchange::initialize_classes_by_random(unsigned int top_word_classes)
{
    multimap<int, int> sorted_words;
    for (unsigned int i=0; i<m_word_counts.size(); ++i) {
        if (m_vocabulary[i].find("<") != string::npos && m_vocabulary[i] != "<w>") continue;
        sorted_words.insert(make_pair(m_word_counts[i], i));
    }

    m_classes.resize(m_num_classes);
    m_word_classes.resize(m_vocabulary.size(), -1);

    if (top_word_classes > 0) {
        unsigned int widx = 0;
        for (auto swit=sorted_words.rbegin(); swit != sorted_words.rend(); ++swit) {
            m_word_classes[swit->second] = widx+2;
            m_classes[widx+2].insert(swit->second);
            if (++widx >= top_word_classes) break;
        }
    }

    unsigned int class_idx_helper = 2 + top_word_classes;
    for (auto swit=sorted_words.rbegin(); swit != sorted_words.rend(); ++swit) {
        if (m_word_classes[swit->second] != -1) continue;

        unsigned int class_idx = class_idx_helper % m_num_classes;
        m_word_classes[swit->second] = class_idx;
        m_classes[class_idx].insert(swit->second);

        class_idx_helper++;
        while (class_idx_helper % m_num_classes < (2 + top_word_classes))
            class_idx_helper++;
    }

    m_word_classes[m_vocabulary_lookup["<s>"]] = START_CLASS;
    m_word_classes[m_vocabulary_lookup["</s>"]] = START_CLASS;
    m_word_classes[m_vocabulary_lookup["<unk>"]] = UNK_CLASS;
    m_classes[START_CLASS].insert(m_vocabulary_lookup["<s>"]);
    m_classes[START_CLASS].insert(m_vocabulary_lookup["</s>"]);
    m_classes[UNK_CLASS].insert(m_vocabulary_lookup["<unk>"]);
}


void
Exchange::read_class_initialization(string class_fname)
{
    cerr << "Reading class initialization from " << class_fname << endl;
    m_word_classes.resize(m_vocabulary.size());

    SimpleFileInput classf(class_fname);
    string line;
    while (classf.getline(line)) {
        if (!line.length()) continue;
        size_t pos = line.find_first_of(":");
        int class_idx = str2int(line.substr(0, pos));
        string words = line.substr(pos+2);
        stringstream wordss(words);
        string token;

        m_classes.resize(m_classes.size()+1);
        while(std::getline(wordss, token, ',')) {
            auto vlit = m_vocabulary_lookup.find(token);
            if (vlit == m_vocabulary_lookup.end()) continue;
            int widx = vlit->second;
            m_word_classes[widx] = class_idx;
            m_classes.back().insert(widx);
        }
    }
}


void
Exchange::set_class_counts()
{
    m_class_counts.resize(m_num_classes, 0);
    m_class_bigram_counts.resize(m_num_classes);
    for (unsigned int i=0; i<m_class_bigram_counts.size(); i++)
        m_class_bigram_counts[i].resize(m_num_classes);
    m_class_word_counts.resize(m_vocabulary.size());
    m_word_class_counts.resize(m_vocabulary.size());

    for (unsigned int i=0; i<m_word_counts.size(); i++)
        m_class_counts[m_word_classes[i]] += m_word_counts[i];
    for (unsigned int i=0; i<m_word_bigram_counts.size(); i++) {
        int src_class = m_word_classes[i];
        map<int, int> &curr_bigram_ctxt = m_word_bigram_counts[i];
        for (auto bgit = curr_bigram_ctxt.begin(); bgit != curr_bigram_ctxt.end(); ++bgit) {
            int tgt_class = m_word_classes[bgit->first];
            m_class_bigram_counts[src_class][tgt_class] += bgit->second;
            m_class_word_counts[bgit->first][src_class] += bgit->second;
            m_word_class_counts[i][tgt_class] += bgit->second;
        }
    }
}


double
Exchange::log_likelihood() const
{
    double ll = 0.0;
    for (auto cbg1=m_class_bigram_counts.cbegin(); cbg1 != m_class_bigram_counts.cend(); ++cbg1)
        for (auto cbg2=cbg1->cbegin(); cbg2 != cbg1->cend(); ++cbg2)
            if (*cbg2 != 0) ll += *cbg2 * log(*cbg2);
    for (auto wit=m_word_counts.begin(); wit != m_word_counts.end(); ++wit)
        if (*wit != 0) ll += (*wit) * log(*wit);
    for (auto cit=m_class_counts.begin(); cit != m_class_counts.end(); ++cit)
        if (*cit != 0) ll -= 2* (*cit) * log(*cit);

    return ll;
}


inline void
evaluate_ll_diff(double &ll_diff,
                 int old_count,
                 int new_count)
{
    if (old_count != 0)
        ll_diff -= old_count * log(old_count);
    if (new_count != 0)
        ll_diff += new_count * log(new_count);
}


inline int
get_count(const map<int, int> &ctxt,
                     int element)
{
    auto it = ctxt.find(element);
    if (it != ctxt.end()) return it->second;
    else return 0;
}


double
Exchange::evaluate_exchange(int word,
                            int curr_class,
                            int tentative_class) const
{
    double ll_diff = 0.0;
    int wc = m_word_counts[word];
    const map<int, int> &wb_ctxt = m_word_bigram_counts.at(word);
    const map<int, int> &cw_counts = m_class_word_counts.at(word);
    const map<int, int> &wc_counts = m_word_class_counts.at(word);

    ll_diff += 2 * (m_class_counts[curr_class]) * log(m_class_counts[curr_class]);
    ll_diff -= 2 * (m_class_counts[curr_class]-wc) * log(m_class_counts[curr_class]-wc);
    ll_diff += 2 * (m_class_counts[tentative_class]) * log(m_class_counts[tentative_class]);
    ll_diff -= 2 * (m_class_counts[tentative_class]+wc) * log(m_class_counts[tentative_class]+wc);

    for (auto wcit=wc_counts.begin(); wcit != wc_counts.end(); ++wcit) {
        if (wcit->first == curr_class) continue;
        if (wcit->first == tentative_class) continue;

        int curr_count = m_class_bigram_counts[curr_class][wcit->first];
        int new_count = curr_count - wcit->second;
        evaluate_ll_diff(ll_diff, curr_count, new_count);

        curr_count = m_class_bigram_counts[tentative_class][wcit->first];
        new_count = curr_count + wcit->second;
        evaluate_ll_diff(ll_diff, curr_count, new_count);
    }

    for (auto wcit=cw_counts.begin(); wcit != cw_counts.end(); ++wcit) {
        if (wcit->first == curr_class) continue;
        if (wcit->first == tentative_class) continue;

        int curr_count = m_class_bigram_counts[wcit->first][curr_class];
        int new_count = curr_count - wcit->second;
        evaluate_ll_diff(ll_diff, curr_count, new_count);

        curr_count = m_class_bigram_counts[wcit->first][tentative_class];
        new_count = curr_count + wcit->second;
        evaluate_ll_diff(ll_diff, curr_count, new_count);
    }

    int self_count = 0;
    auto scit = wb_ctxt.find(word);
    if (scit != wb_ctxt.end()) self_count = scit->second;

    int curr_count = m_class_bigram_counts[curr_class][tentative_class];
    int new_count = curr_count - get_count(wc_counts, tentative_class)
            + get_count(cw_counts, curr_class) - self_count;
    evaluate_ll_diff(ll_diff, curr_count, new_count);

    curr_count = m_class_bigram_counts[tentative_class][curr_class];
    new_count = curr_count - get_count(cw_counts, tentative_class)
            + get_count(wc_counts, curr_class) - self_count;
    evaluate_ll_diff(ll_diff, curr_count, new_count);

    curr_count = m_class_bigram_counts[curr_class][curr_class];
    new_count = curr_count - get_count(wc_counts, curr_class)
            - get_count(cw_counts, curr_class) + self_count;
    evaluate_ll_diff(ll_diff, curr_count, new_count);

    curr_count = m_class_bigram_counts[tentative_class][tentative_class];
    new_count = curr_count + get_count(wc_counts, tentative_class)
            + get_count(cw_counts, tentative_class) + self_count;
    evaluate_ll_diff(ll_diff, curr_count, new_count);

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
        m_class_word_counts[wit->first][prev_class] -= wit->second;
        m_class_word_counts[wit->first][new_class] += wit->second;
    }

    map<int, int> &rbctxt = m_word_rev_bigram_counts[word];
    for (auto wit = rbctxt.begin(); wit != rbctxt.end(); ++wit) {
        if (wit->first == word) continue;
        int src_class = m_word_classes[wit->first];
        m_class_bigram_counts[src_class][prev_class] -= wit->second;
        m_class_bigram_counts[src_class][new_class] += wit->second;
        m_word_class_counts[wit->first][prev_class] -= wit->second;
        m_word_class_counts[wit->first][new_class] += wit->second;
    }

    auto wit = bctxt.find(word);
    if (wit != bctxt.end()) {
        m_class_bigram_counts[prev_class][prev_class] -= wit->second;
        m_class_bigram_counts[new_class][new_class] += wit->second;
        m_class_word_counts[word][prev_class] -= wit->second;
        m_class_word_counts[word][new_class] += wit->second;
        m_word_class_counts[word][prev_class] -= wit->second;
        m_word_class_counts[word][new_class] += wit->second;
    }

    m_classes[prev_class].erase(word);
    m_classes[new_class].insert(word);
    m_word_classes[word] = new_class;
}


double
Exchange::iterate(int max_iter,
                  int max_seconds,
                  int ll_print_interval,
                  int model_write_interval,
                  string model_base,
                  int num_threads)
{
    time_t start_time, curr_time;
    time_t last_model_write_time;
    start_time = time(0);
    last_model_write_time = start_time;
    int tmp_model_idx = 1;

    int curr_iter = 0;
    while (true) {
        for (int widx=0; widx < (int)m_vocabulary.size(); widx++) {

            if (m_word_classes[widx] == START_CLASS ||
                m_word_classes[widx] == UNK_CLASS) continue;

            int curr_class = m_word_classes[widx];
            if (m_classes[curr_class].size() == 1) continue;
            int best_class = -1;
            double best_ll_diff = -1e20;

            if (num_threads > 1) {
                evaluate_thr(num_threads,
                             widx,
                             curr_class,
                             best_class,
                             best_ll_diff);
            }
            else {
                for (int cidx=2; cidx<(int)m_classes.size(); cidx++) {
                    if (cidx == curr_class) continue;
                    double ll_diff = evaluate_exchange(widx, curr_class, cidx);
                    if (ll_diff > best_ll_diff) {
                        best_ll_diff = ll_diff;
                        best_class = cidx;
                    }
                }
            }

            if (best_class == -1 || best_ll_diff == -1e20) {
                cerr << "problem in word: " << m_vocabulary[widx] << endl;
                exit(1);
            }

            if (best_ll_diff > 0.0)
                do_exchange(widx, curr_class, best_class);

            if ((ll_print_interval > 0 && widx % ll_print_interval == 0)
                || widx+1 == (int)m_vocabulary.size()) {
                double ll = log_likelihood();
                cerr << "log likelihood: " << ll << endl;
            }

            if (widx % 1000 == 0) {
                curr_time = time(0);

                if (curr_time-start_time > max_seconds)
                    return log_likelihood();

                if (model_write_interval > 0 && curr_time-last_model_write_time > model_write_interval) {
                    string temp_base = model_base + ".temp" + int2str(tmp_model_idx);
                    write_word_classes(temp_base + ".cgenprobs.gz");
                    write_class_mem_probs(temp_base + ".cmemprobs.gz");
                    write_classes(temp_base + ".classes.gz");
                    last_model_write_time = curr_time;
                    tmp_model_idx++;
                }
            }
        }

        curr_iter++;
        if (max_iter > 0 && curr_iter >= max_iter) return log_likelihood();
    }

    return log_likelihood();
}


void
Exchange::evaluate_thr_worker(int num_threads,
                              int thread_index,
                              int word_index,
                              int curr_class,
                              int &best_class,
                              double &best_ll_diff)
{
    for (int cidx=2; cidx<(int)m_classes.size(); cidx++) {
        if (cidx == curr_class) continue;
        if (cidx % num_threads != thread_index) continue;
        double ll_diff = evaluate_exchange(word_index, curr_class, cidx);
        if (ll_diff > best_ll_diff) {
            best_ll_diff = ll_diff;
            best_class = cidx;
        }
    }
}


void
Exchange::evaluate_thr(int num_threads,
                       int word_index,
                       int curr_class,
                       int &best_class,
                       double &best_ll_diff)
{
    vector<double> thr_ll_diffs(num_threads, -1e20);
    vector<int> thr_best_classes(num_threads, -1);
    vector<std::thread*> workers;
    for (int t=0; t<num_threads; t++) {
        std::thread *worker = new std::thread(&Exchange::evaluate_thr_worker, this,
                                              num_threads, t,
                                              word_index, curr_class,
                                              std::ref(thr_best_classes[t]),
                                              std::ref(thr_ll_diffs[t]) );
        workers.push_back(worker);
    }
    for (int t=0; t<num_threads; t++) {
        workers[t]->join();
        if (thr_ll_diffs[t] > best_ll_diff) {
            best_ll_diff = thr_ll_diffs[t];
            best_class = thr_best_classes[t];
        }
    }
}

