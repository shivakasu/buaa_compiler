#include "define.h"
#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <cstring>

using namespace std;

extern table Table[200];
extern quaternary code[1000];
extern int codeNum;
int index=0;
int counter=0;//局部变量和临时变量计数
int paraNum=0;
int paraCounter = 0;
int funcTyp=-1;
int tmpLabel=0;
varNode *localList = NULL,*tempList = NULL,*globalList=NULL,*tp,*read;
FILE* asmFile;

void asmHead(char disk){
    fprintf(asmFile,".386\n");
    fprintf(asmFile,".model flat,stdcall\n");
    fprintf(asmFile,"option casemap:none\n");
    fprintf(asmFile,"Include %c:\\masm32\\include\\windows.inc\n",disk);
    fprintf(asmFile,"Include %c:\\masm32\\include\\kernel32.inc\n",disk);
    fprintf(asmFile,"Include %c:\\masm32\\include\\msvcrt.inc\n",disk);
    fprintf(asmFile,"Includelib %c:\\masm32\\lib\\msvcrt.lib\n",disk);
    fprintf(asmFile,"Includelib %c:\\masm32\\lib\\kernel32.lib\n",disk);
    fprintf(asmFile,"Include %c:\\masm32\\macros\\macros.asm\n\n",disk);
}

varNode* search(varNode * head, char *id){
	varNode *t = head;
	while(t != NULL){
		if(strcmp(t->id,id) == 0)
			return t;
		t = t->next;
	}
	return NULL;
}

void initList(varNode* head){
	if(head == NULL)
		return;
	else{
		initList(head->next);
		free(head);
	}
}

void getArrIndex(char *s){
    char name[30];
    strcpy(name,s);
    read = search(localList,name);
    if(read == NULL)read =search(tempList,name);
    if(read != NULL)
        fprintf(asmFile,"\tmov\tebx,\tdword  ptr [ebp+(%d)]\n",read->add);
    else{
        read = search(globalList,name);
        if(read != NULL)//全局或者常量
            fprintf(asmFile,"\tmov\tebx,\t_%s\n",name);
        else
            fprintf(asmFile,"\tmov\tebx,\t%s\n",name);
    }
}

int getArrBase(char* s,int fb){
    char name[30];
    int res=-1;
    strcpy(name,s);
    strcat(name,"[0]");
    read = search(localList,name);
    if(read!=NULL)
        res=read->add;//找不到的话说明是个全局定义的数组
    if(res==-1){
        if(fb==1)
            fprintf(asmFile,"\tmov\teax,\tdword ptr[%s+4*ebx]\n",s);
        else
            fprintf(asmFile,"\tmov\tdword ptr[%s+4*ebx] ,eax\n",s);
        return -1;
    }
    else{
        fprintf(asmFile,"\tneg ebx\n");
        if(fb==1)
            fprintf(asmFile,"\tmov\teax,\tdword ptr[ebp+(%d)+4*ebx]\n",res);
        else
            fprintf(asmFile,"\tmov\tdword ptr[ebp+(%d)+4*ebx] ,eax\n",res);
        return 0;
    }
}

void addLink(varNode **target,char *id,int add,int typ){
    tp = (varNode*)malloc(sizeof(varNode));
    tp->add = add;
    tp->next = NULL;
    strcpy(tp->id,id);
    tp->type = typ;
    if(*target == NULL)
        *target = tp;
    else{
        tp->next=(*target)->next;
        (*target)->next=tp;
    }
}

bool codeOp(char *s){
    return (strcmp(code[index].op,s)==0);
}

bool codeVar1(char *s){
    return (strcmp(code[index].var1,s)==0);
}

bool codeVar2(char *s){
    return (strcmp(code[index].var2,s)==0);
}

bool codeVar3(char *s){
    return (strcmp(code[index].var3,s)==0);
}

