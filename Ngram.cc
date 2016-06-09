#include <algorithm>
#include <fstream>
#include <iostream>

#include "Ngram.hh"
#include "str.hh"
#include "defs.hh"

using namespace std;


int
Ngram::score(int node_idx, int word, double &score) const
{
    while (true) {
        int tmp = find_node(node_idx, word);
        if (tmp != -1) {
            node_idx = tmp;
            score += nodes[node_idx].prob;
            if (nodes[node_idx].first_arc == -1)
                return nodes[node_idx].backoff_node;
            else
                return node_idx;
        }
        else {
            score += nodes[node_idx].backoff_prob;
            node_idx = nodes[node_idx].backoff_node;
        }
    }

    throw string("Problem in assigning an n-gram score.");
}


int
Ngram::score(int node_idx, int word, float &score) const
{
    while (true) {
        int tmp = find_node(node_idx, word);
        if (tmp != -1) {
            node_idx = tmp;
            score += nodes[node_idx].prob;
            if (nodes[node_idx].first_arc == -1)
                return nodes[node_idx].backoff_node;
            else
                return node_idx;
        }
        else {
            score += nodes[node_idx].backoff_prob;
            node_idx = nodes[node_idx].backoff_node;
        }
    }

    throw string("Problem in assigning an n-gram score.");
}


void
Ngram::get_reverse_bigrams(map<int, vector<int> > &reverse_bigrams)
{
    if (order() != 2) throw string("Error, not a bigram model.");

    Node &root_nd = nodes[root_node];
    for (int i=root_nd.first_arc; i<=root_nd.last_arc; i++) {
        int first_word = arc_words[i];
        Node &arc_target_node = nodes[arc_target_nodes[i]];
        for (int j=arc_target_node.first_arc; j<=arc_target_node.last_arc; j++) {
            int second_word = arc_words[j];
            reverse_bigrams[second_word].push_back(first_word);
        }
    }
}


int
Ngram::find_node(int node_idx, int word) const
{
    int first_arc = nodes[node_idx].first_arc;
    if (first_arc == -1) return -1;
    int last_arc = nodes[node_idx].last_arc+1;

    auto lower_b=lower_bound(arc_words.begin()+first_arc, arc_words.begin()+last_arc, word);
    int arc_idx = lower_b-arc_words.begin();
    if (arc_idx == last_arc || *lower_b != word) return -1;
    return arc_target_nodes[arc_idx];
}


void _getline(SimpleFileInput &sfi, string &line, int &linei) {
    const string read_error("Problem reading ARPA file");
    if (!sfi.getline(line)) throw read_error;
    str::clean(line);
    linei++;
}


void
Ngram::write_arpa(string arpafname) {

    SimpleFileOutput arpafile(arpafname);

    arpafile << "\n";
    arpafile << "\\data\\\n";
    for (int order=1; order<=max_order; order++)
        arpafile << "ngram " << order << "=" << ngram_counts_per_order.at(order) << "\n";

    vector<pair<int, vector<string> > > order_nodes, next_order_nodes;
    order_nodes.push_back(make_pair(root_node, vector<string>()));

    for (int order=1; order<=max_order; order++) {
        arpafile << "\n";
        arpafile << "\\" << order << "-grams:\n";
        for (auto cnit = order_nodes.begin(); cnit != order_nodes.end(); ++cnit) {
            int curr_node = cnit->first;
            vector<string> &ctxt = cnit->second;
            Node &nd = nodes[curr_node];
            if (nd.first_arc == -1) continue;
            for (int a=nd.first_arc; a<=nd.last_arc; a++) {
                int target_node_idx = arc_target_nodes[a];
                Node &target_nd = nodes[target_node_idx];
                string word = vocabulary[arc_words[a]];
                arpafile << target_nd.prob << "\t";
                for (auto ctxtit = ctxt.begin(); ctxtit != ctxt.end(); ++ctxtit)
                    arpafile << *ctxtit << " ";
                arpafile << word;
                if (target_nd.backoff_prob != 0.0) arpafile << "\t" << target_nd.backoff_prob;
                arpafile << "\n";
                vector<string> new_ctxt(ctxt);
                new_ctxt.push_back(word);
                next_order_nodes.push_back(make_pair(target_node_idx, new_ctxt));
            }
        }
        order_nodes.swap(next_order_nodes);
        next_order_nodes.clear();
    }

    arpafile << "\n\\end\\\n";
}


