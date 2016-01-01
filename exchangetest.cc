#define BOOST_TEST_MODULE Hello
#include <boost/test/unit_test.hpp>

#include <iostream>
#include <vector>
#include <map>
#include <ctime>

#define private public
#include "ExchangeAlgorithm.hh"
#undef private

using namespace std;


void
print_class_bigram_counts(vector<map<int, int> > &class_bigram_counts)
{
    for (unsigned int swi=0; swi<class_bigram_counts.size(); swi++) {
        map<int, int> &bctxt = class_bigram_counts[swi];
        for (auto bgit=bctxt.begin(); bgit != bctxt.end(); ++bgit)
            cerr << swi << " " << bgit->first << ": " << bgit->second << endl;
    }
}


void
print_classes(Exchange &e)
{
    for (unsigned int c=0; c<e.m_classes.size(); c++) {
        cerr << "Class " << c << ":";
        set<int> &words = e.m_classes[c];
        for (auto wit=words.begin(); wit != words.end(); ++wit)
            cerr << " " << e.m_vocabulary[*wit];
        cerr << endl;
    }
}


void
assert_same(Exchange &e1,
            Exchange &e2)
{
    BOOST_CHECK( e1.m_num_classes == e2.m_num_classes );
    BOOST_CHECK( e1.m_vocabulary == e2.m_vocabulary );
    BOOST_CHECK( e1.m_vocabulary_lookup == e2.m_vocabulary_lookup );
    BOOST_CHECK( e1.m_classes == e2.m_classes );
    BOOST_CHECK( e1.m_word_classes == e2.m_word_classes );
    BOOST_CHECK( e1.m_word_counts == e2.m_word_counts );
    BOOST_CHECK( e1.m_word_bigram_counts == e2.m_word_bigram_counts );
    BOOST_CHECK( e1.m_word_rev_bigram_counts == e2.m_word_rev_bigram_counts );
    BOOST_CHECK( e1.m_class_counts == e2.m_class_counts );
    BOOST_CHECK( e1.m_class_bigram_counts == e2.m_class_bigram_counts );
    BOOST_CHECK( e1.m_word_class_counts == e2.m_word_class_counts );
    BOOST_CHECK( e1.m_class_word_counts == e2.m_class_word_counts );
}


// Test that data is read and things set up properly
BOOST_AUTO_TEST_CASE(DataRead)
{
    cerr << endl;

    long unsigned int num_classes = 2;
    long unsigned int num_words = 8;

    Exchange e(num_classes, "test/corpus1.txt");

    BOOST_CHECK_EQUAL( num_classes+2, (long unsigned int)e.m_num_classes );
    BOOST_CHECK_EQUAL( num_words, e.m_vocabulary.size() );
    BOOST_CHECK_EQUAL( num_words, e.m_vocabulary_lookup.size() );
    BOOST_CHECK_EQUAL( num_classes+2, e.m_classes.size() );
    BOOST_CHECK_EQUAL( num_words, e.m_word_classes.size() );
    BOOST_CHECK_EQUAL( num_words, e.m_word_counts.size() );
    BOOST_CHECK_EQUAL( num_words, e.m_word_bigram_counts.size() );
    BOOST_CHECK_EQUAL( num_words, e.m_word_rev_bigram_counts.size() );
    BOOST_CHECK_EQUAL( num_classes+2, e.m_class_counts.size() );
    BOOST_CHECK_EQUAL( num_classes+2, e.m_class_bigram_counts.size() );
}


