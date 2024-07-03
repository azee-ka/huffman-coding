#include "Huffman.h"

Huffman::Huffman() {
    header = "";
}

void Huffman::compress(const string &in_file, const string &out_file) {
    // call helper func to count frequency of each character
    countCharFrequency(in_file);
    // call helper function to load all char nodes to priority queue
    loadToPriorityQueue();
    // call helper function to build "tree" using the priority queue
    Node *root = buildHuffTree();
    // call helper func to create huff code for each letter by traversing tree
    createHuffCodes(root, "");
    // call helper function to write encoded string to output file
    encode(in_file, out_file);
}

ifstream Huffman::inputFileOpener(const string &in_file) {
    ifstream in(in_file);
    if(!in.is_open()) {
        throw invalid_argument("File couldn't be opened!");
    }
    return in;
}

ofstream Huffman::outputFileOpener(const string &out_file) {
    ofstream out(out_file);
    if(!out.is_open()) {
        throw invalid_argument("File couldn't be opened!");
    }
    return out;
}

void Huffman::countCharFrequency(const string &in_file) {
    // open input file
    ifstream in = inputFileOpener(in_file);
    // Calculate total length of original string
    long totalStrLength = 0;
    char c;
    // traverse, by each character in file
    while(in.get(c)) {
        totalStrLength++;
        frequency_map[c]++; // store frequency of each character in map
    }
    header += to_string(totalStrLength) + '\30'; // Add total length to header
    in.close(); // close input file
}

void Huffman::loadToPriorityQueue() {
    // traverse frequency map to create node for each char and store to priority queue
    for (const pair<char, int> pair: frequency_map) {
        // put new node to queue
        freq_node_queue.push(new Node(pair.first, pair.second));
    }
}

Node* Huffman::buildHuffTree() {
    // traverse nodes queue until only one node is left in queue
    while(freq_node_queue.size() > 1) {
        // store first node
        Node *zero = freq_node_queue.top();
        // remove first node
        freq_node_queue.pop();
        // store second node
        Node *one = freq_node_queue.top();
        // remove second
        freq_node_queue.pop();
        // create parent node with the two above nodes as children
        Node *parentNode = new Node('\0', zero->weight + one->weight, zero, one);
        // put the merged node back to queue
        freq_node_queue.push(parentNode);
    }
    // root is the last node left in the queue
    Node *root = freq_node_queue.top();
    freq_node_queue.pop();
    return root;
}

void Huffman::createHuffCodes(Node *node, const string &huffCode) {
    if(node == nullptr) {
        return;
    }
    if(node->letter != '\0') {
        huffCodes[node->letter] = huffCode;
    }
    createHuffCodes(node->zero, huffCode + '0');
    createHuffCodes(node->one, huffCode + '1');
}


void Huffman::createHeader() {
    // Add character and code info to header
    for (const pair<char, string>& pair : huffCodes) {
        header += pair.first + pair.second + '\30';
    }
}


void Huffman::encode(const string &in_file, const string &out_file) {
    ifstream in = inputFileOpener(in_file);
    // Create a new storage object
    Storage *storage = new Storage();

    if (!storage->open(out_file, "write")) {
        std::cout << "There was an error opening the file." << std::endl;
        exit(0);
    }
    // call helper func to create header for the tree
    createHeader();
    // set the header first
    storage->setHeader(header);

    const int BUFFER_SIZE = 1024; // 1KB buffer
    char buffer[BUFFER_SIZE];
    // read binary file in "chunks" of 1KB, for efficiency
    while (in.read(buffer, BUFFER_SIZE)) {
        // read BUFFER_SIZE bytes into buffer
        stringstream encodedData;
        for (int i = 0; i < in.gcount(); ++i) {
            encodedData << huffCodes[buffer[i]]; // append huff codes to encoded string for each letter
        }
        // insert the "chunk" of bits to the output file (insert func to take care of writing to binary file)
        storage->insert(encodedData.str());
    }

    // Handle the remaining bits (if any)
    if (in.gcount() > 0) {
        stringstream encodedData;
        for (int i = 0; i < in.gcount(); ++i) {
            encodedData << huffCodes[buffer[i]]; // append huff codes to encoded string, for each letter
        }
        // Insert the remaining binary string to the output file
        storage->insert(encodedData.str());
    }

    // close both files
    in.close();
    storage->close();
    delete storage;
}