void
Ngram::read_arpa(string arpafname) {

    SimpleFileInput arpafile(arpafname);
    string header_error("Invalid ARPA header");

    int linei = 0;
    string line;
    _getline(arpafile, line, linei);
    while (line.length() == 0)
        _getline(arpafile, line, linei);

    if (line.find("\\data\\") == string::npos) throw header_error;
    _getline(arpafile, line, linei);

    int curr_ngram_order = 1;
    int total_ngram_count = 0;
    while (line.length() > 0) {
        if (line.find("ngram ") == string::npos) throw header_error;
        stringstream vals(line);
        getline(vals, line, '=');
        getline(vals, line, '=');
        int count = str2int(line);
        ngram_counts_per_order[curr_ngram_order] = count;
        total_ngram_count += count;
        curr_ngram_order++;
        _getline(arpafile, line, linei);
    }

    int max_ngram_order = curr_ngram_order-1;
    int total_ngrams_read = 0;
    curr_ngram_order = 1;
    nodes.resize(total_ngram_count+1);
    arc_words.resize(total_ngram_count);
    arc_target_nodes.resize(total_ngram_count);
    int curr_node_idx = 1;
    int curr_arc_idx = 0;

    while (curr_ngram_order <= max_ngram_order) {

        while (line.length() == 0)
            _getline(arpafile, line, linei);
        if (line.find("-grams") == string::npos) {
            cerr << line;
            throw header_error;
        }

        _getline(arpafile, line, linei);
        vector<NgramInfo> order_ngrams;
        int ngrams_read = read_arpa_read_order(arpafile, order_ngrams, line, curr_ngram_order, linei);

        cerr << "n-grams for order " << curr_ngram_order << ": " << ngrams_read << endl;
        if (ngrams_read != ngram_counts_per_order[curr_ngram_order])
            throw string("Invalid number of n-grams for order: " + curr_ngram_order);

        sort(order_ngrams.begin(), order_ngrams.end());

        read_arpa_insert_order_to_tree(order_ngrams, curr_node_idx, curr_arc_idx, curr_ngram_order);

        max_order = curr_ngram_order;
        curr_ngram_order++;
        total_ngrams_read += ngrams_read;
    }

    sentence_start_symbol_idx = vocabulary_lookup[sentence_start_symbol];
    sentence_start_node = find_node(root_node, sentence_start_symbol_idx);
    if (sentence_start_node == -1) throw string("Sentence start symbol not found.");
}


int
Ngram::read_arpa_read_order(SimpleFileInput &arpafile,
                            vector<NgramInfo> &order_ngrams,
                            string &line,
                            int curr_ngram_order,
                            int &linei)
{
    int ngrams_read = 0;

    while (line.length() > 0) {
        stringstream vals(line);

        NgramInfo ngram;
        vals >> ngram.prob;
        if (ngram.prob > 0.0) {
            throw string("Invalid log probability " + line);
        }

        vector<string> curr_ngram_str;
        string tmp;
        for (int i=0; i<curr_ngram_order; i++) {
            if (vals.eof()) throw string("Invalid ARPA line");
            vals >> tmp;
            curr_ngram_str.push_back(tmp);
        }
        if (curr_ngram_order == 1) {
            vocabulary.push_back(curr_ngram_str[0]);
            vocabulary_lookup[curr_ngram_str[0]] = vocabulary.size()-1;
        }

        vector<int> curr_ngram_words;
        for (auto sit = curr_ngram_str.begin(); sit != curr_ngram_str.end(); ++sit)
            curr_ngram_words.push_back(vocabulary_lookup[*sit]);
        ngram.ngram = curr_ngram_words;

        if (!vals.eof())
            vals >> ngram.backoff_prob;

        order_ngrams.push_back(ngram);
        _getline(arpafile, line, linei);
        ngrams_read++;
    }

    return ngrams_read;
}


void
Ngram::read_arpa_insert_order_to_tree(std::vector<NgramInfo> &order_ngrams,
                                      int &curr_node_idx,
                                      int &curr_arc_idx,
                                      int curr_order)
{
    for (unsigned int ni=0; ni<order_ngrams.size(); ni++) {

        int node_idx_traversal = root_node;
        for (unsigned int i=0; i<order_ngrams[ni].ngram.size()-1; i++) {
            node_idx_traversal = find_node(node_idx_traversal, order_ngrams[ni].ngram[i]);
            if (node_idx_traversal == -1) throw string("Missing lower order n-gram");
        }
        int tmp = find_node(node_idx_traversal, order_ngrams[ni].ngram.back());
        if (tmp != -1)
            throw string("Duplicate n-gram in model");

        if (nodes[node_idx_traversal].first_arc == -1)
            nodes[node_idx_traversal].first_arc = curr_arc_idx;
        nodes[node_idx_traversal].last_arc = curr_arc_idx;

        arc_words[curr_arc_idx] = order_ngrams[ni].ngram.back();
        arc_target_nodes[curr_arc_idx] = curr_node_idx;
        nodes[curr_node_idx].prob = order_ngrams[ni].prob;
        nodes[curr_node_idx].backoff_prob = order_ngrams[ni].backoff_prob;

        int ctxt_start = 1;
        while (true) {
            int bo_traversal = root_node;
            unsigned int i = ctxt_start;
            for (; i<order_ngrams[ni].ngram.size(); i++) {
                int tmp = find_node(bo_traversal, order_ngrams[ni].ngram[i]);
                if (tmp == -1) break;
                bo_traversal = tmp;
            }
            if (i >= order_ngrams[ni].ngram.size()) {
                nodes[curr_node_idx].backoff_node = bo_traversal;
                break;
            }
            else ctxt_start++;
        }

        curr_arc_idx++;
        curr_node_idx++;
    }
}
