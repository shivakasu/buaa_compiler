#include <iostream>
#include "stdio.h"
#include "define.h"
#include <cstring>
#include "error.h"

using namespace std;


char ch;
int symbol=-1;
int   number;
int   err=0;
int lineNum=0;
char token[82]={0};
int tokenLength=0;
int tokenNum=0;
FILE* fin;
FILE* fout;
string filename;

void error(int e){
	printf("error %d : line %d : %s\n",e,lineNum,errcode[e].c_str());
	err++;
}

void readFile(){
	cout<<"input file name"<<endl;
	cin>>filename;
	fin=fopen(filename.c_str(),"r+");
	fout=fopen("14061207_token.txt","w+");
	if(fin==NULL){
	    fprintf(fout,"open failed\n");
        return;
    }
}

void readBack(){
   fseek(fin,-1,SEEK_CUR);
}

void clearToken(){
    int i;
    for(i=0;i<81;i++)
       token[i]='\0';
    tokenLength=0;
}

void insertCh(){
    if(tokenLength<81)
    token[tokenLength++]=ch;
    else{
        fprintf(fout,"%d too long!\n",lineNum);
        return ;
    }
}

bool readEOF(){
    if(ch==EOF){
        symbol = eof;
        fprintf(fout,"EOF!\n");
        return true;
    }else return false;
}

void outToFile(int sym,char *typ,char *value){
    symbol = sym;
    fprintf(fout,"%d %10s %s\n",++tokenNum,typ,value);
}

int getTyp(){
    int i=0;
    if(strcmp(token,"const")==0)
        i=CONST;
    else if(strcmp(token,"int")==0)
        i=INT;
    else if(strcmp(token,"void")==0)
        i=VOID;
    else if(strcmp(token,"char")==0)
        i=CHAR;
    else if(strcmp(token,"main")==0)
        i=MAIN;
    else if(strcmp(token,"if")==0)
        i=IF;
    else if(strcmp(token,"else")==0)
	    i=ELSE;
    else if(strcmp(token,"do")==0)
	    i=DO;
    else if(strcmp(token,"while")==0)
	    i=WHILE;
    else if(strcmp(token,"for")==0)
	    i=FOR;
    else if(strcmp(token,"scanf")==0)
	    i=SCANF;
    else if(strcmp(token,"printf")==0)
	    i=PRINTF;
    else if(strcmp(token,"return")==0)
	    i=RETURN;
    return i;
}

int charToNum(){
    int j=0;
    int num=0;
    for(j=0;j<tokenLength;j++)
       num=num*10+token[j]-'0';
    return num;
}

int isLetter(){
    if((ch>='a'&&ch<='z')||(ch>='A'&&ch<='Z')||ch=='_'){
            return 1;
    }
    else
        return 0;
}

int isDigit(){
   if((ch>='0'&&ch<='9'))
        return 1;
    else
        return 0;
}

int getsym(){
    int symTyp;
	clearToken();
	ch=fgetc(fin);
	if(readEOF()) return -1;
	if(lineNum==0){
		if(readEOF()) return -1;
		else lineNum=1;
	}
	while(ch==' '||ch=='\t'||ch=='\n'){
		if(ch=='\n')
			lineNum++;
		ch=fgetc(fin);
		if(readEOF()) return -1;
     }
	if(isLetter()){
        while(isLetter()||isDigit()){
              insertCh();
              ch=fgetc(fin);
              if(readEOF()) return -1;
        }
        readBack();
        symTyp = getTyp();
        if(tokenLength>30)
            error(0);
        if(symTyp==0)
            outToFile(ident,"ident",token);
        else
            outToFile(symTyp,token,token);
	}
	else if(isDigit()){
         while(isDigit()){
              insertCh();
              ch=fgetc(fin);
              if(readEOF()) return -1;
            }
           if(isLetter())
               error(0);
          readBack();
          if(token[0]=='0'&&token[1]!=0)
             error(0);
          number = charToNum();
          outToFile(integer,"integer",token);
         }
	else if(ch=='='){
              ch=fgetc(fin);
              if(readEOF()) return -1;
			  if(ch=='=')
                  outToFile(D_EQUAL,"d_equal","==");
			  else {
                    readBack();
                    outToFile(EQUAL,"equal","=");
              }
        }
	else if(ch=='<'){
              ch=fgetc(fin);
              if(readEOF()) return -1;
			  if(ch=='=')
                  outToFile(L_EQUAL,"l_equal","<=");
			  else{
                    readBack();
			      outToFile(LESS,"less","<");}
        }
	else if(ch=='>'){
              ch=fgetc(fin);
              if(readEOF()) return -1;
			  if(ch=='=')
                  outToFile(G_EQUAL,"g_equal",">=");
			  else {
                    readBack();
                    outToFile(GREATER,"greater",">");
              }
        }
	else if(ch=='!'){
              ch=fgetc(fin);
              if(readEOF()) return -1;
			  if(ch=='=')
                  outToFile(NOT_EQUAL,"not_equal","!=");
			  else {readBack();
                    error(2);}
        }
	else if (ch=='\''){
		ch=fgetc(fin);
              if(readEOF()) return -1;
              if(ch=='+'||ch=='-'||ch=='*'||ch=='/'||ch=='_'||(ch>='a'&&ch<='z')||
                 (ch>='A'&&ch<='Z')||isDigit())
			  insertCh();
              else error(0);
	    ch=fgetc(fin);
		if(ch!='\'')
			error(3);
		else
            outToFile(character,"character",token);
	}
	else if(ch=='\"'){
		ch=fgetc(fin);
              if(readEOF()) return -1;
			  while (ch!='\"'&&ch!=EOF){
			      if((ch!=33&&ch!=32&&ch<35)||ch>126){
                      error(0);
                      break;
                  }
				  insertCh();
				  ch=fgetc(fin);
				  if (tokenLength>=60){
					  error(4);
					  do{
						   ch=fgetc(fin);
                           if(readEOF()) return -1;
					  } while (ch!=' '&&ch!='\n'&&ch!='	');
					  break;
				  }
			  }
			  outToFile(text,"text",token);
	}
	else if(ch=='+')outToFile(PLUS,"plus","+");
	else if(ch=='-')outToFile(MINUS,"minus","-");
	else if(ch=='*')outToFile(MUL,"mul","*");
    else if(ch=='/')outToFile(DIV,"div","/");
	else if(ch=='(')outToFile(L_SMALL,"l_small","(");
	else if(ch==')')outToFile(R_SMALL,"r_small",")");
	else if(ch=='[')outToFile(L_MIDDLE,"l_middle","[");
	else if(ch==']')outToFile(R_MIDDLE,"r_middle","]");
	else if(ch=='{')outToFile(L_BIG,"l_big","{");
	else if(ch=='}')outToFile(R_BIG,"r_big","}");
	else if(ch==';')outToFile(SEMICOLON,"semicolon",";");
	else if(ch==',')outToFile(COMMA,"comma",",");
	else if(ch==' '||ch=='\t'||ch=='\n');
	else 	error(0);
	return 0;
}
