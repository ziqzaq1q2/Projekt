//Kody Huffmana
//Jan Wojceichowski
//www.algorytm.org

#include <iostream>
#include <fstream>
#include <vector>
#include <algorithm>

const bool bLeft = false;
const bool bRight = true;

typedef unsigned char Byte;

Byte setBit(Byte x, unsigned int index) {
	return x | (1 << index);
}

bool getBit(Byte x, unsigned int index) {
	return (x & (1 << index)) != 0;
}

class Node {
public:
	static bool greaterThan(Node *a, Node *b) {
		if(a != NULL && b != NULL) {
			return a->count > b->count;
		} else if(a != NULL) {
			return true;
		} else {
			return false;
		}
	}

	static bool lessThan(Node *a, Node *b) {
		if(a != NULL && b != NULL) {
			return a->count < b->count;
		} else if(b != NULL) {
			return true;
		} else {
			return false;
		}
	}

	static void removeTree(Node *root) {
		if(root == NULL) {
			return;
		}
		removeTree(root->left);
		removeTree(root->right);
		delete root;
	}

	Node(Byte b, Node *l = NULL, Node *r = NULL, unsigned long c = 0) : left(l), right(r), count(c), byte(b) {
	}

	bool isLeaf() const {
		return this->left == NULL && this->right == NULL;
	}

	Node *left, *right;
	unsigned long count;
	Byte byte;
};

// Wyzncza kod dla ka¿dego liœcia w danym drzewie.
void mapTree(Node *root, std::vector<bool> *codes, std::vector<bool> &prefix = std::vector<bool>()) {
	if(root == NULL) {
		return;
	}

	if(root->left == NULL && root->right == NULL) {
		// Jesteœmy w liœciu. ZnaleŸliœmy kod jednego bajtu.
		codes[root->byte] = prefix;
	}

	if(root->left != NULL) {
		prefix.push_back(bLeft);
		mapTree(root->left, codes, prefix);
		prefix.pop_back();
	}
	if(root->right != NULL) {
		prefix.push_back(bRight);
		mapTree(root->right, codes, prefix);
		prefix.pop_back();
	}
}

// Zapisuje drzewo do strumienia danych.
void saveTree(Node *root, std::ostream &output, Byte &accumulator, unsigned int &bitIndex) {
	if(root == NULL) {
		return;
	}
	if(bitIndex == 8) {
		output.write(reinterpret_cast<char *>(&accumulator), sizeof(accumulator));
		accumulator = 0;
		bitIndex = 0;
	}
	if(root->isLeaf()) {
		accumulator = setBit(accumulator, bitIndex);
		++bitIndex;
		if(bitIndex == 8) {
			output.write(reinterpret_cast<char *>(&accumulator), sizeof(accumulator));
			accumulator = 0;
			bitIndex = 0;
		}
		for(unsigned int i = 0; i < 8; ++i) {
			if(getBit(root->byte, i)) {
				accumulator = setBit(accumulator, bitIndex);
			}
			++bitIndex;
			if(bitIndex == 8) {
				output.write(reinterpret_cast<char *>(&accumulator), sizeof(accumulator));
				accumulator = 0;
				bitIndex = 0;
			}
		}
	} else {
		++bitIndex;
		if(bitIndex == 8) {
			output.write(reinterpret_cast<char *>(&accumulator), sizeof(accumulator));
			accumulator = 0;
			bitIndex = 0;
		}
		saveTree(root->left, output, accumulator, bitIndex);
		saveTree(root->right, output, accumulator, bitIndex);
	}
}

// Wczytuje drzewo ze strumienia danych.
bool loadTree(std::istream &input, Byte &accumulator, unsigned int &bitIndex, Node *&root) {
	if(bitIndex == 8) {
		if(!input.read(reinterpret_cast<char *>(&accumulator), sizeof(accumulator))) {
			return false;
		}
		bitIndex = 0;
	}
	root = new Node(0);
	bool bit = getBit(accumulator, bitIndex);
	++bitIndex;
	if(bit) {
		for(unsigned int i = 0; i < 8; ++i) {
			if(bitIndex == 8) {
				if(!input.read(reinterpret_cast<char *>(&accumulator), sizeof(accumulator))) {
					delete root;
					root = NULL;
					return false;
				}
				bitIndex = 0;
			}
			if(getBit(accumulator, bitIndex)) {
				root->byte = setBit(root->byte, i);
			}
			++bitIndex;
		}
	} else {
		if(!loadTree(input, accumulator, bitIndex, root->left)) {
			delete root;
			root = NULL;
			return false;
		}
		if(!loadTree(input, accumulator, bitIndex, root->right)) {
			Node::removeTree(root);
			root = NULL;
			return false;
		}
	}
	return true;
}

