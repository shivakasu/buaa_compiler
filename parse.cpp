#include <stdio.h>
#include <tchar.h>
#include <iostream>
#include <string>
#include <string.h>
#include <stdlib.h>
#include "define.h"

using namespace std;

int funcTab[200]={0};//分程序（函数）索引数组
int funcIndex =0;//分程序索引指针i，填表时指向的是当前函数的分程序索引数组中的位置（即当前函数在符号表中的起始位置）
int tabIndex;//符号表指针，每增加1符号表表项多1,实际栈顶为tabIndex-1
int paraNums=0,type=-1,value=-1,size,level;
int tempSym;
int hasReturn=0,hasMain=0;
int labelNum=0;
int funcTmpNum=0;
int codeNum=0;
int printChar;
FILE* fp;
char proName[30];
char midValue[30];
table Table[200];
quaternary code[1000];

char   id[30] ;
extern int   symbol;
extern int   number;
extern int   err;
extern int   lineNum;
extern string errorde[30];
extern char token[82];
extern char ch;
extern FILE* fin;

extern void getsym();
extern void error(int e);

void statement();
void stateList();
void valParameter();
void expression();

char* createLabel(){
	char *newLabel = (char *)malloc(10*sizeof(char));
	char counter[10];
	memset(newLabel,0,10*sizeof(char));
	newLabel[0] = 'l';
	newLabel[1] = 'a';
	newLabel[2] = 'b';
	strcat(newLabel,itoa(labelNum++,counter,10));
	return newLabel;
}

char* tempVal(){
	char *newVal = (char *)malloc(10*sizeof(char));
	char counter[10];
	memset(newVal,0,10*sizeof(char));
	newVal[0] = '$';
	itoa(funcTmpNum++,counter,10);
	strcat(newVal,counter);
	return newVal;
}

void emit(char op[],char v1[],char v2[],char v3[]){
	strcpy(code[codeNum].op,op);
	strcpy(code[codeNum].var1,v1);
	strcpy(code[codeNum].var2,v2);
	strcpy(code[codeNum++].var3,v3);
}

bool judgeEOF(){
    if (symbol==eof) return true;
    else return false;
}

void skip(int op){
    if(op==0 || op==1 || op==2){
        while (symbol!=CONST && symbol!=INT && symbol!=CHAR && symbol!=VOID && symbol!=IF
            && symbol!=DO && symbol!=FOR && symbol!=RETURN && symbol!=SCANF && symbol!=PRINTF){
            if(judgeEOF())return;
            getsym();
        }
    }else if(op==3 || op==4){
        while (symbol!=SEMICOLON) {
            if(judgeEOF())return;
            getsym();
        }
    }else if(op==5){
        while (symbol!=INT && symbol!=CHAR&& symbol!=VOID) {
            if(judgeEOF())return;
            getsym();
        }
    }
}

bool checkNextSym(int need,int sym,int e,int sk){
    if(need==1)getsym();
    if(symbol!=sym){
        error(e);
        skip(sk);
        return true;
    }else
        return false;
}

void midcodeToFile(){
	int i= 0;
    fp=fopen("midcode.txt","w+");
	while (i<codeNum) {
        fprintf(fp,"%s,\t",code[i].op);
        fprintf(fp,"%s,\t",code[i].var1);
        fprintf(fp,"%s,\t",code[i].var2);
        fprintf(fp,"%s;\n",code[i].var3);
		if (strcmp(code[i].op,"end")==0)
			fprintf(fp,"\n");
		i++;
	}
	fclose(fp);
	return;
}

void insertTable(char nam[],int typ, int val,int siz,int par,int index,int lev){
     int tmpIndex;
     if(index>199){
         error(1);
         return ;
     }
     if(typ==functype){
        tmpIndex=1;
        while(tmpIndex<=funcIndex){
            if(strcmp(Table[funcTab[tmpIndex]].name,nam)==0&&Table[tmpIndex].level==lev){
                error(5);
                return;
            }
            tmpIndex++;
        }
        tmpIndex=0;
        while(tmpIndex<=funcTab[1]){
            if(strcmp(Table[tmpIndex].name,nam)==0&&Table[tmpIndex].level==lev){
                error(5);
                return;
            }
            tmpIndex++;
        }
     }else {
        tmpIndex=funcTab[funcIndex];
        while(tmpIndex<index){ //优先在当前函数中查找是否有重复定义
            if(strcmp(Table[tmpIndex].name,nam)==0&&Table[tmpIndex].level==lev){
                error(5);
                return;
            }
            tmpIndex++;
        }
        tmpIndex=0;
        if(typ!=paratype)//参数无条件先放入符号表中，非参数才继续查找
          while(tmpIndex<funcTab[1]){//在全局变量中查找是否有重复定义
            if(strcmp(Table[tmpIndex].name,nam)==0&&Table[tmpIndex].level==lev){
                error(5);
                return;
            }
            tmpIndex++;
        }
     }
     strcpy(Table[index].name,nam);
     Table[index].type=typ;
     Table[index].value=val;
     Table[index].size=siz;
     Table[index].paranum=par;
     Table[index].level=lev;
  if(typ==functype){
     funcTab[++funcIndex]=index;
  }
      tabIndex++;
}

