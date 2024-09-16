#pragma once
#include "CommonObject.h"

class DrawThread
{
public:
    void operator()(CommonObjects& common);
};