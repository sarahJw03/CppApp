#pragma once
#include <atomic>
#include <string>
#include <vector>
#include <unordered_map>

struct Book
{
    std::string title;
    std::string author;
    std::string summary;
    std::string isbn;
    std::string coverImage;
    std::string notes; // Notes for personal use
    bool isFavorite = false; // Mark if the book is a favorite
};

struct CommonObjects
{
    std::atomic_bool exit_flag = false;
    std::atomic_bool start_download = false;
    std::atomic_bool data_ready = false;
    std::atomic_bool show_favorites = false; // Flag to show favorite books
    std::string url;
    std::vector<Book> books;
    std::vector<Book> favorite_books; // List of favorite books
};