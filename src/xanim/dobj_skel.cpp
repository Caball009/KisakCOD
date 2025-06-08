#include <stdafx.h>

#include "dobj.h"
#include <universal/profile.h>
#include "xanim.h"
#include "xanim_calc.h"

void __cdecl DObjCalcSkel(const DObj_s *obj, int *partBits)
{
    const unsigned __int8 *pos; // [esp+54h] [ebp-64h]
    int j; // [esp+58h] [ebp-60h]
    const unsigned __int8 *modelParents; // [esp+5Ch] [ebp-5Ch]
    const unsigned __int8 *duplicateParts; // [esp+60h] [ebp-58h]
    unsigned int boneIndex; // [esp+64h] [ebp-54h]
    unsigned int boneIndexa; // [esp+64h] [ebp-54h]
    int controlPartBits[4]; // [esp+68h] [ebp-50h] BYREF
    int numModels; // [esp+78h] [ebp-40h]
    XModel *model; // [esp+7Ch] [ebp-3Ch]
    unsigned int modelParent; // [esp+80h] [ebp-38h]
    DSkel *skel; // [esp+84h] [ebp-34h]
    int calcPartBits[4]; // [esp+88h] [ebp-30h] BYREF
    int ignorePartBits[4]; // [esp+98h] [ebp-20h] BYREF
    int i; // [esp+A8h] [ebp-10h]
    XModel **models; // [esp+ACh] [ebp-Ch]
    bool bFinished; // [esp+B3h] [ebp-5h]
    const int *savedDuplicatePartBits; // [esp+B4h] [ebp-4h]
    int savedregs; // [esp+B8h] [ebp+0h] BYREF

    Profile_Begin(310);
    iassert(obj);
    skel = (DSkel *)&obj->skel;
    iassert(skel);
    bFinished = 1;
    for (i = 0; i < 4; ++i)
    {
        ignorePartBits[i] = skel->partBits.skel[i] | ~partBits[i];
        if (ignorePartBits[i] != -1)
            bFinished = 0;
    }
    if (bFinished)
    {
        Profile_EndInternal(0);
        return;
    }
    else
    {
        DObjCalcAnim(obj, partBits);
        if (!obj->duplicateParts)
            MyAssertHandler(".\\xanim\\dobj_skel.cpp", 404, 0, "%s", "obj->duplicateParts");
        savedDuplicatePartBits = (const int *)SL_ConvertToString(obj->duplicateParts);
        duplicateParts = (const unsigned __int8 *)(savedDuplicatePartBits + 4);
        GetControlAndDuplicatePartBits(obj, partBits, ignorePartBits, savedDuplicatePartBits, calcPartBits, controlPartBits);
        numModels = obj->numModels;
        boneIndex = 0;
        pos = duplicateParts;
        models = obj->models;
        modelParents = (const unsigned __int8 *)&models[numModels];
        for (j = 0; j < numModels; ++j)
        {
            model = models[j];
            modelParent = modelParents[j];
            pos = CalcSkelDuplicateBones(model, skel, boneIndex, pos);
            if (modelParent == 255)
                CalcSkelRootBonesNoParentOrDuplicate(model, skel, boneIndex, calcPartBits);
            else
                CalcSkelRootBonesWithParent(model, skel, boneIndex, modelParent, calcPartBits, controlPartBits);
            CalcSkelNonRootBones(model, skel, boneIndex + model->numRootBones, calcPartBits, controlPartBits);
            boneIndex += model->numBones;
        }

        iassert(!(*pos));
        for (boneIndexa = 0; boneIndexa < obj->numBones; ++boneIndexa)
        {
            //if ((skel->partBits.anim[boneIndexa >> 5] & (0x80000000 >> (boneIndexa & 0x1F))) != 0)
            if (skel->partBits.anim.testBit(boneIndexa))
            {
                iassert(!IS_NAN(skel->mat[boneIndexa].quat[0]) 
                    && !IS_NAN(skel->mat[boneIndexa].quat[1]) 
                    && !IS_NAN(skel->mat[boneIndexa].quat[2]) 
                    && !IS_NAN(skel->mat[boneIndexa].quat[3])
                );
                iassert(!IS_NAN(skel->mat[boneIndexa].trans[0]) && !IS_NAN(skel->mat[boneIndexa].trans[1]) && !IS_NAN(skel->mat[boneIndexa].trans[2]));
            }
        }
        Profile_EndInternal(0);
    }
}

void __cdecl GetControlAndDuplicatePartBits(
    const DObj_s *obj,
    const int *partBits,
    const int *ignorePartBits,
    const int *savedDuplicatePartBits,
    int *calcPartBits,
    int *controlPartBits)
{
    const char *v6; // eax
    int boneIndex; // [esp+4h] [ebp-14h]
    DSkel *skel; // [esp+8h] [ebp-10h]
    int i; // [esp+10h] [ebp-8h]
    unsigned int boneIndexLow; // [esp+14h] [ebp-4h]

    skel = (DSkel *)&obj->skel;
    if (obj == (const DObj_s *)-20)
        MyAssertHandler(".\\xanim\\dobj_skel.cpp", 86, 0, "%s", "skel");
    for (i = 0; i < 4; ++i)
    {
        skel->partBits.skel[i] |= partBits[i];
        controlPartBits[i] = skel->partBits.control[i];
        calcPartBits[i] = ~(savedDuplicatePartBits[i] | ignorePartBits[i]);
        if ((savedDuplicatePartBits[i] & controlPartBits[i]) != 0)
        {
            DObjDumpInfo(obj);
            for (boneIndex = 0; boneIndex < obj->numBones; ++boneIndex)
            {
                boneIndexLow = 0x80000000 >> (boneIndex & 0x1F);
                if ((boneIndexLow & controlPartBits[boneIndex >> 5]) != 0
                    && (boneIndexLow & savedDuplicatePartBits[boneIndex >> 5]) != 0)
                {
                    break;
                }
            }
            if (!alwaysfails)
            {
                v6 = va("control/meld conflict on bone %d - see the console log for details", boneIndex);
                MyAssertHandler(".\\xanim\\dobj_skel.cpp", 107, 0, v6);
            }
        }
    }
}