void clearFuncTab(){
    int temp1 = tabIndex-1;
	int temp2;
	while((Table[temp1].type==inttype || Table[temp1].type==chartype ||Table[temp1].type==constinttype
		   || Table[temp1].type==constchartype|| Table[temp1].type==paratype)){
		Table[temp1].type = 0;
		Table[temp1].size = 0;
		Table[temp1].paranum = 0;
		Table[temp1].value = 0;
		temp2 = 0;
		while (temp2<30)
			Table[temp1].name[temp2++] = '\0';
		temp1--;
	}
	tabIndex = temp1+1;////设置新的符号表栈顶指针
	return;
}

int searchTab(char nam[],int isFunc,int isArr){
   int tmpIndex;
   if(isFunc==1){
      tmpIndex=1;
      while(tmpIndex<=funcIndex){
        if(strcmp(Table[funcTab[tmpIndex]].name,nam)==0)
            break;
        tmpIndex++;
      }
      if(tmpIndex>funcIndex){
        error(6);
        printf("%s\n",nam);
        return -1;
      }
      if (Table[funcTab[tmpIndex]].paranum!=paraNums) {
			error(9);
			return -3;
		}
      return funcTab[tmpIndex];//查找成功返回在符号表中位置
   }else {
         tmpIndex=funcTab[funcIndex];//优先在当前分程序符号表中查找
         while (tmpIndex<tabIndex) {
			if (strcmp(Table[tmpIndex].name,nam)==0 && Table[tmpIndex].type!=functype && (isArr==0 || Table[tmpIndex].size>0)) return tmpIndex;
			tmpIndex++;
         }
		tmpIndex=0;
        while(tmpIndex<funcTab[1]){
            if (strcmp(Table[tmpIndex].name,nam)==0 && Table[tmpIndex].type!=functype && (isArr==0 || Table[tmpIndex].size>0)) return tmpIndex;
			tmpIndex++;
        }
        if(tmpIndex==funcTab[1]){
            error(8);
            return -1;
        }
   }
   return -1;
}

//＜常量定义＞   ::=   int＜标识符＞＝＜整数＞{,＜标识符＞＝＜整数＞} | char＜标识符＞＝＜字符＞{,＜标识符＞＝＜字符＞}
//＜整数＞        ::= ［＋｜－］＜无符号整数＞｜０
void constDefine(int preSym){
    if(checkNextSym(0,ident,11,3))return;
    strcpy(id,token);
    if(checkNextSym(1,EQUAL,10,3))return;
    getsym();
    if(symbol==PLUS||symbol==MINUS){
        int sign= symbol;
        if(checkNextSym(1,integer,12,3))return;
        if(preSym==INT){
            if(sign==PLUS) value = number;
            else if(sign==MINUS) value = 0-number;
        }
        insertTable(id,type,value,0,0,tabIndex,level);
        char tmpVal[30];
        itoa(value,tmpVal,10);
        emit((char*)"const",(char*)"int",tmpVal,id);
    }else {
        if(preSym==INT&&symbol==integer){
            insertTable(id,type,number,0,0,tabIndex,level);
            char tmpVal[30];
            value = number;
            itoa(value,tmpVal,10);
            emit((char*)"const",(char*)"int",tmpVal,id);
        }else if(preSym==CHAR&&symbol==character){
            value=token[0];
            insertTable(id,type,value,0,0,tabIndex,level);
            emit((char*)"const",(char*)"char",token,id);
        }else {
            error(14);
            skip(3);
            return;
        }
    }
    getsym();
    return ;
}

//＜常量说明＞ ::=  const＜常量定义＞;{ const＜常量定义＞;}
void constDeclare(){
    int preSym;
    if(checkNextSym(0,CONST,7,4))return;
    getsym();
    if (symbol==INT || symbol==CHAR) {
        if (symbol==INT) type= constinttype;
        else if (symbol==CHAR) type = constchartype;
        paraNums = 0;
        preSym=symbol;
        getsym();
        constDefine(preSym);
        while (symbol==COMMA) {
            getsym();
            constDefine(preSym);
        }
        if(checkNextSym(0,SEMICOLON,15,0))return;
        tempSym=symbol;
    }else {
        error(16);
        skip(0);
        return;
    }
	getsym();
	return;
}

//定义头部  int a;  char b,c; int a2[3];  int func(){}//注意需要预读，以及记录当时的文件指针位置以及返回！！！！
void defineHead(){
    if(symbol==INT||symbol==CHAR){
        if(symbol==INT) type=inttype;
        else type=chartype;
        if(checkNextSym(1,ident,0,3))return;
        else strcpy(id,token);
    }else {
        error(16);
        while (symbol!=COMMA && symbol!=L_SMALL && symbol!=R_SMALL) {
            if(judgeEOF())return;
            getsym();
        }
        return ;
    }
    getsym();
    return ;
}

