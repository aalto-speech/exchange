#include "exchangetest.hh"
#include "Exchange.hh"

#include <vector>
#include <map>

using namespace std;


CPPUNIT_TEST_SUITE_REGISTRATION (exchangetest);


void exchangetest::setUp (void)
{
}


void exchangetest::tearDown (void)
{
}


void exchangetest::read_fixtures()
{
}


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
    CPPUNIT_ASSERT( e1.m_num_classes == e2.m_num_classes );
    CPPUNIT_ASSERT( e1.m_vocabulary == e2.m_vocabulary );
    CPPUNIT_ASSERT( e1.m_vocabulary_lookup == e2.m_vocabulary_lookup );
    CPPUNIT_ASSERT( e1.m_classes == e2.m_classes );
    CPPUNIT_ASSERT( e1.m_word_classes == e2.m_word_classes );
    CPPUNIT_ASSERT( e1.m_word_counts == e2.m_word_counts );
    CPPUNIT_ASSERT( e1.m_word_bigram_counts == e2.m_word_bigram_counts );
    CPPUNIT_ASSERT( e1.m_word_rev_bigram_counts == e2.m_word_rev_bigram_counts );
    CPPUNIT_ASSERT( e1.m_class_counts == e2.m_class_counts );
    CPPUNIT_ASSERT( e1.m_class_bigram_counts == e2.m_class_bigram_counts );
    CPPUNIT_ASSERT( e1.m_word_class_counts == e2.m_word_class_counts );
    CPPUNIT_ASSERT( e1.m_class_word_counts == e2.m_class_word_counts );
}


// Test that data is read and things set up properly
void exchangetest::ExchangeTest1(void)
{
    cerr << endl;

    long unsigned int num_classes = 2;
    long unsigned int num_words = 7;

    Exchange e(num_classes, "test/corpus1.txt");

    CPPUNIT_ASSERT_EQUAL( num_classes+2, (long unsigned int)e.m_num_classes );
    CPPUNIT_ASSERT_EQUAL( num_words, e.m_vocabulary.size() );
    CPPUNIT_ASSERT_EQUAL( num_words, e.m_vocabulary_lookup.size() );
    CPPUNIT_ASSERT_EQUAL( num_classes+2, e.m_classes.size() );
    CPPUNIT_ASSERT_EQUAL( num_words, e.m_word_classes.size() );
    CPPUNIT_ASSERT_EQUAL( num_words, e.m_word_counts.size() );
    CPPUNIT_ASSERT_EQUAL( num_words, e.m_word_bigram_counts.size() );
    CPPUNIT_ASSERT_EQUAL( num_words, e.m_word_rev_bigram_counts.size() );
    CPPUNIT_ASSERT_EQUAL( num_classes+2, e.m_class_counts.size() );
    CPPUNIT_ASSERT_EQUAL( num_classes+2, e.m_class_bigram_counts.size() );
}


// Test that moving words between classes handles counts correctly
void exchangetest::ExchangeTest2(void)
{
    cerr << endl;
    Exchange e(2, "test/corpus1.txt");

    vector<int> orig_class_counts = e.m_class_counts;
    vector<map<int, int> > orig_class_bigram_counts = e.m_class_bigram_counts;
    vector<map<int, int> > orig_class_word_counts = e.m_class_word_counts;
    vector<map<int, int> > orig_word_class_counts = e.m_word_class_counts;

    int widx = e.m_vocabulary_lookup["d"];
    int curr_class = e.m_word_classes[widx];
    int new_class = (curr_class == 3) ? 2 : 3;

    e.do_exchange(widx, curr_class, new_class);

    CPPUNIT_ASSERT( orig_class_counts != e.m_class_counts );
    CPPUNIT_ASSERT( orig_class_bigram_counts != e.m_class_bigram_counts );
    CPPUNIT_ASSERT( orig_class_word_counts != e.m_class_word_counts );
    CPPUNIT_ASSERT( orig_word_class_counts != e.m_word_class_counts );

    e.do_exchange(widx, new_class, curr_class);

    CPPUNIT_ASSERT( orig_class_counts == e.m_class_counts );
    CPPUNIT_ASSERT( orig_class_bigram_counts == e.m_class_bigram_counts );
    CPPUNIT_ASSERT( orig_class_word_counts == e.m_class_word_counts );
    CPPUNIT_ASSERT( orig_word_class_counts == e.m_word_class_counts );
}


// Another test for count updates
void exchangetest::ExchangeTest3(void)
{
    cerr << endl;

    Exchange e_ref(2);
    e_ref.read_corpus("test/corpus1.txt");
    e_ref.initialize_classes();

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
void exchangetest::ExchangeTest4(void)
{
    cerr << endl;
    Exchange e(2, "test/corpus1.txt");

    int widx = e.m_vocabulary_lookup["d"];
    int curr_class = e.m_word_classes[widx];
    int new_class = (curr_class == 3) ? 2 : 3;

    double evaluated_ll_diff = e.evaluate_exchange(widx, curr_class, new_class);
    double evaluated_ll_diff_2 = e.evaluate_exchange_2(widx, curr_class, new_class);
    double orig_ll = e.log_likelihood();
    e.do_exchange(widx, curr_class, new_class);
    double ref_ll = e.log_likelihood();

    CPPUNIT_ASSERT_EQUAL( orig_ll+evaluated_ll_diff, ref_ll );
    CPPUNIT_ASSERT_EQUAL( orig_ll+evaluated_ll_diff_2, ref_ll );
}


// Test for checking evaluation time
void exchangetest::ExchangeTest5(void)
{
    cerr << endl;
    Exchange e(2, "test/corpus1.txt");

    int widx = e.m_vocabulary_lookup["d"];
    int curr_class = e.m_word_classes[widx];
    int new_class = (curr_class == 3) ? 2 : 3;

    time_t t1, t2;
    t1 = time(0);
    for (int i=0; i<10000000; i++)
        e.evaluate_exchange_2(widx, curr_class, new_class);
    t2 = time(0);
    cerr << "Seconds elapsed: " << (t2-t1) << endl;
}

