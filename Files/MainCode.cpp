#include<bits/stdc++.h>
using namespace std;

struct Node{
    int id;
    string name;
    int capacity;
    double x;
    double y;
    bool ghost;
};

class SmartParking{

public:

    map<int,Node> nodes;
    map<int,vector<pair<int,int>>> road;
    map<int,vector<pair<int,int>>> transport;
    map<int,map<int,int>> slots;
    map<pair<int,int>,vector<int>> visualPaths;

    SmartParking(){
        loadNodes();
        loadGraph();
        loadTransport();
        buildVisualPaths();
    }

    void loadNodes(){

        nodes.clear();
        slots.clear();

        ifstream file("nodes_list.txt");

        int id;
        string name;
        int cap;
        double x,y;
        int ghost;

        while(file >> id >> name >> cap >> x >> y >> ghost){

            Node n;
            n.id=id;
            n.name=name;
            n.capacity=cap;
            n.x=x;
            n.y=y;
            n.ghost=(ghost==1);

            nodes[id]=n;

            // Only init slots for real nodes with actual capacity
            if(!n.ghost && cap > 0){
                for(int h=0;h<24;h++){
                    slots[id][h]=cap;
                }
            }
        }

        file.close();
    }

    void loadGraph(){

        road.clear();

        ifstream file("adjacency_list.txt");

        int u,v,w;

        while(file>>u>>v>>w){
            road[u].push_back({v,w});
            road[v].push_back({u,w});
        }

        file.close();
    }

    void loadTransport(){

        transport.clear();

        ifstream file("transport.txt");

        int u,v;

        while(file>>u>>v){
            transport[u].push_back({v,1});
            transport[v].push_back({u,1});
        }

        file.close();
    }

    void buildVisualPaths(){

        // Format: {realNodeA, realNodeB} -> list of ghost node IDs between them
        // These are the ghost nodes that lie on the road between two real nodes.
        // Add both directions.

        auto addPath = [&](int a, int b, vector<int> ghosts){
            visualPaths[{a,b}] = ghosts;
            vector<int> rev(ghosts.rbegin(), ghosts.rend());
            visualPaths[{b,a}] = rev;
        };

        // Clock Tower (0) <-> Railway Station (3) via Araghar (56)
        // path: 0 -> [1,2] -> 3
        addPath(0, 3, {1,2});

        // Railway Station (3) <-> Pacific Mall (6) via Subhash Nagar (54)
        addPath(3, 6, {4,5});

        // Pacific Mall (6) <-> ISBT (8)
        addPath(6, 8, {7});

        // Clock Tower (0) <-> Ballupur (12)
        addPath(0, 12, {9,10,11});

        // Ballupur (12) <-> Rajpur Road (15)
        addPath(12, 15, {13,14});

        // Ballupur (12) <-> ONGC (24)
        addPath(12, 24, {22,23});

        // ONGC (24) <-> Clock Tower (0)
        addPath(24, 0, {25,26});

        // ISBT (8) <-> Clement Town (27)
        addPath(8, 27, {28,29});

        // Clock Tower (0) <-> Prince Chowk (30)
        addPath(0, 30, {31,32});

        // Rajpur Road (15) <-> Jakhan (33)
        addPath(15, 33, {34,35});

        // Jakhan (33) <-> Mussoorie Diversion (36)
        addPath(33, 36, {37,38});

        // Mussoorie Diversion (36) <-> Sahastradhara (39)
        addPath(36, 39, {40,41});

        // Jakhan (33) <-> IT Park (42)
        addPath(33, 42, {43,44});

        // IT Park (42) <-> DIT University (48)
        addPath(42, 48, {49,50});

        // ISBT (8) <-> Graphic Era (51)
        addPath(8, 51, {52,53});

        // ISBT (8) <-> Subhash Nagar (54)
        addPath(8, 54, {});  // direct, no ghost needed

        // Pacific Mall (6) <-> Subhash Nagar (54)
        addPath(6, 54, {55});

        // Railway Station (3) <-> Araghar (56)
        addPath(3, 56, {57,58});

        // Araghar (56) <-> Clock Tower (0) via Prince Chowk
        addPath(56, 0, {});

        // ISBT (8) <-> Rispana (65)
        addPath(8, 65, {66,67});

        // Clock Tower (0) <-> Doon Hospital (62)
        addPath(0, 62, {63,64});

        // Doon Hospital (62) <-> Fountain Chowk (68)
        addPath(62, 68, {69,70});

        // Fountain Chowk (68) <-> Parade Ground (45)
        addPath(68, 45, {});

        // Parade Ground (45) <-> Prince Chowk (30)
        addPath(45, 30, {46,47});

        // Parade Ground (45) <-> ONGC (24)
        addPath(45, 24, {});

        // ONGC (24) <-> Rispana (65)
        addPath(24, 65, {});

        // Prem Nagar (21) <-> Subhash Nagar (54)
        addPath(21, 54, {});

        // Prem Nagar (21) <-> ISBT (8)
        addPath(21, 8, {});
    }

