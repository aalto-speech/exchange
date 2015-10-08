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


// Test that moving words between classes handles counts correctly
void exchangetest::ExchangeTest1(void)
{
    cerr << endl;
    Exchange e(2, "test/corpus1.txt");

    vector<int> orig_class_counts = e.m_class_counts;
    vector<map<int, int> > orig_class_bigram_counts = e.m_class_bigram_counts;
    vector<map<int, int> > orig_class_rev_bigram_counts = e.m_class_rev_bigram_counts;

    int widx = e.m_vocabulary_lookup["d"];
    int curr_class = e.m_word_classes[widx];
    int new_class = (curr_class == 3) ? 2 : 3;

    e.do_exchange(widx, curr_class, new_class);

    CPPUNIT_ASSERT( orig_class_counts != e.m_class_counts );
    CPPUNIT_ASSERT( orig_class_bigram_counts != e.m_class_bigram_counts );
    CPPUNIT_ASSERT( orig_class_rev_bigram_counts != e.m_class_rev_bigram_counts );

    e.do_exchange(widx, new_class, curr_class);

    CPPUNIT_ASSERT( orig_class_counts == e.m_class_counts );
    CPPUNIT_ASSERT( orig_class_bigram_counts == e.m_class_bigram_counts );
    CPPUNIT_ASSERT( orig_class_rev_bigram_counts == e.m_class_rev_bigram_counts );
}

