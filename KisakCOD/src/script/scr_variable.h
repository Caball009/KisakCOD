#pragma once
#include <universal/q_shared.h>
#include "scr_stringlist.h"

#undef GetObject

#define VAR_STAT_FREE 0
#define VAR_STAT_MOVABLE 0x20
#define VAR_STAT_MASK 0x60
#define VAR_STAT_EXTERNAL 0x60

#define VAR_UNDEFINED 0
#define VAR_POINTER 1
#define VAR_STRING 2
#define VAR_INTEGER 6

#define VAR_STACK 10
#define VAR_MASK 0x1F
#define VAR_THREAD 0xE
#define VAR_NOTIFY_THREAD 0xF
#define VAR_TIME_THREAD 0x10
#define VAR_CHILD_THREAD 0x11
#define VAR_ENTITY 0x14
#define VAR_ARRAY 0x15
#define VAR_COUNT 0x16

#define FIRST_DEAD_OBJECT 0x16

#define CLASS_NUM_COUNT 4
#define VAR_NAME_BITS 8

#define VARIABLELIST_CHILD_SIZE 0xFFFEu
#define VARIABLELIST_CHILD_BEGIN 0x8002 // 32770 // XBOX(0x6000) // 
#define VARIABLELIST_PARENT_BEGIN 1
#define VARIABLELIST_PARENT_SIZE 0x8000

#define VAR_NAME_HIGH_MASK 0xFFFFFF00


#define FIRST_OBJECT 0xE
#define FIRST_CLEARABLE_OBJECT 0x12
#define FIRST_NONFIELD_OBJECT 0x15
#define FIRST_DEAD_OBJECT 0x16

#define VAR_NAME_LOW_MASK 0xFF000000

struct VariableStackBuffer // sizeof=0xC
{
    const char *pos;
    unsigned __int16 size;
    unsigned __int16 bufLen;
    unsigned __int16 localId;
    unsigned __int8 time;
    char buf[1];
};
union VariableUnion // sizeof=0x4
{                                       // ...
    operator int()
    {
        return intValue;
    }
    VariableUnion(float f)
    {
        floatValue = f;
    }
    VariableUnion(int i)
    {
        intValue = i;
    }
    VariableUnion(char *str)
    {
        codePosValue = str;
    }
    VariableUnion(const char *str)
    {
        codePosValue = str;
    }
    VariableUnion()
    {
        intValue = 0;
    }

    int intValue;
    float floatValue;
    unsigned int stringValue;
    const float *vectorValue;
    const char *codePosValue;
    unsigned int pointerValue;
    VariableStackBuffer *stackValue;
    unsigned int entityOffset;
};
struct VariableValue // sizeof=0x8
{   
    VariableValue()
    {
        u.intValue = 0;
        type = 0;
    }
    VariableValue(int i)
    {
        u.intValue = i;
        type = VAR_INTEGER;
    }
    VariableValue(VariableUnion un)
    {
        u = un;
    }
    // ...
    VariableUnion u;                    // ...
    int type;                           // ...
};

union ObjectInfo_u // sizeof=0x2
{                                       // ...
    unsigned __int16 size;
    unsigned __int16 entnum;
    unsigned __int16 nextEntId;
    unsigned __int16 self;
};

struct ObjectInfo // sizeof=0x4
{                                       // ...
    unsigned __int16 refCount;
    ObjectInfo_u u;
};

union Variable_u // sizeof=0x2
{                                       // ...
    unsigned __int16 prev;
    unsigned __int16 prevSibling;
};

struct Variable // sizeof=0x4
{                                       // ...
    unsigned __int16 id;                // ...
    Variable_u u;                       // ...
};

union VariableValueInternal_u // sizeof=0x4
{                                       // ...
    operator int()
    {
        return u.intValue;
    }
    VariableValueInternal_u(int i)
    {
        u.intValue = i;
    }
    VariableValueInternal_u()
    {
        u.intValue = 0;
    }

