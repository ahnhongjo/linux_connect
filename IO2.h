// This code is part of the project "Ligra: A Lightweight Graph Processing
// Framework for Shared Memory", presented at Principles and Practice of
// Parallel Programming, 2013.
// Copyright (c) 2013 Julian Shun and Guy Blelloch
//
// Permission is hereby granted, free of charge, to any person obtaining a
// copy of this software and associated documentation files (the
// "Software"), to deal in the Software without restriction, including
// without limitation the rights (to use, copy, modify, merge, publish,
// distribute, sublicense, and/or sell copies of the Software, and to
// permit persons to whom the Software is furnished to do so, subject to
// the following conditions:
//
// The above copyright notice and this permission notice shall be included
// in all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
// OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
// MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
// NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
// LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
// OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
// WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
#pragma once

#include <iostream>
#include <fstream>
#include <stdlib.h>
#include <cmath>
#include <sys/mman.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include "parallel.h"
#include "blockRadixSort.h"
#include "quickSort.h"
#include "utils.h"
#include "graph.h"

#include <libpmem.h>
#include <libpmemobj.h>

#include <string.h>

#include <chrono>
#include <cmath>

using namespace std;
using namespace chrono;

typedef pair <uintE, uintE> intPair;
typedef pair <uintE, pair<uintE, intE>> intTriple;

template<class E>
struct pairFirstCmp {
    bool operator()(pair <uintE, E> a, pair <uintE, E> b) {
        return a.first < b.first;
    }
};

template<class E>
struct getFirst {
    uintE operator()(pair <uintE, E> a) { return a.first; }
};

template<class IntType>
struct pairBothCmp {
    bool operator()(pair <uintE, IntType> a, pair <uintE, IntType> b) {
        if (a.first != b.first) return a.first < b.first;
        return a.second < b.second;
    }
};

//sslab
struct graph_data {
    size_t v_size;
    size_t edges_size;
    size_t inEdges_size;
    size_t offsets_size;
    size_t tOffsets_size;
    long m;
    long n;
};

// A structure that keeps a sequence of strings all allocated from
// the same block of memory
struct words {
    long n; // total number of characters
    char *Chars;  // array storing all strings
    long m; // number of substrings
    char **Strings; // pointers to strings (all should be null terminated)
    words() {}

    words(char *C, long nn, char **S, long mm)
            : Chars(C), n(nn), Strings(S), m(mm) {}

    void del() {
        free(Chars);
        free(Strings);
    }
};

inline bool isSpace(char c) {
    switch (c) {
        case '\r':
        case '\t':
        case '\n':
        case 0:
        case ' ' :
            return true;
        default :
            return false;
    }
}

_seq<char> readStringFromFile(char *fileName) {
    ifstream file(fileName, ios::in | ios::binary | ios::ate);
    if (!file.is_open()) {
        std::cout << "Unable to open file: " << fileName << std::endl;
        abort();
    }
    long end = file.tellg();
    file.seekg(0, ios::beg);
    long n = end - file.tellg();
    char *bytes = newA(char, n + 1);
    file.read(bytes, n);
    file.close();
    return _seq<char>(bytes, n);
}

// parallel code for converting a string to words
words stringToWords(char *Str, long n) {
    {
        parallel_for(long
        i = 0;
        i < n;
        i++)
        if (isSpace(Str[i])) Str[i] = 0;
    }

    // mark start of words
    bool *FL = newA(bool, n);
    FL[0] = Str[0];
    {
        parallel_for(long
        i = 1;
        i < n;
        i++) FL[i] = Str[i] && !Str[i - 1];
    }

    // offset for each start of word
    _seq<long> Off = sequence::packIndex<long>(FL, n);
    long m = Off.n;
    long *offsets = Off.A;

    // pointer to each start of word
    char **SA = newA(char * , m);
    {
        parallel_for(long
        j = 0;
        j < m;
        j++) SA[j] = Str + offsets[j];
    }

    free(offsets);
    free(FL);
    return words(Str, n, SA, m);
}

