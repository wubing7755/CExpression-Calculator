/**
 * @file command.c
 * @brief 命令处理模块实现
 * 
 * ## 模块职责
 * 
 * 这个模块负责处理用户在控制台输入的特殊命令（如 "quit"、"show help" 等）。
 * 与数学表达式不同，这些命令不需要经过词法分析和语法分析。
 * 
 * ## 设计思路
 * 
 * ### 命令规格表（Command Specification）
 * 
 * 使用"规格驱动"的设计模式：
 *   - 定义 CommandSpec 结构存储每个命令的元信息
 *   - 使用静态数组 COMMAND_SPECS 存储所有支持的命令
 *   - 命令处理函数以函数指针形式存储
 * 
 * 这种设计的优点：
 *   1. 添加新命令只需在数组中添加一项
 *   2. 命令逻辑与匹配逻辑分离
 *   3. 代码结构清晰，易于维护
 * 
 * ### 分词与匹配
 * 
 * 命令输入需要先分词（Tokenize），然后与规格表逐一比对。
 * 使用大小写不敏感匹配（Case-Insensitive）。
 */

#include "command.h"

#include <ctype.h>
#include <stddef.h>
#include <string.h>

#include "logger.h"

/* ========================================================================
 * 配置常量
 * ======================================================================== */

/** 命令输入缓冲区的最大长度 */
#define COMMAND_INPUT_COPY_MAX 1024U

/** 命令分词后的最大词数 */
#define COMMAND_MAX_TOKENS 8U

/* ========================================================================
 * 类型定义
 * ======================================================================== */

/**
 * @brief 命令处理函数类型
 * 
 * 每个命令对应一个处理函数，接收 CommandState 指针。
 * 处理函数负责修改状态（如设置 should_exit）或输出信息。
 */
typedef void (*CommandHandler)(CommandState* state);

/**
 * @brief 命令规格结构
 * 
 * 描述一个命令的所有必要信息。
 * 
 * ## 字段说明
 *   - syntax     : 命令语法，如 "show help"
 *   - description: 命令描述，用于帮助信息
 *   - tokens     : 命令分词后的词数组
 *   - token_count: 词的数量
 *   - handler    : 命令处理函数指针
 */
typedef struct {
    const char* syntax;            /**< 命令语法 */
    const char* description;       /**< 命令描述（用于帮助） */
    const char* const* tokens;      /**< 分词后的词数组 */
    size_t token_count;             /**< 词的数量 */
    CommandHandler handler;         /**< 处理函数 */
} CommandSpec;

/* ========================================================================
 * 工具函数
 * ======================================================================== */

/**
 * @brief 忽略大小写的字符串比较
 * 
 * 用于命令匹配，支持大小写不敏感。
 * 例如："QUIT"、"Quit"、"quit" 都应该匹配退出命令。
 * 
 * @param a 第一个字符串
 * @param b 第二个字符串
 * @return 如果相等返回 1，否则返回 0
 * 
 * ## 算法说明
 * 
 * 逐字符比较，将两个字符都转换为小写后再比较。
 * 只有当两个字符串完全相等（长度相同）时才返回 1。
 */
static int equalsIgnoreCase(const char* a, const char* b)
{
    while (*a != '\0' && *b != '\0') {
        /* tolower 处理 unsigned char，避免有符号转换问题 */
        if (tolower((unsigned char)*a) != tolower((unsigned char)*b)) {
            return 0;  /* 不相等 */
        }
        a++;
        b++;
    }
    /* 确保两个字符串都到达末尾（长度相同） */
    return (*a == '\0' && *b == '\0');
}

/**
 * @brief 字符串分词（Tokenize）
 * 
 * 将输入字符串分割成多个单词（token）。
 * 单词由空白字符（空格、制表符等）分隔。
 * 
 * @param text 要分词的字符串（会被修改，会在分隔处插入 '\0'）
 * @param tokens 输出参数，存储每个单词的起始位置指针
 * @param max_tokens tokens 数组的最大容量
 * @return 分词的数量
 * 
 * ## 分词示例
 * 
 * 输入: "  show   help  "
 * 输出: ["show", "help"]
 * 返回: 2
 * 
 * ## 实现细节
 * 
 *   1. 跳过开头的空白字符
 *   2. 记录单词开始位置
 *   3. 找到单词结束位置（下一个空白字符前）
 *   4. 在结束位置插入 '\0'（原地修改）
 *   5. 重复直到字符串结束或达到最大词数
 */
