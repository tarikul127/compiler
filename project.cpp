#include <bits/stdc++.h>
using namespace std;

// ----------------- Structures -----------------
enum TokenType {KEYWORD, IDENTIFIER, NUMBER, OPERATOR, DELIMITER, HEADER, STRING_LITERAL, UNKNOWN};

struct Token {
    int id;
    string lexeme;
    TokenType type;
};

// ----------------- Globals -----------------
vector<Token> tokens;
map<string, int> symbolTable;
vector<string> tac;
vector<string> assembly;

// ----------------- Keywords, Operators, Delimiters -----------------
vector<string> keywords = {"int","float","return","printf","main"};
vector<string> operators = {"+","-","*","/","="};
vector<string> delimiters = {";","(",")","{","}",","};

// ----------------- Utility Functions -----------------
bool isKeyword(string s){return find(keywords.begin(),keywords.end(),s)!=keywords.end();}
bool isOperator(string s){return find(operators.begin(),operators.end(),s)!=operators.end();}
bool isDelimiter(string s){return find(delimiters.begin(),delimiters.end(),s)!=delimiters.end();}

// ----------------- 1. COMMENT REMOVAL -----------------
string removeComments(string code){
    string result;
    bool block=false;
    for(size_t i=0;i<code.size();i++){
        if(!block && i+1<code.size() && code[i]=='/' && code[i+1]=='/') {
            while(i<code.size() && code[i]!='\n') i++;
        }
        else if(!block && i+1<code.size() && code[i]=='/' && code[i+1]=='*') {
            block=true; i++;
        }
        else if(block && i+1<code.size() && code[i]=='*' && code[i+1]=='/') {block=false; i++;}
        else if(!block) result+=code[i];
    }
    return result;
}

// ----------------- 2. LEXICAL ANALYSIS -----------------
void tokenize(string code){
    int idCounter=1;
    for(size_t i=0;i<code.size();i++){
        if(isspace(code[i])) continue;

        // HEADER
        if(code[i]=='#'){
            string header;
            while(i<code.size() && code[i]!='\n'){header+=code[i]; i++;}
            tokens.push_back({idCounter++,header,HEADER});
        }
        // STRING LITERAL
        else if(code[i]=='"'){
            string str="\""; i++;
            while(i<code.size() && code[i]!='"'){str+=code[i]; i++;}
            str+="\""; tokens.push_back({idCounter++,str,STRING_LITERAL});
        }
        // IDENTIFIER or KEYWORD
        else if(isalpha(code[i]) || code[i]=='_'){
            string word;
            while(i<code.size() && (isalnum(code[i])||code[i]=='_')){word+=code[i]; i++;}
            i--;
            TokenType type = isKeyword(word)?KEYWORD:IDENTIFIER;
            tokens.push_back({idCounter++,word,type});
        }
        // NUMBER
        else if(isdigit(code[i])){
            string num;
            while(i<code.size() && isdigit(code[i])){num+=code[i]; i++;}
            i--;
            tokens.push_back({idCounter++,num,NUMBER});
        }
        // OPERATOR
        else if(isOperator(string(1,code[i]))){
            string op(1,code[i]);
            if(i+1<code.size() && isOperator(op+code[i+1])){op+=code[i+1]; i++;}
            tokens.push_back({idCounter++,op,OPERATOR});
        }
        // DELIMITER
        else if(isDelimiter(string(1,code[i]))){
            tokens.push_back({idCounter++,string(1,code[i]),DELIMITER});
        }
        else{
            tokens.push_back({idCounter++,string(1,code[i]),UNKNOWN});
        }
    }
}

// ----------------- 3. SYMBOL TABLE -----------------
void buildSymbolTable(){
    for(auto t:tokens){
        if(t.type==IDENTIFIER || t.type==NUMBER || t.type==STRING_LITERAL || t.type==HEADER){
            symbolTable[t.lexeme]=t.id;
        }
    }
}

// ----------------- 4. SIMPLE SYNTAX CHECK -----------------
bool syntaxValid = true;
void checkSyntax(){
    for(size_t i=0;i<tokens.size();i++){
        if(tokens[i].type==IDENTIFIER && i+1<tokens.size() && tokens[i+1].lexeme!="=" && tokens[i+1].lexeme!=";" && tokens[i+1].lexeme!=","){
            cout<<"Syntax Error near token: "<<tokens[i].lexeme<<endl;
            syntaxValid=false;
        }
    }
}

