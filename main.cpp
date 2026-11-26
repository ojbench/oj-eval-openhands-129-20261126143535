#include <iostream>
#include <string>
#include <vector>
#include <unordered_map>

using namespace std;

struct VarInfo {
    char type; // 0='int', 1='string'
    string value; // stored as string
};

// Find variable from innermost to outermost scope
static inline VarInfo* findVar(const string& name, vector<unordered_map<string, VarInfo>>& scopes) {
    for (int i = (int)scopes.size() - 1; i >= 0; i--) {
        auto it = scopes[i].find(name);
        if (it != scopes[i].end()) {
            return &it->second;
        }
    }
    return nullptr;
}

// Fast integer parsing - returns true if valid integer
static inline bool parseLongLong(const char* s, long long& out) {
    const char* p = s;
    bool neg = false;
    if (*p == '-') { neg = true; p++; }
    else if (*p == '+') { p++; }
    if (*p < '0' || *p > '9') return false;
    unsigned long long val = 0;
    while (*p >= '0' && *p <= '9') {
        val = val * 10 + (*p - '0');
        p++;
    }
    // Skip trailing whitespace (\r, \n, etc.)
    while (*p == ' ' || *p == '\t' || *p == '\r' || *p == '\n') p++;
    if (*p != '\0') return false;
    out = neg ? -(long long)val : (long long)val;
    return true;
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int n;
    cin >> n;
    string dummy;
    getline(cin, dummy); // consume rest of first line

    vector<unordered_map<string, VarInfo>> scopes;
    scopes.reserve(128);
    scopes.emplace_back(); // global scope

    string line;
    while (n--) {
        getline(cin, line);
        
        // Remove trailing \r if present (Windows line endings)
        if (!line.empty() && line.back() == '\r') {
            line.pop_back();
        }

        // Find first space to get command
        size_t sp = line.find(' ');
        
        if (sp == string::npos) {
            // No space - could be Indent or Dedent (no-argument commands)
            if (line == "Indent") {
                scopes.emplace_back();
            } else if (line == "Dedent") {
                if (scopes.size() <= 1) {
                    cout << "Invalid operation\n";
                } else {
                    scopes.pop_back();
                }
            }
            // else: unknown command, skip
            continue;
        }

        // Command dispatch based on first character
        char c0 = line[0];
        
        if (c0 == 'D') { // Dedent or Declare
            if (line.size() >= 7 && line[0] == 'D' && line[1] == 'e' && line[2] == 'c' && line[3] == 'l' && line[4] == 'a' && line[5] == 'r' && line[6] == 'e') { // Declare
                // Declare [type] [variable_name] [value]
                size_t p1 = sp + 1;
                size_t p2 = line.find(' ', p1);
                size_t p3 = line.find(' ', p2 + 1);
                
                string type(line.data() + p1, p2 - p1);
                string varName(line.data() + p2 + 1, p3 - p2 - 1);
                string value(line.data() + p3 + 1, (int)line.size() - p3 - 1);

                // Check if variable already in current scope
                if (scopes.back().count(varName)) {
                    cout << "Invalid operation\n";
                } else {
                    VarInfo vi;
                    vi.type = (type == "string") ? 1 : 0;
                    vi.value = (type == "string") ? value.substr(1, value.size() - 2) : value;
                    scopes.back()[varName] = vi;
                }
            } else { // Dedent
                if (scopes.size() <= 1) {
                    cout << "Invalid operation\n";
                } else {
                    scopes.pop_back();
                }
            }

        } else if (c0 == 'A') { // Add
            // Add [result] [value1] [value2]
            size_t p1 = sp + 1;
            size_t p2 = line.find(' ', p1);
            size_t p3 = line.find(' ', p2 + 1);
            
            string rstr(line.data() + p1, p2 - p1);
            string v1str(line.data() + p2 + 1, p3 - p2 - 1);
            string v2str(line.data() + p3 + 1, (int)line.size() - p3 - 1);

            VarInfo* vr = findVar(rstr, scopes);
            VarInfo* v1 = findVar(v1str, scopes);
            VarInfo* v2 = findVar(v2str, scopes);

            if (!v1 || !v2 || !vr) {
                cout << "Invalid operation\n";
            } else if (v1->type != v2->type || v1->type != vr->type) {
                cout << "Invalid operation\n";
            } else if (v1->type == 0) { // int
                long long a, b;
                if (!parseLongLong(v1->value.c_str(), a) || !parseLongLong(v2->value.c_str(), b)) {
                    cout << "Invalid operation\n";
                } else {
                    vr->value = to_string(a + b);
                }
            } else { // string
                vr->value = v1->value + v2->value;
            }

        } else if (c0 == 'S') { // SelfAdd
            // SelfAdd [variable_name] [value]  OR  SelfAdd [type] [variable_name] [value]
            size_t p1 = sp + 1;
            size_t p2 = line.find(' ', p1);
            
            string first(line.data() + p1, p2 - p1);
            
            bool typePrefixed = false;
            if (first == "int" || first == "string") {
                size_t p3 = line.find(' ', p2 + 1);
                if (p3 != string::npos) {
                    typePrefixed = true;
                }
            }

            if (typePrefixed) {
                size_t p3 = line.find(' ', p2 + 1);
                string nstr(line.data() + p2 + 1, p3 - p2 - 1);
                string vstr(line.data() + p3 + 1, (int)line.size() - p3 - 1);
                
                VarInfo* v = findVar(nstr, scopes);
                if (!v) {
                    cout << "Invalid operation\n";
                } else if (v->type == 0 && first != "int") {
                    cout << "Invalid operation\n";
                } else if (v->type == 1 && first != "string") {
                    cout << "Invalid operation\n";
                } else {
                    if (v->type == 0) { // int
                        long long a, b;
                        if (!parseLongLong(v->value.c_str(), a) || !parseLongLong(vstr.c_str(), b)) {
                            cout << "Invalid operation\n";
                        } else {
                            v->value = to_string(a + b);
                        }
                    } else { // string
                        if (vstr.size() >= 2 && vstr.front() == '"' && vstr.back() == '"') {
                            v->value += vstr.substr(1, vstr.size() - 2);
                        } else {
                            cout << "Invalid operation\n";
                        }
                    }
                }
            } else {
                string nstr(first);
                string vstr(line.data() + p2 + 1);
                
                VarInfo* v = findVar(nstr, scopes);
                if (!v) {
                    cout << "Invalid operation\n";
                } else {
                    if (v->type == 0) { // int
                        if (vstr.size() >= 2 && vstr.front() == '"' && vstr.back() == '"') {
                            cout << "Invalid operation\n";
                        } else {
                            long long a, b;
                            if (!parseLongLong(v->value.c_str(), a) || !parseLongLong(vstr.c_str(), b)) {
                                cout << "Invalid operation\n";
                            } else {
                                v->value = to_string(a + b);
                            }
                        }
                    } else { // string
                        if (vstr.size() >= 2 && vstr.front() == '"' && vstr.back() == '"') {
                            v->value += vstr.substr(1, vstr.size() - 2);
                        } else {
                            cout << "Invalid operation\n";
                        }
                    }
                }
            }

        } else if (c0 == 'P') { // Print
            string nstr(line.data() + sp + 1);
            VarInfo* v = findVar(nstr, scopes);
            if (!v) {
                cout << "Invalid operation\n";
            } else {
                cout << nstr << ":" << v->value << "\n";
            }
        }
        // else: unknown command, skip
    }

    return 0;
}