void compress(std::istream &input, std::ostream &output) {
	Byte buffer;
	std::istream::pos_type start = input.tellg();

	std::vector<Node *> nodes(256);
	for(unsigned int i = 0; i < 256; ++i) {
		nodes[i] = new Node(i);
	}

	// Przechodzimy po strumieniu wejœciowym, ¿eby policzyæ liczbê wyst¹pienia ka¿dego bajtu.
	while(input.read(reinterpret_cast<char *>(&buffer), sizeof(buffer))) {
		++(nodes[buffer]->count);
	}

	// Tworzymy drzewo.
	std::sort(nodes.begin(), nodes.end(), Node::greaterThan);
	while(nodes.size() > 1) {
		Node *a, *b, *c;
		a = nodes.back();
		nodes.pop_back();
		b = nodes.back();
		nodes.pop_back();
		c = new Node(0, a, b, a->count + b->count);
		nodes.insert(std::upper_bound(nodes.begin(), nodes.end(), c, Node::greaterThan), c);
	}
	Node *root = nodes.back();
	nodes.clear();

	Byte accumulator = 0; // Akumulator bitów.
	unsigned int bitIndex = 0; // Licznik bitów.

	saveTree(root, output, accumulator, bitIndex); // Zapisujemy drzewo.

	// Dla usprawnienia dalszych operacji zapisujemy kody w tablicy.
	std::vector<bool> ;
	mapTree(root, codes);

	Node::removeTree(root); // Drzewo ju¿ nie jest potrzebne.

	// Wracamy na pocz¹tek strumienia danych.
	input.clear();
	input.seekg(start);

	while(input.read(reinterpret_cast<char *>(&buffer), sizeof(buffer))) {
		for(std::vector<bool>::const_iterator i = codes[buffer].begin(); i != codes[buffer].end(); ++i) {
			if(*i) {
				accumulator = setBit(accumulator, bitIndex);
			}
			++bitIndex;
			if(bitIndex == 8) {
				output.write(reinterpret_cast<char *>(&accumulator), sizeof(accumulator));
				bitIndex = 0;
				accumulator = 0;
			}
		}
	}
	if(bitIndex > 0) {
		output.write(reinterpret_cast<char *>(&accumulator), sizeof(accumulator));
		bitIndex = 0;
		accumulator = 0;
	}
}

bool decompress(std::istream &input, std::ostream &output) {
	Node *root = NULL;

	Byte accumulator = 0;
	unsigned int bitIndex = 8;

	// Wczytujemy drzewo.
	if(!loadTree(input, accumulator, bitIndex, root)) {
		return false;
	}

	Node *current = root;
	while(true) {
		// Sprawdzamy czy nie ma b³êdu.
		if(current == NULL) {
			Node::removeTree(root); // Sprz¹tanie.
			return false;
		}

		if(current->isLeaf()) {
			// Zapisujemy jeden bajt do wyniku.
			output.write(reinterpret_cast<char *>(&(current->byte)), sizeof(current->byte));
			current = root;
		}

		// Odczytujemy kolejny bajt.
		if(bitIndex == 8) {
			if(!input.read(reinterpret_cast<char *>(&accumulator), sizeof(accumulator))) {
				break;
			}
			bitIndex = 0;
		}

		// Odczytujemy jeden bit.
		bool bit = getBit(accumulator, bitIndex);
		++bitIndex;
		if(bit == bLeft) {
			current = current->left;
		} else {
			current = current->right;
		}
	}

	Node::removeTree(root); // Sprz¹tanie.

	return true;
}

int main() {
	std::ifstream input;
	std::ofstream output;

	std::cout << "Wybierz opcje:" << std::endl
		<< "c - kompresuj plik \"plik.txt\"" << std::endl
		<< "d - dekompresuj plik \"plik skompresowany\"" << std::endl
		<< "<inny znak> - zakoncz program" << std::endl;
	char option;
	std::cin >> option;
	if(option == 'c') {
		input.open("plik.txt", std::ios::binary);
		if(!input.is_open()) {
			std::cout << "Nie udalo sie otworzyc pliku \"plik.txt\".";
			::getchar();
			return 0;
		}

		output.open("plik skompresowany", std::ios::binary);
		if(!output.is_open()) {
			std::cout << "Nie udalo sie otworzyc pliku \"plik skompresowany\".";
			input.close();
			::getchar();
			return 0;
		}

		compress(input, output);

		output.close();
		input.close();
	} else if(option == 'd') {
		input.open("plik skompresowany", std::ios::binary);
		if(!input.is_open()) {
			std::cout << "Nie udalo sie otworzyc pliku \"plik skompresowany\".";
			::getchar();
			return 0;
		}

		output.open("plik zdekompresowany.txt", std::ios::binary);
		if(!output.is_open()) {
			std::cout << "Nie udalo sie otworzyc pliku \"plik zdekompresowany.txt\".";
			input.close();
			::getchar();
			return 0;
		}

		if(!decompress(input, output)) {
			std::cout << "Nie udalo sie zdekompresowac pliku \"plik skompresowany\"." << std::endl;
			::getchar();
		}

		output.close();
		input.close();
	}

	return 0;
}
