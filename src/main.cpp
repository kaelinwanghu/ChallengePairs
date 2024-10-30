#include "graph.hpp"

void fast() {
    std::ios_base::sync_with_stdio(false);
    std::cin.tie(nullptr);
}
// Custom hasher using xxHash
struct XXHash {
    size_t operator()(const int& key) const {
        return XXH64(&key, sizeof(key), 0);
    }
};

// Shared hash map
emhash8::HashMap<uint16_t, uint16_t, XXHash> myHashMap;
std::mutex mapMutex;

int main() {
    fast();
    return 0;
}