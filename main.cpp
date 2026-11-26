#include <iostream>
#include <string>
#include <vector>
#include <unordered_map>

using namespace std;

struct VarInfo {
    string type; // "int" or "string"
    string value; // stored as string
};

// Find variable from innermost to outermost scope
static VarInfo* findVar(const string& name, vector<unordered_map<string, VarInfo>>& scopes) {
    for (int i = (int)scopes.size() - 1; i >= 0; i--) {
        auto it = scopes[i].find(name);
        if (it != scopes[i].end()) {
            return &it->second;
        }
    }
    return nullptr;
}

// Strip surrounding quotes from a string value
static string stripQuotes(const string& s) {
    if (s.size() >= 2 && s.front() == '"' && s.back() == '"') {
        return s.substr(1, s.size() - 2);
    }
    return s;
}

// Check if a string looks like a quoted string literal
static bool isQuotedString(const string& s) {
    return s.size() >= 2 && s.front() == '"' && s.back() == '"';
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int n;
    cin >> n;
    string dummy;
    getline(cin, dummy); // consume rest of first line

    vector<unordered_map<string, VarInfo>> scopes;
    scopes.emplace_back(); // global scope

    string line;
    while (n--) {
        getline(cin, line);

        // Extract command name (first word)
        size_t sp = line.find(' ');
        string cmd = (sp == string::npos) ? line : line.substr(0, sp);

        if (cmd == "Indent") {
            scopes.emplace_back();

        } else if (cmd == "Dedent") {
            if (scopes.size() <= 1) {
                cout << "Invalid operation\n";
            } else {
                scopes.pop_back();
            }

        } else if (cmd == "Declare") {
            // Declare [type] [variable_name] [value]
            string rest = line.substr(sp + 1);
            size_t p = rest.find(' ');
            string type = rest.substr(0, p);
            rest = rest.substr(p + 1);
            p = rest.find(' ');
            string varName = rest.substr(0, p);
            string value = rest.substr(p + 1);

            // Check if variable already in current scope
            if (scopes.back().count(varName)) {
                cout << "Invalid operation\n";
            } else {
                VarInfo vi;
                vi.type = type;
                vi.value = (type == "string") ? stripQuotes(value) : value;
                scopes.back()[varName] = vi;
            }

        } else if (cmd == "Add") {
            // Add [result] [value1] [value2]
            string rest = line.substr(sp + 1);
            size_t p = rest.find(' ');
            string result = rest.substr(0, p);
            rest = rest.substr(p + 1);
            p = rest.find(' ');
            string value1 = rest.substr(0, p);
            string value2 = rest.substr(p + 1);

            VarInfo* v1 = findVar(value1, scopes);
            VarInfo* v2 = findVar(value2, scopes);
            VarInfo* vr = findVar(result, scopes);

            if (!v1 || !v2 || !vr) {
                cout << "Invalid operation\n";
            } else if (v1->type != v2->type) {
                cout << "Invalid operation\n";
            } else {
                if (v1->type == "int") {
                    long long a = stoll(v1->value);
                    long long b = stoll(v2->value);
                    vr->value = to_string(a + b);
                } else {
                    vr->value = v1->value + v2->value;
                }
            }

        } else if (cmd == "SelfAdd") {
            // SelfAdd [variable_name] [value]  OR  SelfAdd [type] [variable_name] [value]
            string rest = line.substr(sp + 1);
            size_t p = rest.find(' ');
            string first = rest.substr(0, p);

            // Determine if type-prefixed: first token is "int"/"string" AND there's a second space
            bool typePrefixed = false;
            string afterFirst;
            size_t p2 = string::npos;
            if ((first == "int" || first == "string")) {
                afterFirst = rest.substr(p + 1);
                p2 = afterFirst.find(' ');
                if (p2 != string::npos) {
                    typePrefixed = true;
                }
            }

            if (typePrefixed) {
                // Type-prefixed form: SelfAdd [type] [variable_name] [value]
                string varName = afterFirst.substr(0, p2);
                string value = afterFirst.substr(p2 + 1);

                VarInfo* v = findVar(varName, scopes);
                if (!v) {
                    cout << "Invalid operation\n";
                } else if (v->type != first) {
                    cout << "Invalid operation\n";
                } else {
                    if (v->type == "int") {
                        long long a = stoll(v->value);
                        long long b = stoll(value);
                        v->value = to_string(a + b);
                    } else {
                        v->value += stripQuotes(value);
                    }
                }
            } else {
                // Non-type-prefixed form: SelfAdd [variable_name] [value]
                string varName = first;
                string value = rest.substr(p + 1);

                VarInfo* v = findVar(varName, scopes);
                if (!v) {
                    cout << "Invalid operation\n";
                } else {
                    if (v->type == "int") {
                        if (!isQuotedString(value)) {
                            long long a = stoll(v->value);
                            long long b = stoll(value);
                            v->value = to_string(a + b);
                        } else {
                            cout << "Invalid operation\n";
                        }
                    } else {
                        if (isQuotedString(value)) {
                            v->value += stripQuotes(value);
                        } else {
                            cout << "Invalid operation\n";
                        }
                    }
                }
            }

        } else if (cmd == "Print") {
            // Print [variable_name]
            string varName = line.substr(sp + 1);

            VarInfo* v = findVar(varName, scopes);
            if (!v) {
                cout << "Invalid operation\n";
            } else {
                cout << varName << ":" << v->value << "\n";
            }
        }
    }

    return 0;
}
