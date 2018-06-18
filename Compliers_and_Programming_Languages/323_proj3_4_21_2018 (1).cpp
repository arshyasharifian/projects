#include <iostream>
#include <string>
#include <fstream>
#include <iomanip>
using namespace std;

int line_counter = 1;

//struct for holding LA record
struct LA_output{
    string token;   //category
    string lexeme;  //actual
};

//functions to write to file
template <typename T>
void writetofile(ofstream& ofs, T& x, string token, LA_output & val) {
    val.lexeme = x;
    val.token = token;
    ofs << setw(10) << x << "\t\t\t" << token << endl;
}
template <typename T>
void writetofile(T* x, string token, LA_output & val) {
    val.lexeme = x;
    val.token = token;
    writetofile(*x, token);
}

void SA_output(ofstream & ofs, string production){
    ofs << production;
}

void SA_output_error(ofstream & ofs, string lexeme, int line_counter){
    ofs << "ERROR: line " << line_counter << "\t" << lexeme << endl;
    
}

///////////////////////////////////////////////////////////////////////////////////
void m_id_key(ofstream& ofs, char *&p, string * keyword, LA_output & val) {
    //since starting state is 1 and entering this function requires a letter input
    int state = 2;
    bool found_keyword = false;
    string word = "";
    /*
     A finite state machine for identifiers and keywords;
     will continue checking character inputs as long as
     they are valid inputs (letter, digit, $).
     Accepting states 2, 3, 5
     */
    int id_FSM[6][4]{
        //state   L   D   $
        1,        2,  6,  6,
        2,        3,  4,  5,
        3,        3,  4,  5,
        4,        3,  4,  5,
        5,        6,  6,  6,
        6,        6,  6,  6
        
    };
    //while valid input
    while (((isalpha(*p)) || (isdigit(*p))) || (*p == '$')) {
        if (isalpha(*p))
            state = id_FSM[state - 1][1];
        else if (isdigit(*p))
            state = id_FSM[state - 1][2];
        else
            state = id_FSM[state - 1][3];
        word += *p;
        ++p;
    }
    //if accepting state
    if (((state == 2) || (state == 3)) || (state == 5)) {
        //check if keyword
        for (int i = 0; i < 13; i++) {
            if (word == keyword[i]) {
                found_keyword = true;
                writetofile(ofs, word, "keyword", val);
            }
        }
        //otherwise its an identifier
        if (!found_keyword)
            writetofile(ofs, word, "identifier", val);
    }
    //not accepting state
    else
        writetofile(ofs, word, "unknown", val);
}
//determines whether the input characters are integers or real numbers
void m_int_real(ofstream& ofs, char*&p, LA_output & val) {
    int state = 2; //set to 2 since input char was a digit
    /*
     combination of integer and real number FSM
     states 2 is accepting states for integer numbers
     state 4 is the accepting state for real numbers
     state 5 is an error state.
     */
    int int_real_FSM[5][3]{
        //state     D   .
        1,          2,  5,
        2,          2,  3,
        3,          4,  5,
        4,          4,  5,
        5,          5,  5,
    };
    string word = "";
    //while valid input
    while (isdigit(*p) || (*p) == '.') {
        if (isdigit(*p))
            state = int_real_FSM[state - 1][1];
        else
            state = int_real_FSM[state - 1][2];
        word += *p;
        ++p;
    }
    if (state == 2)
        writetofile(ofs, word, "integer", val);
    else if (state == 4)
        writetofile(ofs, word, "real", val);
    else
        writetofile(ofs, word, "unknown", val);
}

//determines if the character is a separator
bool is_sep(ofstream& ofs, char *&p, char * sep, LA_output & val) {
    for (int i = 0; i < 10; i++){
        if (*p == sep[i]){
            if (*p == '%'){
                //if next char is also %
                if (*(++p) == '%'){
                    writetofile(ofs, "%%","separator", val);
                    ++p;
                    return true;
                }
                else{
                    //decrement to original p for error message
                    --p;
                    char tmp = p[0];
                    writetofile(ofs, tmp,"unknown", val);
                    ++p;
                    return true;
                }
            }
            else{
                char tmp = p[0];
                writetofile(ofs, tmp,"separator", val);
                ++p;
                return true;
            }
        }
    }
    return false;
}
//determines if the current character and next char are operators
bool is_op(ofstream& ofs, char *&p, char * ope, LA_output & val) {
    string two_char_operator [4] = {"==", "=>", "=<", "^="};
    
    string word_ope = "";
    for (int i = 0; i < 8; i++) {
        if (*p == ope[i]) {
            word_ope += *p;
            ++p;
            //second loop to check if next char is part of operator
            for (int k = 0; k < 8; k++) {
                if (*p == ope[k]) {
                    word_ope += *p;
                    for (int j = 0; j < 4; j++){
                        if (word_ope == two_char_operator[j]){
                            writetofile(ofs, word_ope, "operator", val);
                            //cout << word_ope << " is valid" << endl;
                            ++p;
                            return true;
                        }//end if two_char operator
                    }//end for checking valid two_char operator
                    //if second char isn't a valid operator
                    ++p;
                    writetofile(ofs, word_ope, "unknown", val);
                    //cout << word_ope << " is not valid" << endl;
                    return true;
                }//end if second char is an operator
            }//end for k
            writetofile(ofs, word_ope, "operator", val);
            //cout << word_ope << " is valid" << endl;
            return true;
        }//end if first char is an operator
    }
    //cout << word_ope << " is not valid" << endl;
    return false;
}


