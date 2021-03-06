# 编译原理课程设计报告

曾俊浩 贺情怡 范安东

[toc]

## 0 总体介绍

我们使用的工具如下：

* CMake 3
* LLVM 12.0.0
* flex
* bison

我们的分工如下：

* 曾俊浩：负责词法分析、语法分析、语义分析、解析树生成、错误提示实现
* 范安东：负责 LLVM IR 生成、函数式编程实现
* 贺情怡：负责语言设计、测试程序编写

## 1 语言

### 1.1 语言描述
我们设计了一个类 C 的编程语言 CMM（C--），支持以下特性：

* 简单语义分析
    - 属性计算
    - 符号表
    - 类型检查
* 全局/局部变量定义
* 函数定义与调用
* 算数/逻辑表达式求值
* 控制流语句（`if`, `loop`, `break`, `continue`, `return`...）
* 任意维度的数组
    - 支持复杂数组下标  `a[b[c[x]]][d[y]]`
* 类 C 的输入输出接口
* 闭包
    - 支持值捕获
* 高阶函数（支持传入 int -> int 函数作为函数参数）

附录中将提供 CMM 语言 BNF 文法，以及一段样例代码的词法分析、解析树、符号表。

### 1.2 词法
主要使用lex工具完成编译器的词法分析工作，具体代码见scanner.l

即把字符串转换成为相应的token

#### 变量名
我们的语言中对变量名的规定如下：
- 函数类型变量名（包括函数名）以字符'_'开始，后面为至少一个字母
- 其余类型变量名由至少一个字母构成

#### 运算符
我们的语言将以下符号识别为运算符：
- 逻辑运算符： || && == <= >= != < >
- 算术运算符： + - * / %
- 赋值运算符： =

#### 关键字
我们的语言中定义了如下关键字：
- 类型：int void function fn
- 流程控制： if loop break continue return
- 代码组成： do done declare
- 标准函数: _input _output

#### 注释
我们的语言采用类C的注释风格 /* this is a comment */

使用正则表达式"/*"([^\*]|(\*)*[^\*/])*(\*)*"*/"来识别注释

#### 数字、字符和字符串
我们的语言支持整数常量、字符常量，可以识别一个整数、由一对单引号包裹的单个字符、由一对单引号包裹的转义字符作为int类型的常量

字符将会以ASCII码的方式以整数形式存储，包括'\t', '\n'等转义字符

我们的语言暂未支持整个字符串的操作，但是依旧可以使用正则表达式\"(\\.|[^"\\])*\"来识别由一对双引号包裹的0个或多个字符、转义字符的序列作为一个字符串常量

### 1.3 语法与解析树（parse tree）
主要使用yacc工具完成编译器的语法分析工作，具体代码见parser.y
即把token转换成为对应的语法树节点，并建立一棵解析树

整个程序语言由以下模块组成，每个模块中有若干条相应语句
```
|- Program
   |- Def_List
   |  |- Def
   |
   |- Fun_Def_List
      |- Fun_Def
         |- Def_List
         |- Exp_List
```

#### 解析树
每个程序在编译过程中都会依照语法定义生成一颗解析树，
解析树的每个节点中主要存储以下信息：
- 节点ID：用于可视化时识别节点
- 节点类型：主要分为列表类节点与普通节点
- 子节点列表：用于存储该节点的下属节点
- 节点参数：名称、数据类型、运算符类型等

列表类节点的典型代表为变量定义列表（Def_List）
```
Def_List        : Def_List Def ';' {$$=$1; $$->Insert($2);}
                | {$$=new AST(Type::list, "Def_List");}
                ;
```
该类节点拥有若干个相同类型的孩子，如变量声明、函数参数传递、语句块等等

普通节点拥有固定数量的孩子，每个孩子所表达的含义也早已约定好，
如条件语句节点（If_Stmt）
```
If_Stmt         : IF '(' Cond_Exp ')' DO Exp_List DONE {$$=new AST(Type::expr, "If_Stmt"); $$->Insert($3); $$->Insert($6);}
                ;
```
该节点共有两个孩子，一个为条件语句，另一个为待执行语句块


