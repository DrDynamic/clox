#pragma once

#include "../common.h"
#include "chunkDefs.h"
#include "../values/value.h"
#include "sourceinfo.h"

typedef enum {
    OP_CONSTANT,
    OP_CONSTANT_LONG,
    OP_NIL,
    OP_TRUE,
    OP_FALSE,
    OP_POP,
    OP_GET_LOCAL,
    OP_GET_LOCAL_LONG,
    OP_SET_LOCAL,
    OP_SET_LOCAL_LONG,
    OP_GET_GLOBAL,
    OP_GET_GLOBAL_LONG,
    OP_DEFINE_GLOBAL,
    OP_DEFINE_GLOBAL_LONG,
    OP_SET_GLOBAL,
    OP_SET_GLOBAL_LONG,
    OP_GET_UPVALUE,
    OP_SET_UPVALUE,
    OP_GET_PROPERTY,
    OP_GET_PROPERTY_LONG,
    OP_GET_PROPERTY_STACK,
    OP_SET_PROPERTY,
    OP_SET_PROPERTY_LONG,
    OP_SET_PROPERTY_STACK,
    OP_GET_SUPER,
    OP_GET_SUPER_LONG,
    OP_EQUAL,
    OP_NOT_EQUAL,
    OP_GREATER,
    OP_GREATER_EQUAL,
    OP_LESS,
    OP_LESS_EQUAL,
    OP_ADD,
    OP_SUBTRACT,
    OP_MULTIPLY,
    OP_DIVIDE,
    OP_NOT,
    OP_NEGATE,
    OP_PRINT,
    OP_JUMP,
    OP_JUMP_IF_FALSE,
    OP_LOOP,
    OP_CALL,
    OP_INVOKE,
    OP_INVOKE_LONG,
    OP_SUPER_INVOKE,
    OP_SUPER_INVOKE_LONG,
    OP_CLOSURE,
    OP_CLOSURE_LONG,
    OP_CLOSE_UPVALUE,
    OP_RETURN,
    OP_CLASS,
    OP_CLASS_LONG,
    OP_INHERIT,
    OP_METHOD,
    OP_METHOD_LONG,
    OP_ARRAY_INIT,
    OP_ARRAY_ADD,
    OP_UNDEFINED = 0xFF,
} OpCode;

typedef struct {
    BytecodeIndex count;
    BytecodeIndex capacity;
    uint8_t* code;
    ValueArray constants;
    SourceInfo sourceinfo;
} Chunk;

void initChunk(Chunk* chunk);
void writeChunk(Chunk* chunk, uint8_t byte, int line);
void writeConstant(Chunk* chunk, Value value, int line, OpCode opCodeShort, OpCode opCodeLong);
uint32_t addConstant(Chunk* chunk, Value value);
Linenumber getLinenumber(Chunk* chunk, BytecodeIndex offset);
void freeChunk(Chunk* chunk);
