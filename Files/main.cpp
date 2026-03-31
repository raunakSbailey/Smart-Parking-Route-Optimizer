#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <sstream>
#include <limits>
#include <queue>
#include <map>

using namespace std;

const int MAX_NODES = 20;
const int INF = numeric_limits<int>::max();


struct CampusBlock {
    int id;
    string name;
    int parkingCapacity;
    int occupiedSlots;
};


class CampusNavigator {
private:
    int trafficMatrix[MAX_NODES][MAX_NODES];
    map<int, CampusBlock> blocks;
    int numBlocks = 0;

public:
    CampusNavigator() {
        for (int i = 0; i < MAX_NODES; i++)
            for (int j = 0; j < MAX_NODES; j++)
                trafficMatrix[i][j] = 0;
    }

    void loadCampusData(string cityFile, string adjFile) {
        ifstream cFile(cityFile);
        int id, cap;
        string name;
        while (cFile >> id >> name >> cap) {
            blocks[id] = {id, name, cap, 0};
            numBlocks++;
        }

        ifstream aFile(adjFile);
        string line;
        int row = 0;
        while (getline(aFile, line)) {
            stringstream ss(line);
            string val;
            int col = 0;
            while (getline(ss, val, ',')) {
                trafficMatrix[row][col] = stoi(val);
                col++;
            }
            row++;
        }
    }

    int getBlockId(string name) {
        for (auto const& [id, block] : blocks) {
            if (block.name == name) return id;
        }
        return -1;
    }

    void findCampusRoute(string start, string end) {
        int src = getBlockId(start);
        int dest = getBlockId(end);

        if (src == -1 || dest == -1) {
            cout << "Block name not recognized. Use underscores for spaces (e.g., Gate_1).\n";
            return;
        }

        vector<int> dist(MAX_NODES, INF);
        vector<int> parent(MAX_NODES, -1);
        vector<bool> visited(MAX_NODES, false);
        dist[src] = 0;

        for (int i = 0; i < numBlocks - 1; i++) {
            int u = -1;
            for (int j = 0; j < numBlocks; j++) {
                if (!visited[j] && (u == -1 || dist[j] < dist[u])) u = j;
            }
            if (dist[u] == INF) break;
            visited[u] = true;

            for (int v = 0; v < numBlocks; v++) {
                if (trafficMatrix[u][v] != 0 && dist[u] + trafficMatrix[u][v] < dist[v]) {
                    dist[v] = dist[u] + trafficMatrix[u][v];
                    parent[v] = u;
                }
            }
        }

        if (dist[dest] == INF) {
            cout << "No walking/driving path found between these blocks.\n";
        } else {
            cout << "\n[Optimal Campus Route]: ";
            printPath(parent, dest);
            cout << "\nEstimated Time Delay: " << dist[dest] << " units\n";
            checkParking(dest);
        }
    }

    void printPath(vector<int>& parent, int j) {
        if (parent[j] == -1) {
            cout << blocks[j].name;
            return;
        }
        printPath(parent, parent[j]);
        cout << " -> " << blocks[j].name;
    }

    void checkParking(int blockId) {
        CampusBlock &b = blocks[blockId];
        cout << "Parking at " << b.name << ": ";
        if (b.occupiedSlots < b.parkingCapacity) {
            cout << (b.parkingCapacity - b.occupiedSlots) << " slots available.\n";
        } else {
            cout << "FULL. Please use the nearest overflow lot.\n";
        }
    }

    void updateOccupancy(string name, int change) {
        int id = getBlockId(name);
        if (id != -1) {
            blocks[id].occupiedSlots = max(0, min(blocks[id].parkingCapacity, blocks[id].occupiedSlots + change));
            cout << "Updated " << name << " occupancy.\n";
        }
    }
};

int main() {
    CampusNavigator uniMap;
    uniMap.loadCampusData("nodes_list.txt", "adjacency_matrix.txt");

    int choice;
    while (true) {
        cout << "\n--- University Travel Assistant ---\n";
        cout << "1. Find Route & Check Parking\n2. Update Block Parking (Admin)\n3. Exit\nChoice: ";
        cin >> choice;

        if (choice == 3) break;

        string b1, b2;
        if (choice == 1) {
            cout << "Current Location: "; cin.ignore(); getline (cin, b1);
            cout << "Destination Block: "; getline (cin, b2);
            cout << b1 << " " << b2 << endl;
            uniMap.findCampusRoute(b1, b2);
        } else if (choice == 2) {
            int change;
            cout << "Block Name: "; cin >> b1;
            cout << "Change (e.g., 1 for entry, -1 for exit): "; cin >> change;
            uniMap.updateOccupancy(b1, change);
        }
    }
    return 0;
}