### 1.4 语义与符号表
主要使用两次遍历来完成对代码语义的检测工作，具体代码见AST.cpp
#### 变量表与函数表
每个函数拥有一张变量表和一张函数表，另外还有一张全局变量表和一张全局函数表

变量表中存储以下信息：
- 变量名：唯一字段，用于在同一作用域中唯一确定一个变量
- 变量数据类型：表示变量的类型，同时用于传递错误
- 变量归属：用于表示变量的作用域
- 变量维度：用于向量型变量，存储维度

函数表中存储以下信息：
- 函数名：唯一字段，用于在同一作用域中唯一确定一个函数
- 函数返回类型：表示函数返回值的类型，同时用于传递错误
- 函数参数表：存储函数参数的数据类型与名称
- 函数归属：用于表示函数的作用域
- 下属变量表与函数表：用于查找定义在该函数内的变量与函数
- 父函数变量表：用于查找定义于该函数同级的变量

#### 语义检查
首先对解析树进行先序遍历，根据节点类型（主要是定义类型节点）和节点数据建立若干张变量表与函数表；
生成符号表中各项的类型、作用域等，在此过程中也会检测是否有变量、函数的重复定义

第二次遍历则是根据已生成的符号表，检测变量、函数的使用是否符合类型要求

这两次遍历完成了对代码语义的审查，可以认为通过了语义检查的代码是完全符合语言要求的

## 2 中间代码与可执行文件生成

我们使用 LLVM 作为编译器的后端，因此我们需要按照解析树生成 LLVM IR。生成 LLVM IR 后，直接用 clang 将 LLVM IR 编译为目标架构的汇编，再编译汇编到可执行文件即可。

LLVM IR 生成的过程实际上就是解析树的遍历过程。从解析树根开始，依次访问每一个子节点，然后递归地生成子节点的 IR。生成 IR 全部通过调用 LLVM IRBuilder 等各种 API 实现。

报告篇幅有限，在此仅大致描述 LLVM IR 生成。

* `Program` 生成

  * 程序由全局变量定义与函数定义组成。因此生成全局变量定义 IR 代码后逐个生成函数代码即可。

* `Func` 生成

  * 函数由函数类型声明、函数名、函数参数表、局部变量定义、函数体语句组成。

  * 生成函数时，首先用函数类型声明、函数名、函数参数表生成函数签名，

    然后为函数参数生成局部变量，并将函数参数值拷贝进局部变量中。然后逐个生成局部变量定义代码。

  * 函数体由一系列 `Expr` 组成，逐个生成即可。

* `Expr` 生成

  `Expr` 种类丰富，其对应了 BNF 中的 Exp。简单来说，`Expr` 可以分为以下几类：

  * 赋值语句

    * 直接生成 store 代码即可。

  * 变量访问语句

    * 访问单一变量时，直接生成 load 代码即可。
    * 访问数组时，需要将多维数组下标展开成一位数组下标，然后 load 数组对应位置。

  * 函数调用语句

    * 根据函数名找到对应的函数，创建对应的参数向量，生成函数调用的 IR 即可。

  * 逻辑/算数运算语句

    * 对于运算表达式的代码生成，只需要生成 LHS 的代码，再生成 RHS 的代码，然后生成将 LHS 与 RHS 进行对应的运算操作的代码即可。
    * 递归生成 LHS / RHS 时，会先生成 Factor 的代码，再生成 Term 的代码，最后生成 Expr 的代码，这样保证了运算的优先级顺序。

  * 控制流语句

    控制流语句分为分支语句、循环语句与循环控制语句。其中分支语句与循环语句中包含一个 `Expr` 列表。

    * 对于分支语句与循环语句，需要生成新的 LLVM BasicBlock，分别是条件判断 Block 与语句 Block。
      * 分支结构中，条件判断是否进入语句 Block。语句 Block 执行结束后离开分支结构。
      * 循环结构中，条件判断后决定进入语句 Block 或离开循环结构。在语句 Block 执行结束后回到条件判断 Block。
    * 对于循环控制语句，根据需要进入条件判断 Block（continue）或直接离开循环结构（break）。

* 全局与局部变量生成

  * 对于全局变量，直接生成全局变量的 IR 代码即可。
  * 对于局部变量，需要在栈上申请空间（alloca），获得栈上变量的指针。

