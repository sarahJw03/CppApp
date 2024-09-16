#include "DrawThread.h"
#include "GuiMain.h"
#include <../../shared/ImGuiSrc/imgui.h>
#include <httplib.h>
// Buffer for search input and note input
static char search_buff[200] = "";
static char note_buff[1024] = ""; // Buffer to hold note input
bool addNote = false;
bool details = false;
static Book* selected_book = nullptr; // To keep track of the currently selected book for notes
static std::vector<Book> filtered_books; // To keep the filtered list of books
static bool no_books_found = false; // Flag to indicate if no books were found

// Helper function to convert strings to lowercase
std::string toLower(const std::string& str) {
    std::string lower_str = str;
    std::transform(lower_str.begin(), lower_str.end(), lower_str.begin(), ::tolower);
    return lower_str;
}

void DrawFavoritesWindow(CommonObjects* common)
{
    ImGui::Begin("Favorite Books");

    if (ImGui::BeginTable("Favorites", 5)) // Change column count to 5
    {
        ImGui::TableSetupColumn("Title", ImGuiTableColumnFlags_WidthStretch); // Stretchable width
        ImGui::TableSetupColumn("Author");
        ImGui::TableSetupColumn("Details");
        ImGui::TableSetupColumn("Add Note"); // Add Add Note column
        ImGui::TableSetupColumn("Remove"); // Add Remove column
        ImGui::TableHeadersRow();

        for (int i = 0; i < common->favorite_books.size(); i++)
        {
            Book& book = common->favorite_books[i];
            ImGui::TableNextRow();

            // Title
            ImGui::TableSetColumnIndex(0);
            ImGui::Text("%s", book.title.c_str());

            // Author
            ImGui::TableSetColumnIndex(1);
            ImGui::Text("%s", book.author.c_str());

            // Details
            ImGui::TableSetColumnIndex(2);
            ImGui::PushID(i + 10000); // Use a unique ID for the button

            if (ImGui::Button("Details")) // Details button
            {
                details = true;
                addNote = false;
                selected_book = &book;
                ImGui::OpenPopup("Book Details"); // Open details popup on click
            }

            ImGui::PopID();

            // Add Note
            ImGui::TableSetColumnIndex(3);
            ImGui::PushID(i + 20000); // Use a unique ID for the button

            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(1.0f, 1.0f, 0.0f, 1.0f)); // Yellow color
            if (ImGui::Button("Add Note")) // Add Note button
            {
                addNote = true;
                details = false;
                selected_book = &book;
                strncpy_s(note_buff, sizeof(note_buff), selected_book->notes.c_str(), _TRUNCATE); // Load current note
                ImGui::OpenPopup("Add Note"); // Open Add Note popup on click
            }
            ImGui::PopStyleColor(); // Restore previous color

            ImGui::PopID();

            // Remove
            ImGui::TableSetColumnIndex(4);
            ImGui::PushID(i); // Use a unique ID for the button

            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(1.0f, 0.0f, 0.0f, 1.0f)); // Red color
            if (ImGui::Button("Remove")) // Remove button
            {
                // Remove the book from the favorite_books list
                auto it = std::remove_if(common->favorite_books.begin(), common->favorite_books.end(), [&](const Book& b) {
                    return b.isbn == book.isbn;
                    });
                common->favorite_books.erase(it, common->favorite_books.end());

                // Also, mark the book as not favorite in the books list
                for (auto& b : common->books)
                {
                    if (b.isbn == book.isbn)
                    {
                        b.isFavorite = false;
                        break;
                    }
                }

                // Reset selected_book if it's pointing to the removed book
                if (selected_book && selected_book->isbn == book.isbn)
                {
                    selected_book = nullptr;
                }

                // Refilter books to update UI
                std::string search_query = std::string(search_buff);
                filtered_books.clear();
                if (!search_query.empty())
                {
                    for (const auto& b : common->books)
                    {
                        if (b.title.find(search_query) != std::string::npos ||
                            b.author.find(search_query) != std::string::npos)
                        {
                            filtered_books.push_back(b);
                        }
                    }
                    no_books_found = filtered_books.empty(); // Set flag if no books found
                }
                else
                {
                    filtered_books = common->books; // Show all books if search query is empty
                    no_books_found = false; // No need to show message if search is cleared
                }
            }
            ImGui::PopStyleColor(); // Restore previous color

            ImGui::PopID();
        }
        ImGui::EndTable();
    }
    ImGui::End();
}

