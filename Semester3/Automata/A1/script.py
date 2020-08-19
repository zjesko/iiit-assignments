"""
Author: Akshat Chhajer
Date: 14 August 2019
Purpose: Part of Automata Theory Assignment - 1. A script to read a NFA from json file and convert it to a DFA
"""

import json
from itertools import chain,combinations

def powerset(l):
    return list(chain.from_iterable(combinations(l, r) for r in range(len(l)+1)))

def final_states(dfa, nfa, dfa_states):
    for final_state in nfa['final']:
        for i in dfa_states:
            if final_state in i:
                dfa['final'].append(i)

def dfa_tfunc(dfa, nfa, dfa_states):
    
    for state in dfa_states:
        for letter in nfa['letters']:
            out = set()
            for s in state:
                for func in nfa['t_func']:
                    if s == func[0] and letter == func[1]:
                        for ns in func[2]:
                            out.add(ns)
            dfa['t_func'].append([list(state), letter, list(out)])

def main():

    with open("./input.json", 'r') as f:
        nfa = json.load(f)

    dfa = {}

    dfa['states'] = 2 ** nfa['states']
    dfa['letters'] = nfa['letters']
    dfa['start'] = [nfa['start']]
    dfa['final'] = []
    dfa['t_func'] = []

    nfa_states = [ i for i in range(nfa['states']) ]
    dfa_states = powerset(nfa_states)
    
    final_states(dfa, nfa, dfa_states)
    dfa_tfunc(dfa, nfa, dfa_states)

    with open("./output.json", 'w') as outjson:
        json.dump(dfa, outjson)

if __name__ == "__main__":
    main()