## 3 进阶主题

### 3.1 错误提示
编译错误时如果没有提示，需要从整个代码里寻找错误之处，一个好的编译器需要能提示编译错误出现的地点，更好的则是提出改正的方式，所以错误提示对于一个编译器是十分重要，甚至是必备的

错误提示在词法检测、语法检测、语义检测中均有出现：
#### 词法错误
词法错误提示在词法分析时给出；
词法错误的原因一般是出现了不能识别的字符串，不能被归类于任何一种token；此时会提示出现错误的行号

#### 语法错误
语法错误提示在生成解析树时给出；
语法错误的原因大多没有按照语法规则进行代码书写，如关键字错误；此时会提示出现错误的行号与出错的文本
#### 语义错误
语义错误提示在生成符号表或利用符号表检查代码时给出；
语义错误的种类多样，主要包括运算符类型不匹配、函数参数类型不匹配等；此时会给出提示出现错误的类型
### 3.2 简易函数式编程

由于项目时间与规模有限，我们只选择了实现函数式编程的两个关键特性：闭包与高阶函数。

#### 3.2.1 闭包

##### 闭包

如果函数 f 内定义了函数 g，那么如果 g 存在自由变量（g 中未定义的变量），且这些自由变量没有在编译过程中被优化掉，那么将产生闭包。闭包需要捕获自由变量。下面一个 CMM 代码实例将比较清晰地展示闭包的功能。

```shell
function void _closureTest(void)
declare
    int x ;
do
    x = 1 ;
    function int _closure(int y)
    declare
    do
        x = x + y ;
        return x ;
    done
done
```

代码中在函数 `_closureTest` 中定义了一个函数 `_closure`，`_closure` 中存在自由变量 `x`，则 `_closure` 成为一个闭包，其将捕获自由变量 `x`。尝试在主函数中调用 `_closure`。

```
_closureTest();
_closure(4); // 5
_closure(5); // 10
_closure(6); // 16 
```

`_closureTest` 被调用后产生了一个闭包 `_closure`。调用 `_closure`， 传入参数 `4`、`5`、`6`，`_closure` 函数将分别返回 `5`、`10`、`16`。可见，即使 `_closureTest` 中 `x` 的生命周期已经结束，闭包 `_closure` 仍能对 `x` 的值进行操作。

##### 语法与语义

为了支持函数的嵌套定义，我们需要改造语法。

Fun_Def:    "function" FUN_TYPE Fun_ID "("Fun_Var_List")" "declare" Def_List "do" ***Exp_List*** "done"

Exp_List:    Exp_List ***Exp*** | ***Exp***

Exp:     As_Exp ";" | Op_Exp ";" | Cond_Exp ";"

​            | If_Stmt | Lop_Stmt

​            | "break"";" | "continue"";" | "return" Op_Exp";" | "return" ";"

​            | Input_Exp | Output_Exp | ***Fun_Def***

语法中，Fun_Def 内有语句列表 Exp_List，列表中的语句 Exp 可以是一个 Fun_Def。

##### 中间代码生成

生成中间代码时，我们使用了 Lambda Lifting 这一技术。

> Lambda lifting is a meta-process that restructures a computer program so that functions are defined independently of each other in a global scope.
>
> * An individual lift transforms a local function into a global function. It is a two step process, consisting of;
>   * Eliminating free variables in the function by adding parameters.
>   * Moving functions from a restricted scope to broader or global scope.

Lambda Lifting 将程序中所有函数组织成在全局作用域中各自独立定义的结构。对于局部定义的函数，我们需要消除其自由变量，并将其移动到全局作用域中。

在消除自由变量前，我们需要捕获它们，这一步骤由语义分析完成。语义分析将返回给编译器中端自由变量的所有名字。为了让闭包能访问被捕获的变量，我们将被捕获变量“提升”到全局作用域，并在生成闭包时将被捕获变量的值拷贝到提升后的捕获变量中。在闭包中对被捕获变量的访问都通过提升后的变量进行。这样的实现可以保证闭包能在原自由变量的生命周期结束后，仍然可以通过访问提升变量的方式来对捕获值进行操作。同时，因为被捕获的值是仅供此闭包内部使用的，在编译器中端也会对提升变量的访问进行控制。

