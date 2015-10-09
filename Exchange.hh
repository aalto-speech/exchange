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
    Exchange(int num_classes, std::string fname);
    ~Exchange() { };

    void read_corpus(std::string fname);
    void write_word_classes(std::string fname) const;
    void write_classes(std::string fname) const;
    void initialize_classes();
    void set_class_counts();
    double log_likelihood() const;
    double evaluate_exchange(int word,
                             int curr_class,
                             int tentative_class) const;
    double evaluate_exchange_2(int word,
                               int curr_class,
                               int tentative_class) const;
    void do_exchange(int word,
                     int prev_class,
                     int new_class);
    double iterate(int ll_print_interval=10000);

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
    std::vector<std::map<int, int> > m_class_bigram_counts;

    std::vector<std::map<int, int> > m_class_word_counts;
    std::vector<std::map<int, int> > m_word_class_counts;
};


#endif /* EXCHANGE */

