#include <stdafx.h>

#include "buildnumber_mp.h"
#include <stdio.h>
#include <version.h>

char buildnumbuf[128];

char *__cdecl getBuildNumber()
{
	sprintf_s(buildnumbuf, "%d %s %s", VERSION_NUMBER, VERSION_HOST, VERSION_STRING);
	return buildnumbuf;
}

int getBuildNumberAsInt()
{
	return VERSION_NUMBER;
}