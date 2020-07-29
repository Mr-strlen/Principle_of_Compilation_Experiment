# Principle_of_Compilation_Experiment
这是我们编译原理最后的大作业。  
主要是在原有的PL/0程序上，添加了一位数组变量，for语句，无符号浮点数，打印格式write。  

# 实验要求
1. 本身提供了一个C语言编写的 PL/0 编译程序，要求在解读PL/0编译程序的基础上，学习PL/0语言语法规定、程序结构、词法分析、语法分析和目标代码结构、代码生成、语法错误处理和目标代码在执行时的存储分配。进一步理解编译程序中的各个组成模块，并且能够对编译的过程有个整体的把握。  
2. 根据要求实现对 PL/0 的语言功能进行扩充，对C语言编写的PL/0语言编译程序修改，完成扩充功能的对应编译功能。  
3. 在C语言编译环境下，上机调试修改后的PL/0语言编译程序，编译通过并运行。给出扩充后 PL/0 测试用例的源程序、目标程序和运行结果。

# PL/0编译程序
## PL/0程序数据结构
### 全局变量定义
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


### 数据结构定义
* 枚举类型，表示单词种类，源程序中总共有32种：  
```C
enum symbol{ nul,    ident,…… varsym,  procsym,}
```
* 枚举类型，表示变量类型，这里提供的是常量、变量、过程三种：  
```C
enum object{ constant,variable,procedur,};
```
* 枚举类型，表示指令类型，这里提供了8种类型的指令：  
```C
enum fct{lit, opr,  lod,  sto,  cal,  inte,  jmp,  jpc,};
```
* 结构体类型，指令寄存器，指令的结构定义由指令类型f、层数l、和a构成，a在不同指令表示不同含义：  
```C
struct instruction{
         enum fct f;
         int l;
         int a;};
```
* 结构体类型，名字表的结构定义由变量名字、变量类型、变量值、所处层数、地址、和分配的数据区空间构成：   
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
* 运行栈：整数数组  
```C
int s[stacksize]
```

### 数据结构定义
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

## PL/0函数
这部分大家应该都知道，略过惹|･ω･｀)


## PL/0指令解释
| 指令 | 解释 |
|:----- | :----- |
| LIT 0 A | 立即数存入栈顶，即置T所指存储单元的值为A |
| LOD L A | 将层差为L、偏移量为A的存储单元的值取到栈顶 |
| STO L A | 将栈顶的值存入层差为L、 偏移量为A元的存储单元 |
| OPR 0 1 | 求栈顶元素的相反数，结果值留在栈顶 |
| OPR 0 6 | 栈顶元素的奇偶判断，若为奇数，结果为1；若为偶数，结果为0；结果值留在栈顶 |
| OPR 0 2 | 次栈顶与栈顶的值相加，结果存入次栈顶 |
| OPR 0 3 | 次栈顶的值减去栈顶的值，结果存入次栈顶 |
| OPR 0 4 | 次栈顶的值乘以栈顶的值，结果存入次栈顶 |
| OPR 0 5 | 次栈顶的值除以栈顶的值，结果存入次栈顶 |
| OPR 0 8 | 比较次栈顶与栈顶是否相等，若相等，结果为0 ；存结果至次栈顶； |
| OPR 0 9 | 比较次栈顶与栈顶是否不相等，若不相等，结果为0 ；存结果至次栈顶 |
| OPR 0 10 | 比较次栈顶是否小于栈顶，若小于，结果为0 ；存结果至次栈顶 |
| OPR 0 11 | 比较次栈顶是否大于等于栈顶，若大于等于，结果为0 ，存结果至次栈顶 |
| OPR 0 12 | 比较次栈顶是否大于栈顶，若大于，结果为0，存结果至次栈顶； |
| OPR 0 13 | 比较次栈顶是否小于等于栈顶，若小于等于，结果为0 ；存结果至次栈顶； |
| JMP 0 A | 无条件转移至地址 A ，即置指令地址寄存器为A |
| JPC 0 A | 若栈顶为0，则转移至地址A，即置指令地址寄存器为A  |
| OPR 0 14 | 栈顶的值输出至控制台屏幕 |
| OPR 0 15 | 控制台屏幕输出一个换行 |
| OPR 0 16 | 从控制台读入一行输入，置入栈顶 |


# 主程序及各个函数程序流程图
太多了，就直接看`Flow Chart`文件夹吧  

# 功能扩充
这部分是大作业的关键，扩充了一些新的功能。  
## 数组功能扩充
### 功能需求
**增加整型一维数组变量**  
  
1. 其声明形式表示为：VAR 〈数组标识名〉[〈下界〉：〈上界〉]  
〈下界〉和〈上界〉可用常量标识名,上界必须大于或等于下界。(或 var a,b,c:array[1..100] of integer;)  
  
