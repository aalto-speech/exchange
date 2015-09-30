#ifndef EXCHANGE
#define EXCHANGE

#include <map>
#include <set>
#include <string>
#include <vector>



class Exchange {
public:
    Exchange(int num_classes) : m_num_classes(num_classes) { };
    Exchange(int num_classes, std::string fname) : m_num_classes(num_classes) { read_corpus(fname); };
    ~Exchange() { };

    void read_corpus(std::string fname);
    void initialize_classes();
    double likelihood();

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
    std::vector<std::map<int, int> > m_class_rev_bigram_counts;
};


#endif /* EXCHANGE */

