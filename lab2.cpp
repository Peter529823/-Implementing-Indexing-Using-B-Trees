#include <iostream>
#include <fstream>
#include <vector>
#include <cstring>
#include <sstream>
#include <iomanip>
#include <cstdint> 

using namespace std;

// Constants for fixed field sizes in student records
const int ID_SIZE = 9;
const int LAST_NAME_SIZE = 15;
const int FIRST_NAME_SIZE = 15;
const int YEAR_SIZE = 1;
const int MAJOR_SIZE = 4;
const int EMAIL_SIZE = 20;
const int RECORD_SIZE = 64;  // Total record size in bytes

// Represents a student record with fixed-length fields
struct StudentRecord {
    char id[ID_SIZE + 1];
    char lastName[LAST_NAME_SIZE + 1];
    char firstName[FIRST_NAME_SIZE + 1];
    char year[YEAR_SIZE + 1];
    char major[MAJOR_SIZE + 1];
    char email[EMAIL_SIZE + 1];
};

// Node structure for the B-Tree
class BTreeNode {
public:
    vector<string> keys;        // Keys (e.g., ID or Last Name)
    vector<long> positions;     // Positions of records in the database
    vector<BTreeNode*> children; // Child nodes
    bool leaf;                  // True if this is a leaf node
    int childCount;             // Number of children

    BTreeNode(bool isLeaf) : leaf(isLeaf), childCount(0) {}
};

// B-Tree structure for indexing
class BTree {
private:
    int order;  // Order of the B-Tree
    BTreeNode* root;
    fstream indexFile;

public:
    BTree(int m, const string& indexFileName) : order(m), root(new BTreeNode(true)) { // Initialize root
        indexFile.open(indexFileName, ios::binary | ios::in | ios::out | ios::app);
        if (!indexFile) {
            cerr << "Error opening index file." << endl;
        }
    }

    ~BTree();
    BTreeNode* getRoot() { return root; } // Public method to access root
    void insert(const string& key, long pos);
    long search(const string& key);
    void splitChild(BTreeNode* parent, int i, BTreeNode* child);
    long appendNode(BTreeNode* node);
    void writeNodeToFile(BTreeNode* node);
    void readNodeFromFile(BTreeNode* node, long position);
    void printTree();

private:
    void printTreeHelper(BTreeNode* node, int level);
};

// Appends a new node to the index file and returns its position
long BTree::appendNode(BTreeNode* node) {
    indexFile.seekp(0, ios::end);
    long currentPos = indexFile.tellp();
    writeNodeToFile(node);
    return currentPos;
}

// Writes a node's data to the file
void BTree::writeNodeToFile(BTreeNode* node) {
    indexFile.write(reinterpret_cast<char*>(&node->childCount), sizeof(int));

    for (const auto& key : node->keys) {
        char paddedKey[ID_SIZE + 1] = {0};
        strncpy(paddedKey, key.c_str(), ID_SIZE);
        indexFile.write(paddedKey, ID_SIZE);
    }

    for (const auto& pos : node->positions) {
        indexFile.write(reinterpret_cast<const char*>(&pos), sizeof(long));
    }

    for (const auto& child : node->children) {
        long childOffset = reinterpret_cast<long>(child);
        indexFile.write(reinterpret_cast<const char*>(&childOffset), sizeof(long));
    }
}

// Reads a node's data from the file
void BTree::readNodeFromFile(BTreeNode* node, long position) {
    indexFile.seekg(position);
    indexFile.read(reinterpret_cast<char*>(&node->childCount), sizeof(int));

    node->keys.clear();
    node->positions.clear();
    node->children.clear();

    for (int i = 0; i < node->childCount - 1; ++i) {
        char key[ID_SIZE + 1] = {0};
        indexFile.read(key, ID_SIZE);
        node->keys.push_back(string(key));
    }

    for (int i = 0; i < node->childCount - 1; ++i) {
        long pos;
        indexFile.read(reinterpret_cast<char*>(&pos), sizeof(long));
        node->positions.push_back(pos);
    }

    for (int i = 0; i < node->childCount; ++i) {
        long childOffset;
        indexFile.read(reinterpret_cast<char*>(&childOffset), sizeof(long));
        node->children.push_back(reinterpret_cast<BTreeNode*>(childOffset));
    }
}

// Splits a child node
void BTree::splitChild(BTreeNode* parent, int i, BTreeNode* child) {
    BTreeNode* newNode = new BTreeNode(child->leaf);
    int mid = order / 2;

    newNode->keys.assign(child->keys.begin() + mid + 1, child->keys.end());
    newNode->positions.assign(child->positions.begin() + mid + 1, child->positions.end());
    if (!child->leaf) {
        newNode->children.assign(child->children.begin() + mid + 1, child->children.end());
    }

    child->keys.resize(mid);
    child->positions.resize(mid);
    if (!child->leaf) {
        child->children.resize(mid + 1);
    }

    long newNodeOffset = appendNode(newNode);
    parent->keys.insert(parent->keys.begin() + i, child->keys[mid]);
    parent->positions.insert(parent->positions.begin() + i, child->positions[mid]);
    parent->children.insert(parent->children.begin() + i + 1, reinterpret_cast<BTreeNode*>(newNodeOffset));

    writeNodeToFile(parent); // Update parent node in the file
}