    unsigned __int16 next;
    VariableUnion u;
    ObjectInfo o;
};
union VariableValueInternal_w // sizeof=0x4
{                                       // ...
    unsigned int status;
    unsigned int type;
    unsigned int name;
    unsigned int classnum;
    unsigned int notifyName;
    unsigned int waitTime;
    unsigned int parentLocalId;
};
union VariableValueInternal_v // sizeof=0x2
{                                       // ...
    unsigned __int16 next;
    unsigned __int16 index;
};
struct VariableValueInternal // sizeof=0x10
{                                       // ...
    Variable hash;                      // ...
    VariableValueInternal_u u;          // ...
    VariableValueInternal_w w;          // ...
    VariableValueInternal_v v;          // ...
    unsigned __int16 nextSibling;       // ...
};

struct scrVarDebugPub_t // sizeof=0xE0004
{                                       // ...
    const char* varUsage[0x18000];
    unsigned __int16 extRefCount[0x8000];
    unsigned __int16 refCount[0x8000];
    int leakCount[0x18000];
    bool dummy;
    // padding byte
    // padding byte
    // padding byte
};
struct scrVarGlob_t // sizeof=0x180000
{                                       // ...
    VariableValueInternal variableList[0x18000]; // ...
};

struct scr_entref_t // sizeof=0x4
{                                       // ...
    scr_entref_t()
    {
        entnum = 0;
        classnum = 0;
    }
    scr_entref_t(int i)
    {
        entnum = i;
        classnum = i;
    }
    unsigned __int16 entnum;            // ...
    unsigned __int16 classnum;          // ...
};

struct scr_classStruct_t // sizeof=0xC
{
    scr_classStruct_t(unsigned __int16 _id, unsigned __int16 _entArrayId, char _charID, const char* _name)
    {
        id = _id;
        entArrayId = _entArrayId;
        charId = _charID;
        name = _name;
    }
    unsigned __int16 id;
    unsigned __int16 entArrayId;
    char charId;
    // padding byte
    // padding byte
    // padding byte
    const char *name;
};

struct VariableDebugInfo // sizeof=0x10
{
    const char *pos;
    const char *fileName;
    const char *functionName;
    int varUsage;
};