const unsigned __int8 *__cdecl CalcSkelDuplicateBones(
    const XModel *model,
    DSkel *skel,
    int minBoneIndex,
    const unsigned __int8 *pos)
{
    int boneIndex; // [esp+8h] [ebp-10h]
    DObjAnimMat *mat; // [esp+Ch] [ebp-Ch]
    int parentIndex; // [esp+10h] [ebp-8h]
    unsigned int maxBoneIndex; // [esp+14h] [ebp-4h]

    mat = skel->mat;
    maxBoneIndex = minBoneIndex + model->numBones;
    while (1)
    {
        boneIndex = *pos - 1;
        if (boneIndex >= maxBoneIndex)
            break;
        parentIndex = pos[1] - 1;
        if (parentIndex >= boneIndex)
            MyAssertHandler(".\\xanim\\dobj_skel.cpp", 131, 0, "%s", "parentIndex < boneIndex");
        memcpy(&mat[boneIndex], &mat[parentIndex], sizeof(DObjAnimMat));
        pos += 2;
    }
    return pos;
}

void __cdecl CalcSkelRootBonesNoParentOrDuplicate(
    const XModel *model,
    DSkel *skel,
    int minBoneIndex,
    int *calcPartBits)
{
    DWORD v5; // eax
    int v6; // [esp+0h] [ebp-50h]
    float *v; // [esp+20h] [ebp-30h]
    float v8; // [esp+24h] [ebp-2Ch]
    int boneIndex; // [esp+2Ch] [ebp-24h]
    int maxBoneIndexHigh; // [esp+30h] [ebp-20h]
    unsigned int bits; // [esp+38h] [ebp-18h]
    DObjAnimMat *mat; // [esp+3Ch] [ebp-14h]
    int boneIndexHigh; // [esp+40h] [ebp-10h]
    int boneIndexLow; // [esp+44h] [ebp-Ch]
    int maxBoneIndexa; // [esp+4Ch] [ebp-4h]
    int maxBoneIndex; // [esp+4Ch] [ebp-4h]

    maxBoneIndexa = minBoneIndex + model->numRootBones;
    boneIndexHigh = minBoneIndex >> 5;
    maxBoneIndexHigh = (maxBoneIndexa - 1) >> 5;
    maxBoneIndex = maxBoneIndexa - 32 * (minBoneIndex >> 5);
    mat = skel->mat;
    while (boneIndexHigh <= maxBoneIndexHigh)
    {
        bits = calcPartBits[boneIndexHigh];
        if (maxBoneIndex > 32)
            v6 = 32;
        else
            v6 = maxBoneIndex;
        while (1)
        {
            if (!_BitScanReverse(&v5, bits))
                v5 = 63; // `CountLeadingZeros'::`2': : notFound;
            boneIndexLow = v5 ^ 0x1F;
            if ((v5 ^ 0x1F) >= v6)
                break;
            boneIndex = boneIndexLow + 32 * boneIndexHigh;
            if (((0x80000000 >> boneIndexLow) & bits) == 0)
                MyAssertHandler(".\\xanim\\dobj_skel.cpp", 174, 0, "%s", "bits & boneBit");
            bits &= ~(0x80000000 >> boneIndexLow);
            calcPartBits[boneIndexHigh] = bits;
            v = mat[boneIndex].quat;
            v8 = Vec4LengthSq(v);
            if (v8 == 0.0)
            {
                v[3] = 1.0;
                v[7] = 2.0;
            }
            else
            {
                v[7] = 2.0 / v8;
            }
            if ((COERCE_UNSIGNED_INT(mat[boneIndex].quat[0]) & 0x7F800000) == 0x7F800000
                || (COERCE_UNSIGNED_INT(mat[boneIndex].quat[1]) & 0x7F800000) == 0x7F800000
                || (COERCE_UNSIGNED_INT(mat[boneIndex].quat[2]) & 0x7F800000) == 0x7F800000
                || (COERCE_UNSIGNED_INT(mat[boneIndex].quat[3]) & 0x7F800000) == 0x7F800000)
            {
                MyAssertHandler(
                    ".\\xanim\\dobj_skel.cpp",
                    180,
                    0,
                    "%s",
                    "!IS_NAN((mat[boneIndex].quat)[0]) && !IS_NAN((mat[boneIndex].quat)[1]) && !IS_NAN((mat[boneIndex].quat)[2]) &&"
                    " !IS_NAN((mat[boneIndex].quat)[3])");
            }
            if ((COERCE_UNSIGNED_INT(mat[boneIndex].trans[0]) & 0x7F800000) == 0x7F800000
                || (COERCE_UNSIGNED_INT(mat[boneIndex].trans[1]) & 0x7F800000) == 0x7F800000
                || (COERCE_UNSIGNED_INT(mat[boneIndex].trans[2]) & 0x7F800000) == 0x7F800000)
            {
                MyAssertHandler(
                    ".\\xanim\\dobj_skel.cpp",
                    181,
                    0,
                    "%s",
                    "!IS_NAN((mat[boneIndex].trans)[0]) && !IS_NAN((mat[boneIndex].trans)[1]) && !IS_NAN((mat[boneIndex].trans)[2])");
            }
        }
        ++boneIndexHigh;
        maxBoneIndex -= 32;
    }
}

