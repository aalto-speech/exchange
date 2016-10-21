**An efficient C++ implementation of the exchange word clustering algorithm**

Optimizes bigram perplexity by swapping word between classes. The evaluations
can be done in parallel in multiple threads.

One sentence per line is assumed. Sentence begin and end markers (`<s>` and
`</s>`) are added to each line if not present in the corpus. Perplexity values
include the sentence end symbol. Subword text can include word boundary markers
(`<w>`).

For more details:  
Martin, Liermann, Ney: Algorithms for bigram and trigram word clustering, Speech Communication 1998  
Botros, Irie, Sundermeyer, Ney: On efficient training of word classes and their application to recurrent neural network language models, Interspeech 2015  

Usage example:  
exchange -c 1000 -a 1000 -m 10000 -t 2 corpus.txt exchange.c1000  
class_corpus.py exchange.c1000.cmemprobs.gz <corpus.txt >corpus.classes.txt  
class_corpus.py exchange.c1000.cmemprobs.gz <devel.txt >devel.classes.txt  
varigram_kn -3 -C -Z -a -n 5 -D 0.02 -E 0.04 -o devel.classes.txt corpus.classes.txt exchange.vkn.5g.arpa.gz  
classppl exchange.vkn.5g.arpa.gz exchange.c1000.cmemprobs.gz eval.txt  
