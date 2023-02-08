#define _CRT_SECURE_NO_WARNINGS
#include <iostream>
#include <string>
#include <fstream>
#define ll unsigned long long int


using namespace std;

// »ерархи€ классов, определ€юща€ алгоритмы разжати€ файлов
class Decompression
{
public:
    virtual ~Decompression() {}
    virtual void decompress(const string& file) = 0;
    ll GetFileSize(const char* filename) {
        FILE* p_file = fopen(filename, "rb");
        fseek(p_file, 0, SEEK_END);
        ll size = ftell(p_file);
        fclose(p_file);
        return size;
    }

};

class Haffman_Decompression : public Decompression
{
public:
    void decompress(const string& file) {
        string cmd = "a.out -dc test.txt.myzip";
        system(cmd.c_str());
    }
};

class Lab1_Decompression : public Decompression
{
public:
    void decompress(const string& file) {
        FILE* iptr = fopen(file.c_str(), "rb"), * optr = fopen(string("noalgodecode" + file.substr(0, file.length() - 6)).c_str(), "wb");
        ll Filesize = Decompression::GetFileSize(file.c_str());
        fseek(iptr, 34, 0);
        

        char* content = (char*)malloc(Filesize);
        fgets(content, Filesize, iptr);
        fputs(content, optr);
        free(content);

        fclose(iptr);
        fclose(optr);
    }
};

//  ласс дл€ использовани€
class Decompressor
{
public:
    Decompressor() {}
    Decompressor(Decompression* comp) : p(comp) {}
    ~Decompressor() { delete p; }
    void decompress(const string& file) {
        p->decompress(file);
    }
private:
    Decompression* p;
};

int main()
{
    FILE* ptrFile;
    char ch;
    int i = 0;
    string code = "";
    string filename="lab1test.txt.myzip";
    //cin >> filename;
    if ((ptrFile = fopen(filename.c_str(), "rb")) == NULL) {
        perror("Cannot open file.\n");
        exit(1);
    }
    fseek(ptrFile, 28, SEEK_SET);
    for (int i = 0; i < 2; i++) {
        code += fgetc(ptrFile);
    }
    fclose(ptrFile);

    /*for (int i = 0; i < 2;i++) {
        cout << code[i] ;
    }*/
    Decompressor* p = new Decompressor();
    if (code == "00") {
        p = new Decompressor(new Lab1_Decompression);
        p->decompress(filename);
    }
    else if (code == "01") {
        p = new Decompressor(new Haffman_Decompression);
        p->decompress(filename);
    }

    delete p;
    return 0;
}