//＜变量定义＞  ::= ＜类型标识符＞(＜标识符＞|＜标识符＞‘[’＜无符号整数＞‘]’){,(＜标识符＞|＜标识符＞‘[’＜无符号整数＞‘]’ )}
void varDefine(){
    defineHead();
    if(symbol==COMMA){
        insertTable(id,type,0,0,0,tabIndex,level);
        if(type==inttype) emit((char*)"int",(char*)" ",(char*)" ",id);
        else if(type==chartype) emit((char*)"char",(char*)" ",(char*)" ",id);
        while (symbol==COMMA){
		    if(checkNextSym(1,ident,0,4))return;
		    strcpy(id,token);
            getsym();
		    if(symbol!=L_MIDDLE){
                insertTable(id,type,0,0,0,tabIndex,level);
                if(type==inttype) emit((char*)"int",(char*)" ",(char*)" ",id);
                else if(type==chartype) emit((char*)"char",(char*)" ",(char*)" ",id);
            }else if(symbol==L_MIDDLE){
                if(checkNextSym(1,integer,12,4))return;
                size=number;
                if(checkNextSym(1,R_MIDDLE,18,4))return;
                insertTable(id,type,0,size,0,tabIndex,level);
                char q[30];
                itoa(size,q,10);
                if(type==inttype) emit((char*)"int",(char*)"[]",q,id);
                else if(type==chartype) emit((char*)"char",(char*)"[]",(char*)q,id);
                size=0;
                 getsym();
                 continue;
            }
        }
    }else if(symbol==L_MIDDLE){ //int a[10]
        if(checkNextSym(1,integer,12,4))return;
        size=number;
        if(checkNextSym(1,R_MIDDLE,18,4))return;
        insertTable(id,type,0,size,0,tabIndex,level);
        char q[30];
        itoa(size,q,10);
        if(type==inttype) emit((char*)"int",(char*)"[]",q,id);
        else if(type==chartype) emit((char*)"char",(char*)"[]",q,id);
        getsym();
        if(symbol==COMMA){//int a[10],b.......
            while (symbol==COMMA) {
                if(checkNextSym(1,ident,0,4))return;
		        strcpy(id,token);
                getsym();
		        if(symbol!=L_MIDDLE){
                    insertTable(id,type,0,0,0,tabIndex,level);
                    if(type==inttype) emit((char*)"int",(char*)" ",(char*)" ",id);
                    else if(type==chartype) emit((char*)"char",(char*)" ",(char*)" ",id);
                }else if(symbol==L_MIDDLE){//int a[10],b[10],......
                    if(checkNextSym(1,integer,12,4))return;
                    size=number;
                    if(checkNextSym(1,R_MIDDLE,18,4))return;
                    insertTable(id,type,0,size,0,tabIndex,level);
                    char q[30];
                    itoa(size,q,10);
                    if(type==inttype) emit((char*)"int",(char*)"[]",q,id);
                    else if(type==chartype) emit((char*)"char",(char*)"[]",(char*)"q",id);
                     getsym();
                     continue;
                }
            }
        }//int a[10],b...的特例讨论结束
    }//int a[10] 情况讨论结束;
    else {//既不是逗号也不是中括号，其实就是;或者 (，但是小括号不在这里分析 那么就把这个标识符填表即可
        insertTable(id,type,0,0,0,tabIndex,level);
        if(type==inttype) emit((char*)"int",(char*)" ",(char*)" ",id);
        else if(type==chartype) emit((char*)"char",(char*)" ",(char*)" ",id);
    }
    return ;
}

//＜变量说明部分＞  ::= ＜变量定义＞;{＜变量定义＞;}
void varDeclare(){
    varDefine();
    if(checkNextSym(0,SEMICOLON,15,0))return;
    getsym();
	return;
}

//＜参数表＞    ::=  ＜类型标识符＞＜标识符＞{,＜类型标识符＞＜标识符＞}| ＜空＞
void parameters(){
    int i=0;     //记录参数个数
	if( symbol==INT||symbol==CHAR ) {
            int tmpSym1;
            tmpSym1=symbol;
		do{
             tmpSym1=symbol;
			if (symbol==COMMA)
                getsym();
            if(tmpSym1==COMMA&&(symbol==INT||symbol==CHAR))
                tmpSym1=symbol;
			defineHead();
			type = paratype;
	    	insertTable(id,type,0,0,0,tabIndex,level);  //将行数参数插入符号表,参数也要入表
            if(tmpSym1==INT)
                emit((char*)"para",(char*)"int",(char*)"0",id);
            else if(tmpSym1==CHAR)
                emit((char*)"para",(char*)"char",(char*)"0",id);
    		i++;
		}while(symbol==COMMA);
	}
	paraNums = i;
	Table[funcTab[funcIndex]].paranum = paraNums;//插入函数的参数个数
	return;
}