void pushNew(int t){
    if(!codeVar2(" ")){
        int arrLength = atoi(code[index].var2);
        if(t==1)
            fprintf(asmFile,"\t%s\tDWORD\t%d DUP(?)\n",code[index].var3,arrLength);
        for(int i=0;i<arrLength;i++){
            char arrIndex[20],tmpName[20];
            int tmpTyp=0;
            strcpy(tmpName,code[index].var3);
            itoa(i,arrIndex,10);
            strcat(tmpName,"[");
            strcat(tmpName,arrIndex);
            strcat(tmpName,"]");
            if(codeOp("char"))
                tmpTyp=1;
            if(t==1)
                addLink(&globalList,tmpName,0,tmpTyp);
            else{
                addLink(&localList,tmpName,(-1)*4*(++counter),tmpTyp);
                fprintf(asmFile,"\tmov\teax,\t0;LocalVar\n");//变量赋值默认为0
                fprintf(asmFile,"\tpush\teax\n");//局部变量压栈
            }
        }
    }else{
        int tmpTyp=0;
        if(codeOp("char"))
            tmpTyp=1;
        if(t==1){
            fprintf(asmFile,"\t_%s\tdword\t?\n",code[index].var3);
            addLink(&globalList,code[index].var3,0,tmpTyp);
        }else{
            addLink(&localList,code[index].var3,(-1)*4*(++counter),tmpTyp);
            fprintf(asmFile,"\tmov\teax,\t0;LocalVar\n");//变量赋值默认为0
            fprintf(asmFile,"\tpush\teax\n");//局部变量压栈
        }
    }
}

void newTmpVal(int add,int tmpTyp,char *s){
    add=(-4)*(++counter);
    addLink(&tempList,s,add,tmpTyp);
    fprintf(asmFile,"\tmov\tdword  ptr [ebp+(%d)],\teax\n",add);
    fprintf(asmFile,"\tmov   ebx,ebp\n");//临时变量放在内存中
    fprintf(asmFile,"\tadd   ebx,%d\n",add);
    fprintf(asmFile,"\tcmp   esp ,ebx\n");
    fprintf(asmFile,"\tjle  espp%d\n",tmpLabel);
    fprintf(asmFile,"\tmov esp, ebx\n");
    fprintf(asmFile,"espp%d:\n",tmpLabel++);
}

void findAllList(int i){
    char s[30];
    if(i==1)strcpy(s,code[index].var1);
    else if(i==2)strcpy(s,code[index].var2);
    else if(i==3)strcpy(s,code[index].var3);
    read = search(localList,s);
    if(read == NULL)read = search(tempList,s);
    if(read != NULL)
        fprintf(asmFile,"\tmov\teax,\tdword  ptr [ebp+(%d)]\n",read->add);
    else{
        read=search(globalList,s);
        if(read==NULL){//都找不到是常数
            int arrSize,i;
            char temp[30];
            strcpy(temp,s);
            arrSize = strlen(temp);
            for(i=0;i<arrSize;i++)
                if(temp[i]=='[')
                    break;
            if(i==arrSize){
                if(s[0]=='$')
                    newTmpVal((-4)*(++counter),0,s);
                else if((s[0]>='0'&&s[0]<='9')||((s[0]=='-'||s[0]=='+')&&s[1]>='0'&&s[1]<='9'))
                    fprintf(asmFile,"\tmov\teax,\t%s\n",s);
                else
                    fprintf(asmFile,"\tmov\teax,\t\'%s\'\n",s);
            }
            else{//change[$0]
                char arrIndex[30];
                int j,k=0;
                for(j=i+1;j<arrSize;j++)
                    if(temp[j]!=']')
                        arrIndex[k++]=temp[j];
                arrIndex[k]='\0';
                getArrIndex(arrIndex);
                for(;i<arrSize;i++)
                    temp[i]='\0';
                getArrBase(temp,1);
            }
        }
        else{//是全局变量
            int arrSize,i;
            char temp[30];
            strcpy(temp,s);
            arrSize = strlen(temp);
            for(i=0;i<arrSize;i++)
                if(temp[i]=='[')
                    break;
            if(i==arrSize)
                fprintf(asmFile,"\tmov\teax,\t_%s\n",temp);
            else {//数组
                char arrIndex[30];
                int j,k=0;
                for(j=i+1;j<arrSize;j++){
                    if(temp[j]!=']')
                    arrIndex[k++]=temp[j];
                }
                arrIndex[k]='\0';
                getArrIndex(arrIndex);
                for(;i<arrSize;i++)
                    temp[i]='\0';
                getArrBase(temp,1);
            }
        }
    }
}

void dataSeg(){
    fprintf(asmFile,".data\n");
    while (codeOp("const")) {
        int tmpTyp=0;
        if (codeVar1("int"))
            fprintf(asmFile,"\t_%s\tequ\t%s\n",code[index].var3,code[index].var2);
        else{
            fprintf(asmFile,"\t_%s\tequ\t\'%s\'\n",code[index].var3,code[index].var2);
            tmpTyp=1;
        }
        addLink(&globalList,code[index++].var3,0,tmpTyp);
	}
	while(codeOp("int")||codeOp("char")){
        pushNew(1);
        index++;
	}
	fprintf(asmFile,"\n");
}