// Test that moving words between classes handles counts correctly
BOOST_AUTO_TEST_CASE(DoExchange)
{
    cerr << endl;
    Exchange e(2, "test/corpus1.txt");

    vector<int> orig_class_counts = e.m_class_counts;
    vector<vector<int> > orig_class_bigram_counts = e.m_class_bigram_counts;
    vector<map<int, int> > orig_class_word_counts = e.m_class_word_counts;
    vector<map<int, int> > orig_word_class_counts = e.m_word_class_counts;

    int widx = e.m_vocabulary_lookup["d"];
    int curr_class = e.m_word_classes[widx];
    int new_class = (curr_class == 3) ? 2 : 3;

    e.do_exchange(widx, curr_class, new_class);

    BOOST_CHECK( orig_class_counts != e.m_class_counts );
    BOOST_CHECK( orig_class_bigram_counts != e.m_class_bigram_counts );
    BOOST_CHECK( orig_class_word_counts != e.m_class_word_counts );
    BOOST_CHECK( orig_word_class_counts != e.m_word_class_counts );

    e.do_exchange(widx, new_class, curr_class);

    BOOST_CHECK( orig_class_counts == e.m_class_counts );
    BOOST_CHECK( orig_class_bigram_counts == e.m_class_bigram_counts );
    BOOST_CHECK( orig_class_word_counts == e.m_class_word_counts );
    BOOST_CHECK( orig_word_class_counts == e.m_word_class_counts );
}


// Another test for count updates
BOOST_AUTO_TEST_CASE(DoExchange2)
{
    cerr << endl;

    Exchange e_ref(2);
    e_ref.read_corpus("test/corpus1.txt");
    e_ref.initialize_classes_by_random();

    int widx = e_ref.m_vocabulary_lookup["d"];
    int curr_class = e_ref.m_word_classes[widx];
    int new_class = (curr_class == 3) ? 2 : 3;

    e_ref.m_classes[curr_class].erase(widx);
    e_ref.m_classes[new_class].insert(widx);
    e_ref.m_word_classes[widx] = new_class;

    e_ref.set_class_counts();

    Exchange e_test(2, "test/corpus1.txt");
    e_test.do_exchange(widx, curr_class, new_class);

    assert_same( e_ref, e_test );
}


// Test for evaluating ll change for one exchange
BOOST_AUTO_TEST_CASE(EvalExchange)
{
    cerr << endl;
    Exchange e(2, "test/corpus1.txt");

    int widx = e.m_vocabulary_lookup["d"];
    int curr_class = e.m_word_classes[widx];
    int new_class = (curr_class == 3) ? 2 : 3;

    double evaluated_ll_diff = e.evaluate_exchange(widx, curr_class, new_class);
    double orig_ll = e.log_likelihood();
    e.do_exchange(widx, curr_class, new_class);
    double ref_ll = e.log_likelihood();

    BOOST_CHECK_EQUAL( orig_ll+evaluated_ll_diff, ref_ll );
}


// Test for checking evaluation time
BOOST_AUTO_TEST_CASE(EvalExchangeTime)
{
    cerr << endl;
    Exchange e(2, "test/corpus1.txt");

    int widx = e.m_vocabulary_lookup["d"];
    int curr_class = e.m_word_classes[widx];
    int new_class = (curr_class == 3) ? 2 : 3;

    time_t t1, t2;
    t1 = time(0);
    for (int i=0; i<5000000; i++)
        e.evaluate_exchange(widx, curr_class, new_class);
    t2 = time(0);
    cerr << "Seconds elapsed: " << (t2-t1) << endl;
    BOOST_CHECK( (t2-t1) < 5 );
}


// Test for merging two classes
BOOST_AUTO_TEST_CASE(EvalMerge)
{
    cerr << endl;

    Exchange e_ref(3);
    e_ref.read_corpus("test/corpus1.txt");
    e_ref.initialize_classes_by_random();
    e_ref.set_class_counts();

    for (unsigned int i=0; i<e_ref.m_vocabulary.size(); i++)
        if (e_ref.m_word_classes[i] == 4)
            e_ref.do_exchange(i, 4, 3);
    double ref_ll = e_ref.log_likelihood();

    Exchange e_test(3);
    e_test.read_corpus("test/corpus1.txt");
    e_test.initialize_classes_by_random();
    e_test.set_class_counts();
    e_test.do_merge(3, 4);
    double test_ll = e_test.log_likelihood();

    BOOST_CHECK_EQUAL(ref_ll, test_ll);
}

