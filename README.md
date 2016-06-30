An efficient C++ implementation of the exchange algorithm for a bigram model  
Supports multithreading  

For more details:  
Martin, Liermann, Ney: Algorithms for bigram and trigram word clustering, Speech Communication 1998  
Botros, Irie, Sundermeyer, Ney: On efficient training of word classes and their application to recurrent neural network language models, Interspeech 2015  

One sentence per line is assumed.  
Sentence begin and end markers are added to each line if not present in the corpus.
Perplexity values include the sentence end symbol.

Usage example:  
exchange -c 1000 -a 1000 -m 10000 -t 2 corpus.txt exchange.c1000  
cat corpus.txt|class_corpus.py exchange.c1000.cmemprobs.gz > corpus.classes.txt  
cat devel.txt|class_corpus.py exchange.c1000.cmemprobs.gz > devel.classes.txt  
varigram_kn -3 -C -Z -n 5 -D 0.02 -E 0.04 -o devel.classes.txt corpus.classes.txt exchange.vkn.5g.arpa.gz  
classppl exchange.vkn.5g.arpa.gz exchange.c1000.cmemprobs.gz eval.txt  

