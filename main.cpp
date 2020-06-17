//Analizor lexical pentru un subset al limbajului C++

#include <bits/stdc++.h>

using namespace std;

ofstream fout("output.txt");

inline bool valid_first_ch_for_identifier(char c)//pentru nume de variabile
{
    if(c>='a' && c<='z')    return true;
    if(c>='A' && c<='Z')    return true;
    if(c>='0' && c<='9')    return false;//sa nu inceapa cu o cifra
    if(c=='_')    return true;
    return false;
}



inline bool is_operator(char c)
{
    ///momentan doar pentru operatorii care ocupa un singur caracter(fara ++, +=, etc)
    char op[] = {'.','+',    '-',    '*',    '/',    '%',  '>',    '<',   '!','~',    '&',    '^',    '|','='};
    for(int i=0;i<14;++i)
        if(op[i]==c)
            return true;
    return false;
}

bool is_keyword(string c)
{
    string kw[]={"auto",     "break",        "case",     "char",     "const",        "continue",     "default",      "do",
        "double",   "else",         "enum",     "extern",   "float",        "for",          "goto",         "if",
        "int",      "long",         "register", "return",   "short",        "signed",       "sizeof",       "static",
        "struct",   "switch",       "typedef",  "union",    "unsigned",     "void",         "volatile",     "while"};
    for(int i=0;i<32;++i)
        if(kw[i]==c)
            return true;
    return false;
}

bool is_Punctuator(char c)
{
    char pct[] = {',',':',';','(',')','{','}','[',']','#'};
    for(int i=0;i<10;++i)
        if(pct[i]==c)
            return true;
    return false;
}

enum TokenType
{
    None,
    Error,
    Identifier,
    Operator,
    Keyword,
    Numeric_Literal,
    String_Literal,
    Comment,
    Punctuator,

};

class Token
{
public:
    string type;
    string token;
public:
    Token(TokenType type, string value)
    {
        this->type = type;
        this->token="";
    }
    Token()
    {
        this->type = None;
        this->token = "";
    }
};

class State
{
public:
    string name;
    bool finall;
    bool skip;//daca e stare finala iar skip=true, nu mai afisez tokenul format(comentarii, spatii )
    bool dead_end;
    TokenType token_type;
public:

    State(string name, bool finall, bool dead_end, TokenType token_type)
    {
        this->name = name;
        this->finall = finall;
        this->dead_end = dead_end;
        this->token_type = token_type;
    }

    State()
    {
        cout<<"apelare";
    }
};

//De pus intr-o lista pentru versiunea 2
//void DeclareStates()
//{
///Comments
State *s_start = new State("start", false, false, None);//starea initiala
State *s_comm_singleline = new State("comm_singleline", true, false, None);

    //start of a multiline comment
State *s_comm_multiline = new State("comm_multi", false, false, None);
State *s_comm_multilin_maybeEnd = new State("comm_multi_maybe_end", false, false, None);
State *s_comm_multilin_end = new State("end_comm_multi", true, false, None);

State *s_slash = new State("slash", false, false, None);

///Words
State *s_word = new State("word", true, false, None);

///Punctuator
State *s_punctuator = new State("punctuator", true, false,None);

///Operator
State *s_operator = new State("operator", true, false,None);

///String
State *s_string = new State("string", false, false,None);
State *s_string_escape = new State("escape", false, false,None);//cand intalnesc backslash
State *s_string_end = new State("string_end", true, false,None);//cand intalnesc " din nou, dar nu imediat dupa backslash

///Number
State *s_integer = new State("integer", true, false,None);//inca nu am intalnit virgula
State *s_float = new State("float", true, false,None);//din starea asta nu mai pot intalni alta virgula, am gasit deja una
State *s_float_err = new State("float_err", false, false,None);//float cu eroare(exp: 3.141.001)


class Transition
{
public:
    State *source;
    State *destination;
public:
    Transition(State *src, State *dst)
    {
        source = src;
        destination = dst;
    }

