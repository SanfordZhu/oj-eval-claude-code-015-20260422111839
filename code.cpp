#include <bits/stdc++.h>
using namespace std;

static const int BUCKETS = 16; // keep under 20 file count limit

static inline uint32_t hash_str(const string &s) {
    // FNV-1a 32-bit
    uint32_t h = 2166136261u;
    for (unsigned char c : s) {
        h ^= c;
        h *= 16777619u;
    }
    return h;
}

static inline string bucket_path(int b) {
    char buf[64];
    snprintf(buf, sizeof(buf), "kvstore_bucket_%02d.dat", b);
    return string(buf);
}

struct Record {
    uint8_t op;      // 1 = insert, 0 = delete
    uint16_t klen;   // length of key
    int32_t value;   // non-negative
    string key;      // klen bytes
};

static void append_record(const string &key, int32_t value, uint8_t op) {
    uint32_t h = hash_str(key);
    int b = (int)(h % BUCKETS);
    string path = bucket_path(b);
    ofstream out(path, ios::binary | ios::app);
    if (!out) return; // if cannot open, silently ignore
    uint8_t op8 = op;
    uint16_t klen = (uint16_t)key.size();
    int32_t val = value;
    out.write(reinterpret_cast<const char*>(&op8), 1);
    out.write(reinterpret_cast<const char*>(&klen), 2);
    out.write(reinterpret_cast<const char*>(&val), 4);
    out.write(key.data(), key.size());
}

static void find_and_print(const string &key) {
    uint32_t h = hash_str(key);
    int b = (int)(h % BUCKETS);
    string path = bucket_path(b);
    ifstream in(path, ios::binary);
    if (!in) { cout << "null\n"; return; }
    unordered_set<int32_t> setv;
    // scan bucket file sequentially
    while (true) {
        uint8_t op8; uint16_t klen; int32_t val;
        in.read(reinterpret_cast<char*>(&op8), 1);
        if (!in) break;
        in.read(reinterpret_cast<char*>(&klen), 2);
        if (!in) break;
        in.read(reinterpret_cast<char*>(&val), 4);
        if (!in) break;
        string k; k.resize(klen);
        in.read(&k[0], klen);
        if (!in) break;
        if (k == key) {
            if (op8 == 1) {
                setv.insert(val);
            } else {
                auto it = setv.find(val);
                if (it != setv.end()) setv.erase(it);
            }
        }
    }
    if (setv.empty()) { cout << "null\n"; return; }
    vector<int32_t> vals; vals.reserve(setv.size());
    for (auto v : setv) vals.push_back(v);
    sort(vals.begin(), vals.end());
    for (size_t i = 0; i < vals.size(); ++i) {
        if (i) cout << ' ';
        cout << vals[i];
    }
    cout << '\n';
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int n; if (!(cin >> n)) return 0;
    for (int i = 0; i < n; ++i) {
        string cmd; cin >> cmd;
        if (cmd == "insert") {
            string idx; long long v; cin >> idx >> v;
            append_record(idx, (int32_t)v, 1);
        } else if (cmd == "delete") {
            string idx; long long v; cin >> idx >> v;
            append_record(idx, (int32_t)v, 0);
        } else if (cmd == "find") {
            string idx; cin >> idx;
            find_and_print(idx);
        } else {
            // ignore unknown
            string line;
            getline(cin, line);
        }
    }
    return 0;
}