//void  TRACK_scr_variable(void);
void __cdecl Scr_Cleanup();
bool  IsObject(VariableValueInternal* entryValue);
bool  IsObject(VariableValue* value);
void  Scr_InitVariables(void);
void  Scr_InitVariableRange(unsigned int begin, unsigned int end);
void  Scr_InitClassMap(void);
unsigned int  Scr_GetNumScriptVars(void);
unsigned int  GetVariableKeyObject(unsigned int id);
unsigned int  Scr_GetVarId(unsigned int index);
void  Scr_SetThreadNotifyName(unsigned int startLocalId, unsigned int stringValue);
unsigned short  Scr_GetThreadNotifyName(unsigned int startLocalId);
void  Scr_SetThreadWaitTime(unsigned int startLocalId, unsigned int waitTime);
void  Scr_ClearWaitTime(unsigned int startLocalId);
unsigned int  Scr_GetThreadWaitTime(unsigned int startLocalId);
unsigned int  GetParentLocalId(unsigned int threadId);
unsigned int  GetSafeParentLocalId(unsigned int threadId);
unsigned int  GetStartLocalId(unsigned int);
unsigned int  AllocValue(void);
unsigned int  AllocObject(void);
unsigned int  Scr_AllocArray(void);
unsigned int  AllocThread(ObjectInfo_u self);
unsigned int  AllocChildThread(ObjectInfo_u self, unsigned int parentLocalId);
unsigned int  Scr_GetSelf(unsigned int threadId);
void  AddRefToObject(unsigned int id);
void  RemoveRefToEmptyObject(unsigned int id);
int  Scr_GetRefCountToObject(unsigned int id);
float const*  Scr_AllocVector(float const* v);
void  AddRefToVector(float const* vectorValue);
void  RemoveRefToVector(float const* vectorValue);
void  AddRefToValue(int type, VariableUnion u);
void  RemoveRefToValue(int type, VariableUnion u);
bool  IsValidArrayIndex(unsigned int unsignedValue);
unsigned int  GetInternalVariableIndex(unsigned int unsignedValue);
unsigned int  FindArrayVariable(unsigned int parentId, int intValue);
unsigned int  FindVariable(unsigned int parentId, unsigned int unsignedValue);
unsigned int  FindObjectVariable(unsigned int parentId, unsigned int id);
struct VariableValue  Scr_GetArrayIndexValue(unsigned int name);
void  SetVariableValue(unsigned int id, struct VariableValue* value);
void  SetNewVariableValue(unsigned int id, struct VariableValue* value);
VariableValueInternal_u*  GetVariableValueAddress(unsigned int id);
void  ClearVariableValue(unsigned int id);
VariableValueInternal_u Scr_EvalVariableObject(unsigned int id);
unsigned int  GetArraySize(unsigned int id);
unsigned int  FindFirstSibling(unsigned int id);
unsigned int  FindNextSibling(unsigned int id);
unsigned int  FindLastSibling(unsigned int parentId);
unsigned int  FindPrevSibling(unsigned int index);
unsigned int  GetVariableName(unsigned int id);
VariableValueInternal_u GetObject(unsigned int id);
VariableValueInternal_u GetArray(unsigned int id);
VariableValueInternal_u FindObject(unsigned int id);
bool  IsFieldObject(unsigned int id);
int  Scr_IsThreadAlive(unsigned int);
bool  IsObjectFree(unsigned int id);
unsigned int GetValueType(unsigned int id);
unsigned int GetObjectType(unsigned int id);
void  Scr_SetClassMap(unsigned int classnum);
VariableValueInternal_u Scr_GetOffset(unsigned int classnum, const char* name);
VariableValueInternal_u FindEntityId(unsigned int entnum, unsigned int classnum);
void  SetEmptyArray(unsigned int parentId);
void  Scr_AddArrayKeys(unsigned int parentId);
scr_entref_t Scr_GetEntityIdRef(unsigned int entId);
unsigned int  Scr_FindField(char const* name, int* type);
void  Scr_AddFields(char const* path, char const* extension);
void  Scr_AllocGameVariable(void);
//void  Scr_GetChecksum(int* const);
int  Scr_GetClassnumForCharId(char charId);
//unsigned int  Scr_InitStringSet(void);
unsigned int  Scr_FindAllThreads(unsigned int selfId, unsigned int* threads, unsigned int localId);
unsigned int  Scr_FindAllEndons(unsigned int threadId, unsigned int* names);
//bool  CheckReferences(void);

void  Scr_DumpScriptVariables(bool spreadsheet,
    bool summary,
    bool total,
    bool functionSummary,
    bool lineSort,
    const char* fileName,
    const char* functionName,
    int minCount);

