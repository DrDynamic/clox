#include <stdio.h>
#include <string.h>

#include "../util/memory.h"
#include "object.h"
#include "../table.h"
#include "value.h"
#include "../vm.h"

#define ALLOCATE_OBJ(type, objectType) (type*)allocateObject(sizeof(type), objectType)


static Obj* allocateObject(size_t size, ObjType type)
{
    Obj* object = (Obj*)reallocate(NULL, 0, size);
    object->type = type;
    object->isMarked = false;

    object->next = vm.objects;
    vm.objects = object;

#ifdef DEBUG_LOG_GC
    printf("%p allocate %zu for %d\n", (void*)object, size, type);
#endif

    return object;
}

void viewInterned()
{
    printf("== Interned [%d/%d] ==\n", vm.strings.count, vm.strings.capacity);
    for (int i = 0; i < vm.strings.capacity; i++) {
        Entry* entry = &vm.strings.entries[i];
        if (entry->key == NULL) {
            printf("  [%d] NULL = '%d'\n", i, entry->as.uint32);
        } else {
            printf("  [%d] %s = '%d'\n", i, entry->key->chars, entry->as.uint32);
        }
    }
    printf("==================\n");
}

static ObjString* allocateString(char* chars, int length, uint32_t hash)
{
    ObjString* string = ALLOCATE_OBJ(ObjString, OBJ_STRING);
    string->length = length;
    string->chars = chars;
    string->hash = hash;

    push(OBJ_VAL(string));
    tableSet(&vm.strings, string, NIL_VAL);
    pop();
    return string;
}

static uint32_t hashString(const char* key, int length)
{
    uint32_t hash = 2166136261u;
    for (int i = 0; i < length; i++) {
        hash ^= (uint8_t)key[i];
        hash *= 16777619;
    }
    return hash;
}

ObjArray* newArray()
{
    ObjArray* array = ALLOCATE_OBJ(ObjArray, OBJ_ARRAY);
    initValueArray(&array->valueArray);
    return array;
}

ObjBoundMethod* newBoundMethod(Value receiver, ObjClosure* method)
{
    ObjBoundMethod* bound = ALLOCATE_OBJ(ObjBoundMethod, OBJ_BOUND_METHOD);
    bound->receiver = receiver;
    bound->method = method;
    return bound;
}

ObjInstance* newInstance(ObjClass* klass)
{
    ObjInstance* instance = ALLOCATE_OBJ(ObjInstance, OBJ_INSTANCE);
    instance->klass = klass;
    initTable(&instance->fields);
    return instance;
}

ObjClass* newClass(ObjString* name)
{
    ObjClass* klass = ALLOCATE_OBJ(ObjClass, OBJ_CLASS);
    klass->name = name;
    initTable(&klass->methods);
    return klass;
}

ObjClosure* newClosure(ObjFunction* function)
{
    ObjUpvalue** upvalues = ALLOCATE(ObjUpvalue*, function->upvalueCount);
    for (int i = 0; i < function->upvalueCount; i++) {
        upvalues[i] = NULL;
    }

    ObjClosure* closure = ALLOCATE_OBJ(ObjClosure, OBJ_CLOSURE);
    closure->function = function;
    closure->upvalues = upvalues;
    closure->upvalueCount = function->upvalueCount;

    return closure;
}

ObjFunction* newFunction()
{
    ObjFunction* function = ALLOCATE_OBJ(ObjFunction, OBJ_FUNCTION);
    function->arity = 0;
    function->upvalueCount = 0;
    function->name = NULL;
    initChunk(&function->chunk);
    return function;
}

ObjNative* newNative(NativeFn function)
{
    ObjNative* native = ALLOCATE_OBJ(ObjNative, OBJ_NATIVE);
    native->function = function;
    return native;
}

ObjString* takeString(char* chars, int length)
{
    uint32_t hash = hashString(chars, length);

    ObjString* interned = tableFindString(&vm.strings, chars, length, hash);
    if (interned != NULL) {
        FREE_ARRAY(char, chars, length + 1);
        return interned;
    }

    return allocateString(chars, length, hash);
}

ObjString* copyString(const char* chars, int length)
{
    uint32_t hash = hashString(chars, length);

    ObjString* interned = tableFindString(&vm.strings, chars, length, hash);
    if (interned != NULL) {
        return interned;
    }

    char* heapChars = ALLOCATE(char, length + 1);
    memcpy(heapChars, chars, length);
    heapChars[length] = '\0';
    return allocateString(heapChars, length, hash);
}

ObjUpvalue* newUpvalue(Value* slot)
{
    ObjUpvalue* upvalue = ALLOCATE_OBJ(ObjUpvalue, OBJ_UPVALUE);
    upvalue->location = slot;
    upvalue->closed = NIL_VAL;
    upvalue->next = NULL;
    return upvalue;
}

const char* objectGet(Value receiver, Value address, Value* value)
{
    if (IS_ARRAY(receiver)) {
        if (!IS_NUMBER(address)) {
            return "Arrays index needs to be of type Number.";
        }

        ObjArray* array = AS_ARRAY(receiver);
        uint64_t index = (uint64_t)AS_NUMBER(address);
        if (index >= array->valueArray.count) {
            return "Index out of bounds.";
        }

        *value = array->valueArray.values[index];
        return NULL;
    }
    return "Value can not accessed with [].";
}

const char* objectSet(Value receiver, Value address, Value value)
{
    if (IS_ARRAY(receiver)) {
        if (!IS_NUMBER(address)) {
            return "Arrays index needs to be of type Number.";
        }

        ObjArray* array = AS_ARRAY(receiver);
        uint64_t index = (uint64_t)AS_NUMBER(address);
        if (index >= array->valueArray.count) {
            return "Index out of bounds.";
        }

        array->valueArray.values[index] = value;
        return NULL;
    }
    return "Value can not accessed with [].";
}

static void printFunction(ObjFunction* function)
{
    if (function->name == NULL) {
        printf("<script>");
        return;
    }
    printf("<fn %s>", function->name->chars);
}

static void printArray(ObjArray* array)
{
    printf("[");
    for (unsigned int i = 0; i < array->valueArray.count; i++) {
        printValue(array->valueArray.values[i]);
        if (i != array->valueArray.count - 1) {
            printf(", ");
        }
    }
    printf("]");
}

void printObject(Value value)
{
    switch (OBJ_TYPE(value)) {
    case OBJ_ARRAY:
        printArray(AS_ARRAY(value));
        break;
    case OBJ_BOUND_METHOD:
        printFunction(AS_BOUND_METHOD(value)->method->function);
        break;
    case OBJ_INSTANCE:
        printf("<obj %s>", AS_INSTANCE(value)->klass->name->chars);
        break;
    case OBJ_CLASS:
        printf("<cls %s>", AS_CLASS(value)->name->chars);
        break;
    case OBJ_CLOSURE:
        printFunction(AS_CLOSURE(value)->function);
        break;
    case OBJ_FUNCTION:
        printFunction(AS_FUNCTION(value));
        break;
    case OBJ_NATIVE:
        printf("<native fn>");
        break;
    case OBJ_STRING:
        printf("%s", AS_CSTRING(value));
        break;
    case OBJ_UPVALUE:
        printf("upvalue");
        break;
    default:
        printf("<Object type: %d>", OBJ_TYPE(value));
        break;
    }
}