    int getId(const string& name) const{

        for(const auto &p : nodes){
            if(p.second.name == name){
                return p.first;
            }
        }
        return -1;
    }

    vector<int> dijkstra(
        int src,
        int dest,
        const map<int,vector<pair<int,int>>>& graph
    ) const {

        priority_queue<
            pair<int,int>,
            vector<pair<int,int>>,
            greater<pair<int,int>>
        > pq;

        map<int,int> dist;
        map<int,int> parent;

        for(const auto &p : nodes){
            if(!p.second.ghost){
                dist[p.first] = INT_MAX;
            }
        }

        if(dist.find(src) == dist.end() || dist.find(dest) == dist.end()){
            return {};
        }

        dist[src] = 0;
        pq.push({0, src});

        while(!pq.empty()){

            auto [cost, node] = pq.top();
            pq.pop();

            if(cost > dist[node]) continue;

            if(graph.count(node)){
                for(const auto &[next, wt] : graph.at(node)){

                    if(nodes.count(next) && nodes.at(next).ghost)
                        continue;

                    if(dist.count(next) && dist[node] + wt < dist[next]){
                        dist[next] = dist[node] + wt;
                        parent[next] = node;
                        pq.push({dist[next], next});
                    }
                }
            }
        }

        if(!dist.count(dest) || dist[dest] == INT_MAX){
            return {};
        }

        vector<int> path;
        int cur = dest;

        while(cur != src){
            path.push_back(cur);
            if(!parent.count(cur)) return {};
            cur = parent[cur];
        }

        path.push_back(src);
        reverse(path.begin(), path.end());

        return path;
    }

    vector<int> buildVisualRoute(const vector<int>& realPath) const {

        vector<int> finalPath;

        if(realPath.empty()) return finalPath;

        for(int i = 0; i < (int)realPath.size()-1; i++){

            int a = realPath[i];
            int b = realPath[i+1];

            finalPath.push_back(a);

            auto key = make_pair(a,b);
            auto it = visualPaths.find(key);

            if(it != visualPaths.end()){
                for(int g : it->second){
                    finalPath.push_back(g);
                }
            }
        }

        finalPath.push_back(realPath.back());

        return finalPath;
    }

    bool availableAtTime(int node, int startH, int endH) const {

        // Validate range
        if(startH < 0 || endH > 24 || startH >= endH) return false;

        // Ghost nodes can never be parked at
        if(!nodes.count(node) || nodes.at(node).ghost) return false;

        if(!slots.count(node)) return false;

        for(int h = startH; h < endH; h++){
            const auto& nodeSlots = slots.at(node);
            if(!nodeSlots.count(h) || nodeSlots.at(h) <= 0){
                return false;
            }
        }

        return true;
    }

    void reserveSlot(int node, int startH, int endH){

        if(startH < 0 || endH > 24 || startH >= endH) return;
        if(!slots.count(node)) return;

        for(int h = startH; h < endH; h++){
            if(slots[node].count(h) && slots[node][h] > 0){
                slots[node][h]--;
            }
        }
    }

    // Find fallback parking along the REAL path (not visual)
    int fallbackParking(
        const vector<int>& realPath,
        int startH,
        int endH
    ){
        // Search backwards from destination
        for(int i = (int)realPath.size()-1; i >= 0; i--){

            int id = realPath[i];

            if(!nodes.count(id) || nodes[id].ghost) continue;

            if(availableAtTime(id, startH, endH)){
                return id;
            }
        }
        return -1;
    }

    // Update capacity WITHOUT wiping existing reservations.
    // Only updates the base capacity for display; reservations are kept.
    void updateCapacity(int id, int newCap){

        if(!nodes.count(id)) return;

        int oldCap = nodes[id].capacity;
        nodes[id].capacity = newCap;

        // Adjust slots proportionally: only change free slots, not reserved ones
        for(int h = 0; h < 24; h++){
            int reserved = oldCap - slots[id][h];
            reserved = max(0, reserved);
            slots[id][h] = max(0, newCap - reserved);
        }

        saveNodes();
    }

    void saveNodes(){

        ofstream file("nodes_list.txt");

        for(const auto &p : nodes){
            const auto &n = p.second;
            file << n.id << " "
                 << n.name << " "
                 << n.capacity << " "
                 << n.x << " "
                 << n.y << " "
                 << (n.ghost ? 1 : 0)
                 << "\n";
        }

        file.close();
    }
};

SmartParking sys;