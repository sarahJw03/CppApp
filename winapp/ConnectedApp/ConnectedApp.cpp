#include <iostream>
#include <thread>
#include "CommonObject.h"
#include "DrawThread.h"
#include "DownloadThread.h"

int main()
{
    CommonObjects common;
    DrawThread draw;
    auto draw_th = std::jthread([&] {draw(common); });
    DownloadThread down;
    auto down_th = std::jthread([&] {down(common); });
    down.SetUrl("https://openlibrary.org/api/books?bibkeys=ISBN:0451526538&format=json&jscmd=data");
    std::cout << "running...\n";

    // Simulate user interaction
    std::this_thread::sleep_for(std::chrono::seconds(5)); // Allow time for some data to be processed
    common.show_favorites = true; // Toggle to show favorites

    down_th.join();
    draw_th.join();
}