下面的一段 LLVM IR 代码可以比较清楚地展现闭包的实现原理。

```
@x_prm = common global i32 0

define i32 @_closure(i32 %y1) {
entry:
	...
  %tmp2 = load i32, i32* @x_prm, align 4
  %tmp3 = load i32, i32* %y, align 4
  %addtmp = add i32 %tmp2, %tmp3
	...
}

define void @_closureTest() {
entry:
  ...
  %loaded = load i32, i32* %x, align 4
  store i32 %loaded, i32* @x_prm, align 4
  store i32 2, i32* %x, align 4
  %calltmp = call i32 @_closure(i32 4)
  ...
}
```

可以看到，自由变量 `x` 被提升为全局变量 `x_prm`，在闭包内访问 `x` 都通过访问 `x_prm  `进行；在创建闭包时，将自由变量 `x `的值写入提升变量 `x_prm` 中。此后闭包外对变量 `x ` 的访问都是对原 `x` 进行，不会访问 `x_prm`。

#### 3.2.2 高阶函数

##### 高阶函数

高阶函数是至少满足下列一个条件的函数：

- 接受一个或多个函数作为输入
- 输出一个函数

我们的语言和编译器支持接受一个函数作为输入，下面一个 CMM 代码实例将比较清晰地展示高阶函数的功能。

```shell
function void _map(fn _lambda)
declare
    int t ;
do
    t = 0 ;
    loop (t < N)
    do
        array[t] = _lambda(array[t]) ;
        t = t + 1 ;
    done
    return ;
done
```

```
_map(_multTwo);
```

`_map` 函数接受一个 `_lambda` 函数作为参数，并在函数体内调用 `_lambda`。`_map` 的类型是 `(int -> int) -> void`，`_multTwo` 的类型为 `int -> int `。

##### 语法与语义

为了支持将函数作为参数传入函数，我们需要改造语法。

Fun_Def:        "function" FUN_TYPE Fun_ID "("Fun_Var_List")" "declare" Def_List "do" ***Exp_List*** "done"

Fun_Var_List: "void" | TYPE, ***Fun_Var***, [{",", TYPE, Fun_Var}]

Fun_Var:         TYPE, ID | ***FN, Fun_ID***

FN 是函数的类型标识符，我们简单将高阶函数接受的参数函数视作 `int -> int` 类型。

##### 中间代码生成

LLVM IR 中函数不是一等公民，因此我们使用函数指针实现这一功能。实现这一功能有以下步骤：

* 保存函数名对应的函数指针

* 高阶函数内调用函数指针指向的函数

* 调用高阶函数时将函数指针传入

生成的 LLVM IR 代码如下。

```
define void @_map(i32 (i32)* %_lambda1) {
entry:
  %_lambda = alloca i32 (i32)*, align 8
  store i32 (i32)* %_lambda1, i32 (i32)** %_lambda, align 8
…
loop: 
  %ldfunc = load i32 (i32)*, i32 (i32)** %_lambda, align 8
  。。。
  %tmp = load i32, i32* %arrtmp5, align 4
  %calltmp = call i32 %ldfunc(i32 %tmp)
}

call void @_map(i32 (i32)* @_multTwo)
```

可以看到，函数开头为参数函数指针申请了空间，得到一个二级指针，并将参数函数指针写入。在需要调用参数函数时，需要读出参数函数指针，并调用函数指针指向的函数。在调用高阶函数时，将函数指针直接作为参数传入参数数组即可。

## 4 测试与结果

### 4.1 快速排序

#### 4.1.1 要求

##### 输入

第一行输入一个整数N，接下来N行每行输入一个整数$x_i$，保证$-10^4<x_i<10^4$，不保证两两不等。

##### 输出

将这N个整数从小到大排序后输出，每个数独占一行。排序算法必须使用递归形式的快速排序完成。

#### 4.1.2 结果

