#ifndef NGRAM_HH
#define NGRAM_HH

#include <map>
#include <string>
#include <vector>

#include "io.hh"

class Ngram {
public:

    class Node {
    public:
        Node() : prob(0.0), backoff_prob(0.0), backoff_node(-1),
            first_arc(-1), last_arc(-1) { }
        double prob;
        double backoff_prob;
        int backoff_node;
        int first_arc;
        int last_arc;
    };

    Ngram() : root_node(0),
        sentence_start_node(-1),
        sentence_start_symbol_idx(-1),
        sentence_start_symbol("<s>"),
        sentence_end_symbol_idx(-1),
        sentence_end_symbol("</s>"),
        unk_symbol_idx(-1),
        unk_symbol("<unk>"),
        max_order(-1) { };
    ~Ngram() {};
    virtual void read_arpa(std::string arpafname);
    virtual void write_arpa(std::string arpafname);
    int score(int node_idx, int word, double &score) const;
    int score(int node_idx, int word, float &score) const;
    int advance(int node_idx, int word) const { float tmp; return score(node_idx, word, tmp); }
    int order() { return max_order; };
    void get_reverse_bigrams(std::map<int, std::vector<int> > &reverse_bigrams);

    int root_node;
    int sentence_start_node;
    int sentence_start_symbol_idx;
    std::string sentence_start_symbol;

    int sentence_end_symbol_idx;
    std::string sentence_end_symbol;

    int unk_symbol_idx;
    std::string unk_symbol;

    std::vector<std::string> vocabulary;
    std::map<std::string, int> vocabulary_lookup;


//private:

    class NgramInfo {
    public:
        NgramInfo() : prob(0.0), backoff_prob(0.0) { }
        std::vector<int> ngram;
        double prob;
        double backoff_prob;
        bool operator<(const NgramInfo &ngri) const
        {
            if (ngram.size() != ngri.ngram.size())
                throw std::string("Comparing ngrams of different order");
            for (unsigned int i=0; i<ngram.size(); i++)
                if (ngram[i] < ngri.ngram[i]) return true;
                else if (ngri.ngram[i] < ngram[i]) return false;
            std::cerr << "Warning, comparing same n-grams" << std::endl;
            return false;
        }
    };

    int find_node(int node_idx, int word) const;
    int read_arpa_read_order(SimpleFileInput &arpafile,
                             std::vector<NgramInfo> &order_ngrams,
                             std::string &line,
                             int curr_ngram_order,
                             int &linei);
    void read_arpa_insert_order_to_tree(std::vector<NgramInfo> &order_ngrams,
                                        int &curr_node_idx,
                                        int &curr_arc_idx,
                                        int curr_order);

    std::vector<Node> nodes;
    std::vector<int> arc_words;
    std::vector<int> arc_target_nodes;
    std::map<int, int> ngram_counts_per_order;
    int max_order;
};

class LNNgram : public Ngram {
public:
    void read_arpa(std::string arpafname);
    void write_arpa(std::string arpafname);
    void multiply_probs(double multiplier);
};


#endif