template<class vertex>
graph <vertex> readGraphFromFile(char *fname) {



    PMEMobjpool *graph_data_pool = pmemobj_open("/pmem/ahj/graph_data", "ligra-graph_data");

    if (!graph_data_pool) {
        std::chrono::system_clock::time_point start = std::chrono::system_clock::now();

        words W;
        _seq<char> S = readStringFromFile(fname);
        W = stringToWords(S.A, S.n);
#ifndef WEIGHTED
        if (W.Strings[0] != (string) "AdjacencyGraph") {
#else
            if (W.Strings[0] != (string) "WeightedAdjacencyGraph") {
#endif
            cout << "Bad input file" << endl;
            abort();
        }

        long len = W.m - 1;
        long n = atol(W.Strings[1]);
        long m = atol(W.Strings[2]);
#ifndef WEIGHTED
        if (len != n + m + 2) {
#else
            if (len != n + 2*m + 2) {
#endif
            cout << "Bad input file" << endl;
            abort();
        }

        uintT *offsets = newA(uintT, n);
#ifndef WEIGHTED
        //sslab: here!
        uintE *edges = newA(uintE, m);
        //printf("sizeof edges: %ld, m: %d\n", m * sizeof(uintE), m);
#else
        intE* edges = newA(intE,2*m);
#endif

        {
            parallel_for(long
            i = 0;
            i < n;
            i++) offsets[i] = atol(W.Strings[i + 3]);
        }
        {
            parallel_for(long
            i = 0;
            i < m;
            i++) {
#ifndef WEIGHTED
                edges[i] = atol(W.Strings[i + n + 3]);
#else
                edges[2*i] = atol(W.Strings[i+n+3]);
        edges[2*i+1] = atol(W.Strings[i+n+m+3]);
#endif
            }
        }
        //W.del(); // to deal with performance bug in malloc

        //sslab: here?
        vertex *v = newA(vertex, n);
        //printf("sizeof vertex v: %ld, n: %d\n", n * sizeof(vertex), n);

        {
            parallel_for(uintT
            i = 0;
            i < n;
            i++) {
                uintT o = offsets[i];
                uintT l = ((i == n - 1) ? m : offsets[i + 1]) - offsets[i];
                v[i].setOutDegree(l);
#ifndef WEIGHTED
                v[i].setOutNeighbors(edges + o);
#else
                v[i].setOutNeighbors(edges+2*o);
#endif
            }
        }

        uintT *tOffsets = newA(uintT, n);
        {
            parallel_for(long
            i = 0;
            i < n;
            i++) tOffsets[i] = INT_T_MAX;
        }
#ifndef WEIGHTED
        intPair *temp = newA(intPair, m);
#else
        intTriple* temp = newA(intTriple,m);
#endif
        {
            parallel_for(long
            i = 0;
            i < n;
            i++){
                uintT o = offsets[i];
                for (uintT j = 0; j < v[i].getOutDegree(); j++) {
#ifndef WEIGHTED
                    temp[o + j] = make_pair(v[i].getOutNeighbor(j), i);
#else
                    temp[o+j] = make_pair(v[i].getOutNeighbor(j),make_pair(i,v[i].getOutWeight(j)));
#endif
                }
            }
        }

        size_t size_offsets = n * sizeof(uintT);
        PMEMobjpool *offsets_pool = pmemobj_create("/pmem/ahj/offsets", "ligra-offsets",
                                                   size_offsets + PMEMOBJ_MIN_POOL, 0666);
        PMEMoid offsets_root = pmemobj_root(offsets_pool, size_offsets);
        pmemobj_memcpy_persist(offsets_pool, pmemobj_direct(offsets_root), offsets, size_offsets);
        pmemobj_close(offsets_pool);

        free(offsets);

#ifndef WEIGHTED
#ifndef LOWMEM
        intSort::iSort(temp, m, n + 1, getFirst<uintE>());
#else
        quickSort(temp,m,pairFirstCmp<uintE>());
#endif
#else
#ifndef LOWMEM
        intSort::iSort(temp,m,n+1,getFirst<intPair>());
#else
        quickSort(temp,m,pairFirstCmp<intPair>());
#endif
#endif

        tOffsets[temp[0].first] = 0;
#ifndef WEIGHTED
        //sslab: here!
        uintE *inEdges = newA(uintE, m);
        //printf("sizeof inEdges: %ld, m: %d\n", m * sizeof(uintE), m);
        inEdges[0] = temp[0].second;
#else
        intE* inEdges = newA(intE,2*m);
        inEdges[0] = temp[0].second.first;
        inEdges[1] = temp[0].second.second;
#endif
        {
            parallel_for(long
            i = 1;
            i < m;
            i++) {
#ifndef WEIGHTED
                inEdges[i] = temp[i].second;
#else
                inEdges[2*i] = temp[i].second.first;
    inEdges[2*i+1] = temp[i].second.second;
#endif
                if (temp[i].first != temp[i - 1].first) {
                    tOffsets[temp[i].first] = i;
                }
            }
        }

        free(temp);

        //fill in offsets of degree 0 vertices by taking closest non-zero
        //offset to the right
        sequence::scanIBack(tOffsets, tOffsets, n, minF<uintT>(), (uintT) m);

        {
            parallel_for(long
            i = 0;
            i < n;
            i++){
                uintT o = tOffsets[i];
                uintT l = ((i == n - 1) ? m : tOffsets[i + 1]) - tOffsets[i];
                v[i].setInDegree(l);
#ifndef WEIGHTED
                v[i].setInNeighbors(inEdges + o);
#else
                v[i].setInNeighbors(inEdges+2*o);
#endif
            }
        }

        size_t size_tOffsets = n * sizeof(uintT);

        PMEMobjpool *tOffsets_pool = pmemobj_create("/pmem/ahj/tOffsets", "ligra-tOffsets",
                                                    size_tOffsets + PMEMOBJ_MIN_POOL, 0666);
        PMEMoid tOffsets_root = pmemobj_root(tOffsets_pool, size_tOffsets);
        pmemobj_memcpy_persist(tOffsets_pool, pmemobj_direct(tOffsets_root), tOffsets, size_tOffsets);
        pmemobj_close(tOffsets_pool);

        free(tOffsets);
        //sslab: here!

        size_t size_inEdges = m * sizeof(uintE);
        size_t size_edges = m * sizeof(uintE);
        size_t size_v = n * sizeof(vertex);


        printf("%lu %lu %lu %lu %lu\n", m, n, size_inEdges, size_edges, size_v);

        graph_data_pool = pmemobj_create("/pmem/ahj/graph_data", "ligra-graph_data", PMEMOBJ_MIN_POOL, 0666);
        PMEMoid graph_data_root = pmemobj_root(graph_data_pool, sizeof(struct graph_data));
        struct graph_data *gd_tmp = (struct graph_data *) malloc(sizeof(struct graph_data));
        gd_tmp->v_size = size_v;
        gd_tmp->edges_size = size_edges;
        gd_tmp->inEdges_size = size_inEdges;
        gd_tmp->m = m;
        gd_tmp->n = n;
        gd_tmp->offsets_size = size_offsets;
        gd_tmp->tOffsets_size = size_tOffsets;

        pmemobj_memcpy_persist(graph_data_pool, pmemobj_direct(graph_data_root), gd_tmp, sizeof(struct graph_data));
        free(gd_tmp);
        pmemobj_close(graph_data_pool);


        printf("size_inEdges: %lu\n", size_inEdges);

        PMEMobjpool *inEdges_pool = pmemobj_create("/pmem/ahj/inEdges", "ligra-inEdges",
                                                   size_inEdges + PMEMOBJ_MIN_POOL, 0666);
        PMEMoid inEdges_root = pmemobj_root(inEdges_pool, size_inEdges);
        pmemobj_memcpy_persist(inEdges_pool, pmemobj_direct(inEdges_root), inEdges, size_inEdges);
        pmemobj_close(inEdges_pool);


        printf("size_edges: %lu\n", size_edges);

        PMEMobjpool *edges_pool = pmemobj_create("/pmem/ahj/edges", "ligra-edges", size_edges + PMEMOBJ_MIN_POOL,
                                                 0666);
        PMEMoid edges_root = pmemobj_root(edges_pool, size_edges);
        pmemobj_memcpy_persist(edges_pool, pmemobj_direct(edges_root), edges, size_edges);
        pmemobj_close(edges_pool);


        printf("size_v: %lu\n", size_v);

        PMEMobjpool *v_pool = pmemobj_create("/pmem/ahj/v", "ligra-v", size_v + PMEMOBJ_MIN_POOL, 0666);
        PMEMoid v_root = pmemobj_root(v_pool, size_v);
        pmemobj_memcpy_persist(v_pool, pmemobj_direct(v_root), v, size_v);
        pmemobj_close(v_pool);

        Uncompressed_Mem <vertex> *mem = new Uncompressed_Mem<vertex>(v, n, m, edges, inEdges);

        std::chrono::duration<double> sec = std::chrono::system_clock::now() - start;
        std::cout << "load time: " << sec.count() << std::endl;

        return graph<vertex>(v, n, m, mem);

    } else {
        return graph_pmem<vertex>()
    }
}

template<class vertex>
graph_pmem<vertex>() {
    std::chrono::system_clock::time_point start = std::chrono::system_clock::now();
    
    long m;
    long n;
    size_t size_inEdges, size_edges, size_v, size_offsets, size_tOffsets;
    vertex *v;
    uintT *tOffsets;
    uintT *offsets;

    //sslab: here!

    PMEMoid graph_data_root = pmemobj_root(graph_data_pool, sizeof(struct graph_data));
    struct graph_data *gd_now = (struct graph_data *) pmemobj_direct(graph_data_root);
    m = gd_now->m;
    n = gd_now->n;
    size_inEdges = gd_now->inEdges_size;

    size_edges = gd_now->edges_size;
    size_v = gd_now->v_size;
    size_offsets = gd_now->offsets_size;
    size_tOffsets = gd_now->tOffsets_size;
    printf("%lu %lu %lu %lu %lu\n", m, n, size_inEdges, size_edges, size_v);

    printf("size_inEdges: %lu\n", size_inEdges);
    PMEMobjpool *inEdges_pool = pmemobj_open("/pmem/ahj/inEdges", "ligra-inEdges");
    PMEMoid inEdges_root = pmemobj_root(inEdges_pool, size_inEdges);

    //inEdges = (uintE *) pmemobj_direct(inEdges_root);
#ifndef WEIGHTED
    uintE *inEdges = newA(uintE, m);
#else
    intE* inEdges = newA(intE,2*m);
#endif
    memcpy(inEdges, pmemobj_direct(inEdges_root), size_inEdges);

    printf("size_edges: %lu\n", size_edges);
    PMEMobjpool *edges_pool = pmemobj_open("/pmem/ahj/edges", "ligra-edges");
    PMEMoid edges_root = pmemobj_root(edges_pool, size_edges);

#ifndef WEIGHTED
    uintE *edges = newA(uintE, m);
#else
    intE* edges = newA(intE,2*m);
#endif

    memcpy(edges, pmemobj_direct(edges_root), size_edges);
    //edges = (uintE *) pmemobj_direct(edges_root);

    printf("size_v: %lu\n", size_v);
    PMEMobjpool *v_pool = pmemobj_open("/pmem/ahj/v", "ligra-v");
    PMEMoid v_root = pmemobj_root(v_pool, size_v);

    v = newA(vertex, n);
    memcpy(v, pmemobj_direct(v_root), size_v);

    PMEMobjpool *offsets_pool = pmemobj_open("/pmem/ahj/offsets", "ligra-offsets");
    PMEMoid offsets_root = pmemobj_root(offsets_pool, size_offsets);

    offsets = newA(uintT, n);
    memcpy(offsets, pmemobj_direct(offsets_root), size_offsets);


    PMEMobjpool *tOffsets_pool = pmemobj_open("/pmem/ahj/tOffsets", "ligra-tOffsets");
    PMEMoid tOffsets_root = pmemobj_root(tOffsets_pool, size_tOffsets);

    tOffsets = newA(uintT, n);
    memcpy(tOffsets, pmemobj_direct(tOffsets_root), size_tOffsets);

    for (uintT i = 0; i < n; i++) {
        uintT o1 = offsets[i];
        uintT o2 = tOffsets[i];
        v[i].setOutNeighbors(edges + o1);
        v[i].setInNeighbors(inEdges + o2);
    }

    Uncompressed_Mem <vertex> *mem = new Uncompressed_Mem<vertex>(v, n, m, edges, inEdges);

    std::chrono::duration<double> sec = std::chrono::system_clock::now() - start;
    std::cout << "load time: " << sec.count() << std::endl;

    return graph<vertex>(v, n, m, mem);

}