//checks for comments
bool is_comment(char *&p, fstream & fin, string & x){
    if (*p == '!'){
        ++p;
        while (*p != '!')
        {
            if (p != &x[x.length()])
                ++p;
            else
            {
                getline(fin, x);
                p = &x[0];
                line_counter++;
            }
        }
        ++p;
        return true;
    }
    return false;
}

//the lexer function considers all valid inputs
LA_output lexer(char *& p, ofstream & outfile, fstream & fin, string & x){
    //valid keywords, separators, and operators
    string keywords[13] = { "int","boolean","real","function","return","true",
        "false","if","else","endif","while","get","put" };
    char separators[10] = { '(',')',':',',','{','}', ']', '[', ';', '%' };
    char operators[8] = { '<','>','=','^','+','-','*','/' };
    
    LA_output ex;
    if (isalpha(*p))m_id_key(outfile, p, keywords, ex);
    else if (isdigit(*p))m_int_real(outfile, p, ex);
    else if (is_op(outfile, p, operators, ex)) {}
    else if (is_sep(outfile, p, separators, ex)) {}
    else if ((int)*p <= 32){
        if ((int)(*p) == 13) ex.token = "return carridge";
        else if ((int)*p <= 0) ex.token = "error";
        else {
            //cout << (int)(*p) << endl;
            ex.token = "undefined space";
        }
        ++p;
    }
    else if (*p =='!'){
        ++p;
        while (*p != '!')
        {
            if (p != &x[x.length()])
                ++p;
            else
            {
                getline(fin, x);
                line_counter++;
                p = &x[0];
            }
        }
        ex.lexeme = "";
        ++p;
    }
    else {
        ++p;
        ex.token = "unknown";
    }
    return ex;
}


//super header
void SA(char *&p, ofstream & outfile, fstream & fin, LA_output & record, string & line_code);
void new_line(char *&p, ofstream & outfile, fstream & fin, LA_output & record, string & line_code);
void get_token(char *&p, ofstream & outfile, fstream & fin, LA_output & record, string & line_code);


void error(char *&p, ofstream & outfile, fstream & fin, LA_output & record, string & line_code){
    SA_output_error(outfile, record.lexeme, line_counter-1);
    get_token(p, outfile, fin, record, line_code);
    SA(p, outfile, fin, record, line_code);
}

//the end of the line doesn't mean the end of SA
void get_token(char *&p, ofstream & outfile, fstream & fin, LA_output & record, string & line_code){
    
    record = lexer (p, outfile, fin, line_code);
    bool prob = true;
    
    while (prob){
        if (record.token == "unknown"){
            if (getline(fin, line_code)){
                p = &line_code[0];
                line_counter++;
            }
            else{
                //cout << "empty file" << endl;
                outfile.close();
                fin.close();
                exit(0);
            }
            record = lexer(p, outfile, fin, line_code);
            
        }
        else if((record.token == "return carridge") || (record.token == "error")){
            new_line(p, outfile, fin, record, line_code);
            record = lexer(p, outfile, fin, line_code);
        }
        else if (record.lexeme.length() == 0){
            record = lexer(p, outfile, fin, line_code);
        }
        else{
            prob = false;
        }
        
    }
}

//when we run into an error, we can call new_line
void new_line(char *&p, ofstream & outfile, fstream & fin, LA_output & record, string & line_code){
    if (getline(fin, line_code)){
        p = &line_code[0];
        line_counter++;
    }
    else {
        //cout << "empty file" << endl;
        outfile.close();
        fin.close();
        exit(0);
    }
}

//function headers
void functiondefinitions(char *&p, ofstream & outfile, fstream & fin, LA_output & record, string & line_code);
void statementlistP(char *&p, ofstream & outfile, fstream & fin, LA_output & record, string & line_code);
void declarationlist(char *&p, ofstream & outfile, fstream & fin, LA_output & record, string & line_code);
void parameterlist(char *&p, ofstream & outfile, fstream & fin, LA_output & record, string & line_code);
void term (char *&p, ofstream & outfile, fstream & fin, LA_output & record, string & line_code);
void expression (char *&p, ofstream & outfile, fstream & fin, LA_output & record, string & line_code);
void qualifier (char *&p, ofstream & outfile, fstream & fin, LA_output & record, string & line_code);
void statementlist(char *&p, ofstream & outfile, fstream & fin, LA_output & record, string & line_code);
void statement(char *&p, ofstream & outfile, fstream & fin, LA_output & record, string & line_code);
void ids(char *&p, ofstream & outfile, fstream & fin, LA_output & record, string & line_code);

