#include <stdio.h>
#include <stdlib.h>
#include "AL_graph.h"

Graph* createGraph() {
    Graph* g = (Graph*) malloc(sizeof(Graph));
    if (g == NULL) {
        return NULL;
    }

    g->vertexCount = 0;
    g->vertices = NULL;
    return g;
}

void destroyGraph(Graph* g) {
    if (g == NULL) {
        return;
    }

    while(g->vertices != NULL) {
        Vertex* next = g->vertices->next;
        destroyVertex(g->vertices);
        g->vertices = next;
    }

    free(g);
    g = NULL;
}

Vertex* createVertex(GraphElementType data) {
    Vertex* v = (Vertex*)malloc(sizeof(Vertex));
    if (v == NULL) {
        return NULL;
    }

    v->data = data;
    v->index = -1;
    v->isVisited = NotVisited;
    v->next = NULL;
    v->adjacencyList = NULL;
    return v;
}

void destroyVertex(Vertex* v) {
    if (v == NULL) {
        return;
    }

    while(v->adjacencyList != NULL) {
        Edge* next = v->adjacencyList->next;
        destroyEdge(v->adjacencyList);
        v->adjacencyList = next;
    }

    free(v);
    v = NULL;
}

Edge* createEdge(Vertex* from, Vertex* to, int weight) {
    Edge* e = (Edge*) malloc(sizeof(Edge));
    if (e == NULL) {
        return NULL;
    }

    e->weight = weight;
    e->from = from;
    e->to = to;
    e->next = NULL;
    return e;
}

void destroyEdge(Edge* e) {
    if (e == NULL) {
        return;
    }

    free(e);
    e = NULL;
}

void addVertex(Graph* g, Vertex* v) {
    if (g == NULL || v == NULL) {
        return;
    }

    Vertex* vList = g->vertices;
    if (vList == NULL) {
        g->vertices = v;
    }
    else {
        while(vList->next != NULL) {
            vList = vList->next;
        }
        vList->next = v;
    }

    v->index = g->vertexCount++;
}

void addEdge(Edge* e) {
    if (e == NULL) {
        return;
    }

    Vertex* v=e->from;

    Edge* eList = v->adjacencyList;
    if (eList == NULL) {
        v->adjacencyList = e;
    }
    else {
        while(eList->next != NULL) {
            eList = eList->next;
        }
        eList->next = e;
    }
}

void printGraph(Graph* g) {
    Vertex* v = NULL;
    Edge* e = NULL;

    v = g->vertices;
    if (v == NULL) {
        return;
    }

    while(v != NULL) {
        printf("[% 2d][v:%d]->", v->index, v->data);

        e = v->adjacencyList;
        while (e != NULL) {
            printf("[v:%d][w:%d]->", e->to->data, e->weight);
            e = e->next;
        }

        printf("NULL\n");
        v = v->next;
    }
}

int main() {
    Graph *g = createGraph();

    Vertex* v1 = createVertex(1);
    Vertex* v2 = createVertex(2);
    Vertex* v3 = createVertex(3);
    Vertex* v4 = createVertex(4);
    Vertex* v5 = createVertex(5);

    addVertex(g, v1);
    addVertex(g, v2);
    addVertex(g, v3);
    addVertex(g, v4);
    addVertex(g, v5);

    addEdge(createEdge(v1, v2, 0));
    addEdge(createEdge(v1, v3, 0));
    addEdge(createEdge(v1, v4, 0));
    addEdge(createEdge(v1, v5, 0));

    addEdge(createEdge(v2, v1, 0));
    addEdge(createEdge(v2, v3, 0));
    addEdge(createEdge(v2, v5, 0));

    addEdge(createEdge(v3, v1, 0));
    addEdge(createEdge(v3, v2, 0));

    addEdge(createEdge(v4, v1, 0));
    addEdge(createEdge(v4, v5, 0));

    addEdge(createEdge(v5, v1, 0));
    addEdge(createEdge(v5, v2, 0));
    addEdge(createEdge(v5, v4, 0));

    printGraph(g);

    destroyGraph(g);

    FILE *fp;
    fp = fopen("../graph.txt", "r");
    if(fp == NULL){
        printf("fail\n");
    } else {
        printf("success\n");
    }
    while (feof(fp) == 0) {
        char str[100];
        fgets(str, 100, fp);
        printf("%s", str);
    }

    fclose(fp);

    return 0;
}