static void Scr_DumpScriptVariablesDefault(void)
{
    Scr_DumpScriptVariables(0, 0, 0, 0, 0, 0, 0, 0);
}
unsigned int  GetVariableIndexInternal(unsigned int parentId, unsigned int name);
void  ClearObject(unsigned int parentId);
void  Scr_RemoveThreadNotifyName(unsigned int startLocalId);
//void  Scr_RemoveThreadEmptyNotifyName(unsigned int startLocalId);
void  FreeValue(unsigned int id);
unsigned int  GetArrayVariableIndex(unsigned int parentId, unsigned int unsignedValue);
unsigned int  Scr_GetVariableFieldIndex(unsigned int parentId, unsigned int name);
unsigned int  Scr_FindAllVariableField(unsigned int parentId, unsigned int* names);
unsigned int  GetArrayVariable(unsigned int parentId, unsigned int unsignedValue);
unsigned int  GetNewArrayVariable(unsigned int parentId, unsigned int unsignedValue);
unsigned int  GetVariable(unsigned int parentId, unsigned int unsignedValue);
unsigned int  GetNewVariable(unsigned int parentId, unsigned int unsignedValue);
unsigned int  GetObjectVariable(unsigned int parentId, unsigned int id);
unsigned int  GetNewObjectVariable(unsigned int parentId, unsigned int id);
unsigned int  GetNewObjectVariableReverse(unsigned int parentId, unsigned int id);
void  RemoveVariable(unsigned int parentId, unsigned int unsignedValue);
void  RemoveNextVariable(unsigned int parentId);
void  RemoveObjectVariable(unsigned int parentId, unsigned int id);
void  SafeRemoveVariable(unsigned int parentId, unsigned int unsignedValue);
void  RemoveVariableValue(unsigned int parentId, unsigned int index);
void  SetVariableEntityFieldValue(unsigned int entId, unsigned int fieldName, VariableValue* value);
void  SetVariableFieldValue(unsigned int id, VariableValue* value);
VariableValue  Scr_EvalVariable(unsigned int id);
void  Scr_EvalboolComplement(VariableValue* value);
void  Scr_CastBool(VariableValue* value);
bool  Scr_CastString(VariableValue* value);
void  Scr_CastDebugString(VariableValue* value);
char  Scr_GetEntClassId(unsigned int id);
int  Scr_GetEntNum(unsigned int id);
void  Scr_ClearVector(VariableValue* value);
void  Scr_CastVector(VariableValue* value);
VariableUnion  Scr_EvalFieldObject(unsigned int tempVariable, VariableValue* value);
void  Scr_UnmatchingTypesError(VariableValue* value1, VariableValue* value2);

void  Scr_EvalOr(VariableValue* value1, VariableValue* value2);
void  Scr_EvalExOr(VariableValue* value1, VariableValue* value2);
void  Scr_EvalAnd(VariableValue* value1, VariableValue* value2);
void  Scr_EvalLess(VariableValue* value1, VariableValue* value2);
void  Scr_EvalGreaterEqual(VariableValue* value1, VariableValue* value2);
void  Scr_EvalGreater(VariableValue* value1, VariableValue* value2);
void  Scr_EvalLessEqual(VariableValue* value1, VariableValue* value2);
void  Scr_EvalShiftLeft(VariableValue* value1, VariableValue* value2);
void  Scr_EvalShiftRight(VariableValue* value1, VariableValue* value2);
void  Scr_EvalPlus(VariableValue* value1, VariableValue* value2);
void  Scr_EvalMinus(VariableValue* value1, VariableValue* value2);
void  Scr_EvalMultiply(VariableValue* value1, VariableValue* value2);
void  Scr_EvalDivide(VariableValue* value1, VariableValue* value2);
void  Scr_EvalMod(VariableValue* value1, VariableValue* value2);

void  Scr_FreeEntityNum(unsigned int entnum, unsigned int classnum);
void  Scr_FreeObjects(void);
void  Scr_AddClassField(unsigned int classnum, char* name, unsigned int offset);
unsigned int  Scr_GetEntityId(unsigned int entnum, unsigned int classnum);
//void  Scr_CopyEntityNum(int, int, unsigned int);
void  Scr_FreeGameVariable(int bComplete);
//bool  Scr_AddStringSet(unsigned int, char const*);

struct ThreadDebugInfo // sizeof=0x8C
{                                       // ...
    const char *pos[32];                // ...
    int posSize;                        // ...
    float varUsage;                     // ...
    float endonUsage;                   // ...
};
void  Scr_DumpScriptThreads(void);
void  Scr_ShutdownVariables(void);
void  RemoveRefToObject(unsigned int id);
void  ClearVariableField(unsigned int parentId, unsigned int name, VariableValue* value);
VariableValue  Scr_EvalVariableField(unsigned int id);
void  Scr_EvalSizeValue(VariableValue* value);
void  Scr_EvalboolNot(VariableValue* value);
void  Scr_EvalEquality(VariableValue* value1, VariableValue* value2);
void  Scr_EvalInequality(VariableValue* value1, VariableValue* value2);
void  Scr_EvalBinaryOperator(int op, VariableValue* value1, VariableValue* value2);
void  Scr_FreeEntityList(void);
void  Scr_RemoveClassMap(unsigned int classnum);
void  Scr_EvalArray(VariableValue* value, VariableValue* index);
VariableUnion Scr_EvalArrayRef(unsigned int parentId);
void  ClearArray(unsigned int parentId, VariableValue* value);
void  Scr_FreeValue(unsigned int id);
//void  Scr_ShutdownStringSet(unsigned int);
void  Scr_StopThread(unsigned int threadId);
void  Scr_KillEndonThread(unsigned int threadId);
VariableValue Scr_FindVariableField(unsigned int parentId, unsigned int name);
void  Scr_KillThread(unsigned int parentId);
void  Scr_CheckLeakRange(unsigned int begin, unsigned int end);
void  Scr_CheckLeaks(void);