//r29
void empty(char *&p, ofstream & outfile, fstream & fin, LA_output & record, string & line_code){
    SA_output(outfile, "<Empty>\n");
}

//r28p
void primaryP(char *&p, ofstream & outfile, fstream & fin, LA_output & record, string & line_code){
    SA_output(outfile, "<Primary Prime> -> (<Ids>) | Empty\n");

    if (record.lexeme == "("){
        get_token(p, outfile, fin, record, line_code);
        ids(p, outfile, fin, record, line_code);
        if (record.lexeme == ")"){
            get_token(p, outfile, fin, record, line_code);
        }
        else{
            error(p, outfile, fin, record, line_code);
        }
    }
    else{
        SA_output (outfile, "<Primary Prime> -> <Empty>\n");
    }
}

//r28
//problem with <identifier>:
void primary(char *&p, ofstream & outfile, fstream & fin, LA_output & record, string & line_code){
    SA_output (outfile, "<Primary> ->  <Identifier> <Primary Prime> | <Integer> | (<Expression>) | <Real> | true | false\n");

    //automatically called
    if (record.token == "identifier"){
        get_token(p, outfile, fin, record, line_code);
        primaryP(p, outfile, fin, record, line_code);
    }
    else if (record.token == "integer"){
        get_token(p, outfile, fin, record, line_code);
        SA_output(outfile, "<Primary> -> <integer>\n");
    }
    else if (record.lexeme == "("){
        get_token(p, outfile, fin, record, line_code);
        SA_output(outfile, "<Primary> -> (<Expression>)\n");
        expression(p, outfile, fin, record, line_code);
        if (record.lexeme == ")"){
            get_token(p, outfile, fin, record, line_code);
            //SA(p, outfile, fin, record, line_code);
        }
        else{
            error(p, outfile, fin, record, line_code);
        }
    }
    else if (record.token == "Real"){
        get_token(p, outfile, fin, record, line_code);
        SA_output(outfile, "<Primary> -> <real>\n");
    }
    else if (record.lexeme == "true"){
        get_token(p, outfile, fin, record, line_code);
        SA_output(outfile, "<Primary> -> true\n");
    }
    else if (record.lexeme == "false"){
        get_token(p, outfile, fin, record, line_code);
        SA_output(outfile, "<Primary> -> false\n");

    }
    else{
        error(p, outfile, fin, record, line_code);
    }
}

//r27
void factor (char *&p, ofstream & outfile, fstream & fin, LA_output & record, string & line_code){
    SA_output (outfile, "<Factor> -> -<Primary> | <Primary>\n");
    //automatically called
    
    if (record.lexeme == "-"){
        
        //gen_instr(SUB, nil);

        
        get_token(p, outfile, fin, record, line_code);
        SA_output (outfile, "<Factor> -> - <Primary>\n");
        primary(p, outfile, fin, record, line_code);
        
    }
    else {
        SA_output (outfile, "<Factor> -> <Primary>\n");
        primary(p, outfile, fin, record, line_code);
    }
}


//r26 prime
void termP (char *&p, ofstream & outfile, fstream & fin, LA_output & record, string & line_code){
    SA_output (outfile, "<Term Prime> -> *<Factor> <Term Prime> | /<Factor> <Term Prime> |<Empty>\n");
    //automatically called
    if (record.lexeme == "*"){
        get_token(p, outfile, fin, record, line_code);
        SA_output (outfile, "<Term Prime> -> * <Factor> <Term Prime>\n");
        factor(p, outfile, fin, record, line_code);
        //gen_instr(MUL, nil);
        termP(p, outfile, fin, record, line_code);
    }
    else if (record.lexeme == "/"){
        get_token(p, outfile, fin, record, line_code);
        SA_output (outfile, "<Term Prime> -> / <Factor> <Term Prime>\n");
        factor (p, outfile, fin, record, line_code);
        //gen_instr(DIV, nil);
        termP(p, outfile, fin, record, line_code);
    }
    else{
        SA_output (outfile, "<Term Prime> -> <Empty>\n");
        
    }
}

//r26
void term (char *&p, ofstream & outfile, fstream & fin, LA_output & record, string & line_code){
    SA_output (outfile, "<Term> -> <Factor> <Term Prime>\n");
    factor(p, outfile, fin, record, line_code);
    termP(p, outfile, fin, record, line_code);
}