Node* Huffman::buildHuffTreeDecode() {
    // assign root to new node
    Node* root = new Node('\0', 0);
    // traverse through huffCodes map
    for (const pair<char, string>& pair : huffCodes) {
        char letter = pair.first; // store letter from current key
        string code = pair.second; // store code for the corresponding letter
        // node to traverse to build tree
        Node* currentNode = root;
        // loop through the huff code string for the corresponding letter
        for (char c : code) {
            // check if 0 is encountered
            if (c == '0') {
                // if current node child zero is null, "zero" child is set new node
                if (currentNode->zero == nullptr) {
                    currentNode->zero = new Node('\0', 0);
                }
                // traverse to "zero"
                currentNode = currentNode->zero;
            } // check if 1 is encountered
            else {
                // if current node child one is null, "one" child is set new node
                if (currentNode->one == nullptr) {
                    currentNode->one = new Node('\0', 0);
                }
                // traverse to "one"
                currentNode = currentNode->one;
            }
        }
        // store letter found to current node
        currentNode->letter = letter;
    }
    return root;
}



void Huffman::decompress(const string &in_file, const string &out_file) {
    // call helper function to store returned binary string
    string binaryStr = readBinaryFile(in_file).str();
    // call helper function readHeader to read header
    // and store the original string length returned by it
    int totalStrLength = readHeader();
    // call helper function to rebuild tree for decoding
    Node* root = buildHuffTreeDecode();
    // call helper function to traverse through "tree"
    // to get and write each letter to output file
    decode(binaryStr, totalStrLength, out_file, root);
}


int Huffman::readHeader() {
    istringstream iss(header);
    string token;
    getline(iss, token, '\30'); // get the string original string length from header
    int totalStrLength = std::stoi(token); // convert the length to integer
    // traverse through header string
    while (std::getline(iss, token, '\30')) {
        huffCodes[token[0]] = token.substr(1); // store the huff codes in map, corresponding to each char
    }
    return totalStrLength; // return original string length
}

void Huffman::decode(const string &binaryStr, const int &totalStrLength, const string &out_file,  Node* root) {
    ofstream out = outputFileOpener(out_file); // Open file to write decoded string
    Node *current = root;
    int decodedLength = 0; // Track the length of the decoded string

    // Traverse binary string by each character
    for (char c : binaryStr) {
        // Check if the decoded length matches the original string's length
        if (decodedLength == totalStrLength) {
            break; // Stop decoding if the original length is reached
        }

        // Move to the next node based on the current bit (0 or 1)
        current = (c == '0') ? current->zero : current->one;

        // Check if a leaf node (character) is reached
        if (current->letter != '\0' || (current->zero == nullptr & current->one == nullptr)) {
            // Write the decoded character to the output file
            out << current->letter;
            // Reset back to the root for decoding the next character
            current = root;
            // Increment the decoded length
            decodedLength++;
        }
    }

    out.close();
}


stringstream Huffman::readBinaryFile(const string &in_file) {
    Storage *storage = new Storage();
    if (!storage->open(in_file, "read")) {
        std::cout << "There was an error opening the file." << std::endl;
        exit(0);
    }
    header = storage->getHeader(); // first get header from the binary file
    stringstream binaryResult; // to store binary resulting string
    string byte; // to store 8-bits, returned by storage extract function
    while (storage->extract(byte)) {
        binaryResult << byte; // append each byte to resulting binary string
    }
    delete storage;
    return binaryResult;
}

/*
text_files/100west.txt
output_huff_text_files/gogogophers.huff.txt

text_files/Adventures_of_Sherlock_Holmes.txt
output_huff_text_files/gogogophers.huff.txt

text_files/gogogophers.txt
output_huff_text_files/gogogophers.huff.txt

text_files/MOBY-DICK.txt
output_huff_text_files/gogogophers.huff.txt

text_files/sample-2mb-text-file.txt
output_huff_text_files/gogogophers.huff.txt


output_huff_text_files/gogogophers.huff.txt
output_text_files/gogogophers.txt.txt
 */