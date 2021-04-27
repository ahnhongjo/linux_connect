/*
 * AL_graph.h
 *
 *  Created on: 2017. 5. 25.
 *      Author: cobus
 */

#ifndef SRC_DATA_STRUCTURE_AL_GRAPH_H_
#define SRC_DATA_STRUCTURE_AL_GRAPH_H_

enum VisitMode {
    Visited,
    NotVisited
};

typedef int GraphElementType;

typedef struct tagVertex {
    GraphElementType data;
    int isVisited;
    int index;

    struct tagVertex* next;
    struct tagEdge* adjacencyList;
} Vertex;

typedef struct tagEdge {
    int weight;
    struct tagEdge* next;
    Vertex* from;
    Vertex* to;
} Edge;

typedef struct tagGraph {
    Vertex* vertices;
    int vertexCount;
} Graph;

#ifdef __cplusplus
extern "C" {
#endif

Graph* createGraph();
void destroyGraph(Graph* g);

Vertex* createVertex(GraphElementType data);
void destroyVertex(Vertex* v);

Edge* createEdge(Vertex* from, Vertex* to, int weight);
void destroyEdge(Edge* e);

void addVertex(Graph* g, Vertex* v);
void addEdge(Edge* e);

void printGraph(Graph* g);

#ifdef __cplusplus
}
#endif

#endif /* SRC_DATA_STRUCTURE_AL_GRAPH_H_ */