//r25 prime
void expressionP(char *&p, ofstream & outfile, fstream & fin, LA_output & record, string & line_code){
    SA_output (outfile, "<Expression Prime> -> + <Term> <Expression Prime> | - <Term> <Expression Prime> | Empty\n");
    if ((record.lexeme == "+") || (record.lexeme == "-")){
        get_token(p, outfile, fin, record, line_code);
        term(p, outfile, fin, record, line_code);
        if (record.lexeme == "+") {
           //gen_instr (ADD, nil);
        }
        else{
            //gen_instr (SUB, nil);
        }
        expressionP(p, outfile, fin, record, line_code);
    }
    else{
        SA_output (outfile, "<Expression Prime> -> <Empty>\n");
    }
}

//r25
void expression (char *&p, ofstream & outfile, fstream & fin, LA_output & record, string & line_code){
    SA_output(outfile, "<Expression> -> <Term> <Expression Prime>\n");
    term(p, outfile, fin, record, line_code);
    expressionP(p, outfile, fin, record, line_code);
}

//r24
//this is the R in the C -> ERE
void relop (char *&p, ofstream & outfile, fstream & fin, LA_output & record, string & line_code){
    SA_output(outfile, "<Relop> -> == | ^= | > | < | => | =< \n");
    
    //save the operator (>, <, etc.) to be used for instruct table later
    //op = token
    
    if ((record.lexeme == "==") || (record.lexeme == "^=")){
        if (record.lexeme == "=="){
            //gen_instr(EQU, nil);
        }
        else{
            //?
        }
        get_token(p, outfile, fin, record, line_code);
    }
    else if ((record.lexeme == ">") || (record.lexeme == "<")){
        if (record.lexeme == ">"){
            //gen_instr (LES, nil);
        }
        else{
            //gen_instr(GRT, nil);
        }
        //suggests a loop will occur
        //push_jumpstack(intr_address);
        //gen_instr(JUMPZ, nil);
        get_token(p, outfile, fin, record, line_code);
    }
    else if ((record.lexeme == "=>") || (record.lexeme == "=<")){
        if (record.lexeme == "=>"){
            //?
            //check if equal first
            //else if, check greater
            //else 0
        }
        else{
            //?
            //check if equal first
            //else if, check less than
            //else 0
        }
        get_token(p, outfile, fin, record, line_code);
    }
    else{
        error (p, outfile, fin, record, line_code);
    }
    
    

}

//r23
void condition (char *&p, ofstream & outfile, fstream & fin, LA_output & record, string & line_code){
    SA_output(outfile, "<Condition> -> <Expression> <Relop> <Expression>\n");
    expression(p, outfile, fin, record, line_code);
    relop(p, outfile, fin, record, line_code);
    
    expression(p, outfile, fin, record, line_code);
    
    
    /*
     if (op == "==){
        //gen_instr(EQU, nil);
     }
     else if (op == "^="){
        //I think this is not equals
        //gen_instr(NEQ, nil);
     }
     else if (op == ">"){
        ////gen_instr(GRT, nil);
        //suggests a loop will occur
        //push_jumpstack(intr_address);
        //gen_instr(JUMPZ, nil);
     }
     else if (op == "<"){
        //gen_instr (LES, nil);
        //suggests a loop will occur
        //push_jumpstack(intr_address);
        //gen_instr(JUMPZ, nil);
     }
     else if (op == "=>"){
        
        //gen_instr (GEQ, nil);
     
        //suggests a loop will occur
        //push_jumpstack(intr_address);
        //gen_instr(JUMPZ, nil);
     }
     else if (op == "=<"){
        //gen_instr (LEQ, nil);
     
        //suggests a loop will occur
        //push_jumpstack(intr_address);
        //gen_instr(JUMPZ, nil);
    }
     */
}

//r22
void while_r(char *&p, ofstream & outfile, fstream & fin, LA_output & record, string & line_code){
    SA_output(outfile, "<While> -> while (<Condition>) <Statement>\n");
    
    
    if (record.lexeme == "while"){
        //(global) addr = (global) instr_address
        /*
         once we are in the while loop 
         we use label to describe the beginning of a loop
         gen_instr("LABEL", nil); //gen_instr is a function
         */
        
        get_token(p, outfile, fin, record, line_code);
        if (record.lexeme == "("){
            get_token(p, outfile, fin, record, line_code);
            
            //this is the C(); function
            condition(p, outfile, fin, record, line_code);
            if (record.lexeme == ")"){
                get_token(p, outfile, fin, record, line_code);
                statement(p, outfile, fin, record, line_code);
                //gen_instr(JUMP, addr);
                //back_patch(inst_address); //function
            }
            else{
                error(p, outfile, fin, record, line_code);
            }
        }
        else{
            error(p, outfile, fin, record, line_code);
        }
    }
}

