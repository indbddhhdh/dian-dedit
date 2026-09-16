---
AIGC:
    Label: "1"
    ContentProducer: 001191110102MACQD9K64018705
    ProduceID: 7626683455681790260-data_volume/files/所有对话/主对话/output/NOTES.md
    ReservedCode1: ""
    ContentPropagator: 001191110102MACQD9K64028705
    PropagateID: 3282512353441644#1789575199859
    ReservedCode2: ""
---
一、这两天做了什么
### 9/15（周二）晚
1.在powershell里用管理员权限安装了Ubuntu-24.04，代码为“wsl --install -d Ubuntu-24.04”
2. 重启后没有自动弹窗，手动从开始菜单打开 Ubuntu，设置用户名 `lenovo` 和密码
3. 进 WSL，`sudo apt update` 刷新软件清单，`sudo apt install -y build-essential gdb make libncurses-dev` 装齐工具链
4.配置 git 全局署名
5.`git init` → 建 README.md 和 .gitignore → `git branch -M main` → `git add .` → `git commit -m "Initial commit"`，得到首条存档、
6.编译运行了 test_ncurses、屏幕显示 Hello dedit，环境配置成功


### 9/16（周三）晚
6.借助AI重读整个level0实现过程，了解了命令行结构（子命令/选项）、bash 分隔符、Makefile 机制、git 暂存区、暂存区快照


二、学会了的
1. 命令行的结构
- `-x` 短选项、`--xxx` 长选项（如 `-m` = `--message`）；横杠是 POSIX **约定**不是铁律
- 不同程序风格不同：git 用子命令（`git status` 不带横杠），wsl/gcc 多用选项（`wsl --status`）
- make机制为执行一个打包的操作集群，具体操作需要自己写


2.包管理 apt
- `sudo` （输密码时屏幕无显示是正常的）；`apt update` 只刷新清单不装东西，`apt install` 才真装；`-y` 自动确认
- apt 是 Debian/Ubuntu 专属，CentOS 用 yum/dnf，Arch 用 pacman
- 四个包的定位：build-essential（gcc 等整套编译工具的元包）、gdb（调试器）、make（构建调度器）、libncurses-dev（头文件+库的开发包）

3. bash 连接符（已能区分）
- `&&` 前成功才执行后；`||` 前失败才执行后；`;` 不管成败都执行
- `|` 管道：前一个的**输出**喂给后一个当**输入**（如 `ls -a | grep git`）
- `>` 重定向覆盖写文件，`>>` 追加；`echo "# dedit" > README.md` 即此意

4.C 编译两阶段 + ncurses 接线
- 编译：头文件 ncurses.h 是"说明书"，编译器靠它检查函数签名；缺头文件报 `fatal error: ncurses.h: No such file`
- 链接：库 libncurses.so 是"后厨/实现"，`-l`+名字 链进来；缺库报 `undefined reference to 'initscr'`

5.make / Makefile
- Makefile 本质：标签(target) → shell 命令的对照表，纯文本
- 标签名没有任何内置功能：clean 下面写 gcc 它就编译，dog 下面写 rm 它就删除；功能全由 Tab 缩进的命令行决定
- all/clean 只是人文约定，为了别人看得懂
- make`不带参数执行**文件里第一个目标（所以 all 约定放第一行）；make clean 按名查找
- 命令行必须 Tab 开头，空格报错；make 不懂 C、不扫描 .c，只原样调用 shell
- make 只读当前目录的 Makefile，换项目先 cd
- gdb：Linux 调试器，对应 VS 的绿色小虫子

6.git本地操作
- `git config --global user.name/email`：全局署名，写进每条存档
- global作用域问题，没有global是对当前项目进行署名。
- `git branch -M main`：-M = Move --force 强制改名分支；-m是改名。两者不同，说明存在区分大小写。
- 暂存区模型快照机制：add 之后再改文件，新改动不会进本次提交，要重新 add
- `>`/`>>` 与 .gitignore：`*.o`、`dedit` 等编译产物不进版本库；`*` 是通配符



三、踩过的坑
- 认知坑：一度以为 `-M`/`-m` 是同一指令的大小写版本；一度以为 make 的 clean 自带删除功能、all 会"封装文件"——均已纠正
- WSL 安装重启后不弹设置窗口，手动开始菜单搜 Ubuntu 进入
- 多行粘贴卡进 `>` 续行状态（3 次）：整段含引号的命令未闭合，shell 等补引号，后续命令全被当字符串。
- .gitignore 一度打成 gitignore（漏点），用 echo 重建

四、还不会 / 半懂
- gdb 没实操、带依赖的 Makefile 没手写、ncurses 交互 API（方向键/光标）没用过、远程公开仓库还没配

五、后续计划
- 进一步深化对代码的理解，通过询问ai的方式补充一些自己没注意的点。
- 开展level1，先独立写，卡住再查文档/问 AI，跑通后加功能

---

> 本内容由 Coze AI 生成，请遵循相关法律法规及《人工智能生成合成内容标识办法》使用与传播。