//＜因子＞ ::= ＜标识符＞｜＜标识符＞‘[’＜表达式＞‘]’｜＜整数＞|＜字符＞｜＜有返回值函数调用语句＞|‘(’＜表达式＞‘)’
void factor(){
    char names[30],tmp[30];
    int res;
	if (symbol==ident) {
        strcpy(id,token);
		strcpy(names,id);
		getsym();
		if (symbol==L_SMALL) {
			getsym();
			valParameter();
			if(checkNextSym(0,R_SMALL,20,1))return;
			res = searchTab(names,1,0);
			if (res==-1 || Table[res].type!=functype ||(Table[res].type==functype&&Table[res].value==0) ) {//找不到定义的标识符或者不是函数或者不是有返回值的函数
				error(8);
			     skip(1);
				 return;
			}
			if(Table[res].size==0)
                printChar++;
			strcpy(tmp,tempVal());//生成临时变量
			if(Table[res].size==0)
			emit((char*)"call",names,(char*)"int",tmp);//将调用的返回值存放在临时变量里面
			if(Table[res].size==1)
			emit((char*)"call",names,(char*)"char",tmp);//将调用的返回值存放在临时变量里面
			strcpy(midValue,tmp);
			getsym();
			return;
		}
        else if(symbol==L_MIDDLE){//＜标识符＞‘[’＜表达式＞‘]’
            res = searchTab(names,0,1);
            if(res<0||Table[res].size==0){////找不到变量或者变量不是个数组变量
                error(8);            //"="左边是不合法的标识符
				skip(1);
                return ;
            }
            int vl=Table[res].size;
            getsym();
            expression();
            char tmpArrVal[30];
            strcpy(tmpArrVal,midValue);
             if(tmpArrVal[0]>='0'&&tmpArrVal[0]<='9'){/////如果tmpArrVal是个数字，要检查数组越界
                int a;
                a=atoi(tmpArrVal);
                if(a>=vl){
                    error(13);
                    skip(1);
                    return ;
                }
             }
             if(checkNextSym(0,R_MIDDLE,18,1))return;
            strcpy(tmp,tempVal());
            emit((char*)"=[]",names,tmpArrVal,tmp);
			strcpy(midValue,tmp);/////因子的返回值存入midValue,  返回的数组名字+[tmpArrVal]
			getsym();
			return ;
       }else {
			res = searchTab(names,0,0);
			if (res==-1) {
				error(8);
				 skip(1);
				return;
			}
		}
		strcpy(midValue,names);
		return;
	}
	if (symbol==L_SMALL) {
		getsym();
		expression();
		if (symbol!=R_SMALL) {
			error(20);
			skip(0);
			return;
		}
		getsym();
		return;
	}
	// ＜整数＞|＜字符＞
	//＜整数＞ ::= ［＋｜－］＜无符号整数＞｜０
	if (symbol==PLUS || symbol==MINUS|| symbol==integer||symbol==character) {
		if (symbol==PLUS) getsym();
		else if (symbol==MINUS) {
			int k=0;
			getsym();
			while (token[k++]!='\0');
			token[k+1] = '\0';
			while (k!=0) {
				token[k] = token[k-1];
				k--;
			}
			token[0] = '-';
		}
		strcpy(midValue,token);//整数赋值给midValue
		if(symbol==character){
            char charVal[30];
            itoa(int(token[0]),charVal,10);
            strcpy(midValue,charVal);
            printChar--;
		}
		getsym();
		return;
	}
	error(0);                  //表达式缺失或错误
	skip(0);
    return;
}

//＜项＞ ::= ＜因子＞{＜乘法运算符＞＜因子＞}
void term(){
    char fact1[30],fact2[30],fact3[30];
	factor();
	strcpy(fact3,midValue);//这种操作是为了对付只有赋值的情况,，如果无乘除，那么p1、p2没什么用
	while (symbol==MUL|| symbol==DIV) {
        printChar++;
        int tmpSym=symbol;
 		strcpy(fact1,fact3);////有乘除时第一个操作数放在p1
 		getsym();
        factor();
        strcpy(fact2,midValue);/////第二个操作数放在p2
        strcpy(fact3,tempVal());//申请临时变量
		if (tmpSym==MUL)
			emit((char*)"imul",fact1,fact2,fact3);
		if (tmpSym==DIV)
			emit((char*)"idiv",fact1,fact2,fact3);
        continue;
	}
	strcpy(midValue,fact3);  //每一个项，计算后的值都在midValue里面
	return;
}