//r21
void scan (char *&p, ofstream & outfile, fstream & fin, LA_output & record, string & line_code){
    SA_output(outfile, "<Scan> -> get (<Scan>);\n");
    //assume get
    if (record.lexeme == "get"){
        
        get_token(p, outfile, fin, record, line_code);
        if (record.lexeme == "("){
            get_token(p, outfile, fin, record, line_code);
            ids(p, outfile, fin, record, line_code);
            if(record.lexeme == ")"){
                get_token(p, outfile, fin, record, line_code);
                if (record.lexeme == ";"){
                    get_token(p, outfile, fin, record, line_code);
                    //? not sure if I should use empty after semicolons
                    empty(p, outfile, fin, record, line_code);
                }
                else{
                    error(p, outfile, fin, record, line_code);
                }
            }
            else{
                error(p, outfile, fin, record, line_code);
            }
        }
        else{
            error(p, outfile, fin, record, line_code);
        }
    }
    

}

//r20
void print(char *&p, ofstream & outfile, fstream & fin, LA_output & record, string & line_code){
    //assume token put
    SA_output(outfile, "<Print> -> put ( <Expression> );\n");
    
    if (record.lexeme == "put"){
        get_token(p, outfile, fin, record, line_code);
        if (record.lexeme == "("){
            get_token(p, outfile, fin, record, line_code);
            expression(p, outfile, fin, record, line_code);
            if (record.lexeme == ")"){
                get_token(p, outfile, fin, record, line_code);
                //OUTPUT THE EXPRESSION
                //get_instr(STDOUT, nil);
                
                if (record.lexeme == ";"){
                    get_token(p, outfile, fin, record, line_code);
                }
                else{
                    error(p, outfile, fin, record, line_code);
                }
            }
            else{
                error(p, outfile, fin, record, line_code);
            }
        }
        else{
            error(p, outfile, fin, record, line_code);
        }
    }
    
}

//r19p
void return_rp(char *&p, ofstream & outfile, fstream & fin, LA_output & record, string & line_code){
    SA_output(outfile, "<ReturnP> -> ;|<Expression>;\n");
    
    if (record.lexeme == ";"){
        get_token(p, outfile, fin, record, line_code);
    }
    else{
        expression(p, outfile, fin, record, line_code);
        if (record.lexeme == ";"){
            get_token(p, outfile, fin, record, line_code);
        }
        else{
            error(p, outfile, fin, record, line_code);
        }
    }

}

//r19
void return_r (char *&p, ofstream & outfile, fstream & fin, LA_output & record, string & line_code){
    SA_output(outfile, "<Return> -> return <ReturnP>\n");
    
    if (record.lexeme == "return"){
       get_token(p, outfile, fin, record, line_code);
       return_rp (p, outfile, fin, record, line_code);
    }
    else{
        error(p, outfile, fin, record, line_code);
    }
    
}

//r18p
void if_rp(char *&p, ofstream & outfile, fstream & fin, LA_output & record, string & line_code){
    SA_output(outfile, "<If Prime> -> endif | else <Statement> endif\n");
    
    if (record.lexeme == "endif"){
        get_token(p, outfile, fin, record, line_code);
    }
    else if (record.lexeme == "else"){
        get_token(p, outfile, fin, record, line_code);
        statement(p, outfile, fin, record, line_code);
        if (record.lexeme == "endif"){
            //back_patch (instr_address);
            get_token(p, outfile, fin, record, line_code);
        }
        else{
            error(p, outfile, fin, record, line_code);
        }
        
    }
    else{
        error(p, outfile, fin, record, line_code);
    }

}


//r18
void if_r (char *&p, ofstream & outfile, fstream & fin, LA_output & record, string & line_code){
    SA_output(outfile, "<If> -> if ( <Condition> ) <Statement> <ifP> \n");
    
    if (record.lexeme == "if"){
        //? NOT SURE WHY WE NEED THIS
        //? IS INSTR_ADDRESS(); A FUNCTION OR A PARAMETER?
        //for the back_patch line # i.e. when the if condition starts
        //addr = instr_address();
        get_token(p, outfile, fin, record, line_code);
        if (record.lexeme == "("){
            get_token(p, outfile, fin, record, line_code);
            condition(p, outfile, fin, record, line_code);
            if(record.lexeme == ")"){
                get_token(p, outfile, fin, record, line_code);
                statement(p, outfile, fin, record, line_code);
                
                if_rp(p, outfile, fin, record, line_code);
            }
            else{
                error(p, outfile, fin, record, line_code);
            }
        }
        else{
            error(p, outfile, fin, record, line_code);
        }

    }
}

