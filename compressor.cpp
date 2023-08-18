#include <iostream>
#include <fstream>
#include <vector>
#include <queue>
#include <unordered_map>
#include <bitset>

using namespace std;

// Huffman tree node
struct Node {
    char data;
    int frequency;
    Node* left, * right;

    Node(char data, int frequency) : data(data), frequency(frequency), left(nullptr), right(nullptr) {}
};

// Function to compare nodes based on their frequency
struct compare {
    bool operator()(Node* l, Node* r) {
        return l->frequency > r->frequency;
    }
};

// Function to generate Huffman codes for each character
void generateCodes(Node* root, string code, unordered_map<char, string>& codes) {
    if (root == nullptr)
        return;

    if (!root->left && !root->right) {
        codes[root->data] = code;
        return;
    }

    generateCodes(root->left, code + "0", codes);
    generateCodes(root->right, code + "1", codes);
}

// Function to build the Huffman tree and generate codes
void buildHuffmanTree(const string& text, unordered_map<char, string>& codes) {
    if (text.empty())
        return;

    // Calculate the frequency of each character
    unordered_map<char, int> freq;
    for (char c : text)
        freq[c]++;

    // Create a priority queue to store nodes
    priority_queue<Node*, vector<Node*>, compare> pq;

    // Create a leaf node for each character and add it to the priority queue
    for (const auto& pair : freq) {
        pq.push(new Node(pair.first, pair.second));
    }

    // Build the Huffman tree
    while (pq.size() > 1) {
        Node* left = pq.top(); pq.pop();
        Node* right = pq.top(); pq.pop();

        Node* newNode = new Node('$', left->frequency + right->frequency);
        newNode->left = left;
        newNode->right = right;
        pq.push(newNode);
    }

    Node* root = pq.top();

    // Generate Huffman codes
    generateCodes(root, "", codes);

    // Clean up memory
    delete root;
}

// Function to compress the input file using Huffman coding
void compressFile(const string& inputFile, const string& outputFile) {
    // Read the input file
    ifstream inFile(inputFile, ios::binary);
    if (!inFile) {
        cerr << "Failed to open the input file." << endl;
        return;
    }

    // Read the file content into a string
    string text((istreambuf_iterator<char>(inFile)), (istreambuf_iterator<char>()));
    inFile.close();

    // Build Huffman tree and generate codes
    unordered_map<char, string> codes;
    buildHuffmanTree(text, codes);

    // Compressed binary output stream
    ofstream outFile(outputFile, ios::binary);
    if (!outFile) {
        cerr << "Failed to create the output file." << endl;
        return;
    }

    // Write the number of distinct characters to the output file
    size_t numDistinctChars = codes.size();
    outFile.write(reinterpret_cast<const char*>(&numDistinctChars), sizeof(size_t));

    // Write the mapping of characters and their codes to the output file
    for (const auto& pair : codes) {
        outFile.write(reinterpret_cast<const char*>(&pair.first), sizeof(char));
        size_t codeSize = pair.second.size();
        outFile.write(reinterpret_cast<const char*>(&codeSize), sizeof(size_t));
        outFile.write(pair.second.data(), codeSize);
    }

    // Compress the file content
    string compressedBits;
    for (char c : text) {
        compressedBits += codes[c];
        while (compressedBits.size() >= 8) {
            bitset<8> bits(compressedBits.substr(0, 8));
            char byte = static_cast<char>(bits.to_ulong());
            outFile.write(reinterpret_cast<const char*>(&byte), sizeof(char));
            compressedBits = compressedBits.substr(8);
        }
    }

    // Write the remaining bits (less than 8 bits) to the output file
    if (!compressedBits.empty()) {
        bitset<8> bits(compressedBits);
        char byte = static_cast<char>(bits.to_ulong());
        outFile.write(reinterpret_cast<const char*>(&byte), sizeof(char));
    }

    outFile.close();
    cout << "File compressed successfully." << endl;
}

// Function to decompress the input file using Huffman coding
void decompressFile(const string& inputFile, const string& outputFile) {
    // Read the compressed input file
    ifstream inFile(inputFile, ios::binary);
    if (!inFile) {
        cerr << "Failed to open the input file." << endl;
        return;
    }

    // Read the number of distinct characters
    size_t numDistinctChars;
    inFile.read(reinterpret_cast<char*>(&numDistinctChars), sizeof(size_t));

    // Read the mapping of characters and their codes
    unordered_map<string, char> reverseCodes;
    for (size_t i = 0; i < numDistinctChars; ++i) {
        char c;
        inFile.read(reinterpret_cast<char*>(&c), sizeof(char));
        size_t codeSize;
        inFile.read(reinterpret_cast<char*>(&codeSize), sizeof(size_t));
        string code(codeSize, '\0');
        inFile.read(&code[0], codeSize);
        reverseCodes[code] = c;
    }

    // Decompress the file content
    ofstream outFile(outputFile, ios::binary);
    if (!outFile) {
        cerr << "Failed to create the output file." << endl;
        return;
    }

    string compressedBits;
    char byte;
    while (inFile.read(reinterpret_cast<char*>(&byte), sizeof(char))) {
        bitset<8> bits(byte);
        compressedBits += bits.to_string();
    }

    string code;
    for (char bit : compressedBits) {
        code += bit;
        if (reverseCodes.find(code) != reverseCodes.end()) {
            char c = reverseCodes[code];
            outFile.write(&c, sizeof(char));
            code.clear();
        }
    }

    inFile.close();
    outFile.close();
    cout << "File decompressed successfully." << endl;
}

int main() {
    string inputFile = "input.txt";
    string compressedFile = "compressed.bin";
    string decompressedFile = "decompressed.txt";

    // // Compress the input file
    // compressFile(inputFile, compressedFile);

    // Decompress the compressed file
    decompressFile(compressedFile, decompressedFile);

    return 0;
}