    bool condition(char c)
    {
        /*cout<<"\tCondition\n\n";
        cout<<"\tnumele sunt:\n";
        cout<<"\tsrc = "<<source->name<<"\n";
        cout<<"\tdst = "<<destination->name<<"\n\n";
        return true;*/

        ///Conditiile pentru tranzitii

        if(source->name=="start")
        {
            if(c=='/')//merg in starea "slash"
                return (destination->name=="slash");
            if(valid_first_ch_for_identifier(c))//identificator sau keyword, deci word
                return (destination->name=="word");
            if(is_Punctuator(c))
                return (destination->name=="punctuator");
            if(is_operator(c))
                return (destination->name=="operator");
            if(c=='\"')
                return (destination->name=="string");
            if(c>='0' && c<='9')
                return (destination->name=="integer");
            if(c=='.')
                return (destination->name=="float");
        }
        if(source->name=="slash")
        {
            if(c=='*')//comentariu pe mai multe linii(abia inceput)
            {
                return (destination->name=="comm_multi");
            }
            if(c=='/')
            {
                return (destination->name=="comm_singleline");
            }
        }
        if(source->name=="comm_multi")
        {
            //doua cazuri: intalnesc o steluta sau intalnesc orice altceva
            if(c=='*')
            {
                return (destination->name=="comm_multi_maybe_end");
            }
            else//comentariul continua
                return (destination->name=="comm_multi");
        }
        if(source->name=="comm_multi_maybe_end")
        {
            //doua cazuri: intalnesc un slash sau orice altceva
            if(c=='/')
                return (destination->name=="end_comm_multi");
            return (destination->name=="comm_multi");
        }
        if(source->name=="end_comm_multi")
        {
            //nu trebuie sa existe tranzitie spre altceva
            return false;
        }
        if(source->name=="comm_singleline")
        {
            if(c=='\n')
                return false;//(destination->name=="comm_singleline_end");
            return (destination->name=="comm_singleline");
        }
        if(source->name=="word")
        {
            if(valid_first_ch_for_identifier(c) || (c>='0' && c<='9'))
            {
                return (destination->name=="word");
            }
        }
        if(source->name=="string")
        {
            if(c=='\\')//secventa escape
            {
                int x = 5/0;
                return (destination->name=="escape");
            }
            if(c=='\"')
                return (destination->name=="string_end");
            return (destination->name=="string");
        }
        if(source->name=="integer")
        {
            if(c>='0' && c<='9')
                return (destination->name=="integer");
            if(c=='.')
                return (destination->name=="float");
            //daca gasesc litere returnez false
        }
        if(source->name=="float")
        {
            if(c>='0' && c<='9')
                return (destination->name=="float");
            if(c=='.')
                return (destination->name=="float_err");
        }

        return false;
    }
};

vector<Transition*>transitions_list;

///start transitions
Transition *t_start_slash = new Transition(s_start, s_slash);

///comments
//multiline
Transition *t_slash_star = new Transition(s_slash, s_comm_multiline);
Transition *t_comm_inside = new Transition(s_comm_multiline, s_comm_multiline);
Transition *t_comm_maybeEnd = new Transition(s_comm_multiline, s_comm_multilin_maybeEnd);
Transition *t_maybeEnd_comm = new Transition(s_comm_multilin_maybeEnd, s_comm_multiline);
Transition *t_comm_multi_end = new Transition(s_comm_multilin_maybeEnd, s_comm_multilin_end);
//singleline
Transition *t_slash_slash = new Transition(s_slash, s_comm_singleline);
Transition *t_comsingle_comsingle = new Transition(s_comm_singleline, s_comm_singleline);

///Variabile si keyword-uri
Transition *t_start_word = new Transition(s_start, s_word);
Transition *t_word_word = new Transition(s_word, s_word);

///Punctuator
Transition *t_start_punctuator = new Transition(s_start, s_punctuator);

///Operator
Transition *t_start_operator = new Transition(s_start, s_operator);

///String literal
Transition *t_start_string = new Transition(s_start, s_string);
Transition *t_escape_string = new Transition(s_string, s_string);
Transition *t_string_string = new Transition(s_string, s_string);
Transition *t_string_end = new Transition(s_string, s_string_end);

///Number
Transition *t_start_integer = new Transition(s_start, s_integer);
Transition *t_start_float = new Transition(s_start, s_float);//este corect sa scriu x=.1;
Transition *t_integer_integer = new Transition(s_integer, s_integer);
Transition *t_integer_float = new Transition(s_integer, s_float);//este corect sa scriu x=5.;
Transition *t_float_float = new Transition(s_float, s_float);//este corect sa scriu x=5.;
Transition *t_float_float_err = new Transition(s_float, s_float_err);//este corect sa scriu x=5.;