//＜表达式＞ ::= ［＋｜－］＜项＞{＜加法运算符＞＜项＞}
void expression(){
    char term1[30],term2[30],term3[30];
	if (symbol==PLUS || symbol==MINUS) {
        printChar++;
		if (symbol==PLUS) {
			getsym();
			term();
			strcpy(term3,midValue);
		}
		if (symbol==MINUS) {
			getsym();
			term();
			strcpy(term1,midValue);
			strcpy(term3,tempVal());//place3 为临时变量,tempVal 生成临时变量以$开头
			emit((char*)"sub",(char*)"0",term1,term3);
		}
	}
	else {
		term();
		strcpy(term3,midValue);
	}
	while (symbol==PLUS || symbol==MINUS) {
        printChar++;
		strcpy(term1,term3);//////把上一个项与项计算得到的结果记为新的源操作数1
		if (symbol==PLUS) {
			getsym();
			term();
			strcpy(term2,midValue);////当前项的计算结果
			strcpy(term3,tempVal());//新成新的临时变量
			emit((char*)"add",term1,term2,term3);//将项与项的加法运算结果存到新的临时变量里
			continue ;
		}
		if (symbol==MINUS) {
			getsym();
			term();
			strcpy(term2,midValue);
			strcpy(term3,tempVal());
			emit((char*)"sub",term1,term2,term3);
			continue ;
		}
	}
	strcpy(midValue,term3);//把表达式的最终值存放在midValue之中
	return;
}

//＜条件＞    ::=  ＜表达式＞＜关系运算符＞＜表达式＞｜＜表达式＞ //表达式为0条件为假，否则为真
void ifLabel(char* label){
   char express1[30],express2[30];
	expression();
	strcpy(express1,midValue);  //条件至少有一个表达式
	int tmpSym1=symbol;
	if (tmpSym1==LESS || tmpSym1==G_EQUAL || tmpSym1==GREATER || tmpSym1==L_EQUAL || tmpSym1==NOT_EQUAL || tmpSym1==D_EQUAL) {
        getsym();
        expression();
        strcpy(express2,midValue);
		if (tmpSym1==LESS)
			emit((char*)"jge",express1,express2,label);
		else if (tmpSym1==L_EQUAL)
			emit((char*)"jnle",express1,express2,label);
		else if (tmpSym1==GREATER)
			emit((char*)"jle",express1,express2,label);
		else if (tmpSym1==G_EQUAL)
			emit((char*)"jnge",express1,express2,label);
		else if (tmpSym1==NOT_EQUAL)
			emit((char*)"jz",express1,express2,label);
		else if (tmpSym1==D_EQUAL)
			emit((char*)"jnz",express1,express2,label);
		return;
	}
	strcpy(express2,"0");
	emit((char*)"jz",express1,express2,label);/////看表达式的值是不是等于0，为0跳转
	return;
}

//＜条件语句＞  ::=  if ‘(’＜条件＞‘)’＜语句＞［else＜语句＞］
void ifStatement(){
    char label1[30],label2[30];
    char ifLabelvalue[30];
    strcpy(label1,createLabel());
    strcpy(label2,createLabel());
    if(checkNextSym(1,L_SMALL,19,2))return;
    getsym();
    ifLabel(label1);
    strcpy(ifLabelvalue,midValue);
    if(checkNextSym(0,R_SMALL,20,2))return;
    getsym();
    statement();
    emit((char*)"jmp",(char*)" ",(char*)" ",label2);
    emit((char*)"label",(char*)" ",(char*)" ",label1);
    if(symbol==ELSE){
        getsym();
        statement();
    }
    emit((char*)"label",(char*)" ",(char*)" ",label2);
    return ;
}

//＜循环语句＞
void doStatement(){
	char label1[10],label2[10];
	strcpy(label1,createLabel());
	strcpy(label2,createLabel());
    getsym();
    emit((char*)"label",(char*)" ",(char*)" ",label1);//设置跳转回来的标签
    statement();
    if(checkNextSym(0,WHILE,26,2))return;
    if(checkNextSym(1,L_SMALL,19,2))return;
    getsym();
    ifLabel(label2);//条件为假时跳到label2
    if(checkNextSym(0,R_SMALL,18,2))return;
    emit((char*)"jmp",(char*)" ",(char*)" ",label1);
    emit((char*)"label",(char*)" ",(char*)" ",label2);
    getsym();
    return;
}