// ----------------- 5. LL(1) PARSING TRACE -----------------
void showParsingTrace(){
    cout<<"\nLL(1) PARSING TRACE\n------------------------------\n";
    stack<string> st;
    st.push("$");
    st.push("Program");
    size_t index=0;
    while(!st.empty() && index<tokens.size()){
        string top = st.top(); st.pop();
        string current = tokens[index].lexeme;
        cout<<"Stack top: "<<top<<" | Input: "<<current<<endl;

        if(top=="Program") st.push("StmtList");
        else if(top=="StmtList"){
            if(tokens[index].type==KEYWORD || tokens[index].type==IDENTIFIER || tokens[index].lexeme=="printf")
                st.push("Stmt");
        }
        else if(top=="Stmt") st.push("Assign");
        else if(top=="Assign" && tokens[index].type==IDENTIFIER){
            index+=3; // skip simple assignment: id = value ;
        }
        else if(top=="$") break;
    }
}

// ----------------- 6. TAC & 7. ASSEMBLY -----------------
void generateTACandAssembly(){
    map<string,string> tempMap;
    int t=1;
    for(size_t i=0;i<tokens.size();i++){
        if(tokens[i].type==IDENTIFIER && i+2<tokens.size() && tokens[i+1].lexeme=="="){
            string lhs = tokens[i].lexeme;
            string rhs = tokens[i+2].lexeme;
            // Check for addition
            if(i+3<tokens.size() && tokens[i+3].lexeme=="+"){
                string rhs2 = tokens[i+4].lexeme;
                string tvar = "t"+to_string(t++);
                tac.push_back(tvar+" = "+rhs+" + "+rhs2);
                tac.push_back(lhs+" = "+tvar);

                assembly.push_back("MOV R0, "+rhs);
                assembly.push_back("ADD R0, "+rhs2);
                assembly.push_back("MOV "+tvar+", R0");
                assembly.push_back("MOV "+lhs+", "+tvar);
                i+=4;
            } else {
                tac.push_back(lhs+" = "+rhs);
                assembly.push_back("MOV "+lhs+", "+rhs);
                i+=2;
            }
        }
    }
}

// ----------------- MAIN -----------------
int main(){
    cout<<"=====================================\n";
    cout<<"           COMPILER STARTED\n";
    cout<<"=====================================\n\n";

    cout<<"[1] COMMENT REMOVAL PHASE\n\n";

    ifstream fin("input.txt");
    if(!fin){cerr<<"input.txt not found\n"; return 0;}
    stringstream ss; ss<<fin.rdbuf();
    string code = ss.str();

    // comment remove
    string cleanCode = removeComments(code);

    cout<<"----- Output After Comment Removal -----\n\n";
    cout<<cleanCode<<"\n";

    cout<<"Comment removal successful.\n";
    cout<<"Clean source shown above (no output file created).\n";

    cout<<"\n[2] LEXICAL ANALYSIS PHASE\n\n";
    tokenize(cleanCode);

    cout<<"TOKEN TABLE (LEXER OUTPUT)\n-------------------------------------------------\n";
    cout<<"TOK_ID\tLEX_ID\tTYPE\t\tLEXEME\n-------------------------------------------------\n";
    for(auto t:tokens){
        cout<<t.id<<"\t"<<t.id<<"\t";
        switch(t.type){
            case KEYWORD: cout<<"KEYWORD\t\t"; break;
            case IDENTIFIER: cout<<"IDENTIFIER\t"; break;
            case NUMBER: cout<<"NUMBER\t\t"; break;
            case OPERATOR: cout<<"OPERATOR\t"; break;
            case DELIMITER: cout<<"DELIMITER\t"; break;
            case HEADER: cout<<"HEADER\t\t"; break;
            case STRING_LITERAL: cout<<"STRING_LITERAL\t"; break;
            default: cout<<"UNKNOWN\t\t"; break;
        }
        cout<<t.lexeme<<"\n";
    }

    buildSymbolTable();
    cout<<"\nSYMBOL TABLE\n-------------------------\n";
    cout<<"ID\tLEXEME\n-------------------------\n";
    for(auto s:symbolTable) cout<<s.second<<"\t"<<s.first<<"\n";

    syntaxValid=true;
    checkSyntax();

    cout<<"\n[3] SYNTAX ANALYSIS PHASE\n";
    if(syntaxValid) cout<<"Syntax analysis successful.\n";
    else cout<<"Syntax errors detected.\n";

    showParsingTrace();

    cout<<"\nTHREE ADDRESS CODE (TAC)\n-----------------------------\n";
    generateTACandAssembly();
    for(auto t:tac) cout<<t<<"\n";

    cout<<"\nFINAL ASSEMBLY CODE\n-----------------------------\n";
    for(auto a:assembly) cout<<a<<"\n";

    cout<<"\n=====================================\n";
    cout<<"        COMPILATION COMPLETED\n";
    cout<<"=====================================\n";

    return 0;
}