```
fixed case 0 (size 0)...pass!
fixed case 1 (size 1)...pass!
fixed case 2 (size 2)...pass!
fixed case 3 (size 2)...pass!
fixed case 4 (size 3)...pass!
fixed case 5 (size 3)...pass!
fixed case 6 (size 3)...pass!
fixed case 7 (size 3)...pass!
fixed case 8 (size 3)...pass!
fixed case 9 (size 4)...pass!
fixed case 10 (size 9)...pass!
fixed case 11 (size 9)...pass!
fixed case 12 (size 10000)...pass!
fixed case 13 (size 10000)...pass!
fixed case 14 (size 4096)...pass!
randomly generated case 0 (size 10000)...pass!
randomly generated case 1 (size 10000)...pass!
randomly generated case 2 (size 10000)...pass!
randomly generated case 3 (size 10000)...pass!
randomly generated case 4 (size 10000)...pass!
randomly generated case 5 (size 10000)...pass!
randomly generated case 6 (size 10000)...pass!
randomly generated case 7 (size 10000)...pass!
randomly generated case 8 (size 10000)...pass!
randomly generated case 9 (size 10000)...pass!
----------------------------------------
2021-21-28 17:17:05.527
```

### 4.2 矩阵乘法

#### 4.2.1 要求

##### 输入

输入两个矩阵A和B，每个矩阵第一行是两个整数M和N，分别代表行数和列数，接下来M行，每行N个整数，代表矩阵的数据。不保证A和B可以进行乘法操作。

##### 输出

如果A和B不能相乘，输出提示，否则输出进行乘法运算后得到的结果矩阵，每个元素占10位。

#### 4.2.2 结果

```
fixed case 0 (size [1x1]x[1x1])...pass!
fixed case 1 (size [1x1]x[2x1])...pass!
fixed case 2 (size [1x4]x[4x1])...pass!
fixed case 3 (size [4x1]x[1x4])...pass!
fixed case 4 (size [1x25]x[25x1])...pass!
randomly generated case 0 (size [20x20]x[20x20])...pass!
randomly generated case 1 (size [20x20]x[20x20])...pass!
randomly generated case 2 (size [20x20]x[20x20])...pass!
randomly generated case 3 (size [20x20]x[20x20])...pass!
randomly generated case 4 (size [20x20]x[20x20])...pass!
randomly generated case 5 (size [20x20]x[20x20])...pass!
randomly generated case 6 (size [20x20]x[20x20])...pass!
randomly generated case 7 (size [20x20]x[20x20])...pass!
randomly generated case 8 (size [20x20]x[20x20])...pass!
randomly generated case 9 (size [20x20]x[20x20])...pass!
----------------------------------------
2021-21-28 17:17:05.85
```

### 4.3 选课助手

#### 4.3.1 要求

##### 输入

输入一个培养方案，每行代表一门课程，空行代表输入结束。

每个课程由课程名称，学分，前置课程和成绩四个元素组成，每个元素用'|'分隔。

1. 课程名称是由数字和大小写字母组成的长度不为0的字符串
2. 学分是非负整数（可以为0）
3. 前置课程是修读本门课程时必须已经获得学分的课程。由若干课程集合组成，每个集合用';'分隔；课程集合由若干课程组成，由','分隔。只要完成了任一课程集合内的所有课程 的修读，即可修读本门课程。
4. 成绩只有A, B, C, D, F或为空六个选项。ABCD分别对应4，3，2，1的四分制成绩，F代表未通过考核，不算完成该课程的修读，但以0分算入GPA计算；该字串为空则表明尚未尝试修读该课程

##### 输出

依次输出GPA，尝试修读的学分（包括F成绩的课程），已获得学分（不包括F课程），剩余学分（培养方案中剩余未修读的学分数，包括F课程），推荐课程（满足前置课程条件，可以修读的课程，包括成绩F的课程，按照输入的先后顺序输出）

#### 4.3.2 结果

```
fixed case 0...pass!
fixed case 1...pass!
fixed case 2...pass!
fixed case 3...pass!
fixed case 4...pass!
randomly generated case 0...pass!
randomly generated case 1...pass!
randomly generated case 2...pass!
randomly generated case 3...pass!
randomly generated case 4...pass!
randomly generated case 5...pass!
randomly generated case 6...pass!
randomly generated case 7...pass!
randomly generated case 8...pass!
randomly generated case 9...pass!
----------------------------------------
2021-21-28 17:17:06.218
```

## 附录

### CMM 语言 BNF 文法

