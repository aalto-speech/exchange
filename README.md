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

For converting the training and possible development sets to class sequences, the class_corpus.py script is provided.  
To handle possible out-of-vocabulary words in these sets, the unk symbol must be specified.  
If the unk symbol is written in capitals (for instance VariKN), use the --cap_unk switch and for lowercase  
unk symbol (for instance SRILM) use the --lc_unk switch.  
ngramppl/classppl/classintppl checks for the correct unk symbol.  

Usage example:  
exchange -c 1000 -a 1000 -m 10000 -t 2 corpus.txt exchange.c1000  
class_corpus.py --cap_unk exchange.c1000.cmemprobs.gz <train.txt >train.classes.txt  
class_corpus.py --cap_unk exchange.c1000.cmemprobs.gz <devel.txt >devel.classes.txt  
varigram_kn -3 -C -Z -a -n 5 -D 0.02 -E 0.04 -o devel.classes.txt train.classes.txt exchange.vkn.5g.arpa.gz  
classppl exchange.vkn.5g.arpa.gz exchange.c1000.cmemprobs.gz eval.txt  