void forStatement(){
	char label1[10],label2[10],names1[30],names2[30],tmp_num[30],tmp_mid[30];
	int res,sign;
	strcpy(label1,createLabel());
	strcpy(label2,createLabel());
	if(checkNextSym(1,L_SMALL,19,2))return;
	if(checkNextSym(1,ident,0,2))return;
	res = searchTab(token,0,0);
    if (res<0||Table[res].value!=0||Table[res].size!=0) {//找不到变量或者找到的是个常量或者是个数组变量
        error(12);            //"="左边是不合法的标识符
        skip(1);
        return ;
    }
    strcpy(names1,token);
	if(checkNextSym(1,EQUAL,10,2))return;
    getsym();
    expression();
    emit((char*)"mov", midValue,(char*)" ",names1);
    if(checkNextSym(0,SEMICOLON,15,2))return;
    getsym();
    emit((char*)"label",(char*)" ",(char*)" ",label1);
    ifLabel(label2);
    if(checkNextSym(0,SEMICOLON,15,2))return;
    if(checkNextSym(1,ident,0,2))return;
    if(checkNextSym(1,EQUAL,10,2))return;
    if(checkNextSym(1,ident,0,2))return;
    strcpy(names2,token);
    getsym();
    if(symbol!=PLUS && symbol!=MINUS){
        error(17);
        skip(2);
        return;
    }
    sign=symbol;
    if(checkNextSym(1,integer,12,2))return;
    strcpy(tmp_num,token);
    if(checkNextSym(1,R_SMALL,20,2))return;
    getsym();
    statement();
    strcpy(tmp_mid,tempVal());
    if(sign==PLUS)emit((char*)"add",names2,tmp_num,tmp_mid);
    else  emit((char*)"sub",names2,tmp_num,tmp_mid);
    emit((char*)"mov", tmp_mid,(char*)" ",names1);
    emit((char*)"jmp",(char*)" ",(char*)" ",label1);
    emit((char*)"label",(char*)" ",(char*)" ",label2);
}

//＜读语句＞    ::=  scanf ‘(’＜标识符＞{,＜标识符＞}‘)’
void readStatement(){
    char names[30];
	int res;
	if(checkNextSym(1,L_SMALL,19,2))return;
	do{
        if(checkNextSym(1,ident,0,2))return;
		strcpy(id,token);
		strcpy(names,id);
		res = searchTab(names,0,0);
		if (res==-1 || Table[res].type==functype||Table[res].size!=0) {
			error(8);            //不合法的标识符
			skip(2);
	    	return;
		}
		if(Table[res].type==inttype)
		emit((char*)"scanf",(char*)"int",(char*)" ",names);
		else if(Table[res].type==chartype)
        emit((char*)"scanf",(char*)"char",(char*)" ",names);
		getsym();
	}while(symbol==COMMA);
	if(checkNextSym(0,R_SMALL,20,2))return;
	getsym();
	return;
}

//＜写语句＞    ::= printf ‘(’ ＜字符串＞,＜表达式＞ ‘)’| printf ‘(’＜字符串＞ ‘)’| printf ‘(’＜表达式＞‘)’
void writeStatement(){
    char str1[200]="\0",express1[30]=" ";
    if(checkNextSym(1,L_SMALL,19,2))return;
	getsym();
	if (symbol==text) {
		strcpy(str1,token);
		getsym();
		if (symbol==COMMA) {
			getsym();
			printChar=0;
			expression();
			strcpy(express1,midValue);
		 }
    }else {
        printChar=0;
		expression();
		strcpy(express1,midValue);
    }
    if(checkNextSym(0,R_SMALL,20,2))return;
    if(printChar==-1 && strcmp(express1," ")!=0){
        emit((char*)"printf",str1,express1,(char*)"1");
    }
	else
	    emit((char*)"printf",str1,express1,(char*)" ");
	printChar=0;
	getsym();
	return;
}

//＜返回语句＞   ::=  return[‘(’＜表达式＞‘)’]
void returnStatement(){
   char express[30];
   getsym();
   if(symbol==L_SMALL){
    getsym();
    expression();
    strcpy(express,midValue);
    if(checkNextSym(0,R_SMALL,20,2))return;
      emit((char*)"ret",(char*)" ",(char*)" ",express);
      hasReturn++;
      getsym();
   }else emit((char*)"ret",(char*)" ",(char*)" ",(char*)" ");
   return ;
}

//＜值参数表＞   ::= ＜表达式＞{,＜表达式＞}｜＜空＞
void valParameter(){
    char paras[100][100];
    int j = 0;
    do {
		if (symbol==COMMA) getsym();
		if (symbol==PLUS || symbol==MINUS || symbol==ident ||symbol==L_SMALL || symbol==character||symbol==integer) {
			expression();
			strcpy(paras[j],midValue);
			j++;
		}
	}while (symbol==COMMA);
	paraNums = j;
	for(j=0;j<paraNums;j++)
        emit((char*)"para",(char*)" ",(char*)"1",paras[j]);
	return ;
}

