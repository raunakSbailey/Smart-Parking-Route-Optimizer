#include<bits/stdc++.h>
#include "Node.cpp"
using namespace std;


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

        auto addPath = [&](int a, int b, vector<int> ghosts){
            visualPaths[{a,b}] = ghosts;
            vector<int> rev(ghosts.rbegin(), ghosts.rend());
            visualPaths[{b,a}] = rev;
        };

        addPath(0, 3, {1,2});
        addPath(3, 6, {4,5});
        addPath(6, 8, {7});
        addPath(0, 12, {9,10,11});
        addPath(12, 15, {13,14});
        addPath(12, 24, {22,23});
        addPath(24, 0, {25,26});
        addPath(8, 27, {28,29});
        addPath(0, 30, {31,32});
        addPath(15, 33, {34,35});
        addPath(33, 36, {37,38});
        addPath(36, 39, {40,41});
        addPath(33, 42, {43,44});
        addPath(42, 48, {49,50});
        addPath(8, 51, {52,53});
        addPath(8, 54, {}); 
        addPath(6, 54, {55});
        addPath(3, 56, {57,58});
        addPath(56, 0, {});
        addPath(8, 65, {66,67});
        addPath(0, 62, {63,64});
        addPath(62, 68, {69,70});
        addPath(68, 45, {});
        addPath(45, 30, {46,47});
        addPath(45, 24, {});
        addPath(24, 65, {});
        addPath(21, 54, {});
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

        if(startH < 0 || endH > 24 || startH >= endH) return false;

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

    int fallbackParking(
        const vector<int>& realPath,
        int startH,
        int endH
    ){
        for(int i = (int)realPath.size()-1; i >= 0; i--){

            int id = realPath[i];

            if(!nodes.count(id) || nodes[id].ghost) continue;

            if(availableAtTime(id, startH, endH)){
                return id;
            }
        }
        return -1;
    }

    void updateCapacity(int id, int newCap){

        if(!nodes.count(id)) return;

        int oldCap = nodes[id].capacity;
        nodes[id].capacity = newCap;

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