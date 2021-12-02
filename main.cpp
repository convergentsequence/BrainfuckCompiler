#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <stack>
#include <stdlib.h>

#define ADD_ASM(x) assem += x; assem += "\n"
#define MEMSIZE "30000"

using namespace std;

string cleanup(string source){
    string clean;
    for(int i = 0; i < source.size(); i++){
        switch(source[i]){
            case '>':
            case '<':
            case '+':
            case '-':
            case '.':
            case ',':
            case '[':
            case ']':
                clean += source[i];
        }
    }
    return clean;
}

struct instruction{
    int count;
    char name;
    int bid; // bracket id, if an opening and closing bracket have the same id they jump to each other
};

int buildInstructionList(struct instruction* ilist, string source){
    int latestAdded = -1;
    char original;
    int count;
    stack<int> bracketstack;
    int bracketindex;
    for(int i = 0; i < source.size(); i++){
        switch(source[i]){
            case '+':
            case '-':
            case '>':
            case '<':
                original = source[i];
                count = 0;
                while(i < source.size()){
                    if(source[i] != original){
                        i--;
                        break;
                    }
                    count++;
                    i++;
                }
                ilist[++latestAdded].count = count;
                ilist[latestAdded].name = original;
                break;
            case '[':
                ilist[++latestAdded].bid = bracketindex;
                bracketstack.push(bracketindex++);
                ilist[latestAdded].count = 1;
                ilist[latestAdded].name = source[i];
                break;
            case ']':
                if(bracketstack.size() < 1){
                    cout << "Error: Unbalanced brackets" << endl;
                    exit(1);
                }
                ilist[++latestAdded].bid = bracketstack.top();
                bracketstack.pop();
                ilist[latestAdded].count = 1;
                ilist[latestAdded].name = source[i];
                break;
            default:
                ilist[++latestAdded].count = 1;
                ilist[latestAdded].name = source[i];
         }
    }
    return latestAdded+1;
}

void printInstructionList(struct instruction* ilist, int programsize){
    
    for(int i = 0; i < programsize; i++){
        cout << "Instruction: " << ilist[i].name << " Count: " << ilist[i].count << endl;
    }
}


