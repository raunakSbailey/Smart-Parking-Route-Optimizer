#include <bits/stdc++.h>

using namespace std;

const int MAX_NODES = 20;
const int INF = numeric_limits<int>::max();

class User {
    private:
        string currentUserName;

    public:
        User () {
            currentUserName = "admin";
        }

        bool userLogin (string userName, string userPassword) {
            if (userName == "admin" && userPassword == "123") return true;

            vector <string> userNames; //Remember that you have to store the names in sorted form later
            vector <string> userPasswords;

            ifstream names ("E:\\Smart-Parking-Route-Optimizer\\Files\\UserFiles\\userNames.txt");
            ifstream passwords ("E:\\Smart-Parking-Route-Optimizer\\Files\\UserFiles\\userPasswords.txt");

            string name, password;

            for (int i = 0; getline (names, name); ++i) {
                userNames.push_back (name);
            }

            while (getline (passwords, password)) {
                userPasswords.push_back (password);
            }

            for (int i = 0; i < userNames.size (); ++i) {
                if (userNames [i] == userName && userPasswords [i] == userPassword) {
                    currentUserName = userName;
                    return true;
                }
            }

            currentUserName = "Unidentified";

            names.close ();
            passwords.close ();

            return false;
        }

        bool userRegister (string userName, string userPassword) {
            ofstream userNames (".\\UserFiles\\userNames.txt", ios::app);
            ofstream userPasswords (".\\UserFiles\\userPasswords.txt", ios::app);

            userNames << userName << endl;
            userPasswords << userPassword << endl;

            currentUserName = userName;

            userNames.close ();
            userPasswords.close ();

            return true;
        }

        string getUserName () {return currentUserName;}
};


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
            cout << "Block name not recognized.\n";
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

    bool initiateParking (int slotID, string userName, int fromHour, int fromMin, int toHour, int toMin) {
        if (slotID < 0) false;

        CampusBlock& tempBlock = blocks [slotID];

        if (tempBlock.occupiedSlots < tempBlock.parkingCapacity) {
            tempBlock.occupiedSlots++;
            return true;
        } else {
            return false;
        }
    }
};

int main() {
    User user;

    cout << "1. Login" << endl << "2. Register" << endl << ": ";
    int choice;
    cin >> choice;

    if (choice == 1) {
        cout << "Please enter your Username and Password (uN pW): ";
        string userName, userPassword;
        cin >> userName >> userPassword;

        if (!user.userLogin (userName, userPassword)) {
            cout << "Please enter correct details." << endl;
            exit (0);
        }

        cout << "Welcome! " << userName << endl;
    } else {
        cout << "Please enter your Username and Password (uN pW): ";
        string userName, userPassword;
        cin >> userName >> userPassword;

        if (!user.userRegister (userName, userPassword)) {
            cout << "Please enter correct details." << endl;
            exit (0);
        }

        cout << "Welcome! " << userName << endl;
    }

    CampusNavigator uniMap;
    uniMap.loadCampusData("nodes_list.txt", "adjacency_matrix.txt");

    while (true) {
        cout << "\n--- University Travel Assistant ---\n";
        cout << "1. Find Route & Check Parking\n2. Update Block Parking (Admin)\n3. Exit\nChoice: ";
        cin >> choice;

        if (choice == 3) break;

        string b1, b2;
        if (choice == 1) {
            cout << "Current Location: "; cin.ignore(); getline (cin, b1);
            cout << "Destination Block: "; getline (cin, b2);
            cout << "Route from: " << b1 << endl << "To: " << b2 << endl;
            uniMap.findCampusRoute(b1, b2);

            cout << "Do you want to book? (1/0) ";
            cin >> choice;

            if (choice) {
                int blockID = uniMap.getBlockId (b2);

                int fromHour, fromMin, toHour, toMin;

                cout << "Please enter start time (Hour Min): ";
                cin >> fromHour >> fromMin;

                cout << "Please enter end time (Hour Min): ";
                cin >> toHour >> toMin;

                if (uniMap.initiateParking (blockID, user.getUserName(), fromHour, fromMin, toHour, toMin)) {
                    cout << "Parking is Book!" << endl;
                } else {
                    cout << "An Error has occured (404)" << endl;
                }
            }
        } else if (choice == 2) {
            int change;
            cout << "Block Name: "; cin >> b1;
            cout << "Change (e.g., 1 for entry, -1 for exit): "; cin >> change;
            uniMap.updateOccupancy(b1, change);
        }
    }
    return 0;
}


/*
To Do
1. Add QR code generator*/