// Searches for a key in the B-Tree
long BTree::search(const string& key) {
    if (!root) return -1;

    BTreeNode* node = root;
    while (node) {
        size_t i = 0;
        while (i < node->keys.size() && key > node->keys[i]) {
            ++i;
        }
        if (i < node->keys.size() && node->keys[i] == key) {
            return node->positions[i];
        }
        if (node->leaf) break;
        node = node->children[i];
    }
    return -1;
}

// Print B-tree structure for debugging
void BTree::printTree() {
    printTreeHelper(root, 0);
}

void BTree::printTreeHelper(BTreeNode* node, int level) {
    if (node) {
        for (const auto& key : node->keys) {
            cout << string(level, ' ') << key << endl;
        }
    }
}

// Pads a field to its maximum size
void padField(char* field, int maxSize) {
    int len = strlen(field);
    for (int i = len; i < maxSize; ++i) {
        field[i] = ' ';
    }
    field[maxSize] = '\0';
}

// Dumps the B-Tree structure to the output file
void dumpTree(BTreeNode* node, ofstream& outFile, const string& type, int level = 0) {
    if (!node) return;

    for (size_t i = 0; i < node->keys.size(); ++i) {
        outFile << string(level * 2, ' ') << node->positions[i] << ":" << node->keys[i] << endl;
    }

    for (auto* child : node->children) {
        dumpTree(child, outFile, type, level + 1);
    }
}

// Builds the database and indexes from a data file
void buildDatabase(const string& dataFile, const string& dbName, BTree& idIndex, BTree& nameIndex) {
    ifstream inputFile(dataFile);
    ofstream dbFile(dbName + ".dat", ios::binary);
    if (!inputFile || !dbFile) {
        cerr << "Error opening file." << endl;
        return;
    }
    string line;
    long pos = 0;
    while (getline(inputFile, line)) {
        StudentRecord record;
        stringstream ss(line);
        ss >> record.id >> record.lastName >> record.firstName >> record.year >> record.major >> record.email;

        padField(record.id, ID_SIZE);
        padField(record.lastName, LAST_NAME_SIZE);
        padField(record.firstName, FIRST_NAME_SIZE);
        padField(record.year, YEAR_SIZE);
        padField(record.major, MAJOR_SIZE);
        padField(record.email, EMAIL_SIZE);

        dbFile.write(reinterpret_cast<char*>(&record), sizeof(StudentRecord));

        idIndex.insert(record.id, pos);
        nameIndex.insert(record.lastName, pos);

        pos += RECORD_SIZE;
    }

    inputFile.close();
    dbFile.close();
}

// Processes commands from a file
void processCommands(const string& cmdFileName, ofstream& outFile, BTree& idIndex, BTree& nameIndex) {
    ifstream cmdFile(cmdFileName);
    string line;
    long pos = 0; // Track position for added records
    while (getline(cmdFile, line)) {
        stringstream ss(line);
        string command;
        ss >> command;

        if (command == "find") {
            string type, key;
            ss >> type >> key;
            outFile << "COMMAND: find " << type << " " << key << "  " << endl;
            long pos = (type == "ID") ? idIndex.search(key) : nameIndex.search(key);
            if (pos != -1) {
                outFile << pos << ":" << key << endl;
            } else {
                outFile << "NOT FOUND" << endl;
            }
        } else if (command == "findname" || command == "find name") { // Fix handling
            string key;
            ss >> key;
            outFile << "COMMAND: find name " << key << "  " << endl;
            long pos = nameIndex.search(key);
            if (pos != -1) {
                outFile << pos << ":" << key << endl;
            } else {
                outFile << "NOT FOUND" << endl;
            }
        } else if (command == "add") {
            // Write the new record to the database file
            ofstream dbFile("student1.dat", ios::binary | ios::app);
            StudentRecord record;
            ss >> record.id >> record.lastName >> record.firstName >> record.year >> record.major >> record.email;

            padField(record.id, ID_SIZE);
            padField(record.lastName, LAST_NAME_SIZE);
            padField(record.firstName, FIRST_NAME_SIZE);
            padField(record.year, YEAR_SIZE);
            padField(record.major, MAJOR_SIZE);
            padField(record.email, EMAIL_SIZE);

            dbFile.write(reinterpret_cast<char*>(&record), sizeof(StudentRecord));
            dbFile.close();

            idIndex.insert(record.id, pos);
            nameIndex.insert(record.lastName, pos);

            outFile << "STUDENT " << record.id << " ADDED (" << pos << ";0;0)" << endl; // Fix position format
            pos += RECORD_SIZE; // Update position for the next record
        } else if (command == "dump") {
            string type;
            ss >> type;
            outFile << "COMMAND: dump by " << type << "  " << endl;
            if (type == "ID") {
                dumpTree(idIndex.getRoot(), outFile, type); // Use getRoot
            } else if (type == "name") {
                dumpTree(nameIndex.getRoot(), outFile, type); // Use getRoot
            }
        }
    }
}

int main() {
    BTree idIndex(3, "student1.ix1");
    BTree nameIndex(3, "student1.ix2");

    buildDatabase("datafile1.txt", "student1", idIndex, nameIndex);

    ofstream outFile("outfile1.txt");
    processCommands("cmdfile1.txt", outFile, idIndex, nameIndex);
    outFile.close();

    return 0;
}