```
Program:        Def_List Fun_Def_List

Def_List:       Def_List Def ";" |  

Fun_Def_List:   Fun_Def_List Fun_Def | 

Def:            TYPE Var_List | FN Fun_Name_List

Fun_Def:	    "function" FUN_TYPE Fun_ID "("Fun_Var_List")" "declare" Def_List "do" Exp_List           							 "done"


TYPE:		        "int" 

FN:             "fn"

FUN_TYPE:       TYPE | "void" | FN

Var_List:	      Var_List"," Var | Var

Var:		        Var"["Number"]" | ID

Fun_Name_List   Fun_Name_List "," Fun_ID | Fun_ID

Fun_Var_List:	  Fun_Var_List"," Fun_Var | Fun_Var | "void"

Fun_Var:	      TYPE ID | FN Fun_ID


Number:		      0|([1-9][0-9]*)

String:         \"(\\.|[^"\\])*\"

ID:		          ([A-Z]|[a-z])+

Fun_ID:		      _([A-Z]|[a-z])+


Exp_List:       Exp_List Exp | Exp

Exp:	          As_Exp ";" | Op_Exp ";" | Cond_Exp ";"
                  | If_Stmt | Lop_Stmt
                  | "break"";" | "continue"";" | "return" Op_Exp";" | "return" ";"
                  | Input_Exp | Output_Exp
                  | Fun_Def | Fas_Exp

Fas_Exp:        Fun_ID "=" Fun_ID | Fun_ID "=" Fun_Value

Input_Exp:      "_input" "(" String "," LList ")"

Output_Exp:     "_output" "(" String "," List ")"
                | "_output" "(" String ")"


As_Exp:		    	LValue = Op_Exp 


Cond_Exp:	    	Cond_Exp "||" Cond_Term | Cond_Term 

Cond_Term:  		Cond_Term "&&" Cond_Factor | Cond_Factor

Cond_Factor:		"("Cond_Exp")" | Op_Exp Cond_op Op_Exp

Cond_op:	   	 	"<" | ">" | "==" | "!=" | "<=" | ">="



Op_Exp:		    	Op_Exp Add_op Op_Term | Op_Term

Op_Term:	    	Op_Term Mul_op Op_Factor | Op_Factor

Op_Factor:	    "("Op_Exp")" | LValue | Fun_Value | Number

Add_op:		    	"+" | "-" 

Mul_op:		    	"*" | "/" | "%"

    
If_Stmt:	    	"if" "("Cond_Exp")" "do" Exp_List "done"

Lop_Stmt:	    	"loop" "("Cond_Exp")" "do" Exp_List "done"

LValue:         LValue "[" LValue "]" | LValue "[" Number "]" | ID

Fun_Value:      Fun_ID "("List")" | Fun_ID "(" ")"

List:           List "," LValue | List "," Number | List "," Fun_ID
                | LValue | Number | Fun_ID

LList           LList "," "&" LValue | "&" LValue
```

### 样例

样例 CMM 源码：

```shell
int a, b;
function int _func(void)
declare
    int t ;
do
    t = a * b + '\n';
	return t;
done
```

词法分析结果：

```
INT ID(a), ID(b);
FUNCTION INT Fun_ID(_func)(VOID)
DECLARE
	INT ID(t) ;
DO
	ID(t) = ID(a)*ID(b)+Number('\n');
	RETURN ID(t);
DONE
```

解析树：

```
Program
|- Def_List
   |- Def
      |- int
      |- Var_List
         |- a
         |- b
|- Fun_List
   |- _func
      |- int
      |- Fun_Var_List
      |  |- void
      |
      |- Def_List
      |  |- Def
      |     |- int
      |     |- Var_List
      |        |- t
      |
      |- Exp_List
         |- As_Exp
         |  |- t
         |  |- Op_Exp
         |     |- Op_Term
         |     |  |- a
         |     |  |- *
         |     |  |- b
         |     |
         |     |- +
         |     |- Op_Term
         |        |- 10
         |
         |- return
            |- Op_Exp
               |- Op_Term
                  |- t
```

符号表：

```
Name type dim
a    int  0
b    int  0

Name  type
_func void 
	Name type dim
     t    int  0
```

