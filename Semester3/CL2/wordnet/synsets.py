from nltk.corpus import wordnet
import json

words = ['bank','computer', 'tarnish', 'euphoria', 'frog']

output = {}
for word in words:
    syn = wordnet.synsets(word)
    syn.sort()
    synsets = []
    for each_syn in syn:
        if(each_syn.name().split('.')[0] == word):
            syn_details = {}
            syn_details['synset_name'] = each_syn.name()
            syn_details['definition'] = each_syn.definition()
            if each_syn.examples():
                syn_details['example'] =  each_syn.examples()
            synonyms = []
            antonyms = []
            if each_syn.hypernyms():
                syn_details['hypernym'] =  each_syn.hypernyms()[0].name().split('.')[0]
            syn_details['hyponyms'] = sorted(lemma.name() for hyponym in each_syn.hyponyms() for lemma in hyponym.lemmas())
            syn_details['part_meronyms'] = sorted([lemma.name().split('.')[0] for lemma in each_syn.part_meronyms()])
            syn_details['substance_meronyms'] = sorted([lemma.name().split('.')[0] for lemma in each_syn.substance_meronyms()])
            syn_details['entailments'] = sorted([lemma.name().split('.')[0] for lemma in each_syn.entailments()])

            for lemma in each_syn.lemmas():
                if lemma.name() != each_syn.name().split('.')[0]:
                    synonyms.append(lemma.name())
                for antonym in lemma.antonyms():
                    antonyms.append(antonym.name())
            synsets.append(syn_details)
    output[word] = synsets

with open('words.json', 'w+') as f:
    json.dump(output,f)

