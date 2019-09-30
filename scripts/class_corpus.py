#!/usr/bin/python

import sys
import gzip
import argparse


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description='Converts a text corpus to corresponding class sequences.')
    parser.add_argument('class_memberships', action="store",
                        help='Class membership file written by exchange (.gz supported)')
    parser.add_argument('--cap_unk', action="store_true", default=False,
                        help='Unk symbol should be written in capitals i.e. <UNK>')
    parser.add_argument('--lc_unk', action="store_true", default=False,
                        help='Unk symbol should be written in lowercase i.e. <unk>')
    args = parser.parse_args()

    unk = None
    if args.cap_unk:
        unk = "<UNK>"
    elif args.lc_unk:
        unk = "<unk>"
    else:
        print >>sys.stderr, "Define either cap_unk or lc_unk option"
        sys.exit(0)

    if args.class_memberships.endswith(".gz"):
        vocabf = gzip.open(args.class_memberships, "r")
    else:
        vocabf = open(args.class_memberships)

    vocab = dict()
    for line in vocabf:
        line = line.strip()
        tokens = line.split()
        if len(tokens) < 2:
            print >>sys.stderr, "Problem in line: %s" % line
            continue
        vocab[tokens[0]] = int(tokens[1])

    for line in sys.stdin:
        line = line.strip()
        words = line.split()
        sent = []
        for word in words:
            if word == "<s>": continue
            if word == "</s>": continue
            if word in vocab:
                sent.append(str(vocab[word]))
            else:
                sent.append(unk)

        print "<s> %s </s>" % (" ".join(sent))