static size_t tokenize(char* text, char* tokens[], size_t max_tokens)
{
    size_t count = 0;    /* 已分词的个数 */
    char* p = text;       /* 当前遍历位置 */

    while (*p != '\0' && count < max_tokens) {
        /* 跳过空白字符 */
        while (*p != '\0' && isspace((unsigned char)*p)) {
            p++;
        }
        
        /* 遇到字符串末尾，退出 */
        if (*p == '\0') {
            break;
        }

        /* 记录单词开始位置 */
        tokens[count++] = p;

        /* 找到单词结束位置 */
        while (*p != '\0' && !isspace((unsigned char)*p)) {
            p++;
        }
        
        /* 在单词末尾插入 '\0'，原地修改字符串 */
        if (*p != '\0') {
            *p = '\0';
            p++;
        }
    }

    return count;
}

/**
 * @brief 检查是否可能是命令
 * 
 * 用于快速排除明显不是命令的输入。
 * 命令必须以字母开头（如 "quit"、"show"）。
 * 数学表达式以数字开头（如 "2+3"），应该跳过命令处理。
 * 
 * @param token 要检查的词
 * @return 如果可能是命令返回 1，否则返回 0
 */
static int isCandidateCommand(const char* token)
{
    return (token != NULL && isalpha((unsigned char)token[0]));
}

/* ========================================================================
 * 命令处理函数
 * ======================================================================== */

/**
 * @brief 处理 "show process" 命令
 * 
 * 开启计算过程的显示。
 * 开启后，每次计算表达式都会输出详细的计算步骤。
 * 
 * @param state 命令状态指针
 */
static void handleShowProcess(CommandState* state)
{
    state->show_process = true;
    logger_log(LOG_INFO, "已开启计算过程输出。\n\n");
}

/**
 * @brief 处理 "hide process" 命令
 * 
 * 关闭计算过程的显示。
 * 
 * @param state 命令状态指针
 */
static void handleHideProcess(CommandState* state)
{
    state->show_process = false;
    logger_log(LOG_INFO, "已关闭计算过程输出。\n\n");
}

/**
 * @brief 处理退出命令
 * 
 * 设置 should_exit 标志，通知主循环退出程序。
 * 可以被 "quit"、"exit"、"q" 等命令触发。
 * 
 * @param state 命令状态指针
 */
static void handleQuit(CommandState* state)
{
    state->should_exit = true;
}

/**
 * @brief 处理 "show help" 命令
 * 
 * 显示所有可用命令的列表和说明。
 * 这是一个前向声明，因为 handleShowHelp 在 CommandSpec 数组之前定义。
 */
static void handleShowHelp(CommandState* state);

/* ========================================================================
 * 命令规格表
 * ======================================================================== */

/**
 * @brief 命令词数组
 * 
 * 每个命令对应的分词结果。
 * 使用 const char* const* 类型，表示"指向常量字符串数组的常量指针"。
 * 
 * 注意：数组中的字符串必须在 CommandSpec 数组中使用。
 */
static const char* TOK_SHOW_PROCESS[] = {"show", "process"};
static const char* TOK_HIDE_PROCESS[] = {"hide", "process"};
static const char* TOK_SHOW_HELP[] = {"show", "help"};
static const char* TOK_HELP[] = {"help"};
static const char* TOK_QUIT[] = {"quit"};
static const char* TOK_EXIT[] = {"exit"};
static const char* TOK_Q[] = {"q"};

/**
 * @brief 命令规格表
 * 
 * 存储所有支持的命令及其元信息。
 * 
 * ## 设计要点
 * 
 *   1. 按优先级排列：常用命令放前面
 *   2. 每个命令有多个别名（如 quit、exit、q）
 *   3. 使用指定初始化器，清晰明了
 */
static const CommandSpec COMMAND_SPECS[] = {
    /* 语法              描述                    词数组                    词数    处理函数 */
    {"show process",  "开启计算过程输出",   TOK_SHOW_PROCESS,  2U,  handleShowProcess},
    {"hide process",  "关闭计算过程输出",   TOK_HIDE_PROCESS,  2U,  handleHideProcess},
    {"show help",     "显示命令帮助",       TOK_SHOW_HELP,     2U,  handleShowHelp},
    {"help",          "显示命令帮助",       TOK_HELP,          1U,  handleShowHelp},
    {"quit",          "退出程序",           TOK_QUIT,          1U,  handleQuit},
    {"exit",          "退出程序",           TOK_EXIT,          1U,  handleQuit},
    {"q",             "退出程序",           TOK_Q,             1U,  handleQuit}
};

/* ========================================================================
 * 命令处理函数实现
 * ======================================================================== */

/**
 * @brief 显示帮助信息
 * 
 * 遍历命令规格表，输出每个命令的语法和描述。
 * 
 * @param state 命令状态指针（此函数不使用）
 */