void statement(){
    int res;
    if(symbol==SEMICOLON){
        return;
    }
    if(symbol==IF){
        ifStatement();
        return;
    }
	if (symbol==DO) {
		doStatement();
		return;
	}
	if (symbol==FOR) {
		forStatement();
		return;
	}
    if (symbol==L_BIG) {
        getsym();
        stateList();
        if(checkNextSym(0,R_BIG,22,1))return;
        getsym();
        return;
    }
    if (symbol==SCANF) {
        readStatement();
        if(checkNextSym(0,SEMICOLON,15,1))return;
        getsym();
        return;
    }
    if (symbol==PRINTF) {
        writeStatement();
        if(checkNextSym(0,SEMICOLON,15,1))return;
        getsym();
        return;
    }
    if (symbol==RETURN) {
        returnStatement();
        if(checkNextSym(0,SEMICOLON,15,1))return;
        getsym();
        return;
    }
	if (symbol==ident) {
		char names[30];
		strcpy(id,token);
		strcpy(names,id);
		getsym();
		//＜赋值语句＞   ::=  ＜标识符＞＝＜表达式＞|＜标识符＞‘[’＜表达式＞‘]’=＜表达式＞
		if (symbol==EQUAL) {
			res = searchTab(names,0,0);
			if (res<0||Table[res].type==constchartype||Table[res].type==constinttype||Table[res].size!=0) {//找不到变量或者找到的是个常量或者是个数组变量
				error(8);
				skip(1);
                return ;
			}
			getsym();
			expression();
			emit((char*)"mov", midValue,(char*)" ",names);
			if(checkNextSym(0,SEMICOLON,15,1))return;
			getsym();
			return;
		}
		if(symbol==L_MIDDLE){
            res = searchTab(names,0,1);
            if(res<0||Table[res].size==0){//找不到变量或者变量不是个数组变量
                error(8);
				skip(1);
                return ;
            }
            int arrLength=Table[res].size;
            getsym();
            expression();
            char tmpArrVal[30];
            strcpy(tmpArrVal,midValue);
            if(tmpArrVal[0]>='0'&&tmpArrVal[0]<='9'){//如果tmpArrVal是个数字，要检查数组越界
                int a;
                a=atoi(tmpArrVal);
                if(a>=arrLength){
                    error(13);
                    skip(1);
                    return ;
                }
            }
            if(checkNextSym(0,R_MIDDLE,18,1))return;
            if(checkNextSym(1,EQUAL,10,1))return;
			getsym();
			expression();
			char tempplace[30];
			strcpy(tempplace,tempVal());
			emit((char*)"[]=",tmpArrVal,midValue,names);
			if(checkNextSym(0,SEMICOLON,15,1))return;
			getsym();
			return;
		  }
		//函数调用语句
		if (symbol==L_SMALL) {
			getsym();
			valParameter();
			if(checkNextSym(0,R_SMALL,20,1))return;
			res = searchTab(names,1,0);
			if (res==-1 || res==-3||Table[res].type!=functype) {
                error(8);
				skip(1);
				return;
			}
			if(Table[res].size==0)
			emit((char*)"call",names,(char*)"int",(char*)" ");
			else if(Table[res].size==1)
            emit((char*)"call",names,(char*)"char",(char*)" ");
            else
            emit((char*)"call",names,(char*)" ",(char*)" ");
            if(checkNextSym(1,SEMICOLON,15,1))return;
			getsym();
		}
		else {
			error(0);    //不合法的句子
			skip(1);
            return;
		}
		return;
	}
	if(symbol==SEMICOLON){
       getsym();
       return ;
    }
    else if(symbol==R_BIG){
        return;
    }
    error(0);
    skip(1);
    return;
}

void stateList(){
    statement();
    while(symbol==IF||symbol==ELSE||symbol==DO||symbol==FOR||symbol==SCANF||symbol==PRINTF
          ||symbol==L_BIG||symbol==ident||symbol==RETURN||symbol==SEMICOLON){
            if(symbol!=SEMICOLON)statement();
            else getsym();
    }
}

//＜复合语句＞   ::=  ［＜常量说明＞］［＜变量说明＞］＜语句列＞
void comStatement(){
    tempSym=0;
    if(symbol==CONST){
        constDeclare();
    }
    while(tempSym==SEMICOLON){
        if(symbol==CONST)
            constDeclare();
        else break;
    }
    while(symbol==INT||symbol==CHAR){
        int a=ftell(fin);//记录下当前读到的文件位置，以便预读过后返回分析
        int tmpSym2=symbol;
        if(symbol==INT) type=inttype;
        if(symbol==CHAR) type = chartype;
        getsym();
        if(symbol==ident){
            strcpy(id,token);
            getsym();
            if(symbol==COMMA||symbol==L_MIDDLE){
                symbol=tmpSym2;
                fseek(fin,a,SEEK_SET);//继续分析
                varDeclare();
                continue;
            }
            else if(symbol==SEMICOLON){
                insertTable(id,type,0,0,0,tabIndex,level);
                if(type==inttype) emit((char*)"int",(char*)" ",(char*)" ",id);
                else if(type==chartype) emit((char*)"char",(char*)" ",(char*)" ",id);
                getsym();
                continue;
            }
            else {
                error(0);
                skip(1);
                return;
            }
        }
    }
    stateList();
    return;
}

