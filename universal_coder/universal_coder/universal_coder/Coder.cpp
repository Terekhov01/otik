#define _CRT_SECURE_NO_WARNINGS
#include <cstdio>
#include <fstream>
#include <iostream>
#include <map>
#include <unordered_map>
#include <vector>
#include <algorithm>
#include <chrono>
#define ll unsigned long long int

using namespace std;
using namespace std::chrono;
/*
	Compression Library
*/
namespace Huffman {

	string HuffmanValue[256] = { "" };

	typedef struct Node {
	public:
		char character;
		ll count;
		Node* left, * right;

		Node(ll count) {
			this->character = 0;
			this->count = count;
			this->left = this->right = nullptr;
		}

		Node(char character, ll count) {
			this->character = character;
			this->count = count;
			this->left = this->right = nullptr;
		}
	} Node;
	/*
		Common function necessary for both compression and decompression.
	*/
	namespace Utility {

		ll GetFileSize(const char* filename) {
			FILE* p_file = fopen(filename, "rb+");
			if (!p_file) {
				perror("Failed to open");
			}
			fseek(p_file, 0, SEEK_END);
			ll size = ftell(p_file);
			fclose(p_file);
			return size;
		}
		// Test function to print huffman codes for each character.
		void Inorder(Node* root, string& value) {
			if (root) {
				value.push_back('0');
				Inorder(root->left, value);
				value.pop_back();
				if (!root->left && !root->right) {
					printf("Character: %c, Count: %lld, ", root->character, root->count);
					cout << "Huffman Value: " << value << endl;
				}
				value.push_back('1');
				Inorder(root->right, value);
				value.pop_back();
			}
		}
	};

	/*
		Functions necessary for compression.
	*/
	namespace CompressUtility {
		/*
		Combine two nodes
		*/
		Node* Combine(Node* a, Node* b) {
			Node* parent = new Node((a ? a->count : 0) + (b ? b->count : 0));
			parent->left = b;
			parent->right = a;
			return parent;
		}

		bool sortbysec(Node* a, Node* b) {
			return (a->count > b->count);
		}
		// Initial Pass for counting characters.
		map <char, ll> ParseFile(const char* filename, ll Filesize) {
			register FILE* ptr = fopen(filename, "rb");

			if (ptr == NULL) {
				perror("Error: File not found:");
				exit(-1);
			}
			register unsigned char ch;
			register ll size = 0, filesize = Filesize;
			vector<ll>Store(256, 0);

			while (size != filesize) {
				ch = fgetc(ptr);
				++Store[ch];
				++size;
			}
			map <char, ll> store;
			for (int i = 0; i < 256; ++i)
				if (Store[i])
					store[i] = Store[i];
			fclose(ptr);
			return store;
		}

		vector <Node*> SortByCharacterCount(const map <char, ll >& value) {
			vector < Node* > store;
			auto it = begin(value);
			for (; it != end(value); ++it)
				store.push_back(new Node(it->first, it->second));
			sort(begin(store), end(store), sortbysec);

			return store;
		}
		// Generate a header for the file.
		// Format: 
		// 1. Total Unique Character (1 byte)
		// 2. For each unique character:
		// 2a. Character (1 byte)
		// 2b. Length of code (1 byte)
		// 2c. Huffman code (min: 1 byte, max: 255bytes)
		// 3. Padding
		// Worst case header size: 1 + (1+1)*(1+2+3+4+5+...+255) + 1 ~ 32kb... (only happens when skewed Huffman tree is generated)
		// Best case header size: 1 + 1 + 1 + 1 + 1 = 5bytes (Happens only when a single character exists in an entire file).
		string GenerateHeader(char padding) {
			string header = "";
			// UniqueCharacter start from -1 {0 means 1, 1 means 2, to conserve memory}
			unsigned char UniqueCharacter = 255;

			for (int i = 0; i < 256; ++i) {
				if (HuffmanValue[i].size()) {
					header.push_back(i);
					header.push_back(HuffmanValue[i].size());
					header += HuffmanValue[i];
					++UniqueCharacter;
				}
			}
			char value = UniqueCharacter;

			return value + header + (char)padding;
		}