void __cdecl CalcSkelRootBonesWithParent(
    const XModel *model,
    DSkel *skel,
    unsigned int minBoneIndex,
    unsigned int modelParent,
    int *calcPartBits,
    const int *controlPartBits)
{
    DWORD v7; // eax
    unsigned int v8; // [esp+8h] [ebp-104h]
    float *trans; // [esp+18h] [ebp-F4h]
    float v10; // [esp+30h] [ebp-DCh]
    float v11; // [esp+34h] [ebp-D8h]
    float v12; // [esp+38h] [ebp-D4h]
    float v13; // [esp+3Ch] [ebp-D0h]
    float result[3]; // [esp+40h] [ebp-CCh] BYREF
    float v15; // [esp+4Ch] [ebp-C0h]
    float v16; // [esp+50h] [ebp-BCh]
    float v17; // [esp+54h] [ebp-B8h]
    float v18; // [esp+58h] [ebp-B4h]
    float v19; // [esp+5Ch] [ebp-B0h]
    float v20; // [esp+60h] [ebp-ACh]
    float v21; // [esp+64h] [ebp-A8h]
    float v22; // [esp+68h] [ebp-A4h]
    float v23; // [esp+6Ch] [ebp-A0h]
    float v24; // [esp+70h] [ebp-9Ch]
    float v25; // [esp+74h] [ebp-98h]
    float v26; // [esp+78h] [ebp-94h]
    float v27; // [esp+7Ch] [ebp-90h]
    float v28; // [esp+80h] [ebp-8Ch]
    float v29; // [esp+84h] [ebp-88h]
    float v30; // [esp+88h] [ebp-84h]
    float v31; // [esp+8Ch] [ebp-80h]
    float v32; // [esp+90h] [ebp-7Ch]
    float v33; // [esp+94h] [ebp-78h]
    float v34; // [esp+98h] [ebp-74h]
    float v35; // [esp+9Ch] [ebp-70h]
    float v36; // [esp+A0h] [ebp-6Ch]
    float v37; // [esp+A4h] [ebp-68h]
    float v38; // [esp+A8h] [ebp-64h]
    float v39; // [esp+ACh] [ebp-60h]
    float v40; // [esp+B0h] [ebp-5Ch]
    float v41; // [esp+B4h] [ebp-58h]
    float v42; // [esp+B8h] [ebp-54h]
    float v43; // [esp+BCh] [ebp-50h]
    float v44; // [esp+C0h] [ebp-4Ch]
    float v45; // [esp+C4h] [ebp-48h]
    float v46; // [esp+C8h] [ebp-44h]
    float v47; // [esp+CCh] [ebp-40h]
    const DObjAnimMat *parentMat; // [esp+D0h] [ebp-3Ch]
    DObjAnimMat *childMat; // [esp+D4h] [ebp-38h]
    unsigned int boneIndex; // [esp+D8h] [ebp-34h]
    unsigned int maxBoneIndexHigh; // [esp+DCh] [ebp-30h]
    unsigned int maxBoneIndexLow; // [esp+E0h] [ebp-2Ch]
    float quat[4]; // [esp+E4h] [ebp-28h]
    int bits; // [esp+F4h] [ebp-18h]
    const DObjAnimMat *mat; // [esp+F8h] [ebp-14h]
    unsigned int boneIndexHigh; // [esp+FCh] [ebp-10h]
    unsigned int boneIndexLow; // [esp+100h] [ebp-Ch]
    int boneBit; // [esp+104h] [ebp-8h]
    unsigned int maxBoneIndex; // [esp+108h] [ebp-4h]

    maxBoneIndex = minBoneIndex + model->numRootBones;
    boneIndexHigh = minBoneIndex >> 5;
    maxBoneIndexHigh = (maxBoneIndex - 1) >> 5;
    maxBoneIndex -= 32 * (minBoneIndex >> 5);
    mat = skel->mat;
    parentMat = &mat[modelParent];
    while (boneIndexHigh <= maxBoneIndexHigh)
    {
        bits = calcPartBits[boneIndexHigh];
        if (maxBoneIndex > 32)
            v8 = 32;
        else
            v8 = maxBoneIndex;
        maxBoneIndexLow = v8;
        while (1)
        {
            if (!_BitScanReverse(&v7, bits))
                v7 = 63;
            boneIndexLow = v7 ^ 0x1F;
            if ((v7 ^ 0x1Fu) >= maxBoneIndexLow)
                break;
            boneIndex = boneIndexLow + 32 * boneIndexHigh;
            boneBit = 0x80000000 >> boneIndexLow;
            if (((0x80000000 >> boneIndexLow) & bits) == 0)
                MyAssertHandler(".\\xanim\\dobj_skel.cpp", 227, 0, "%s", "bits & boneBit");
            bits &= ~boneBit;
            calcPartBits[boneIndexHigh] = bits;
            if (modelParent >= boneIndex)
                MyAssertHandler(".\\xanim\\dobj_skel.cpp", 231, 0, "%s", "modelParent < boneIndex");
            if ((boneBit & skel->partBits.anim[boneIndexHigh]) == 0)
                MyAssertHandler(".\\xanim\\dobj_skel.cpp", 232, 0, "%s", "skel->partBits.anim[boneIndexHigh] & boneBit");
            if ((skel->partBits.skel[modelParent >> 5] & (0x80000000 >> (modelParent & 0x1F))) == 0)
                MyAssertHandler(
                    ".\\xanim\\dobj_skel.cpp",
                    233,
                    0,
                    "%s",
                    "skel->partBits.skel[modelParent >> 5] & (HIGH_BIT >> (modelParent & 31))");
            if ((skel->partBits.anim[modelParent >> 5] & (0x80000000 >> (modelParent & 0x1F))) == 0)
                MyAssertHandler(
                    ".\\xanim\\dobj_skel.cpp",
                    234,
                    0,
                    "%s",
                    "skel->partBits.anim[modelParent >> 5] & (HIGH_BIT >> (modelParent & 31))");
            childMat = &skel->mat[boneIndex];
            if ((boneBit & controlPartBits[boneIndexHigh]) != 0)
            {
                if (skel->partBits.skel[0] >= 0)
                    MyAssertHandler(".\\xanim\\dobj_skel.cpp", 243, 0, "%s", "skel->partBits.skel[0] & HIGH_BIT");
                if (skel->partBits.anim[0] >= 0)
                    MyAssertHandler(".\\xanim\\dobj_skel.cpp", 244, 0, "%s", "skel->partBits.anim[0] & HIGH_BIT");
                quat[0] = parentMat->quat[0] * mat->quat[3]
                    - parentMat->quat[3] * mat->quat[0]
                    - parentMat->quat[2] * mat->quat[1]
                    + parentMat->quat[1] * mat->quat[2];
                quat[1] = parentMat->quat[1] * mat->quat[3]
                    + parentMat->quat[2] * mat->quat[0]
                    - parentMat->quat[3] * mat->quat[1]
                    - parentMat->quat[0] * mat->quat[2];
                quat[2] = parentMat->quat[2] * mat->quat[3]
                    - parentMat->quat[1] * mat->quat[0]
                    + parentMat->quat[0] * mat->quat[1]
                    - parentMat->quat[3] * mat->quat[2];
                quat[3] = parentMat->quat[3] * mat->quat[3]
                    + parentMat->quat[0] * mat->quat[0]
                    + parentMat->quat[1] * mat->quat[1]
                    + parentMat->quat[2] * mat->quat[2];
                v42 = quat[0] * childMat->quat[3]
                    + quat[3] * childMat->quat[0]
                    + quat[2] * childMat->quat[1]
                    - quat[1] * childMat->quat[2];
                v43 = quat[1] * childMat->quat[3]
                    - quat[2] * childMat->quat[0]
                    + quat[3] * childMat->quat[1]
                    + quat[0] * childMat->quat[2];
                v44 = quat[2] * childMat->quat[3]
                    + quat[1] * childMat->quat[0]
                    - quat[0] * childMat->quat[1]
                    + quat[3] * childMat->quat[2];
                childMat->quat[3] = quat[3] * childMat->quat[3]
                    - quat[0] * childMat->quat[0]
                    - quat[1] * childMat->quat[1]
                    - quat[2] * childMat->quat[2];
                childMat->quat[0] = v42;
                childMat->quat[1] = v43;
                childMat->quat[2] = v44;
                v39 = childMat->quat[0] * mat->quat[3]
                    + childMat->quat[3] * mat->quat[0]
                    + childMat->quat[2] * mat->quat[1]
                    - childMat->quat[1] * mat->quat[2];
                v40 = childMat->quat[1] * mat->quat[3]
                    - childMat->quat[2] * mat->quat[0]
                    + childMat->quat[3] * mat->quat[1]
                    + childMat->quat[0] * mat->quat[2];
                v41 = childMat->quat[2] * mat->quat[3]
                    + childMat->quat[1] * mat->quat[0]
                    - childMat->quat[0] * mat->quat[1]
                    + childMat->quat[3] * mat->quat[2];
                childMat->quat[3] = childMat->quat[3] * mat->quat[3]
                    - childMat->quat[0] * mat->quat[0]
                    - childMat->quat[1] * mat->quat[1]
                    - childMat->quat[2] * mat->quat[2];
                childMat->quat[0] = v39;
                childMat->quat[1] = v40;
                childMat->quat[2] = v41;
            }
            else
            {
                v45 = childMat->quat[0] * parentMat->quat[3]
                    + childMat->quat[3] * parentMat->quat[0]
                    + childMat->quat[2] * parentMat->quat[1]
                    - childMat->quat[1] * parentMat->quat[2];
                v46 = childMat->quat[1] * parentMat->quat[3]
                    - childMat->quat[2] * parentMat->quat[0]
                    + childMat->quat[3] * parentMat->quat[1]
                    + childMat->quat[0] * parentMat->quat[2];
                v47 = childMat->quat[2] * parentMat->quat[3]
                    + childMat->quat[1] * parentMat->quat[0]
                    - childMat->quat[0] * parentMat->quat[1]
                    + childMat->quat[3] * parentMat->quat[2];
                childMat->quat[3] = childMat->quat[3] * parentMat->quat[3]
                    - childMat->quat[0] * parentMat->quat[0]
                    - childMat->quat[1] * parentMat->quat[1]
                    - childMat->quat[2] * parentMat->quat[2];
                childMat->quat[0] = v45;
                childMat->quat[1] = v46;
                childMat->quat[2] = v47;
            }
            v38 = childMat->quat[0];
            if ((LODWORD(v38) & 0x7F800000) == 0x7F800000
                || (v37 = childMat->quat[1], (LODWORD(v37) & 0x7F800000) == 0x7F800000)
                || (v36 = childMat->quat[2], (LODWORD(v36) & 0x7F800000) == 0x7F800000)
                || (v35 = childMat->quat[3], (LODWORD(v35) & 0x7F800000) == 0x7F800000))
            {
                MyAssertHandler(
                    ".\\xanim\\dobj_skel.cpp",
                    251,
                    0,
                    "%s",
                    "!IS_NAN((childMat->quat)[0]) && !IS_NAN((childMat->quat)[1]) && !IS_NAN((childMat->quat)[2]) && !IS_NAN((childMat->quat)[3])");
            }
            v34 = childMat->trans[0];
            if ((LODWORD(v34) & 0x7F800000) == 0x7F800000
                || (v33 = childMat->trans[1], (LODWORD(v33) & 0x7F800000) == 0x7F800000)
                || (v32 = childMat->trans[2], (LODWORD(v32) & 0x7F800000) == 0x7F800000))
            {
                MyAssertHandler(
                    ".\\xanim\\dobj_skel.cpp",
                    252,
                    0,
                    "%s",
                    "!IS_NAN((childMat->trans)[0]) && !IS_NAN((childMat->trans)[1]) && !IS_NAN((childMat->trans)[2])");
            }
            v31 = Vec4LengthSq(childMat->quat);
            if (v31 == 0.0)
            {
                childMat->quat[3] = 1.0;
                childMat->transWeight = 2.0;
            }
            else
            {
                childMat->transWeight = 2.0 / v31;
            }
            trans = childMat->trans;
            if ((COERCE_UNSIGNED_INT(parentMat->quat[0]) & 0x7F800000) == 0x7F800000
                || (COERCE_UNSIGNED_INT(parentMat->quat[1]) & 0x7F800000) == 0x7F800000
                || (COERCE_UNSIGNED_INT(parentMat->quat[2]) & 0x7F800000) == 0x7F800000
                || (COERCE_UNSIGNED_INT(parentMat->quat[3]) & 0x7F800000) == 0x7F800000)
            {
                MyAssertHandler(
                    "c:\\trees\\cod3\\src\\xanim\\xanim_public.h",
                    432,
                    0,
                    "%s",
                    "!IS_NAN((mat->quat)[0]) && !IS_NAN((mat->quat)[1]) && !IS_NAN((mat->quat)[2]) && !IS_NAN((mat->quat)[3])");
            }
            if ((COERCE_UNSIGNED_INT(parentMat->transWeight) & 0x7F800000) == 0x7F800000)
                MyAssertHandler("c:\\trees\\cod3\\src\\xanim\\xanim_public.h", 433, 0, "%s", "!IS_NAN(mat->transWeight)");
            Vec3Scale(parentMat->quat, parentMat->transWeight, result);
            v18 = result[0] * parentMat->quat[0];
            v11 = result[0] * parentMat->quat[1];
            v16 = result[0] * parentMat->quat[2];
            v19 = result[0] * parentMat->quat[3];
            v10 = result[1] * parentMat->quat[1];
            v17 = result[1] * parentMat->quat[2];
            v15 = result[1] * parentMat->quat[3];
            v12 = result[2] * parentMat->quat[2];
            v13 = result[2] * parentMat->quat[3];
            v22 = 1.0 - (v10 + v12);
            v23 = v11 + v13;
            v24 = v16 - v15;
            v25 = v11 - v13;
            v26 = 1.0 - (v18 + v12);
            v27 = v17 + v19;
            v28 = v16 + v15;
            v29 = v17 - v19;
            v30 = 1.0 - (v18 + v10);
            v20 = *trans * v22 + trans[1] * v25 + trans[2] * v28 + parentMat->trans[0];
            v21 = *trans * v23 + trans[1] * v26 + trans[2] * v29 + parentMat->trans[1];
            trans[2] = *trans * v24 + trans[1] * v27 + trans[2] * v30 + parentMat->trans[2];
            *trans = v20;
            trans[1] = v21;
            if ((COERCE_UNSIGNED_INT(childMat->trans[0]) & 0x7F800000) == 0x7F800000
                || (COERCE_UNSIGNED_INT(childMat->trans[1]) & 0x7F800000) == 0x7F800000
                || (COERCE_UNSIGNED_INT(childMat->trans[2]) & 0x7F800000) == 0x7F800000)
            {
                MyAssertHandler(
                    ".\\xanim\\dobj_skel.cpp",
                    257,
                    0,
                    "%s",
                    "!IS_NAN((childMat->trans)[0]) && !IS_NAN((childMat->trans)[1]) && !IS_NAN((childMat->trans)[2])");
            }
        }
        ++boneIndexHigh;
        maxBoneIndex -= 32;
    }
}