int  ThreadInfoCompare(_DWORD* info1, _DWORD* info2);
//int  VariableInfoCompare(void const*, void const*);
int VariableInfoFileNameCompare(_DWORD* info1, _DWORD* info2);
int VariableInfoCountCompare(_DWORD* info1, _DWORD* info2);
int VariableInfoFileLineCompare(_DWORD* info1, _DWORD* info2);
unsigned int  FindVariableIndexInternal2(unsigned int name, unsigned int index);
unsigned int FindVariableIndexInternal(unsigned int parentId, unsigned int name);
unsigned short  AllocVariable(void);
void  FreeVariable(unsigned int id);
unsigned int  AllocEntity(unsigned int classnum, unsigned short entnum);
float*  Scr_AllocVector(void);
unsigned int  FindArrayVariableIndex(unsigned int parentId, unsigned int unsignedValue);
unsigned int  Scr_FindArrayIndex(unsigned int parentId, VariableValue* index);
float  Scr_GetEntryUsage(unsigned int type, VariableUnion u);
float  Scr_GetEntryUsage(VariableValueInternal* entryValue);
float  Scr_GetObjectUsage(unsigned int parentId);
char*  Scr_GetSourceFile_FastFile(char const* filename);
char*  Scr_GetSourceFile(char const* filename);
void  Scr_AddFieldsForFile(char const* filename);
void  Scr_AddFields_FastFile(char const* path, char const* extension);
//void  CheckReferenceRange(unsigned int, unsigned int);
unsigned int  GetNewVariableIndexInternal3(unsigned int parentId, unsigned int name, unsigned int index);
unsigned int  GetNewVariableIndexInternal2(unsigned int parentId, unsigned int name, unsigned int index);
unsigned int  GetNewVariableIndexReverseInternal2(unsigned int parentId, unsigned int name, unsigned int index);
unsigned int  GetNewVariableIndexInternal(unsigned int parentId, unsigned int name);
unsigned int  GetNewVariableIndexReverseInternal(unsigned int parentId, unsigned int name);
void  MakeVariableExternal(unsigned int index, VariableValueInternal* parentValue);
void  FreeChildValue(unsigned int parentId, unsigned int id);
void  ClearObjectInternal(unsigned int parentId);
unsigned int  GetNewArrayVariableIndex(unsigned int parentId, unsigned int unsignedValue);
void  RemoveArrayVariable(unsigned int parentId, unsigned int unsignedValue);
void  CopyArray(unsigned int parentId, unsigned int newParentId);
void  Scr_CastWeakerPair(VariableValue* value1, VariableValue* value2);
void  Scr_CastWeakerStringPair(VariableValue* value1, VariableValue* value2);
//void  CopyEntity(unsigned int, unsigned int);
float  Scr_GetEndonUsage(unsigned int parentId);
float  Scr_GetThreadUsage(const VariableStackBuffer* stackBuf, float* endonUsage);
int  Scr_MakeValuePrimitive(unsigned int parentId);
void  SafeRemoveArrayVariable(unsigned int parentId, unsigned int unsignedValue);
VariableValue  Scr_EvalVariableEntityField(unsigned int entId, unsigned int fieldName);
void  Scr_ClearThread(unsigned int parentId);

//int marker_scr_variable

extern scr_classStruct_t g_classMap[4];
extern scrStringDebugGlob_t *scrStringDebugGlob;
extern scrMemTreePub_t scrMemTreePub;