		// Store Huffman values for each character in string.
		// returns the size of the resulting file (without the header)
		ll StoreHuffmanValue(Node* root, string& value) {
			ll temp = 0;
			if (root) {
				value.push_back('0');
				temp = StoreHuffmanValue(root->left, value);
				value.pop_back();
				if (!root->left && !root->right) {
					HuffmanValue[(unsigned char)root->character] = value;
					temp += value.size() * root->count;
				}
				value.push_back('1');
				temp += StoreHuffmanValue(root->right, value);
				value.pop_back();
			}
			return temp;
		}
		// Create huffman tree during compression...
		Node* GenerateHuffmanTree(const map <char, ll>& value) {
			vector < Node* > store = SortByCharacterCount(value);
			Node* one, * two, * parent;
			if (store.size() == 1) {
				return Combine(store.back(), nullptr);
			}
			while (store.size() > 2) {
				one = *(end(store) - 1); two = *(end(store) - 2);
				parent = Combine(one, two);
				store.pop_back(); store.pop_back();
				store.push_back(parent);

				vector <Node*> ::iterator it1 = end(store) - 2;
				while ((*it1)->count < parent->count && it1 != begin(store))
					--it1;
				sort(it1, end(store), sortbysec);
			}
			one = *(end(store) - 1); two = *(end(store) - 2);
			parent = Combine(one, two);
			return parent;
		}
		void CompressNoAlgo(const char* filename, const ll Filesize) {
			FILE* iptr = fopen(filename, "rb"), * optr = fopen((string(filename) + ".myzip").c_str(), "wb");
			string signature = "123C7500";
			signature += "00000001";//version
			char fs_buf[8];
			_itoa(Filesize, fs_buf, 16);
			for (int c = 7; c >= 0; c--) {
				if (fs_buf[c] - '0' < 0) signature += '0';
			}
			signature += fs_buf;//source file size
			signature += "00";//protection
			signature += "00";//alg with context
			signature += "00";//without
			signature += "00";//encryption
			signature += "00";//ending

			fputs(signature.c_str(), optr);

			char *content = (char*)malloc(Filesize + 1);
			fgets(content, Filesize, iptr);
			fputs(content, optr);
			free(content);
		}

		// Actual compression of a file.
		void Compress(const char* filename, const ll Filesize, const ll PredictedFileSize) {
			const char padding = (8 - ((PredictedFileSize) & (7))) & 7;
			string header = GenerateHeader(padding);
			int header_i = 0;
			const int h_length = header.size();
			cout << "Padding size: " << (int)padding << endl;
			FILE* iptr = fopen(filename, "rb"), * optr = fopen((string(filename) + ".myzip").c_str(), "wb");

			string signature = "123C7500";
			signature += "00000001";//version
			char fs_buf[8];
			_itoa(Filesize, fs_buf, 16);
			for (int c = 7; c >= 0; c--) {
				if (fs_buf[c] - '0' < 0) signature += '0';
			}
			signature += fs_buf;//source file size
			signature += "00";//protection
			signature += "00";//alg with context
			signature += "01";//without
			signature += "00";//encryption
			signature += "00";//ending

			fputs(signature.c_str(), optr);

			while (header_i < h_length) {
				fputc(header[header_i], optr);
				++header_i;
			}

			if (!iptr) {
				perror("Error: File not found: ");
				exit(-1);
			}

			unsigned char ch, fch = 0;
			char counter = 7;
			ll size = 0, i;
			while (size != Filesize) {
				ch = fgetc(iptr);
				i = 0;
				while (HuffmanValue[ch][i] != '\0') {
					fch = fch | ((HuffmanValue[ch][i] - '0') << counter);
					--counter;
					if (counter == -1) {
						fputc(fch, optr);
						counter = 7;
						fch = 0;
					}
					++i;
				}
				++size;
				if (((size * 100 / Filesize)) > ((size - 1) * 100 / Filesize))
					printf("\r%d%% completed  ", (size * 100 / Filesize));
			}
			if (fch)
				fputc(fch, optr);
			printf("\n");
			fclose(iptr);
			fclose(optr);

			fclose(iptr);
			fclose(optr);
		}

	};
};

using namespace Huffman;

int main(int argc, char* argv[]) {

	if (argc != 2) {
		printf("Usage:\n (a.exe|./a.out) FileToBeCompressed");
		exit(-1);
	}
	const char* filename = argv[1];
	printf("%s\n", filename);

	ll filesize, predfilesize = 0;
	filesize = Utility::GetFileSize(filename);
	auto mapper = CompressUtility::ParseFile(filename, filesize);
	Node* const root = CompressUtility::GenerateHuffmanTree(mapper);
	string buf = "";
	predfilesize = CompressUtility::StoreHuffmanValue(root, buf);
	printf("Original File: %lld bytes\n", filesize);
	printf("Compressed File Size (without header): %lld bytes\n", (predfilesize + 7) >> 3);
	if (((predfilesize + 7) >> 3) >= filesize){
		CompressUtility::CompressNoAlgo(filename, filesize);
	}
	else {
		CompressUtility::Compress(filename, filesize, predfilesize);
	}
	return 0;
}
