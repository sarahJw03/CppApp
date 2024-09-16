#pragma once
#include "CommonObject.h"

class DownloadThread
{
public:
    void operator()(CommonObjects& common);
    void SetUrl(std::string_view new_url);
private:
    std::string _download_url;
};