void codeSeg(){
    char funcName[20];
    fprintf(asmFile,".CODE\n");
    while(index<codeNum){
        if(codeOp("label"))
            fprintf(asmFile,"%s:\n",code[index].var3);
        else if(codeOp("jmp"))
            fprintf(asmFile,"\tjmp\t%s\n",code[index].var3);
        else if(codeOp("int")||codeOp("char"))
			pushNew(0);
        else if(codeOp("start")){
            counter = 0;
            paraNum = 0;
            strcpy(funcName,code[index].var3);
            if(codeVar3("main")){
                fprintf(asmFile,"START:\n");
                fprintf(asmFile,"\tpush\tebp\n");
                fprintf(asmFile,"\tmov\tebp,\tesp\n");
            }else{
                int tmpIndex = index+1;
                if(codeVar1("char"))
                    funcTyp=1;
                if(codeVar1("int"))
                    funcTyp=0;
                if(codeVar1("void"))
                    funcTyp=2;
                fprintf(asmFile,"_%s\tPROC\n",code[index].var3);
                while(strcmp(code[tmpIndex].op,"para")==0 && strcmp(code[tmpIndex].var2,"0")==0){
                    paraNum++;
                    tmpIndex++;
                }
                fprintf(asmFile,"\tpush\tebx;saveState\n");
                fprintf(asmFile,"\tpush\tebp\n");//paraCounter	ebp  调用函数的ebp，之前有一个是retadress
                fprintf(asmFile,"\tmov\tebp,\tesp;saveState finish\n");// mov ebp,esp  EBP指向的是当前函数的基地址
            }
        }
        else if(codeOp("end")){
            if(codeVar3("main")){
                fprintf(asmFile,"   invoke ExitProcess, 0; exit with code 0\n");
                fprintf(asmFile,"END START\n");
            }else{
                fprintf(asmFile,"\tmov\tesp,\tebp\n");
                fprintf(asmFile,"\tpop\tebp\n");
                fprintf(asmFile,"\tpop\tebx\n");
                fprintf(asmFile,"\tret\n");
                counter = 0;
                initList(localList);
                localList = NULL;
                initList(tempList);
                tempList = NULL;
                fprintf(asmFile,"_%s\tendp\n",funcName);
            }
        }
        else if(codeOp("para")){
            if(codeVar2("0")){
                int tmpTyp=0;
                if(codeVar1("char"))
                    tmpTyp=1;
                addLink(&localList,code[index].var3,4 * (2 + paraNum--),tmpTyp);
            }else{
                paraCounter++;
                findAllList(3);
                fprintf(asmFile,"\tpush\teax;pass value to function\n");//参数压栈
            }
        }
        else if(codeOp("const")){
            int tmpTyp=0;
            if (codeVar1("int")){
                fprintf(asmFile,"\tmov\teax,\t%s;constVar\n",code[index].var2);
            }else{
                fprintf(asmFile,"\tmov\teax,\t\'%s\';constVar\n",code[index].var2);
                tmpTyp=1;
            }
            addLink(&localList,code[index].var3,(-1)*4*(++counter),tmpTyp);
            fprintf(asmFile,"\tpush\teax\n");
        }
		else if(codeOp("add")||codeOp("sub")|| codeOp("imul") || codeOp("idiv")){
            int tmpTyp=0,add;
            findAllList(2);
			fprintf(asmFile,"\tmov\tebx,\teax\n");//第2个操作数存在了ebx
			// 第1个源操作数
			findAllList(1);
			if(codeOp("add")|| codeOp("sub"))
                fprintf(asmFile,"\t%s\teax,\tebx\n",code[index].op);
			else{
                fprintf(asmFile,"\tcdq\n");
                fprintf(asmFile,"\t%s\tebx\n",code[index].op);
			}
			// 第三个参数，结果值
			read = search(localList,code[index].var3);
			if(read == NULL)read = search(tempList,code[index].var3);
			if(read != NULL)
                fprintf(asmFile,"\tmov\tdword  ptr [ebp+(%d)],\teax\n",read->add);
            else{
                read = search(globalList,code[index].var3);
                if(read == NULL)
                    newTmpVal((-4)*(++counter),tmpTyp,code[index].var3);
                else//为全局变量
                    fprintf(asmFile,"\tmov _%s,\teax\n",code[index].var3);
            }
        }
        else if(codeOp("scanf")) {
			read = search(localList,code[index].var3);
			if(read == NULL)read = search(tempList,code[index].var3);
			if(read != NULL){
                if(codeVar1("int"))
                    fprintf(asmFile,"\tinvoke crt_scanf,SADD(\"%%d\"),addr dword ptr[ebp+(%d)]\n",read->add);
                else if(codeVar1("char"))
                    fprintf(asmFile,"\tinvoke crt_scanf,SADD(\"%%c\"),addr dword ptr[ebp+(%d)]\n",read->add);
			}else{
			    if(codeVar1("int"))
                    fprintf(asmFile,"\tinvoke crt_scanf,SADD(\"%%d\"),addr _%s\n",code[index].var3);
                else if(codeVar1("char"))
                    fprintf(asmFile,"\tinvoke crt_scanf,SADD(\"%%c\"),addr _%s\n",code[index].var3);
			}
		}
        else if(codeOp("printf"))  {
            fprintf(asmFile,"\tpush eax;print begin\n");
			if(!codeVar1("\0")){//有字符串的情况先打印字符串
                char buf[100];
                strcpy(buf,code[index].var1);
                fprintf(asmFile,"invoke crt_printf,ADDR literal(\"%s\")\n",buf);
			}
			if(codeVar3("1")){
                char charOut[2]={char(atoi(code[index].var2))};
                fprintf(asmFile,"invoke crt_printf,ADDR literal(\"%s\")\n",charOut);
			}
			else if(!codeVar2(" ")){//打印的东西有表达式
				read = search(localList,code[index].var2);//在局部变量表中赵
				if(read == NULL)read = search(tempList,code[index].var2);
				if(read != NULL){
                    fprintf(asmFile,"\tmov\teax,\tdword  ptr [ebp+(%d)]\n",read->add);
                    if(read->type==0)
                        fprintf(asmFile,"\tinvoke crt_printf,SADD(\"%%d \"),eax\n");
                    if(read->type==1)
                        fprintf(asmFile,"\tinvoke crt_printf,SADD(\"%%c \"),eax\n");
				}else{//在局部变量和临时变量表中找不到，要么是全局变量要么是数组。
				    read = search(globalList,code[index].var2);
                    if(read == NULL){
                        if((code[index].var2[0]>='0'&&code[index].var2[0]<='9') ||
                           ((code[index].var2[0]=='-'||code[index].var2[0]=='+')&& code[index].var2[1]>='0'&&code[index].var2[1]<='9')){
                            fprintf(asmFile,"\tmov\teax,\t%s\n",code[index].var2);
                            fprintf(asmFile,"invoke crt_printf,SADD(\"%%d \"),eax\n");
                        }
                        else{
                            fprintf(asmFile,"\tmov\teax,\t\'%s\'\n",code[index].var2);
                            fprintf(asmFile,"invoke crt_printf,SADD(\"%%c \"),eax");
                        }
                    }else{
                        int arrSize,i;
                        char temp[30];
                        strcpy(temp,code[index].var2);
                        arrSize = strlen(temp);
                        for(i=0;i<arrSize;i++)
                            if(temp[i]=='[')
                            break;
                        if(i==arrSize){
                            fprintf(asmFile,"\tmov\teax,\t_%s\n",code[index].var2);
                            if(read->type==0)
                                fprintf(asmFile,"\tinvoke crt_printf,SADD(\"%%d \"),eax\n");
                            if(read->type==1)
                                fprintf(asmFile,"\tinvoke crt_printf,SADD(\"%%c \"),eax\n");
                        }
                        else{//是数组
                            varNode* readtemp=read;
                            char arrIndex[30];
                            int j,k=0;
                            for(j=i+1;j<arrSize;j++){
                                if(temp[j]!=']')
                                arrIndex[k++]=temp[j];
                            }
                            arrIndex[k]='\0';
                            getArrIndex(arrIndex);
                            for(;i<arrSize;i++)
                                temp[i]='\0';
                            int d=getArrBase(temp,1);
                            strcat(temp,"[0]");
                            if(d==-1)//全局数组变量
                                readtemp = search(globalList,temp);
                            else
                                readtemp=search(localList,temp);
                            if(readtemp->type==0)
                                fprintf(asmFile,"\tinvoke crt_printf,SADD(\"%%d \"),eax\n");
                            if(readtemp->type==1)
                                fprintf(asmFile,"\tinvoke crt_printf,SADD(\"%%c \"),eax\n");
                        }
                    }
                }
			}
			fprintf(asmFile,"invoke crt_printf,SADD(\"%%c\"),10\n");
			fprintf(asmFile,"\tpop eax;print over\n");
		}
		else if(codeOp("call"))  {
		    char tempstr[30];
			strcpy(tempstr,code[index].var3);
			fprintf(asmFile,"\tcall\t_%s\n",code[index].var1);
			fprintf(asmFile,"\tadd\tesp,\t%d\n",4*paraCounter);
			paraCounter = 0;
			//如果函数的返回值是需要的
			if(index<codeNum && strcmp(tempstr," ") != 0 ){//函数返回的临时变量是新生成的，需要重新申请空间
                int add=(-1)*4*(++counter);
                int tmpTyp=0;
                if(codeVar2("char"))
                      tmpTyp=1;
                newTmpVal(add,tmpTyp,tempstr);
			}
			else{
                index++;
                continue;
            }
		}
		else if(codeOp("mov"))  {
			int temptype=0;
			findAllList(1);
			//再把寄存器中的数据放到第3个操作数中
			read = search(localList,code[index].var3);
			if(read == NULL)read = search(tempList,code[index].var3);
			if(read != NULL)
			    fprintf(asmFile,"\tmov\tdword  ptr [ebp+(%d)],\teax\n",read->add);
            else{
                read = search(globalList,code[index].var3);
                if(read != NULL) {
                    if(!codeVar2(" ")){//全局定义的数组
                        int arrSize,i;
                        char temp[30];
                        strcpy(temp,code[index].var3);
                        arrSize = strlen(temp);
                        for(i=0;i<arrSize;i++)
                            if(temp[i]=='[')
                                break;
                        for(;i<arrSize;i++)
                            temp[i]='\0';
                        getArrIndex(code[index].var2);
                        strcpy(code[index].var3,temp);
                        getArrBase(temp,2);
                    }
                    else //普通全局变量
                        fprintf(asmFile,"\tmov\t_%s,\teax\n",code[index].var3);
                }
                else if(read == NULL)
                    newTmpVal((-1)*4*(++counter),0,code[index].var3);
            }
		}
		else if(codeOp("ret"))  {
            if(strcmp(funcName,"main")==0)
                fprintf(asmFile,"   invoke ExitProcess, 0; exit with code 0\n");
			if(!codeVar3(" "))  {
				read = search(localList,code[index].var3);
				if(read != NULL)// 如果是局部变量
				    fprintf(asmFile,"\tmov\teax,\tdword  ptr [ebp+(%d)]\n",read->add);
				else{
					read = search(tempList,code[index].var3);
					if(read != NULL) {// 返回临时变量
					    fprintf(asmFile,"\tmov\teax,\tdword  ptr [ebp+(%d)]\n",read->add);
						read->type=funcTyp;
					}
					else{//全局
					    read =  search(globalList,code[index].var3);
					    if(read==NULL)
                            fprintf(asmFile,"\tmov\teax,\t%s\n",code[index].var3);
                        else
                            fprintf(asmFile,"\tmov\teax,\t_%s\n",code[index].var3);
					}
				}
			}
            fprintf(asmFile,"\tmov\tesp,\tebp\n");
            fprintf(asmFile,"\tpop\tebp\n");
            fprintf(asmFile,"\tpop\tebx\n");
            if(!(strcmp(code[index+1].op,"end")==0&&strcmp(code[index+1].var3,"main")==0))
                fprintf(asmFile,"\tret\n");
            funcTyp=-1;//保险起见把函数的返回属性重置一下
		}
        else if(codeOp("jnge")||codeOp("jle")||codeOp("jnle")||codeOp("jge") ||codeOp("jz") ||codeOp("jnz") )  {
            findAllList(2);
            fprintf(asmFile,"\tmov\tebx,\teax\n");
            findAllList(1);
            fprintf(asmFile,"\tcmp\teax,\tebx\n");
            fprintf(asmFile,"\t%s\t%s\n",code[index].op,code[index].var3);
		}
		else if(codeOp("[]=")){
		    findAllList(2);
            getArrIndex(code[index].var1);
            getArrBase(code[index].var3,2);
		}
		else if(codeOp("=[]")){
            getArrIndex(code[index].var2);
            getArrBase(code[index].var1,1);
            int t=0;
            if(read!=NULL)t=read->type;
		    read = search(localList,code[index].var3);
            if(read == NULL)read = search(tempList,code[index].var3);
            if(read != NULL)
                fprintf(asmFile,"\tmov\tdword  ptr [ebp+(%d)],\teax\n",read->add);
            else{
                read=search(globalList,code[index].var3);
                if(read==NULL)
                    newTmpVal((-4)*(++counter),t,code[index].var3);
                else//是全局变量
                    fprintf(asmFile,"\tmov\t_%s,\teax\n",code[index].var3);
            }
		}
        index++;
    }
}

void genAsm(){
    asmFile=fopen("target.asm","w+");
    asmHead('e');
    dataSeg();
    codeSeg();
    fclose(asmFile);
}