//r17
void assign (char *&p, ofstream & outfile, fstream & fin, LA_output & record, string & line_code){
    SA_output(outfile, "<Assign> -> <Identifier> = <Expression>;\n");
    
    if (record.token == "identifier"){
        
        
        /*
         want to save the identifier to find the memory address
         which will be used to insert as an operand for the instruction
         table.
         save = record.token
         */
        get_token(p, outfile, fin, record, line_code);
        if (record.lexeme == "="){
            get_token(p, outfile, fin, record, line_code);
            if ((((((record.token == "identifier") || (record.token == "integer")) || (record.token == "real")) || (record.lexeme == "true")) || (record.lexeme == "false")) || (record.lexeme == "(")) {
                expression(p, outfile, fin, record, line_code);
                /*
                 pop the arithmetic operation results to left-side of equal sign
                 get_instr(POPM, get_address (save));
                 */
                if (record.lexeme == ";"){
                    get_token(p, outfile, fin, record, line_code);
                }
                else{
                    error(p, outfile, fin, record, line_code);
                }
            }
            else{
                error(p, outfile, fin, record, line_code);
            }
        }
        else {
            error(p, outfile, fin, record, line_code);
        }
    }
}

//r16
void compound (char *&p, ofstream & outfile, fstream & fin, LA_output & record, string & line_code){
    SA_output(outfile, "<Compound> -> { <Statement List> }\n");
    
    if (record.lexeme == "{"){
        get_token(p, outfile, fin, record, line_code);
        statementlist(p, outfile, fin, record, line_code);
        if (record.lexeme == "}"){
            get_token(p, outfile, fin, record, line_code);
        }
        else{
            error(p, outfile, fin, record, line_code);
        }
    }
    else{
        error(p, outfile, fin, record, line_code);
    }
    
}

//r15
void statement(char *&p, ofstream & outfile, fstream & fin, LA_output & record, string & line_code){
    SA_output(outfile, "<Statement> -> <Compound> | <Assign> | <If> | <Return> | <Print> | <Scan> | <While>\n");
    
    if (record.lexeme == "{"){
        compound (p, outfile, fin, record, line_code);
    }
    else if (record.token == "identifier"){
        assign(p, outfile, fin, record, line_code);
    }
    else if (record.lexeme == "if"){
        if_r(p, outfile, fin, record, line_code);
    }
    else if (record.lexeme == "return"){
        return_r(p, outfile, fin, record, line_code);
    }
    else if (record.lexeme == "put"){
        print(p, outfile, fin, record, line_code);
    }
    else if (record.lexeme == "get"){
        scan(p, outfile, fin, record, line_code);
    }
    else if (record.lexeme == "while"){
        while_r(p, outfile, fin, record, line_code);
    }
    else{
        error(p, outfile, fin, record, line_code);
    }
}

//r14p
void statementlistP(char *&p, ofstream & outfile, fstream & fin, LA_output & record, string & line_code){
    SA_output(outfile, "<Statement ListP> -><Empty>|<Statement List\n");

    while (((record.token == "identifier") || (record.token == "keyword")) || (record.lexeme == "{")){
        statement(p, outfile, fin, record, line_code);
    }
    
    //empty
    SA_output (outfile, "<Statement List Prime> -> <Empty>\n");
    
}

//r14
void statementlist(char *&p, ofstream & outfile, fstream & fin, LA_output & record, string & line_code){
    SA_output(outfile, "<Statement List> -><Statement><Statement ListP\n");
    statement(p, outfile, fin, record, line_code);
    statementlistP(p, outfile, fin, record, line_code);
}

//r13p
void idsP(char *&p, ofstream & outfile, fstream & fin, LA_output & record, string & line_code){
    SA_output(outfile, "<Ids Prime> ->  <Empty> |, <Ids>\n");
    if (record.lexeme == ","){
        get_token(p, outfile, fin, record, line_code);
        ids(p, outfile, fin, record, line_code);
    }
    else{
        //empty
        SA_output (outfile, "<Ids Prime> -> <Empty>\n");
    }
}

//r13
void ids(char *&p, ofstream & outfile, fstream & fin, LA_output & record, string & line_code){
    SA_output(outfile, "<IDs> ->  <identifier> <IDs Prime>\n");
    if (record.token == "identifier"){
        //SYMBOL TABLE:
        //check if identifier exists in symbol
        //if not, add identifier name to symbol table
        //AND add (current) memory address to symbol table
        //THEN incremement memory address by 1
        
        //STDIN
        //if identifier already exists
        //save = token
        //get_instr(STDIN, nil);
        //get_instr(POPM, get_address(get_address(save));
        
        //PROBLEM:
        //what if ids is called under expression so we get something like
        //a(a,b) that isn't expected to be STDIN?
        
        get_token(p, outfile, fin, record, line_code);
        idsP(p, outfile, fin, record, line_code);
        
    }
    else{
        error(p, outfile, fin, record, line_code);
    }
}