2. 在程序中按如下格式使用数组中的数据：  
数组名[<下标表达式>]
其中，<下标表达式>指整型表达式,可以是常量、变量、数字，其值必须在上界和下界之间，包括上界和下界。执行过程是首先计算下标表达式的值，然后再引用数组元素。  
  
例如:  
声明数组如下：
```Pascal
const up:=5 ；
var
a:array [2:up] of integer；
b:integer;
```
在程序中使用数组中元素如下：
``` Pascal
b:=3;
a[2]:=3;
a[b]:=4;
a[2+b]:=a[b]+a[b+1];
```
但不能出现如下的使用 a[1]和 a[6]，我们可以称这种错误为数组下标越界。  
下标变量可以出现在表达式中。  
  
3. 可以通过 read 读下标变量的值：
``` Pascal
read(a[b]);
```
  
4. 可以通过 write 输出下标变量的值：
```Pascal
write(a[2+b]);
```
### 数组语法描述图
图片有点多，这块就略过|･ω･｀)
### BNF语法描述
```
<类型符>::= integer|real
<无符号整数>::= <数字>{<数字>}
<标识符>::= <字母>{<字母>|<数字>}
<数组说明>::=var<标识符>{,<标识符>}：array‘[’<无符号整数>..<无符号整数>‘]’ of <类型符>;
```
### 设计过程
#### 扩展symbol
增加的symbol为：arraysym（数组标志）,leftbracket（左中括号）,rightbracket（右中括号），colon（冒号）
#### 更改数据结构
* 增加数据类型：array
```C
enum object{  
   constant,
   variable,
   procedur,
   array, 
};
```
* 名字表中增加基地址、数组下界、数组上界
```C
struct tablestruct
{
   char name[al];              /*名字*/
   enum object kind;           /*类型：const，var，array or procedure*/
   int val;                   /*数值，仅const使用*/
   int level;                 /*所处层，仅const不使用*/
   int adr;                   /*地址，仅const不使用*/
   int size;                /*需要分配的数据区空间，仅procedure使用*/
   enum numtype tp;       //数据的类型 
   int base;             //base用于存放数组的基地址 
   int low;             //数组的下界
   int high;           //数组的上界
};
```
* 增加数组栈
```C
float arraystore[amax];//涉及浮点型数据所以统一用
```
* 增加地址记录栈
```C
int arraybound[amax];//用来存储数组的上界，存储的位置为数组的基地址，用于对数组越界的判断。下界存放在指令里。
```
* 增加数组栈的游标，以分配空间
```C
int con=0;//数组的游标
```
* 增加读数组的标志，以实现read数组的功能
```C
int readflag=0;//设置读取数组符号
```
* 增加start,endnum，当出现类似var a,b,c:array[1..100] of integer时,标记申明数组名字表中开始的位置和结束位置，以便在读到integer时对他们进行类型的赋值。
* 增加low，high代表增加数组的低地址和高地址。
* 增加arraypos临时存储数组在名字表中的位置，用于对数组进行赋值表达式计算处理。
#### 增加指令
* sto1 l a  
其中l数组的低地址，a代表数组的基地址，实现的功能是以a为基地址，次栈顶为偏移地址，栈顶为需要存到数组里的值，将栈顶的内容存到数组里。如下，i表示指令，s表示栈，arraystore表示数组栈，t表示当前栈顶：
```C
arraystore[int(i.a)+int(s[t-1])]=s[t];
```
同时对数组是否越界进行判断，s[t-1]为要存储的数组地址，通过将其余i.l（数组的低地址）比较以及通过基地址在arraybound里找高地址的方法，在进行上界的判断，如果不满足条件，则error（39），数组越界错误！。  
* lod1 l a  
其中l代表数组的低地址，a代表数组的基地址，实现的功能是以a为基地址，栈顶为偏移地址，将数组里的内容取到栈顶偏移地址的位置。如下，i表示指令，s表示栈，arraystore表示数组栈，t表示当前栈顶：  
```C
s[t]=arraystore[int(i.a)+int(s[t])];
```
跟上条指令一样，也需要对数组的上下界进行判断，不同的是要操作的数组的地址为s[t]。
#### 增加函数
获取数组的上下界函数，参数为数组的上界或下界
```C
int getarraybound(int &loworhigh，int *ptx);//获取数组的上下界函数
```
处理数组的函数，参数为符号集、名字表尾指针、层数
```C
int dealarray(bool*fsys,int*ptx,int lev);
```
### 实现过程
具体的流程图去看文件夹啦|･ω･｀)
#### 数组的申明
**举例：**   
var a,b,c:array[1..100] of integer  
**实现思路：**  
  
