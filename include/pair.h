#ifndef PAIR_H
#define PAIR_H

#define Pair(T1, T2) struct pair

struct pair {
    void* fst;
    void* snd;
};

struct pair new_pair(void* fst, void* snd);
void destroy_pair(struct pair p);
void* pair_fst(struct pair p);
void* pair_snd(struct pair p);

#endif // PAIR_H