//r12
void declaration(char *&p, ofstream & outfile, fstream & fin, LA_output & record, string & line_code){
    SA_output(outfile, "<Declaration> ->  <Qualifier> <Ids>\n");
    qualifier(p, outfile, fin, record, line_code);
    ids(p, outfile, fin, record, line_code);
}

//r11p
void declarationlistP(char *&p, ofstream & outfile, fstream & fin, LA_output & record, string & line_code){
    SA_output(outfile, "<Declaration List Prime> ->  <Empty> | <Declaration List>\n");
    
    
    while (((record.token == "integer") || (record.token == "real")) || ((record.lexeme == "true") || (record.lexeme == "false"))){
        declarationlist(p, outfile, fin, record, line_code);
    }
    
    //empty
    SA_output (outfile, "<Declaration List Prime> -> <Empty>\n");
    

}

//r11
void declarationlist(char *&p, ofstream & outfile, fstream & fin, LA_output & record, string & line_code){
    SA_output(outfile, "<Declaration List> ->  <Declaration> ; <Declaration List Prime>\n");
    declaration(p, outfile, fin, record, line_code);
    if (record.lexeme == ";"){
        get_token(p, outfile, fin, record, line_code);
        declarationlistP(p, outfile, fin, record, line_code);
    }
    else{
        error(p, outfile, fin, record, line_code);
    }
}


//r10
void optDeclarationlist(char *&p, ofstream & outfile, fstream & fin, LA_output & record, string & line_code){
    SA_output(outfile, "<Opt Declaration List> ->  <Declaration List> | <Empty>\n");
    while (((record.lexeme == "int") || (record.lexeme == "real")) || (record.lexeme == "boolean")){
        declarationlist(p, outfile, fin, record, line_code);
    }
    
    //empty
    SA_output (outfile, "<Opt Declaration List> -> <Empty>\n");
    
}

//r9
void body (char *&p, ofstream & outfile, fstream & fin, LA_output & record, string & line_code){
    SA_output(outfile, "<Body> ->  { <Statement List> }\n");
    
    if (record.lexeme == "{"){
        get_token(p, outfile, fin, record, line_code);
        statementlist(p, outfile, fin, record, line_code);
        if (record.lexeme == "}"){
            get_token(p, outfile, fin, record, line_code);
        }
        else{
            error(p, outfile, fin, record, line_code);
        }
    }
    else{
        error(p, outfile, fin, record, line_code);
    }
}

//r8
void qualifier (char *&p, ofstream & outfile, fstream & fin, LA_output & record, string & line_code){
    SA_output(outfile, "<Qualifier> -> int | boolean | real\n");
    
    
    if (((record.lexeme == "int") || (record.lexeme == "real")) || (record.lexeme == "boolean")){
        //qualifier comes before identifier
        //save type (global variable) as this lexeme and use it for all of the IDs provided.
        //error checking: set global type to "NO TYPE" after done with all identifiers
        get_token(p, outfile, fin, record, line_code);
    }
    else{
        error(p, outfile, fin, record, line_code);
    }
}

//r7
void parameter (char *&p, ofstream & outfile, fstream & fin, LA_output & record, string & line_code){
    SA_output(outfile, "<Parameter> -> <Ids> : <Qualifier>\n");
    
    ids(p, outfile, fin, record, line_code);
    if (record.lexeme == ":"){
        get_token(p, outfile, fin, record, line_code);
        qualifier(p, outfile, fin, record, line_code);
    }
    else{
        error(p, outfile, fin, record, line_code);
    }
}

//r6p
void parameterlistP(char *&p, ofstream & outfile, fstream & fin, LA_output & record, string & line_code){
    SA_output(outfile, "<Parameter List Prime> -> <Empty> | , <Parameter List>\n");
    while (record.lexeme == ","){
        get_token(p, outfile, fin, record, line_code);
        parameterlist(p, outfile, fin, record, line_code);
    }
    
    //empty
    SA_output (outfile, "<Parameter List Prime> -> <Empty>\n");
    
}

//r6
void parameterlist(char *&p, ofstream & outfile, fstream & fin, LA_output & record, string & line_code){
    SA_output(outfile, "<Parameter List> -> <Parameter> <Parameter List Prime>\n");
    parameter(p, outfile, fin, record, line_code);
    parameterlistP(p, outfile, fin, record, line_code);

}

//r5
void optParameterlist (char *&p, ofstream & outfile, fstream & fin, LA_output & record, string & line_code){
    SA_output(outfile, "<Opt Parameter List> -> <Parameter List> | <Empty>\n");
    if (record.token == "identifier"){
        parameterlist(p, outfile, fin, record, line_code);
    }
    else{
        //empty
    }
}