void __cdecl CalcSkelNonRootBones(
    const XModel *model,
    DSkel *skel,
    int minBoneIndex,
    int *calcPartBits,
    const int *controlPartBits)
{
    DWORD v6; // eax
    int v7; // [esp+8h] [ebp-124h]
    float *v8; // [esp+18h] [ebp-114h]
    float v9; // [esp+30h] [ebp-FCh]
    float v10; // [esp+34h] [ebp-F8h]
    float v11; // [esp+38h] [ebp-F4h]
    float v12; // [esp+3Ch] [ebp-F0h]
    float result[3]; // [esp+40h] [ebp-ECh] BYREF
    float v14; // [esp+4Ch] [ebp-E0h]
    float v15; // [esp+50h] [ebp-DCh]
    float v16; // [esp+54h] [ebp-D8h]
    float v17; // [esp+58h] [ebp-D4h]
    float v18; // [esp+5Ch] [ebp-D0h]
    float v19; // [esp+60h] [ebp-CCh]
    float v20; // [esp+64h] [ebp-C8h]
    float v21; // [esp+68h] [ebp-C4h]
    float v22; // [esp+6Ch] [ebp-C0h]
    float v23; // [esp+70h] [ebp-BCh]
    float v24; // [esp+74h] [ebp-B8h]
    float v25; // [esp+78h] [ebp-B4h]
    float v26; // [esp+7Ch] [ebp-B0h]
    float v27; // [esp+80h] [ebp-ACh]
    float v28; // [esp+84h] [ebp-A8h]
    float v29; // [esp+88h] [ebp-A4h]
    float v30; // [esp+8Ch] [ebp-A0h]
    float v31; // [esp+90h] [ebp-9Ch]
    float v32; // [esp+94h] [ebp-98h]
    float v33; // [esp+98h] [ebp-94h]
    float v34; // [esp+9Ch] [ebp-90h]
    float v35; // [esp+A0h] [ebp-8Ch]
    float v36; // [esp+A4h] [ebp-88h]
    float v37; // [esp+A8h] [ebp-84h]
    float v38; // [esp+ACh] [ebp-80h]
    float v39; // [esp+B0h] [ebp-7Ch]
    float v40; // [esp+B4h] [ebp-78h]
    float v41; // [esp+B8h] [ebp-74h]
    float v42; // [esp+BCh] [ebp-70h]
    float v43; // [esp+C0h] [ebp-6Ch]
    float v44; // [esp+C4h] [ebp-68h]
    float v45; // [esp+C8h] [ebp-64h]
    float v46; // [esp+CCh] [ebp-60h]
    float v47; // [esp+D0h] [ebp-5Ch]
    float v48; // [esp+D4h] [ebp-58h]
    float v49; // [esp+D8h] [ebp-54h]
    int v50; // [esp+DCh] [ebp-50h]
    DObjAnimMat *childMat; // [esp+E0h] [ebp-4Ch]
    const DObjAnimMat *parentMat; // [esp+E4h] [ebp-48h]
    int boneIndex; // [esp+E8h] [ebp-44h]
    int maxBoneIndexHigh; // [esp+ECh] [ebp-40h]
    int maxBoneIndexLow; // [esp+F0h] [ebp-3Ch]
    const float *trans; // [esp+F4h] [ebp-38h]
    float quat[4]; // [esp+F8h] [ebp-34h]
    int bits; // [esp+108h] [ebp-24h]
    DObjAnimMat *mat; // [esp+10Ch] [ebp-20h]
    int boneOffset; // [esp+110h] [ebp-1Ch]
    const unsigned __int8 *parentList; // [esp+114h] [ebp-18h]
    int parentOffset; // [esp+118h] [ebp-14h]
    int boneIndexHigh; // [esp+11Ch] [ebp-10h]
    int boneIndexLow; // [esp+120h] [ebp-Ch]
    int boneBit; // [esp+124h] [ebp-8h]
    int maxBoneIndex; // [esp+128h] [ebp-4h]

    maxBoneIndex = minBoneIndex + model->numBones - model->numRootBones;
    boneIndexHigh = minBoneIndex >> 5;
    maxBoneIndexHigh = (maxBoneIndex - 1) >> 5;
    maxBoneIndex -= 32 * (minBoneIndex >> 5);
    mat = skel->mat;
    while (boneIndexHigh <= maxBoneIndexHigh)
    {
        bits = calcPartBits[boneIndexHigh];
        if (maxBoneIndex > 32)
            v7 = 32;
        else
            v7 = maxBoneIndex;
        maxBoneIndexLow = v7;
        while (1)
        {
            if (!_BitScanReverse(&v6, bits))
                v6 = 63;
            v50 = v6 ^ 0x1F;
            boneIndexLow = v6 ^ 0x1F;
            if ((v6 ^ 0x1F) >= maxBoneIndexLow)
                break;
            boneIndex = boneIndexLow + 32 * boneIndexHigh;
            boneBit = 0x80000000 >> boneIndexLow;
            if (((0x80000000 >> boneIndexLow) & bits) == 0)
                MyAssertHandler(".\\xanim\\dobj_skel.cpp", 306, 0, "%s", "bits & boneBit");
            bits &= ~boneBit;
            calcPartBits[boneIndexHigh] = bits;
            childMat = &mat[boneIndex];
            boneOffset = boneIndex - minBoneIndex;
            if (boneIndex - minBoneIndex >= model->numBones - model->numRootBones)
                MyAssertHandler(
                    ".\\xanim\\dobj_skel.cpp",
                    313,
                    0,
                    "boneOffset doesn't index model->numBones - model->numRootBones\n\t%i not in [0, %i)",
                    boneOffset,
                    model->numBones - model->numRootBones);
            parentList = &model->parentList[boneOffset];
            parentOffset = *parentList;
            parentMat = &childMat[-parentOffset];
            if ((boneBit & skel->partBits.anim[boneIndexHigh]) == 0)
                MyAssertHandler(".\\xanim\\dobj_skel.cpp", 319, 0, "%s", "skel->partBits.anim[boneIndexHigh] & boneBit");
            if ((skel->partBits.skel[(boneIndex - parentOffset) >> 5] & (0x80000000 >> ((boneIndex - parentOffset) & 0x1F))) == 0)
                MyAssertHandler(
                    ".\\xanim\\dobj_skel.cpp",
                    320,
                    0,
                    "%s",
                    "skel->partBits.skel[(boneIndex - parentOffset) >> 5] & (HIGH_BIT >> ((boneIndex - parentOffset) & 31))");
            if ((skel->partBits.anim[(boneIndex - parentOffset) >> 5] & (0x80000000 >> ((boneIndex - parentOffset) & 0x1F))) == 0)
                MyAssertHandler(
                    ".\\xanim\\dobj_skel.cpp",
                    321,
                    0,
                    "%s",
                    "skel->partBits.anim[(boneIndex - parentOffset) >> 5] & (HIGH_BIT >> ((boneIndex - parentOffset) & 31))");
            if ((boneBit & controlPartBits[boneIndexHigh]) != 0)
            {
                if (skel->partBits.skel[0] >= 0)
                    MyAssertHandler(".\\xanim\\dobj_skel.cpp", 329, 0, "%s", "skel->partBits.skel[0] & HIGH_BIT");
                if (skel->partBits.anim[0] >= 0)
                    MyAssertHandler(".\\xanim\\dobj_skel.cpp", 330, 0, "%s", "skel->partBits.anim[0] & HIGH_BIT");
                quat[0] = parentMat->quat[0] * mat->quat[3]
                    - parentMat->quat[3] * mat->quat[0]
                    - parentMat->quat[2] * mat->quat[1]
                    + parentMat->quat[1] * mat->quat[2];
                quat[1] = parentMat->quat[1] * mat->quat[3]
                    + parentMat->quat[2] * mat->quat[0]
                    - parentMat->quat[3] * mat->quat[1]
                    - parentMat->quat[0] * mat->quat[2];
                quat[2] = parentMat->quat[2] * mat->quat[3]
                    - parentMat->quat[1] * mat->quat[0]
                    + parentMat->quat[0] * mat->quat[1]
                    - parentMat->quat[3] * mat->quat[2];
                quat[3] = parentMat->quat[3] * mat->quat[3]
                    + parentMat->quat[0] * mat->quat[0]
                    + parentMat->quat[1] * mat->quat[1]
                    + parentMat->quat[2] * mat->quat[2];
                v44 = quat[0] * childMat->quat[3]
                    + quat[3] * childMat->quat[0]
                    + quat[2] * childMat->quat[1]
                    - quat[1] * childMat->quat[2];
                v45 = quat[1] * childMat->quat[3]
                    - quat[2] * childMat->quat[0]
                    + quat[3] * childMat->quat[1]
                    + quat[0] * childMat->quat[2];
                v46 = quat[2] * childMat->quat[3]
                    + quat[1] * childMat->quat[0]
                    - quat[0] * childMat->quat[1]
                    + quat[3] * childMat->quat[2];
                childMat->quat[3] = quat[3] * childMat->quat[3]
                    - quat[0] * childMat->quat[0]
                    - quat[1] * childMat->quat[1]
                    - quat[2] * childMat->quat[2];
                childMat->quat[0] = v44;
                childMat->quat[1] = v45;
                childMat->quat[2] = v46;
                v41 = childMat->quat[0] * mat->quat[3]
                    + childMat->quat[3] * mat->quat[0]
                    + childMat->quat[2] * mat->quat[1]
                    - childMat->quat[1] * mat->quat[2];
                v42 = childMat->quat[1] * mat->quat[3]
                    - childMat->quat[2] * mat->quat[0]
                    + childMat->quat[3] * mat->quat[1]
                    + childMat->quat[0] * mat->quat[2];
                v43 = childMat->quat[2] * mat->quat[3]
                    + childMat->quat[1] * mat->quat[0]
                    - childMat->quat[0] * mat->quat[1]
                    + childMat->quat[3] * mat->quat[2];
                childMat->quat[3] = childMat->quat[3] * mat->quat[3]
                    - childMat->quat[0] * mat->quat[0]
                    - childMat->quat[1] * mat->quat[1]
                    - childMat->quat[2] * mat->quat[2];
                childMat->quat[0] = v41;
                childMat->quat[1] = v42;
                childMat->quat[2] = v43;
            }
            else
            {
                v47 = childMat->quat[0] * parentMat->quat[3]
                    + childMat->quat[3] * parentMat->quat[0]
                    + childMat->quat[2] * parentMat->quat[1]
                    - childMat->quat[1] * parentMat->quat[2];
                v48 = childMat->quat[1] * parentMat->quat[3]
                    - childMat->quat[2] * parentMat->quat[0]
                    + childMat->quat[3] * parentMat->quat[1]
                    + childMat->quat[0] * parentMat->quat[2];
                v49 = childMat->quat[2] * parentMat->quat[3]
                    + childMat->quat[1] * parentMat->quat[0]
                    - childMat->quat[0] * parentMat->quat[1]
                    + childMat->quat[3] * parentMat->quat[2];
                childMat->quat[3] = childMat->quat[3] * parentMat->quat[3]
                    - childMat->quat[0] * parentMat->quat[0]
                    - childMat->quat[1] * parentMat->quat[1]
                    - childMat->quat[2] * parentMat->quat[2];
                childMat->quat[0] = v47;
                childMat->quat[1] = v48;
                childMat->quat[2] = v49;
            }
            v40 = childMat->quat[0];
            if ((LODWORD(v40) & 0x7F800000) == 0x7F800000
                || (v39 = childMat->quat[1], (LODWORD(v39) & 0x7F800000) == 0x7F800000)
                || (v38 = childMat->quat[2], (LODWORD(v38) & 0x7F800000) == 0x7F800000)
                || (v37 = childMat->quat[3], (LODWORD(v37) & 0x7F800000) == 0x7F800000))
            {
                MyAssertHandler(
                    ".\\xanim\\dobj_skel.cpp",
                    337,
                    0,
                    "%s",
                    "!IS_NAN((childMat->quat)[0]) && !IS_NAN((childMat->quat)[1]) && !IS_NAN((childMat->quat)[2]) && !IS_NAN((childMat->quat)[3])");
            }
            v36 = childMat->trans[0];
            iassert(!IS_NAN((childMat->trans)[0]) && !IS_NAN((childMat->trans)[1]) && !IS_NAN((childMat->trans)[2]));
            v33 = Vec4LengthSq(childMat->quat);
            if (v33 == 0.0)
            {
                childMat->quat[3] = 1.0;
                childMat->transWeight = 2.0;
            }
            else
            {
                childMat->transWeight = 2.0 / v33;
            }
            trans = &model->trans[3 * boneOffset];
            v32 = *trans;
            if ((LODWORD(v32) & 0x7F800000) == 0x7F800000
                || (v31 = trans[1], (LODWORD(v31) & 0x7F800000) == 0x7F800000)
                || (v30 = trans[2], (LODWORD(v30) & 0x7F800000) == 0x7F800000))
            {
                MyAssertHandler(
                    ".\\xanim\\dobj_skel.cpp",
                    343,
                    0,
                    "%s",
                    "!IS_NAN((trans)[0]) && !IS_NAN((trans)[1]) && !IS_NAN((trans)[2])");
            }
            Vec3Add(childMat->trans, trans, childMat->trans);
            v8 = childMat->trans;
            if ((COERCE_UNSIGNED_INT(parentMat->quat[0]) & 0x7F800000) == 0x7F800000
                || (COERCE_UNSIGNED_INT(parentMat->quat[1]) & 0x7F800000) == 0x7F800000
                || (COERCE_UNSIGNED_INT(parentMat->quat[2]) & 0x7F800000) == 0x7F800000
                || (COERCE_UNSIGNED_INT(parentMat->quat[3]) & 0x7F800000) == 0x7F800000)
            {
                MyAssertHandler(
                    "c:\\trees\\cod3\\src\\xanim\\xanim_public.h",
                    432,
                    0,
                    "%s",
                    "!IS_NAN((mat->quat)[0]) && !IS_NAN((mat->quat)[1]) && !IS_NAN((mat->quat)[2]) && !IS_NAN((mat->quat)[3])");
            }
            if ((COERCE_UNSIGNED_INT(parentMat->transWeight) & 0x7F800000) == 0x7F800000)
                MyAssertHandler("c:\\trees\\cod3\\src\\xanim\\xanim_public.h", 433, 0, "%s", "!IS_NAN(mat->transWeight)");
            Vec3Scale(parentMat->quat, parentMat->transWeight, result);
            v17 = result[0] * parentMat->quat[0];
            v10 = result[0] * parentMat->quat[1];
            v15 = result[0] * parentMat->quat[2];
            v18 = result[0] * parentMat->quat[3];
            v9 = result[1] * parentMat->quat[1];
            v16 = result[1] * parentMat->quat[2];
            v14 = result[1] * parentMat->quat[3];
            v11 = result[2] * parentMat->quat[2];
            v12 = result[2] * parentMat->quat[3];
            v21 = 1.0 - (v9 + v11);
            v22 = v10 + v12;
            v23 = v15 - v14;
            v24 = v10 - v12;
            v25 = 1.0 - (v17 + v11);
            v26 = v16 + v18;
            v27 = v15 + v14;
            v28 = v16 - v18;
            v29 = 1.0 - (v17 + v9);
            v19 = *v8 * v21 + v8[1] * v24 + v8[2] * v27 + parentMat->trans[0];
            v20 = *v8 * v22 + v8[1] * v25 + v8[2] * v28 + parentMat->trans[1];
            v8[2] = *v8 * v23 + v8[1] * v26 + v8[2] * v29 + parentMat->trans[2];
            *v8 = v19;
            v8[1] = v20;
            if ((COERCE_UNSIGNED_INT(childMat->trans[0]) & 0x7F800000) == 0x7F800000
                || (COERCE_UNSIGNED_INT(childMat->trans[1]) & 0x7F800000) == 0x7F800000
                || (COERCE_UNSIGNED_INT(childMat->trans[2]) & 0x7F800000) == 0x7F800000)
            {
                MyAssertHandler(
                    ".\\xanim\\dobj_skel.cpp",
                    348,
                    0,
                    "%s",
                    "!IS_NAN((childMat->trans)[0]) && !IS_NAN((childMat->trans)[1]) && !IS_NAN((childMat->trans)[2])");
            }
        }
        ++boneIndexHigh;
        maxBoneIndex -= 32;
    }
}

