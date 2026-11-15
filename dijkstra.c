#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <limits.h>

#define V 10
#define INF 999999

// Campus location names corresponding to indices 0 to 9
const char* LOCATIONS[V] = {
    "Main Gate",
    "Admin Block",
    "Central Library",
    "Computer Science Dept",
    "Electronics Dept",
    "Main Canteen",
    "Auditorium",
    "Sports Complex",
    "Boys Hostel",
    "Girls Hostel"
};

// 10x10 Adjacency Matrix representing distances in meters (0 = no direct path)
int graph[V][V] = {
    // 0    1    2    3    4    5    6    7    8    9
    {  0, 120,   0,   0,   0,   0, 200,   0,   0,   0}, // 0: Main Gate
    {120,   0, 150, 220,   0,   0, 180,   0,   0,   0}, // 1: Admin Block
    {  0, 150,   0, 140, 160, 200,   0,   0,   0,   0}, // 2: Central Library
    {  0, 220, 140,   0, 110,   0,   0,   0, 350,   0}, // 3: Computer Science Dept
    {  0,   0, 160, 110,   0, 130,   0, 280,   0,   0}, // 4: Electronics Dept
    {  0,   0, 200,   0, 130,   0, 170, 210, 260, 240}, // 5: Main Canteen
    {200, 180,   0,   0,   0, 170,   0, 300,   0, 290}, // 6: Auditorium
    {  0,   0,   0,   0, 280, 210, 300,   0, 190, 250}, // 7: Sports Complex
    {  0,   0,   0, 350,   0, 260,   0, 190,   0, 320}, // 8: Boys Hostel
    {  0,   0,   0,   0,   0, 240, 290, 250, 320,   0}  // 9: Girls Hostel
};

// Helper function to find the unvisited vertex with the minimum distance
int minDistance(int dist[], bool visited[]) {
    int min = INF, min_index = -1;
    for (int v = 0; v < V; v++) {
        if (!visited[v] && dist[v] <= min) {
            min = dist[v];
            min_index = v;
        }
    }
    return min_index;
}

// Dijkstra's Shortest Path Algorithm
void dijkstra(int src, int dest) {
    int dist[V];      // Shortest distance from src to i
    bool visited[V];  // True if vertex i is finalized
    int parent[V];    // Stores shortest path tree

    for (int i = 0; i < V; i++) {
        dist[i] = INF;
        visited[i] = false;
        parent[i] = -1;
    }

    dist[src] = 0;

    for (int count = 0; count < V - 1; count++) {
        int u = minDistance(dist, visited);
        if (u == -1) break;
        visited[u] = true;

        for (int v = 0; v < V; v++) {
            if (!visited[v] && graph[u][v] != 0 && dist[u] != INF &&
                dist[u] + graph[u][v] < dist[v]) {
                dist[v] = dist[u] + graph[u][v];
                parent[v] = u;
            }
        }
    }

    // Reconstruct the path from destination back to source
    int path[V];
    int path_length = 0;
    int curr = dest;

    if (dist[dest] == INF) {
        printf("{\"error\": \"No path found between selected locations.\"}\n");
        return;
    }

    while (curr != -1) {
        path[path_length++] = curr;
        curr = parent[curr];
    }

    // Print output strictly in valid JSON format
    printf("{\n");
    printf("  \"distance\": %d,\n", dist[dest]);

    // Print node indices array
    printf("  \"path\": [");
    for (int i = path_length - 1; i >= 0; i--) {
        printf("%d%s", path[i], (i > 0) ? ", " : "");
    }
    printf("],\n");

    // Print node names array
    printf("  \"path_names\": [");
    for (int i = path_length - 1; i >= 0; i--) {
        printf("\"%s\"%s", LOCATIONS[path[i]], (i > 0) ? ", " : "");
    }
    printf("],\n");

    // Print step-by-step edge distances
    printf("  \"step_distances\": [");
    for (int i = path_length - 1; i > 0; i--) {
        int u = path[i];
        int v = path[i - 1];
        printf("%d%s", graph[u][v], (i > 1) ? ", " : "");
    }
    printf("]\n");
    printf("}\n");
}

int main(int argc, char* argv[]) {
    if (argc != 3) {
        printf("{\"error\": \"Usage: ./dijkstra <src> <dest>\"}\n");
        return 1;
    }

    int src = atoi(argv[1]);
    int dest = atoi(argv[2]);

    if (src < 0 || src >= V || dest < 0 || dest >= V) {
        printf("{\"error\": \"Invalid node index. Must be between 0 and 9.\"}\n");
        return 1;
    }

    dijkstra(src, dest);
    return 0;
}