static void handleShowHelp(CommandState* state)
{
    size_t i;
    (void)state;  /* 显式声明未使用，避免编译器警告 */

    logger_log(LOG_INFO, "【命令列表】\n");
    
    /* 遍历所有命令规格 */
    for (i = 0; i < sizeof(COMMAND_SPECS) / sizeof(COMMAND_SPECS[0]); ++i) {
        /* 使用左对齐格式化，宽度13，确保描述对齐 */
        logger_log(LOG_INFO, "- %-13s : %s\n",
                   COMMAND_SPECS[i].syntax, COMMAND_SPECS[i].description);
    }
    logger_log(LOG_INFO, "\n");
}

/* ========================================================================
 * 公共 API 实现
 * ======================================================================== */

/**
 * @brief 初始化命令状态
 * 
 * 将 CommandState 结构设置为默认值。
 * 
 * @param state 要初始化的状态指针
 */
void commandStateInit(CommandState* state)
{
    state->show_process = false;  /* 默认不显示计算过程 */
    state->should_exit = false;    /* 默认不退出 */
}

/**
 * @brief 分发并执行命令
 * 
 * 这是命令模块的核心公共函数。处理流程：
 * 
 *   1. 复制输入（避免修改原字符串）
 *   2. 分词处理
 *   3. 快速检查是否是命令（以字母开头）
 *   4. 与命令规格表逐一比对
 *   5. 匹配成功则调用处理函数
 *   6. 匹配失败则报告错误
 * 
 * ## 匹配算法
 * 
 * 对于每个规格，检查：
 *   - 词数是否相同
 *   - 每个词是否大小写不敏感相等
 * 
 * 使用短路求值：只要有任何一项不匹配，立即尝试下一个规格。
 * 
 * @param input 用户输入的原始字符串
 * @param state 命令状态指针
 * @return 命令结果枚举
 */
CommandResult commandDispatch(const char* input, CommandState* state)
{
    char copy[COMMAND_INPUT_COPY_MAX];      /* 输入副本 */
    char* tokens[COMMAND_MAX_TOKENS];       /* 分词结果 */
    size_t token_count;                      /* 分词数量 */
    size_t i;                               /* 循环计数器 */

    /* 参数验证 */
    if (input == NULL || state == NULL) {
        return COMMAND_RESULT_ERROR;
    }

    /* ---------------------------------------------------------------
     * 步骤1: 复制输入字符串
     * ---------------------------------------------------------------
     * 
     * 分词函数会修改字符串（插入 '\0'），
     * 所以必须复制一份进行处理。
     * 
     * 使用 strncpy 安全复制：
     *   - 确保不会超出缓冲区
     *   - 最后一个字节设为 '\0' 确保字符串结束
     */
    copy[0] = '\0';
    strncpy(copy, input, sizeof(copy) - 1U);
    copy[sizeof(copy) - 1U] = '\0';

    /* ---------------------------------------------------------------
     * 步骤2: 分词
     * ---------------------------------------------------------------
     */
    token_count = tokenize(copy, tokens, COMMAND_MAX_TOKENS);
    
    /* 无效输入（全是空白） */
    if (token_count == 0) {
        return COMMAND_RESULT_NOT_COMMAND;
    }

    /* ---------------------------------------------------------------
     * 步骤3: 快速检查是否是命令
     * ---------------------------------------------------------------
     * 
     * 数学表达式如 "2+3" 以数字开头，不是命令。
     * 这种快速检查可以避免不必要的匹配循环。
     */
    if (!isCandidateCommand(tokens[0])) {
        return COMMAND_RESULT_NOT_COMMAND;
    }

    /* ---------------------------------------------------------------
     * 步骤4: 与命令规格表逐一比对
     * ---------------------------------------------------------------
     */
    for (i = 0; i < sizeof(COMMAND_SPECS) / sizeof(COMMAND_SPECS[0]); ++i) {
        const CommandSpec* spec = &COMMAND_SPECS[i];
        size_t j;
        int match = 1;  /* 假设匹配成功 */

        /* 首先检查词数是否相同 */
        if (token_count != spec->token_count) {
            continue;  /* 词数不同，尝试下一个规格 */
        }

        /* 逐词比较 */
        for (j = 0; j < spec->token_count; ++j) {
            if (!equalsIgnoreCase(tokens[j], spec->tokens[j])) {
                match = 0;  /* 有一个词不匹配 */
                break;       /* 立即退出内层循环 */
            }
        }

        /* 所有词都匹配 */
        if (match) {
            /* 调用命令处理函数 */
            spec->handler(state);
            return COMMAND_RESULT_HANDLED;
        }
    }

    /* ---------------------------------------------------------------
     * 步骤5: 匹配失败
     * ---------------------------------------------------------------
     * 
     * 输入看起来像命令（以字母开头）但没有匹配的规格。
     * 输出错误提示，帮助用户发现拼写错误。
     */
    logger_log(LOG_ERROR, "未知命令: %s\n", input);
    logger_log(LOG_INFO, "输入 'show help' 查看可用命令。\n\n");
    return COMMAND_RESULT_ERROR;
}
