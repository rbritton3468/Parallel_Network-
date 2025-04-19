// importing necessary things
#include "compression.hpp"
#include <vector>
#include <cstdint>
#include <fstream>
#include <iterator>
#include <iostream>
#include <unordered_map>
#include <queue>


// tree node used for compression
struct TreeNode{
    uint8_t val;
    uint64_t freq;
    TreeNode* right;
    TreeNode* left;

    // leaf nodes have the bytes
    TreeNode(uint8_t value, uint64_t frequency): val(value), freq(frequency), right(nullptr), left(nullptr) {}
    // internal nodes only carry frequency
    TreeNode(TreeNode* leftNode, TreeNode* rightNode): val(0), freq(rightNode->freq + leftNode->freq), right(rightNode), left(leftNode) {}

    bool isLeaf(){
        return (left == nullptr && right == nullptr);
    }
};

// comparator between tree nodes
struct Comparator{
    bool operator()(const TreeNode* l, const TreeNode* r){
        return l->freq > r->freq;
    }
};

// function to read from a binary file
std::vector<uint8_t> readBinaryFile(const std::string& fileName){
    // opening file
    std::ifstream binaryFile(fileName, std::ios::binary);

    if(!binaryFile.is_open()){
        // error handling
        std::cout << "File " << fileName << " could not be opened" << std::endl;
        return {};
    }
    else{
        // returning populated buffer with binary bytes
        return std::vector<uint8_t>((std::istreambuf_iterator<char>(binaryFile)), {});
    }
}


// function that goes through data and maps frequency of each byte
std::unordered_map<uint8_t, uint64_t> findFrequency(std::vector<uint8_t>& binaryFile){
    std::unordered_map<uint8_t, uint64_t> frequencyMap = {};
    for(uint8_t byte: binaryFile){
        if(frequencyMap.find(byte) == frequencyMap.end()){
            frequencyMap[byte] = 1;
        }
        else{
            frequencyMap[byte] += 1;
        }
    }
    return frequencyMap;
}

// function that builds the tree given a frequency map
TreeNode* buildTreeFromMap(std::unordered_map<uint8_t, uint64_t> map){
    // creating a min heap
    std::priority_queue<TreeNode*, std::vector<TreeNode*>, Comparator> minHeap;

    // populating it
    for(const auto& pair: map){
        minHeap.push(new TreeNode(pair.first, pair.second));
    }

    // building the tree
    while(minHeap.size() > 1){
        TreeNode* left = minHeap.top();
        minHeap.pop();
        TreeNode* right = minHeap.top();
        minHeap.pop();

        TreeNode* newNode = new TreeNode(left, right);
        minHeap.push(newNode);
    }

    return minHeap.top();
}

// recursive algorithm
void dfs(TreeNode* node, std::string currPath, std::unordered_map<uint8_t, std::string>& map){
    if(node == nullptr){
        return;
    }

    if(node->isLeaf()){
        map[node->val] = currPath;
    }
    else{
        dfs(node->left, currPath + "0", map);
        dfs(node->right, currPath + "1", map);
    }
}

// function that returns a map of encodings given a tree
std::unordered_map<uint8_t, std::string> getEncodings(TreeNode* root){
    std::unordered_map<uint8_t, std::string> encodings = {};

    // calling recursive function to populate map
    dfs(root, "", encodings);

    return encodings;
}

// function to serialize the tree
void serializeTree(TreeNode* node, std::string& output){
    if(node == nullptr){
        return;
    }

    if(node->isLeaf()){
        output += "1";
        for(int i = 7; i >= 0; i--){
            output += ((node->val >> i) & 1) ? '1' : '0';
        }
    }
    else{
        output += "0";
        serializeTree(node->left, output);
        serializeTree(node->right, output);
    }
}


// function that takes in the binary file, compresses it, and returns a compressed version
std::string compressFile(const std::string& inputExe){
    // first getting the bytes in a vector representation
    std::vector<uint8_t> binary = readBinaryFile(inputExe);

    // getting a frequency map of each pattern
    std::unordered_map<uint8_t, uint64_t> frequencyMap = findFrequency(binary);

    // building a tree with the most frequent bytes near the root
    TreeNode* root = buildTreeFromMap(frequencyMap);

    // once we have the tree, need to traverse it and get a map of each byte to its encoding
    std::unordered_map<uint8_t, std::string> encodings = getEncodings(root);

    // now that we have encodings, we can finally compress the data
    std::string compressedData = "";
    for(uint8_t byte: binary){
        compressedData += encodings[byte];
    }

    // now packing our string into bytes 
    std::vector<uint8_t> finalBytes;
    int bits = 0;
    uint8_t currByte = 0;

    for(char bit: compressedData){
        currByte <<= 1;
        if(bit == '1'){
            currByte |= 1;
        }
        bits += 1;

        if(bits == 8){
            finalBytes.push_back(currByte);
            currByte = 0;
            bits = 0;
        }
    }

    if(bits > 0){
        currByte <<= (8-bits);
        finalBytes.push_back(currByte);
    }

    // need to serialize the tree so decoder can decode
    std::string serializedTree = "";
    serializeTree(root, serializedTree);

    // packing those bits into a vector
    std::vector<uint8_t> finalBytesTree;

    bits = 0;
    currByte = 0;

    for(char bit: serializedTree){
        currByte <<= 1;
        if(bit == '1'){
            currByte |= 1;
        }
        bits += 1;

        if(bits == 8){
            finalBytesTree.push_back(currByte);
            currByte = 0;
            bits = 0;
        }
    }

    if(bits > 0){
        currByte <<= (8-bits);
        finalBytesTree.push_back(currByte);
    }


    // writing to the file
    std::ofstream outputFile(inputExe + ".cmp", std::ios::binary);

    uint32_t treeSize = static_cast<uint32_t>(finalBytesTree.size());
    outputFile.write(reinterpret_cast<const char*>(&treeSize), sizeof(treeSize));
    outputFile.write(reinterpret_cast<const char*>(finalBytesTree.data()), finalBytesTree.size());
    outputFile.write(reinterpret_cast<const char*>(finalBytes.data()), finalBytes.size());

    outputFile.close();

    std::cout << "old had a size of " << binary.size() << " bytes"<< std::endl;
    std::cout << "compressed has a size of " << finalBytes.size() << " bytes" << std::endl;

    // returning name of output
    return inputExe + ".cmp";
}


//--------------------CODE TO DECOMPRESS FILE------------------------------

