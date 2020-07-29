# Principle_of_Compilation_Experiment
这是我们编译原理最后的大作业。  
  
1. 本身提供了一个C语言编写的 PL/0 编译程序，要求在解读PL/0编译程序的基础上，学习PL/0语言语法规定、程序结构、词法分析、语法分析和目标代码结构、代码生成、语法错误处理和目标代码在执行时的存储分配。进一步理解编译程序中的各个组成模块，并且能够对编译的过程有个整体的把握。  
2. 根据要求实现对 PL/0 的语言功能进行扩充，对C语言编写的PL/0语言编译程序修改，完成扩充功能的对应编译功能。  
3. 在C语言编译环境下，上机调试修改后的PL/0语言编译程序，编译通过并运行。给出扩充后 PL/0 测试用例的源程序、目标程序和运行结果。

## PL/0编译程序
### PL/0程序数据结构
#### 全局变量定义
```C
# define norw 13            /*关键字个数*/
# define txmax 100          /*名字表容量*/
# define nmax  14           /*number的最大位数*/
# define al 10              /*符号的最大长度*/
# define amax 2047          /*地址上界*/
# define levmax 3            /*最大允许过程嵌套声明层数[0，lexmax]*/
# define cxmax 200          /*最多的虚拟机代码数*/
# define symnum 32          /*符号个数*/
# define fctnum 8           /*指令个数*/
FILE * fas;  /*存放名字表内容文件*/
FILE * fa; /*存放编译生成的类-Pcode代码文件*/
FILE * fa1; /*存放源程序文件*/
FILE * fa2; /*存放类P-code翻译之后输出的最终结果*/
bool tableswitch; /*是否选择显示名字表*/
bool listswitch;  /*是否选择显示类P-code代码*/
char ch; /*记录当前所取的字符*/
enum symbol sym; /*当前单词的标志类别*/
char id[al+1]; /*当前所取的单词*/
int  num; /*当前所取的数*/
int cc,ll;/* getch使用的计数器，cc表示当前字符(ch)的位置*/
int cx; //虚拟机代码指针
char line[81]; //读取行缓冲区
char a[al+1]; //临时单词符号
struct instruction code[cxmax]; //存放虚拟机代码的数组
char word[norw][al]; //保留字
enum symbol wsym[norw]; //保留字对应的符号值
enum symbol ssym[256]; //单字符的符号值
char mnemonic[fctnum][5]; //虚拟机代码指令名称
bool declbegsys[symnum]; //表示开始的符号集,declaring begin symbol set
bool statbegsys[symnum]; //表示语句开始的符号集合 statment
bool facbegsys[symnum]; //表示因子开始的符号集合 factor
struct tablestruct table[txmax];   /*名字表*/
FILE * fin;   /*输入文件流*/
FILE* fout;  /*输出文件流*/
int err;     /*错误计数器*/
int p; /*指令地址寄存器*/
int b; /*基址寄存器*/
int t；/*栈顶寄存器*/
````


#### 数据结构定义
枚举类型，表示单词种类，源程序中总共有32种：  
```C
enum symbol{ nul,    ident,…… varsym,  procsym,}
```
枚举类型，表示变量类型，这里提供的是常量、变量、过程三种：  
```C
enum object{ constant,variable,procedur,};
```
枚举类型，表示指令类型，这里提供了8种类型的指令：  
```C
enum fct{lit, opr,  lod,  sto,  cal,  inte,  jmp,  jpc,};
```
结构体类型，指令寄存器，指令的结构定义由指令类型f、层数l、和a构成，a在不同指令表示不同含义：  
```C
struct instruction{
         enum fct f;
         int l;
         int a;};
```
结构体类型， 名字表的结构定义由变量名字、变量类型、变量值、所处层数、地址、和分配的数据区空间构成：   
```C
struct tablestruct{
             char name[al]; /*名字*/
             enum object kind;  /*类型：const，var，array or procedure*/
             int val;       /*数值，仅const使用*/
             int level;      /*所处层，仅const不使用*/
             int adr;        /*地址，仅const不使用*/
             int size;     /*需要分配的数据区空间，仅procedure使用*/
};
```
运行栈：整数数组  
```C
int s[stacksize]
```

#### 数据结构定义
```C
#define getsymdo               if(-1==getsym())return -1
#define getchdo                if(-1==getch())return -1
#define testdo(a,b,c)          if(-1==test(a,b,c))return -1
#define gendo(a,b,c)           if(-1==gen(a,b,c))return -1
#define expressiondo(a,b,c)    if(-1==expression(a,b,c))return -1
#define factordo(a,b,c)        if(-1==factor(a,b,c))return -1
#define termdo(a,b,c)          if(-1==term(a,b,c))return -1
#define conditiondo(a,b,c)     if(-1==condition(a,b,c))return -1
#define statementdo(a,b,c)     if(-1==statement(a,b,c))return -1
#define constdeclarationdo(a,b,c) if(-1==constdeclaration(a,b,c))return -1
#define vardeclarationdo(a,b,c)   if(-1==vardeclaration(a,b,c))return -1
/*当函数中会发生fatal error时，返回－1告知调用它的函数，最终退出程序*/
```

### PL/0函数
