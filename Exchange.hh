#ifndef EXCHANGE
#define EXCHANGE

#include <map>
#include <set>
#include <string>
#include <vector>


#define START_CLASS 0
#define UNK_CLASS 1


class Exchange {
public:
    Exchange(int num_classes) : m_num_classes(num_classes+2) { };
    Exchange(int num_classes,
             std::string fname,
             std::string vocab_fname="");
    ~Exchange() { };

    void read_corpus(std::string fname,
                     std::string vocab_fname="");
    void write_word_classes(std::string fname) const;
    void write_classes(std::string fname) const;
    void initialize_classes();
    void set_class_counts();
    double log_likelihood() const;
    double evaluate_exchange(int word,
                             int curr_class,
                             int tentative_class) const;
    void do_exchange(int word,
                     int prev_class,
                     int new_class);
    double iterate(int max_iter=0,
                   int max_seconds=0,
                   int ll_print_interval=0,
                   int num_threads=1);

    void evaluate_thr(int num_threads,
                      int word_index,
                      int curr_class,
                      int &best_class,
                      double &best_ll_diff);
    void evaluate_thr_worker(int num_threads,
                             int thread_index,
                             int word_index,
                             int curr_class,
                             int &best_class,
                             double &best_ll_diff);


private:

    int m_num_classes;

    std::vector<std::string> m_vocabulary;
    std::map<std::string, int> m_vocabulary_lookup;

    std::vector<std::set<int> > m_classes;
    std::vector<int> m_word_classes;

    std::vector<int> m_word_counts;
    std::vector<std::map<int, int> > m_word_bigram_counts;
    std::vector<std::map<int, int> > m_word_rev_bigram_counts;

    std::vector<int> m_class_counts;
    std::vector<std::vector<int> > m_class_bigram_counts;

    std::vector<std::map<int, int> > m_class_word_counts;
    std::vector<std::map<int, int> > m_word_class_counts;
};


#endif /* EXCHANGE */