string buildasm(struct instruction* program, int programsize){
    string assem;
    string bracketname;
    
    ADD_ASM ("global main");
    ADD_ASM ("extern malloc");
    ADD_ASM ("extern free");
    ADD_ASM ("extern memset");
    ADD_ASM ("extern putchar");
    ADD_ASM ("extern readchar");
    ADD_ASM ("extern free");
    ADD_ASM ("extern getchar");
    ADD_ASM ("section .text");

    ADD_ASM ("    ; create the increment \"function\"");
    ADD_ASM ("increment:");
    ADD_ASM ("    mov rax, QWORD [rbp-8]");
    ADD_ASM ("    add rax, QWORD [rbp-16]");
    ADD_ASM ("    add BYTE [rax], cl");
    ADD_ASM ("    ret");

    ADD_ASM ("    ; create the decrement \"function\"");
    ADD_ASM ("decrement:");
    ADD_ASM ("    mov rax, QWORD [rbp-8]");
    ADD_ASM ("    add rax, QWORD [rbp-16]");
    ADD_ASM ("    sub BYTE [rax], cl");
    ADD_ASM ("    ret");


    ADD_ASM ("main:");
    ADD_ASM ("    ; setup the stack frame and create space for the current position pointer and the memory pointer");
    ADD_ASM ("    push rbp");
    ADD_ASM ("    mov rbp, rsp");
    ADD_ASM ("    sub rsp, 16");
    ADD_ASM ("    sub rsp, 32");

    ADD_ASM ("    ; allocate the memory on the heap");
    assem += "    mov rcx, ";
    ADD_ASM (MEMSIZE);
    ADD_ASM ("    call malloc");
    ADD_ASM ("    mov QWORD [rbp-8], rax");

    ADD_ASM ("    ; zero out the memory with memset");
    ADD_ASM ("    mov rcx, QWORD [rbp-8]");
    ADD_ASM ("    mov rdx, 0");
    assem += "    mov r8, ";
    ADD_ASM (MEMSIZE);
    ADD_ASM ("    call memset");

    ADD_ASM ("    ; set the current position pointer to 0");
    ADD_ASM ("    mov QWORD [rbp-16], 0");


    stack<string> bracketstack;
    for(int i = 0; i < programsize; i++){
        assem += "    ; Instruction: ";
        assem += program[i].name;
        assem += " Count: ";
        assem += to_string(program[i].count);
        assem += "\n";
        switch(program[i].name){
            case '+':
            case '-':
                assem += "    mov rcx, ";
                assem += to_string(program[i].count);
                assem += "\n";
                if(program[i].name == '+'){
                    ADD_ASM("    call increment");
                }else{
                    ADD_ASM("    call decrement");
                }
                break;
            case '>':
                if(program[i].count == 1){
                    ADD_ASM ("    inc QWORD [rbp-16]");
                }else{
                    assem += "    add QWORD [rbp-16], ";
                    assem += to_string(program[i].count);
                    assem += "\n";
                }
                break;
            case '<':
                if(program[i].count == 1){
                    ADD_ASM ("    dec QWORD [rbp-16]");
                }else{
                    assem += "    sub QWORD [rbp-16], ";
                    assem += to_string(program[i].count);
                    assem += "\n";
                }
                break;
            case '.':
                ADD_ASM ("    mov rax, QWORD [rbp-8]");
                ADD_ASM ("    add rax, QWORD [rbp-16]");
                ADD_ASM ("    xor rcx, rcx");
                ADD_ASM ("    mov cl, BYTE [rax]");
                ADD_ASM ("    call putchar");
                break;
            case ',':
                ADD_ASM ("    call getchar");
                ADD_ASM ("    mov rcx, QWORD [rbp-8]");
                ADD_ASM ("    add rcx, QWORD [rbp-16]");
                ADD_ASM ("    mov BYTE [rcx], al");
                break;
            case '[':
                bracketname = "bracket_";
                bracketname += to_string(program[i].bid);
                bracketname += "_close";
                ADD_ASM ("    mov rax, QWORD [rbp-8]");
                ADD_ASM ("    add rax, QWORD [rbp-16]");
                ADD_ASM ("    cmp BYTE [rax], 0");
                assem += "    je ";
                ADD_ASM (bracketname);
                bracketname = "bracket_";
                bracketname += to_string(program[i].bid);
                bracketname += "_open:";
                ADD_ASM (bracketname);
                break;
            case ']':
                bracketname = "bracket_";
                bracketname += to_string(program[i].bid);
                bracketname += "_open";
                ADD_ASM ("    mov rax, QWORD [rbp-8]");
                ADD_ASM ("    add rax, QWORD [rbp-16]");
                ADD_ASM ("    cmp BYTE [rax], 0");
                assem += "    jne ";
                ADD_ASM (bracketname);
                bracketname = "bracket_";
                bracketname += to_string(program[i].bid);
                bracketname += "_close:";
                ADD_ASM (bracketname);
                break;
        }
    }

    ADD_ASM ("    ; free the memory on the heap");
    ADD_ASM ("    mov rcx, QWORD [rbp-8]");
    ADD_ASM ("    call free");

    ADD_ASM ("    ; return 0");
    ADD_ASM ("    mov rax, 0");
    ADD_ASM ("    mov rsp, rbp");
    ADD_ASM ("    pop rbp");
    ADD_ASM ("    ret");
    return assem;
}

int main(int argc, char** argv){

    if(argc < 2){
        cout << "Error: No file provided" << endl;
        return 1;
    }
    
    ifstream sourcefile;
    sourcefile.open(argv[1]);
    if(!sourcefile.is_open()){
        cout << "Error: Can't open provided file";
        return 1;
    }
    stringstream sourcestream;
    sourcestream << sourcefile.rdbuf();
    string source = sourcestream.str();
    sourcefile.close();
    
    source = cleanup(source);

    struct instruction* program = (struct instruction*)malloc(source.size() * sizeof(struct instruction));

    int programsize = buildInstructionList(program, source);

    string assem = buildasm(program, programsize);

    ofstream outfile;
    outfile.open("out.asm");
    outfile << assem;
    outfile.close();

    free(program);

    return 0;
}
