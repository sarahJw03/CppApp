#include "DownloadThread.h"
#define CPPHTTPLIB_OPENSSL_SUPPORT
#include "httplib.h"
#include "nlohmann/json.hpp"

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Book, title, author, summary, isbn, coverImage, notes, isFavorite)

void DownloadThread::operator()(CommonObjects& common)
{
    httplib::Client cli("https://openlibrary.org");


    auto res = cli.Get("/search.json?title=programming");
    if (res && res->status == 200)
    {
        auto json_result = nlohmann::json::parse(res->body);
        for (auto& item : json_result["docs"])
        {
            Book book;
            book.title = item.value("title", "Unknown Title");
            book.author = item.contains("author_name") ? item["author_name"][0].get<std::string>() : "Unknown Author";


            if (item.contains("description"))
            {
                if (item["description"].is_string())
                    book.summary = item["description"].get<std::string>();
                else if (item["description"].contains("value"))
                    book.summary = item["description"]["value"].get<std::string>();
            }
            else
            {
                book.summary = "No description available";
            }

            book.isbn = item.contains("isbn") ? item["isbn"][0].get<std::string>() : "N/A";
            book.coverImage = item.contains("cover_i") ? "https://covers.openlibrary.org/b/id/" + std::to_string(item["cover_i"].get<int>()) + "-L.jpg" : "";

            common.books.push_back(book);
        }
        if (!common.books.empty())
            common.data_ready = true;
    }
}

void DownloadThread::SetUrl(std::string_view new_url)
{
    _download_url = new_url;
}