//r4
//NOT CALLED
void function_r(char *&p, ofstream & outfile, fstream & fin, LA_output & record, string & line_code){
    SA_output(outfile, "<Function> -> function <Identifier> [ <Opt Parameter List> ] <Opt Declaration List> <Body>\n");
    
    if (record.lexeme == "function"){
        get_token(p, outfile, fin, record, line_code);
        if (record.token == "identifier"){
            get_token(p, outfile, fin, record, line_code);
            if (record.lexeme == "["){
                get_token(p, outfile, fin, record, line_code);
                optParameterlist(p, outfile, fin, record, line_code);
                if (record.lexeme == "]"){
                    get_token(p, outfile, fin, record, line_code);
                    optDeclarationlist(p, outfile, fin, record, line_code);
                    body(p, outfile, fin, record, line_code);
                }
                else{
                    error(p, outfile, fin, record, line_code);
                }
            }
            else{
                error(p, outfile, fin, record, line_code);
            }
        }
        else{
            error(p, outfile, fin, record, line_code);
        }
    }

    else{
        error(p, outfile, fin, record, line_code);
    }
  
}

//r3p
//NOT CALLED
void functiondefinitionsP(char *&p, ofstream & outfile, fstream & fin, LA_output & record, string & line_code){
    SA_output(outfile, "<Function Definitions Prime> -> <Empty> | <Function Definitions>\n");
    
    if (record.lexeme == "function"){
        functiondefinitions(p, outfile, fin, record, line_code);
    }
    else{
       //empty
        SA_output (outfile, "<Function Definitions Prime> -> <Empty>\n");
    }
}

//r3
//NOT CALLED
void functiondefinitions(char *&p, ofstream & outfile, fstream & fin, LA_output & record, string & line_code){
    SA_output(outfile, "<Function Definitions> -> <Function> <Function Definitions Prime>\n");
    function_r(p, outfile, fin, record, line_code);
    functiondefinitionsP(p, outfile, fin, record, line_code);
}

//r2
//NOT CALLED
void optFunctiondefinitions(char *&p, ofstream & outfile, fstream & fin, LA_output & record, string & line_code){
    SA_output(outfile, "<Opt Function Definitions> -> <Opt Function Definitions> | <Empty> \n");
   
    
    if (record.lexeme == "function"){
        functiondefinitions(p, outfile, fin, record, line_code);
    }
    else{
        //empty
        SA_output (outfile, "<Opt Function Definitions> -> <Empty>\n");
    }
}

//r1
//NOT CALLED
void rat18s (char *&p, ofstream & outfile, fstream & fin, LA_output & record, string & line_code){
    SA_output(outfile, "<rat18s> -><Opt Function Definitions > %% <Opt Declaration List> <Statement List>\n");
    
    optFunctiondefinitions(p, outfile, fin, record, line_code);
    if (record.lexeme == "%%"){
        get_token(p, outfile, fin, record, line_code);
        optDeclarationlist(p, outfile, fin, record, line_code);
        statementlist(p, outfile, fin, record, line_code);
    }
    else{
        error(p, outfile, fin, record, line_code);
    }

}

//The start of the syntax analyzation
void SA(char *&p, ofstream & outfile, fstream & fin, LA_output & record, string & line_code){
    new_line(p, outfile, fin, record, line_code);
    get_token(p, outfile, fin, record, line_code);
    if (record.lexeme == "function"){
        rat18s(p, outfile, fin, record, line_code);
    }
    else if (((record.lexeme == "int") || (record.lexeme == "real")) || (record.lexeme == "boolean")){
        optDeclarationlist(p, outfile, fin, record, line_code);
    }
    else if ((record.token == "identifier") || (record.token == "keyword")){
        statement(p, outfile, fin, record, line_code);
    }
    else{
        empty(p, outfile, fin, record, line_code);
    }
}
/////////////////////////////////////////////////////////////////

int main() {
    
    //read file
    fstream fin;
    
    //candidate lexeme
    char * lex_candid;
    
    //holds each input file line
    string line_code;
    string file_name;
    
    //structure containing lexer function results
    LA_output record;
    
    //prompt user to enter file name
    //cout << "What is the name of the txt file you want to test:  " << endl;
    //cin >> file_name;
    file_name = "vt3.txt";
    fin.open(file_name);
    
    //while unable to open file prompt user to re-enter file name
    while (!(fin.is_open())){
        cout << "Unable to open file; please re-enter file name" << endl;
        cin >> file_name;
        fin.open(file_name);
    }
    
    //creating and formatting output file
    ofstream outfile("output.txt");
    outfile << file_name << endl;
    outfile << setw(10)<<"token"<<"\t\t\t" << "lexeme" <<endl;
    outfile << "--------------------------------------------------------------\n";
    
    
    
    SA(lex_candid, outfile, fin, record, line_code);
    
    
    
    fin.close();
    outfile.close();
    return 0;
}