首先需要识别出varsym标志，对于var后面的标识符a,b,c加入到名字表中，记录这些变量在名字表中的起始位置(start)和终止位置(endnum)，以便后面对a,b,c进行数据类型赋值。  
识别出冒号之后，如果后面是数据类型符integer 或real，则a,b,c只是普通变量。如果识别是array，则申明为数组类型，读取下一个字符，如果不是‘\[’，则报错，缺少左中括号，否则，再往下读取上界和下届，利用getarraybound进行计算，之后再读取‘\]’。  
接下来就是对数组上下界的检查，是否上界要满足大于等于下界，如果不满足，则error（38）。读到of时，如果id！=of，则报错，如果不是，则读取数据类型符，并对从start到endnum开始的名字,赋值类型，数据类型，上下界，基地址，数组游标等。  
数组游标的计算时每次先把当前值赋值给数组，当做基地址，后加上该数组的空间大小，作为下个数组的基地址。  
#### 数组的赋值
**实现思路：**  
  
首先判断当前变量种类是否为数组类型，如果是，则先保留数组的位置arraypos=i,接着获取左中括号，如果不是，则报错，缺少左中括号。  
之后读取中括号内的第一个单词，调用表达式处理中括号内的内容，接下去读右括号和赋值符号。  
当为赋值符号时，调用dealarray函数处理赋值右边的表达式，处理完之后，生成存储数组内容的指令，sto1,l,a。  
  
**右边表达式处理思路：**  
  
判断当前的符号为标识符还是数字，如果是标识符，则需要查找名字表中该标识符的信息，未查到则报错。  
如果改标识符的类型是数组类型，则需要记录当前数组的位置。  
getsymdo获取左’\[’，之后调用表达式程序处理中括号的内容，处理完之后，此时栈顶为表达式的值，生成取数组元素的内容lod1（前面有关于该指令的解释）。  
再去下一个符号，如果是加减乘除等，则递归调用dealarray进行处理。  
处理完之后生成相应的指令，如果当前标识符是varaible时，则只需lod即可，再读取下一个符号，递归处理。  
遇到逗号，分号，右括号结束。  
如果当前sym是number标志，则直接加载立即数即可，判断数据类型是real还是integer生成对应的指令。  
之后再读取下一个符号，递归处理。 
#### 读入数组值
**处理思路**  
  
判断当前符号是否为读入标志，如果是，则继续读取左括号，继而读取括号里的内容。  
如果是标识符，则查找名字表获取位置，判断类型，如果是数组类型，则置readflag=1，调用dealarray处理数组表达式得到地址，生成输入指令得到值。  
调用gendo生成sto1指令，将读入的数存进数组。
#### 输出数组值
**处理思路：** 
  
判断当前符号是否为writesym标志，如果是，则继续读取左括号，继而读取括号里的内容。  
如果是标识符，则查找名字表获取位置，判断类型，如果是数组类型，调用dealarray处理数组表达式将数组的值调到栈顶，调用gendo生成相应的输出指令。  
如果是variable，则调用experssion处理。


## for循环
### 功能需求
for 语句的语法格式为：  
for <循环变量>：=<初值> step <增量> until <终值> do <语句>；  
<初值> <增量> <终值>为表达式,其中，循环体语句可以是任何单个语句或复合语句。
  
例如:定义 for 语句如下：
```Pascal
var i；
for i:=0 step 1 until 10 do
write(i);
```
程序的执行过程如下：
1. 先给变量i赋值为 0；
2. 检察变量i是否等于until后的值10，如果等于10结束for语句。如果不等于 10 转为③继续执行。
3. 执行do后的语句打印i的值：0，
4. 变量增加step后的值1 ，变量i的值变为2，转到②。  
for 语句的执行结果为：  
输出 0123456789
### 数组语法描述图
图片略过|･ω･｀)
### BNF的语法描述
```
<for循环>::=for<标识符>:=<表达式>step<表达式>until<表达式>do<语句>;
<语句>::= <赋值语句>|<条件语句>|< 型循环语句>|<过程调用语句>|<读语句> |<写语句>|<复合语句>|<空>
<赋值语句>::= <标识符>:= <表达式>
<复合语句>::= BEGIN <语句> ;<语句>}END
<表达式>::= [+|-]<项>{<加法运算符><项>}
<项>::= <因子>{<乘法运算符><因子>}
<因子> ::= <标识符>|<无符号整数> |‘（’< 表达式> ‘）’
<标识符>::= <字母>{<字母>|<数字>}
```
### 设计过程
#### 扩展symbol
增加的symbol为：forsym（循环标志）,stepsym（处理step）,until（处理until）。
#### 增加函数
增加int forcondition(bool * fsys,int * ptx,int lev,int p,int cx3);处理for循环，fsys为符号集合，ptx为名字表尾指针，lev表示层数，cx3为指令代码的尾指针。