//＜程序＞ ::= ［＜常量说明＞］［＜变量说明＞］{＜有返回值函数定义＞|＜无返回值函数定义＞}＜主函数＞
void procedure(){
    level=0;
    getsym();
    if(symbol==CONST)
        constDeclare();
    while(tempSym==SEMICOLON){
        if(symbol==CONST)
            constDeclare();
        else break;
    }
    while(symbol==INT||symbol==CHAR){
        int a=ftell(fin);//记录下当前读到的文件位置，以便预读过后返回分析
        int tmpSym2=symbol;
        int tmpLineNum=lineNum;
        if(symbol==INT) type=inttype;
        if(symbol==CHAR) type = chartype;
        getsym();
        if(symbol==ident){
            strcpy(id,token);
            getsym();
            if(symbol==COMMA||symbol==L_MIDDLE){
                symbol=tmpSym2;
                fseek(fin,a,SEEK_SET);//继续分析
                lineNum=tmpLineNum;
                varDeclare();
                continue;
            }else if(symbol==SEMICOLON){
                insertTable(id,type,0,0,0,tabIndex,level);
                if(type==inttype) emit((char*)"int",(char*)" ",(char*)" ",id);
                else if(type==chartype) emit((char*)"char",(char*)" ",(char*)" ",id);
                getsym();
                continue;
            }else {//其他情况其实只剩下一种，小括号的情况，为有返回值函数定义，跳出，进行函数的分析
                symbol=tmpSym2;
                lineNum=tmpLineNum;
                fseek(fin,a,SEEK_SET);
                break;
            }
        }
    }
    if(symbol!=INT&&symbol!=CHAR&&symbol!=VOID){
        error(0);
        skip(5);
    }

//＜有返回值函数定义＞  ::=  ＜声明头部＞‘(’＜参数＞‘)’ ‘{’＜复合语句＞‘}’
    while(symbol==INT||symbol==CHAR||symbol==VOID){
        level=0;
        int tmpSym3=symbol;
         if(symbol==INT||symbol==CHAR){
               if(symbol==INT)size=0;
               if(symbol==CHAR)size=1;
               defineHead();
               if(checkNextSym(0,L_SMALL,19,5))continue;
             funcTmpNum=0;
             hasReturn=0;
             type=functype;
             insertTable(id,type,1,size,0,tabIndex,level);
             level=1;
             strcpy(proName,id);
             if(tmpSym3==INT)
                emit((char*)"start",(char*)"int",(char*)" ",proName);
             if(tmpSym3==CHAR)
                emit((char*)"start",(char*)"char",(char*)" ",proName);
             getsym();
             parameters();
             if(checkNextSym(0,R_SMALL,20,5))continue;
             if(checkNextSym(1,L_BIG,21,5))continue;
             getsym();
             comStatement();
             if(checkNextSym(0,R_BIG,22,5))continue;
             if(hasReturn==0){
                error(23);
                skip(5);
				continue;
             }
             getsym();
             emit((char*)"end",(char*)" ",(char*)" ",proName);
             clearFuncTab();//清空函数定义的符号表
             level=0;
           }
        else if(symbol==VOID){
            level=0;
            getsym();
            if(symbol==MAIN){
               strcpy(id,"main");
               funcTmpNum=0;
               type=functype;
               size=-1;
               insertTable(id,type,0,size,0,tabIndex,level);
                level=1;
               strcpy(proName,id);
               emit((char*)"start",(char*)" ",(char*)" ",proName);
               if(checkNextSym(1,L_SMALL,19,5))continue;
               if(checkNextSym(1,R_SMALL,20,5))continue;
               if(checkNextSym(1,L_BIG,21,5))continue;
               getsym();
               comStatement();
               if(checkNextSym(0,R_BIG,22,5))continue;
             hasMain++;
             emit((char*)"end",(char*)" ",(char*)" ",proName);
             level=0;
             return;
          }
          //＜无返回值函数定义＞  ::= void＜标识符＞‘(’＜参数＞‘)’‘{’＜复合语句＞‘}’
          if(checkNextSym(0,ident,0,5))continue;
             level=0;
             funcTmpNum=0;
             type=functype;
             hasReturn=0;
             size=-1;
             strcpy(id,token);
             insertTable(id,type,0,size,0,tabIndex,level);
             level=1;
             strcpy(proName,id);
             emit((char*)"start",(char*)"void",(char*)" ",proName);
             if(checkNextSym(1,L_SMALL,19,5))continue;
             getsym();
             parameters();
             if(checkNextSym(0,R_SMALL,20,5))continue;
             if(checkNextSym(1,L_BIG,21,5))continue;
             getsym();
             comStatement();
             if(checkNextSym(0,R_BIG,22,5))continue;
             if(hasReturn==1){
                error(24);
                skip(5);
				continue;
             }
             getsym();
             emit((char*)"end",(char*)" ",(char*)" ",proName);
             clearFuncTab();
             level=0;
          }
        }
    if(hasMain==0){
        error(25);
        return;
    }
}

