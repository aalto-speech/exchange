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

    int widx = e.m_vocabulary_lookup["d"];
    int curr_class = e.m_word_classes[widx];
    int new_class = (curr_class == 3) ? 2 : 3;

    e.do_exchange(widx, curr_class, new_class);

    CPPUNIT_ASSERT( orig_class_counts != e.m_class_counts );
    CPPUNIT_ASSERT( orig_class_bigram_counts != e.m_class_bigram_counts );

    e.do_exchange(widx, new_class, curr_class);

    CPPUNIT_ASSERT( orig_class_counts == e.m_class_counts );
    CPPUNIT_ASSERT( orig_class_bigram_counts == e.m_class_bigram_counts );
}


// Test for evaluating ll change for one exchange
void exchangetest::ExchangeTest3(void)
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

    CPPUNIT_ASSERT_EQUAL( orig_ll+evaluated_ll_diff, ref_ll );
}

