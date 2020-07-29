                         /*PL/0 编译系统C版本头文件 pl0.h*/

# define norw 17                 /*关键字个数*/
# define txmax 100               /*名字表容量*/
# define nmax  14                /*number的最大位数*/
# define al 10                   /*符号的最大长度*/
# define amax 2047               /*地址上界*/
# define levmax 3                /*最大允许过程嵌套声明层数[0，lexmax]*/
# define cxmax 200               /*最多的虚拟机代码数*/
/*符号*/
enum symbol{
	 nul,    ident,     number,     plus,     minus,
	 times,   slash,   oddsym,   eql,     neq,//slash?? times?? 
lss,     leq,      gtr,     geq,     lparen,
rparen,  comma,   semicolon,period,  becomes,//peroid句号 
beginsym, endsym, ifsym,    thensym,  whilesym,//调用if then处理条件语句 
writesym, readsym,  dosym,  callsym,  constsym,
varsym,  procsym,lquotation,colon,arraysym,
leftbracket,rightbracket,forsym,stepsym,untilsym,
//设置左引号，右引号 冒号：定义数据类型 
//设置右括号  for循环处理 
//增加数组的处理 leftbracket 左括号 rightbracket 右括号 
};
#define symnum 40
/*-------------*/
enum object{  //五种标识符的类型
   constant,
   variable,
   procedur,
   array, 
};
/*--------------*/
enum fct{//fct类型分别标识类PCODE的各种指令
lit, opr,  lod,  sto,  cal,  inte,  jmp,  jpc,outs,sto1,lod1//outs表示输出字符串指令 sto1存储数组 
};
//sto1用来存储数组元素
//lod1用来加载数组元素 
#define fctnum 12
/*--------------*/
struct instruction//指令
{
  enum fct f;//功能码
  int l;//层次差
  float a;//a在不同的指令中表示不同的含义
};

FILE * fas;//输出名字表
FILE * fa;//输出虚拟机代码
FILE * fa1;//输出源文件及其各行对应的首地址
FILE * fa2;//输出结果

bool tableswitch;//显示名字表与否
bool listswitch;//显示虚拟机代码与否
char ch;//获取字符的缓冲区，getch使用
enum symbol sym;//当前符号
char id[al+1];//当前的ident，多一个字节用于存放0
char st[255]="";//用于保存write函数输出的字符串  
int start=0;//用来记录符号表类型的起始位置 
int endnum=0;//用来记录符号表类型的终止位置 
int  num;//当前number integer数 
float realnum;//当前real数 
int cc,ll;//getch使用的计数器，cc表示当前字符(ch)的位置
int cx;//虚拟机代码指针
char line[81];//读取行缓冲区
char a[al+1];//临时符号
struct instruction code[cxmax];//存放虚拟机代码的数组
char word[norw][al];//保留字
enum symbol wsym[norw];//保留字对应的符号值
enum symbol ssym[256];//单字符的符号值
char mnemonic[fctnum][5];//虚拟机代码指令名称
bool declbegsys[symnum];//表示开始的符号集,declaring begin symbol set
bool statbegsys[symnum];//表示语句开始的符号集合 statment
bool facbegsys[symnum];//表示因子开始的符号集合 factor
int low=0;//数组的下界 
int high=0; //数组的上界 
//*************************
float arraystore[amax];//涉及浮点型数据所以统一用
int arraybound[amax];//用来存储数组的上界，存储的位置为数组的基地址 
int con=0;//数组的游标 
//int arrayadr[amax];//存取数组地址的数据栈 ---多余 
//int adrcon=0;//地址游标 
int readflag=0;//设置读取数组符号 

//*************************
int tailx=0;//名字表的尾指针 
enum numtype{
	real,
	integer,	
};
enum numtype intorreal;//表示当前所取变量的类型--仅在赋值时使用 
struct tablestruct
{
   char name[al];                            /*名字*/
   enum object kind;                         /*类型：const，var，array or procedure*/
   int val;                                  /*数值，仅const使用*/
   int level;                                /*所处层，仅const不使用*/
   int adr;                                  /*地址，仅const不使用*/
   int size;                                 /*需要分配的数据区空间，仅procedure使用*/
   float realnumber;
   enum numtype tp;//数据的类型 
   int base;//base用于存放数组的基地址 
   int low;
   int high; 
};
char ss[amax];  /*字符串栈*/ 
int stail=0;//字符串栈的尾指针 
struct tablestruct table[txmax];             /*名字表*/
FILE * fin;
FILE* fout;
char fname[al];
int err;                                       /*错误计数器*/
/*当函数中会发生fatal error时，返回－1告知调用它的函数，最终退出程序*/
#define getsymdo                              if(-1==getsym())return -1
#define getchdo                               if(-1==getch())return -1
#define testdo(a,b,c)                         if(-1==test(a,b,c))return -1
#define gendo(a,b,c)                          if(-1==gen(a,b,c))return -1
#define expressiondo(a,b,c)                   if(-1==expression(a,b,c))return -1
#define factordo(a,b,c)                       if(-1==factor(a,b,c))return -1
#define termdo(a,b,c)                         if(-1==term(a,b,c))return -1
#define conditiondo(a,b,c)                    if(-1==condition(a,b,c))return -1
#define statementdo(a,b,c)                    if(-1==statement(a,b,c))return -1
#define constdeclarationdo(a,b,c)             if(-1==constdeclaration(a,b,c))return -1
#define vardeclarationdo(a,b,c)               if(-1==vardeclaration(a,b,c))return -1
void error(int n);
int getsym();
int getch();
void init();//初始化函数
int gen(enum fct x,int y,int z);
int test(bool*s1,bool*s2,int n);
int inset(int e,bool*s);
int addset(bool*sr,bool*s1,bool*s2,int n);
int subset(bool*sr,bool*s1,bool*s2,int n);
int mulset(bool*sr,bool*s1,bool*s2,int n);
int block(int lev,int tx,bool* fsys);
void interpret();
int factor(bool* fsys,int* ptx,int lev);
int term(bool*fsys,int*ptx,int lev);
int condition(bool*fsys,int*ptx,int lev);
int forcondition(bool*fsys,int*ptx,int lev,int p,int cx3);
int expression(bool*fsys,int*ptx,int lev);
int statement(bool*fsys,int*ptx,int lev);
void listcode(int cx0);
int vardeclaration(int* ptx,int lev, int* pdx);
int constdeclaration(int* ptx,int lev, int* pdx);
int position(char* idt,int tx);
void enter(enum object k,int* ptx,int lev,int* pdx);
int base(int l,float* s,int b);
int getarraybound(int &loworhigh,int *ptx);//获取数组的上下界函数 
int dealarray(bool*fsys,int*ptx,int lev); 
 

        