void __cdecl DObjCalcBaseSkel(const DObj_s *obj, DObjAnimMat *mat, int *partBits)
{
    const unsigned __int8 *pos; // [esp+8h] [ebp-94h]
    int j; // [esp+Ch] [ebp-90h]
    const unsigned __int8 *modelParents; // [esp+10h] [ebp-8Ch]
    const unsigned __int8 *duplicateParts; // [esp+14h] [ebp-88h]
    unsigned int boneIndex; // [esp+18h] [ebp-84h]
    int controlPartBits[4]; // [esp+1Ch] [ebp-80h] BYREF
    int numModels; // [esp+2Ch] [ebp-70h]
    XModel *model; // [esp+30h] [ebp-6Ch]
    unsigned int modelParent; // [esp+34h] [ebp-68h]
    DSkel skel; // [esp+38h] [ebp-64h] BYREF
    int calcPartBits[4]; // [esp+70h] [ebp-2Ch] BYREF
    int ignorePartBits[4]; // [esp+80h] [ebp-1Ch] BYREF
    int i; // [esp+90h] [ebp-Ch]
    XModel **models; // [esp+94h] [ebp-8h]
    const int *savedDuplicatePartBits; // [esp+98h] [ebp-4h]

    if (!obj)
        MyAssertHandler(".\\xanim\\dobj_skel.cpp", 531, 0, "%s", "obj");
    if (!mat)
        MyAssertHandler(".\\xanim\\dobj_skel.cpp", 532, 0, "%s", "mat");
    skel.mat = mat;
    skel.timeStamp = 0;
    for (i = 0; i < 4; ++i)
    {
        skel.partBits.skel[i] = partBits[i];
        skel.partBits.anim[i] = skel.partBits.skel[i];
        skel.partBits.control[i] = 0;
        ignorePartBits[i] = ~partBits[i];
    }
    DObjCalcBaseAnim(obj, mat, partBits);
    if (!obj->duplicateParts)
        MyAssertHandler(".\\xanim\\dobj_skel.cpp", 546, 0, "%s", "obj->duplicateParts");
    savedDuplicatePartBits = (const int *)SL_ConvertToString(obj->duplicateParts);
    duplicateParts = (const unsigned __int8 *)(savedDuplicatePartBits + 4);
    DObjGetBaseControlAndDuplicatePartBits(
        obj,
        partBits,
        ignorePartBits,
        savedDuplicatePartBits,
        calcPartBits,
        controlPartBits);
    numModels = obj->numModels;
    boneIndex = 0;
    pos = duplicateParts;
    models = obj->models;
    modelParents = (const unsigned __int8 *)&models[numModels];
    for (j = 0; j < numModels; ++j)
    {
        model = models[j];
        modelParent = modelParents[j];
        pos = CalcSkelDuplicateBones(model, &skel, boneIndex, pos);
        if (modelParent == 255)
            CalcSkelRootBonesNoParentOrDuplicate(model, &skel, boneIndex, calcPartBits);
        else
            CalcSkelRootBonesWithParent(model, &skel, boneIndex, modelParent, calcPartBits, controlPartBits);
        CalcSkelNonRootBones(model, &skel, boneIndex + model->numRootBones, calcPartBits, controlPartBits);
        boneIndex += model->numBones;
    }
}

