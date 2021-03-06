#ifndef GRAPH_H
#define GRAPH_H
#include <iostream>
#include <fstream>
#include <stdlib.h>
#include "vertex.h"
#include "compressedVertex.h"
#include "parallel.h"
using namespace std;

// **************************************************************
//    ADJACENCY ARRAY REPRESENTATION
// **************************************************************

// Class that handles implementation specific freeing of memory
// owned by the graph
struct Deletable {
public:
  virtual void del() = 0;
};

struct Uncompressed_Mem : public Deletable {
public:
  asymmetricVertex* V;
  long n;
  long m;
  void* allocatedInplace, * inEdges;

  Uncompressed_Mem(asymmetricVertex* VV, long nn, long mm, void* ai, void* _inEdges = NULL)
  : V(VV), n(nn), m(mm), allocatedInplace(ai), inEdges(_inEdges) { }

  void del() {
    if (allocatedInplace == NULL)
      for (long i=0; i < n; i++) V[i].del();
    else free(allocatedInplace);
    free(V);
    if(inEdges != NULL) free(inEdges);
  }
};

struct graph {
  asymmetricVertex *V;
  long n;
  long m;
  bool transposed;
  uintE* flags;
  Deletable *D;

graph(asymmetricVertex* _V, long _n, long _m, Deletable* _D) : V(_V), n(_n), m(_m),
  D(_D), flags(NULL), transposed(0) {}

graph(asymmetricVertex* _V, long _n, long _m, Deletable* _D, uintE* _flags) : V(_V),
  n(_n), m(_m), D(_D), flags(_flags), transposed(0) {}

  void del() {
    if (flags != NULL) free(flags);
    D->del();
    free(D);
  }

  void transpose() {
    if (sizeof(asymmetricVertex) == sizeof(asymmetricVertex))
    {
      parallel_for(long i=0;i<n;i++) {
        V[i].flipEdges();
      }
      transposed = !transposed;
    }
  }
};

#endif
