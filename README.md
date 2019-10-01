**An efficient C++ implementation of the exchange word clustering algorithm**

Optimizes bigram perplexity by swapping words between classes. The evaluations
can be done in parallel in multiple threads. Word-class and Class-word statistics
are used for efficiency.

One sentence per line is assumed. Sentence begin and end markers (`<s>` and
`</s>`) are added to each line if not present in the corpus. Perplexity values
include the sentence end symbol.

For more details:  
Martin, Liermann, Ney: Algorithms for bigram and trigram word clustering, Speech Communication 1998  
Botros, Irie, Sundermeyer, Ney: On efficient training of word classes and their application to recurrent neural network language models, Interspeech 2015  

#### Compiling executables

Requirements
* Provided Makefile works with a GCC compiler which has C++11 support, i.e. GCC 4.6 or newer. Should work on MinGW as well.
* Zlib libraries and headers. On linux systems these are often included in packages `zlib1g` and `zlib1g-dev`.
* Scripts under the `scripts` folder require python3
* Running unit tests requires boost unit test framework libraries and headers.
On linux systems these are often included in packages `libboost-test` and `libboost-test-dev` packages.
The unit tests may be disabled by setting `NO_UNIT_TESTS=1` in Makefile.local file

These commands should work in most cases  
`cp Makefile.local.example Makefile.local`  
`make`  
The compilation parameters are easiest changed by modifying Makefile.local file which is included by the main Makefile.

#### Usage

For converting the training and possible development sets to class sequences, the class_corpus.py script is provided.  
To handle possible out-of-vocabulary words in these sets, the unk symbol must be specified.  
If the unk symbol is written in capitals (for instance VariKN), use the --cap_unk switch and for lowercase  
unk symbol (for instance SRILM) use the --lc_unk switch.  
ngramppl/classppl/classintppl checks for the correct unk symbol.  

Example:  
`exchange -c 1000 -o 999 -a 1000 -m 10000 -t 2 corpus.txt exchange.c1000`  
`scripts/class_corpus.py --cap_unk exchange.c1000.cmemprobs.gz <train.txt >train.classes.txt`  
`scripts/class_corpus.py --cap_unk exchange.c1000.cmemprobs.gz <devel.txt >devel.classes.txt`  
`varigram_kn -3 -C -Z -a -n 5 -D 0.02 -E 0.04 -o devel.classes.txt train.classes.txt exchange.vkn.5g.arpa.gz`  
`classppl exchange.vkn.5g.arpa.gz exchange.c1000.cmemprobs.gz eval.txt`  