void __cdecl DObjCalcBaseAnim(const DObj_s *obj, DObjAnimMat *mat, int *partBits)
{
    int j; // [esp+18h] [ebp-14h]
    int boneIndex; // [esp+1Ch] [ebp-10h]
    XModel *model; // [esp+20h] [ebp-Ch]
    __int16 *quats; // [esp+24h] [ebp-8h]
    int i; // [esp+28h] [ebp-4h]
    int ia; // [esp+28h] [ebp-4h]

    if (!obj)
        MyAssertHandler(".\\xanim\\dobj_skel.cpp", 459, 0, "%s", "obj");
    if (!mat)
        MyAssertHandler(".\\xanim\\dobj_skel.cpp", 460, 0, "%s", "mat");
    boneIndex = 0;
    for (j = 0; j < obj->numModels; ++j)
    {
        model = obj->models[j];
        for (i = model->numRootBones; i; --i)
        {
            mat->quat[0] = 0.0;
            mat->quat[1] = 0.0;
            mat->quat[2] = 0.0;
            mat->quat[3] = 1.0;
            mat->trans[0] = 0.0;
            mat->trans[1] = 0.0;
            mat->trans[2] = 0.0;
            ++mat;
            ++boneIndex;
        }
        quats = model->quats;
        ia = model->numBones - model->numRootBones;
        while (ia)
        {
            if ((partBits[boneIndex >> 5] & (0x80000000 >> (boneIndex & 0x1F))) != 0)
            {
                mat->quat[0] = (double)*quats * 0.00003051850944757462;
                mat->quat[1] = (double)quats[1] * 0.00003051850944757462;
                mat->quat[2] = (double)quats[2] * 0.00003051850944757462;
                mat->quat[3] = (double)quats[3] * 0.00003051850944757462;
                mat->trans[0] = 0.0;
                mat->trans[1] = 0.0;
                mat->trans[2] = 0.0;
            }
            --ia;
            ++mat;
            ++boneIndex;
            quats += 4;
        }
    }
}

void __cdecl DObjGetBaseControlAndDuplicatePartBits(
    const DObj_s *obj,
    const int *partBits,
    const int *ignorePartBits,
    const int *savedDuplicatePartBits,
    int *calcPartBits,
    int *controlPartBits)
{
    int i; // [esp+4h] [ebp-4h]

    for (i = 0; i < 4; ++i)
    {
        controlPartBits[i] = 0;
        calcPartBits[i] = ~(savedDuplicatePartBits[i] | ignorePartBits[i]);
    }
}

