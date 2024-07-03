#include <string>
#include <queue>
#include <fstream>
#include "Node.h"
#include "Storage/Storage.h"
#include <sstream>

#ifndef HUFFMAN_H
#define HUFFMAN_H

using namespace std;

class Huffman {
private:
    string header;
    unordered_map<char, int> frequency_map;
    unordered_map<char, string> huffCodes;
    priority_queue<Node*, vector<Node*>, compareWeights> freq_node_queue;

    /**
     * Helper function to open file from input file name. Close isolated
     * @param in_file file name string
     * @return opened file
     */
    static ifstream inputFileOpener(const string &in_file);

    /**
     * Helper function to open file from output file name. Close isolated
     * @param out_file file name string
     * @return opened file
     */
    static ofstream outputFileOpener(const string &out_file);

    /**
     * Helper function to read file
     * and count frequency of each character and store in map, a data member
     * @param in_file
     */
    void countCharFrequency(const string &in_file);

    /**
     * Helper function to load each character, frequency node to priority queue
     * from the data member map
     */
    void loadToPriorityQueue();

    /**
     * Helper function to create a header string needed for later decompression
     */
    void createHeader();

    /**
     * Helper function to build huffman tree from the nodes loaded in priority queue
     * @return root node of Huffman tree
     */
    Node* buildHuffTree();

    /**
     * Helper function to traverse the huffman tree
     * and obtain huff code for each character, store in map (key is character)
     * @param node
     * @param huffCode
     */
    void createHuffCodes(Node *node, const string &huffCode);

    /**
     * Helper function to "replace" and write code for each character in binary file
     * @param in_file
     * @param out_file
     */
    void encode(const string &in_file, const string &out_file);

    /**
     * Helper function to read the binary file
     * @param in_file
     * @return string stream containing the codes of for each character of the text
     */
    stringstream readBinaryFile(const string &in_file);

    /**
     * Helper function to interpret header string
     * and store each char huff code in the map
     * @return the length of the original (not encoded) string
     */
    int readHeader();

    /**
     * Helper function to rebuild huffman tree from the huff codes map for decoding
     * @return root node of the rebuilt Huffman tree
     */
    Node* buildHuffTreeDecode();

    /**
     * Helper function to traverse the rebuilt huffman tree and obtain each character
     * given binary version of the text
     * @param binaryStr binary version of the text
     * @param totalStrLength the length of the original (not encoded) string
     * @param out_file output file
     */
    void decode(const string &binaryStr, const int &totalStrLength, const string &out_file,  Node* root);
public:
    /**
     * Default constructor
     */
    Huffman();

    /**
     * Compress a given file
     * @param in_file file to compress
     * @param out_file file to store compressed/encoded version of text
     */
    void compress(const string &in_file, const string &out_file);

    /**
     * Decompress a given file
     * @param in_file file to decompress
     * @param out_file file to store decompressed/decoded version of text
     */
    void decompress(const string &in_file, const string &out_file);
};;

#endif //HUFFMAN_H