void DrawAppWindow(void* common_ptr)
{
    auto common = (CommonObjects*)common_ptr;
    ImGui::Begin("Books Manager");
    ImGui::Text("Search for books by title or author");

    // Input text for search
    if (ImGui::InputText("Search", search_buff, sizeof(search_buff)))
    {
        // Update filtered_books based on search input
        std::string search_query = toLower(std::string(search_buff));  // Convert query to lowercase
        filtered_books.clear();
        if (!search_query.empty())
        {
            bool found_books = false;
            for (const auto& book : common->books)
            {
                // Check if the book title or author matches the search query (case-insensitive)
                if (toLower(book.title).find(search_query) != std::string::npos ||  // Convert title to lowercase
                    toLower(book.author).find(search_query) != std::string::npos)   // Convert author to lowercase
                {
                    filtered_books.push_back(book);
                    found_books = true;
                }
            }
            no_books_found = !found_books; // Set flag if no books found
        }
        else
        {
            filtered_books = common->books; // Show all books if search query is empty
            no_books_found = false; // No need to show message if search is cleared
        }
    }

    // Display the filtered books or all books if no filter is applied
    if (common->data_ready)
    {
        if (ImGui::BeginTable("Books", 5)) // Change column count to 5
        {
            ImGui::TableSetupColumn("Title", ImGuiTableColumnFlags_WidthStretch); // Stretchable width
            ImGui::TableSetupColumn("Author");
            ImGui::TableSetupColumn("Details");
            ImGui::TableSetupColumn("Favorites"); // Changed to "Favorites"
            ImGui::TableSetupColumn("Add Note"); // Add Add Note column
            ImGui::TableHeadersRow();

            // Show the filtered books list
            for (int i = 0; i < filtered_books.size(); i++)
            {
                Book& book = filtered_books[i];
                ImGui::TableNextRow();

                ImGui::TableSetColumnIndex(0);
                ImGui::Text("%s", book.title.c_str());

                ImGui::TableSetColumnIndex(1);
                ImGui::Text("%s", book.author.c_str());

                ImGui::TableSetColumnIndex(2); // Move to "Details" column
                ImGui::PushID(i + 1000); // Use a unique ID for the button

                if (ImGui::Button("Details")) // Details button
                {
                    details = true;
                    addNote = false;
                    selected_book = &book;
                    ImGui::OpenPopup("Book Details"); // Open details popup on click
                }

                ImGui::PopID();

                ImGui::TableSetColumnIndex(3); // Move to "Favorites" column

                ImGui::PushID(i);

                // Favorite button
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0f, 1.0f, 0.0f, 1.0f)); // Green color
                const char* buttonText = book.isFavorite ? "Unfavorite" : "Favorite"; // Toggle text based on favorite status
                if (ImGui::Button(buttonText))
                {
                    // Toggle favorite state
                    book.isFavorite = !book.isFavorite;
                    if (book.isFavorite)
                    {
                        common->favorite_books.push_back(book);
                    }
                    else
                    {
                        auto it = std::remove_if(common->favorite_books.begin(), common->favorite_books.end(), [&](const Book& b) {
                            return b.isbn == book.isbn;
                            });
                        common->favorite_books.erase(it, common->favorite_books.end());
                    }
                }
                ImGui::PopStyleColor(); // Restore previous color

                ImGui::PopID();

                ImGui::TableSetColumnIndex(4); // Move to "Add Note" column
                ImGui::PushID(i + 2000); // Use a unique ID for the button

                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(1.0f, 1.0f, 0.0f, 1.0f)); // Yellow color
                if (ImGui::Button("Add Note")) // Add Note button
                {
                    addNote = true;
                    details = false;
                    selected_book = &book;
                    strncpy_s(note_buff, sizeof(note_buff), selected_book->notes.c_str(), _TRUNCATE); // Load current note
                    ImGui::OpenPopup("Add Note"); // Open Add Note popup on click
                }
                ImGui::PopStyleColor(); // Restore previous color

                ImGui::PopID();
            }
            ImGui::EndTable();
        }

        // Show "No books found" message if no books match the search
        if (no_books_found)
        {
            ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "No books found matching your search.");
        }
    }

    // Call to draw favorites window
    DrawFavoritesWindow(common);

    // Modal for adding notes
    if (selected_book)
    {
        ImGui::OpenPopup("Add Note");
        if (addNote && ImGui::BeginPopupModal("Add Note", NULL, ImGuiWindowFlags_AlwaysAutoResize))
        {
            ImGui::InputTextMultiline("Notes", note_buff, sizeof(note_buff), ImVec2(400, 200));
            if (ImGui::Button("Save"))
            {
                if (selected_book)
                {
                    selected_book->notes = note_buff;
                }
                addNote = false;

                // Update notes in the favorite_books list
             //   auto it = std::find_if(common->favorite_books.begin(), common->favorite_books.end(), [&](Book& b) {
//return b.isbn == selected_book->isbn;
                 //       });
                //    if (it != common->favorite_books.end())
                //    {
                 //       it->notes = note_buff;
                 //   }


            //    selected_book = nullptr; // Reset after saving
            //    ImGui::CloseCurrentPopup();
           // }

          //  ImGui::SameLine();
          //  if (ImGui::Button("Cancel"))
          //  {
          //      selected_book = nullptr; // Reset after canceling
          //      ImGui::CloseCurrentPopup();
          //  }
                selected_book = nullptr;
                ImGui::CloseCurrentPopup();
            }
            ImGui::SameLine();
            if (ImGui::Button("Cancel")) {
                addNote = false; // إغلاق حالة الملاحظة عند الإلغاء
                selected_book = nullptr;
                ImGui::CloseCurrentPopup();
            }

            ImGui::EndPopup();
        }
    }

    // Modal for book details
    if (details && selected_book && ImGui::BeginPopupModal("Book Details", NULL, ImGuiWindowFlags_AlwaysAutoResize))
    {
        ImGui::Text("Title: %s", selected_book->title.c_str());
        ImGui::Text("Author: %s", selected_book->author.c_str());
        ImGui::TextWrapped("Summary: %s", selected_book->summary.c_str());
        ImGui::Text("ISBN: %s", selected_book->isbn.c_str());

        if (ImGui::Button("Close"))
        {
            details = false;
            selected_book = nullptr; // Reset after closing
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }

    ImGui::End();
}

void DrawThread::operator()(CommonObjects& common)
{
    // Initialize the filtered_books to show all books at the start
    filtered_books = common.books;
    no_books_found = false;
    GuiMain(DrawAppWindow, &common);
    common.exit_flag = true;
}