void adauga_tranzitii()
{
    ///comments
    //multiline
    transitions_list.push_back(t_start_slash);
    transitions_list.push_back(t_slash_star);
    transitions_list.push_back(t_comm_inside);
    transitions_list.push_back(t_comm_maybeEnd);
    transitions_list.push_back(t_maybeEnd_comm);
    transitions_list.push_back(t_comm_multi_end);
    //singleline
    transitions_list.push_back(t_slash_slash);
    transitions_list.push_back(t_comsingle_comsingle);
    //variabile si keywords
    transitions_list.push_back(t_start_word);
    transitions_list.push_back(t_word_word);
    //punctuator
    transitions_list.push_back(t_start_punctuator);
    //operator
    transitions_list.push_back(t_start_operator);
    //string literal
    transitions_list.push_back(t_start_string);
    transitions_list.push_back(t_string_string);
    transitions_list.push_back(t_string_end);
    //numbers
    transitions_list.push_back(t_start_integer);
    transitions_list.push_back(t_start_float);
    transitions_list.push_back(t_integer_integer);
    transitions_list.push_back(t_integer_float);
    transitions_list.push_back(t_float_float);
    transitions_list.push_back(t_float_float_err);
}

class Lexer
{
private:
    int current_idx;
    char current_character;
    State *current_state;
    int line_nb;//
    string source_file;
    string token_curent;
public:

    Lexer(char* file_name)
    {
        current_state = new State();
        current_idx = 0;
        //in constructor fac citirea datelor
        std::ifstream t(file_name);
        std::string file((std::istreambuf_iterator<char>(t)),
                 std::istreambuf_iterator<char>());
        source_file = file;

    }

    string formeaza_token()
    {
        current_state = s_start;//inca nu am intalnit nimic

        string token = "";//initial am un token vid

        bool token_ended = false;
        while(source_file[current_idx]!=0 && !token_ended)//didn't reach the end of the token yet
        {
            current_character = source_file[current_idx];
            current_idx++;
            if((current_character==' ' || current_character=='\n') && token=="")
                continue;

            unsigned int i;
            bool moved = false;
            for(i=0;i<transitions_list.size();++i)
            {
                if(transitions_list[i]->source->name==current_state->name &&
                   transitions_list[i]->condition(current_character))
                {
                    ///am gasit tranzitia potrivita
                    token = token + current_character;
                    cout<<"trec de la "<<current_state->name<<" la "<<transitions_list[i]->destination->name<<"\n";
                    cout<<"\tc = "<<current_character<<"\n";
                    current_state = transitions_list[i]->destination;
                    moved = true;
                    break;//nu mai caut si prin celelalte tranzitii
                }
            }
            if(!moved)
            {
                token_ended = true;
                current_idx--;//ca sa procesez la pasul viitor caracterul neprocesat acum
                cout<<"m-am oprit. current state este "<<current_state->name<<" iar c="<<current_character<<"\n";
                cout<<"iar current state este "<<current_state->name<<"\n";
            }

        }
        if(!current_state->finall && token!="")
            cout<<"\n\nEroare: stare nefinala\n\n";
        cout<<"\t token returnat este: "<<token<<"\n";
        return token;
    }

    void Run()
    {
        cout<<"Sunt in functia Run. Am citit:\n\n"<<source_file;
        vector<string>string_table;
        while(true)
        {

            string new_token = formeaza_token();
            if (new_token == "")//end of file
            {
                fout<<"\nTabela de string-uri unice este:\n";
                for(unsigned int ui=0;ui<string_table.size();++ui)
                    fout<<string_table[ui]<<"\n";
                return;
            }
            Token tk;
            tk.token = new_token;

            if(current_state == s_string_end)
            {
                tk.type = "String Literal";
                string_table.push_back(new_token);
            }
            if(current_state == s_word && !is_keyword(tk.token))   tk.type = "Identifier";
            if(current_state == s_word && is_keyword(tk.token))   tk.type = "Keyword";
            if(current_state == s_punctuator)   tk.type = "Punctuator";
            if(current_state == s_operator)   tk.type = "Operator";
            if(current_state == s_float || current_state == s_integer)   tk.type = "Numeric Literal";

            if(current_state->finall==false)   tk.type = "Error";

            if(current_state!=s_comm_multilin_end && current_state!=s_comm_singleline)
            {
                fout<<tk.type<<": "<<new_token<<"\n";
                if(tk.type=="Error")    exit(0);//instructiune care opreste programul
            }
            current_state = s_start;
        }
    }
};

int main()
{
    adauga_tranzitii();
    Lexer lex("test.txt");
    lex.Run();
    return 0;
}
