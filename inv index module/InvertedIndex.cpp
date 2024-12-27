#include <iostream>
#include <unordered_map>
#include <vector>
#include <string>
#include <shared_mutex>
#include <sstream>
#include <mutex>
#include <filesystem>
#include <algorithm>
#include <fstream>
#include <nlohmann/json.hpp>
#include "threadpool.cpp"

using json = nlohmann::json;
namespace fs = std::filesystem;

class InvertedIndex {
private:
    std::unordered_map<std::string, std::vector<std::pair<int, int>>> index;
    std::shared_mutex mutex;
    std::mutex j_mutex;

    // Приватна функція для токенізації тексту
    std::vector<std::string> tokenize(const std::string& text) {
        std::vector<std::string> tokens;
        std::istringstream stream(text);
        std::string word;
        std::locale loc("");
        while (stream >> word) {
            // Видалення пунктуації та переведення в нижній регістр
            word.erase(std::remove_if(word.begin(), word.end(),
                [&loc](char ch) { return std::ispunct(ch, loc); }),
                word.end());
            std::transform(word.begin(), word.end(), word.begin(),
                [&loc](char ch) { return std::tolower(ch, loc); });
            tokens.push_back(word);
        }
        return tokens;
    }

public:
    // Додавання документа до індексу
    void addDocument(int docId, const std::string& content) {
        auto tokens = tokenize(content);
        std::unordered_map<std::string, int> termFrequency;

        // Обчислення частоти термінів у документі
        for (const auto& term : tokens) {
            termFrequency[term]++;
        }

        // Додавання термінів до індексу
        std::unique_lock lock(mutex);
        for (const auto& [term, freq] : termFrequency) {
            index[term].emplace_back(docId, freq);
        }
    }

    // Build the index in parallel for a number of files in range of indexes
    void buildIndexParallel(const std::vector<std::string>& directories, size_t numThreads, size_t startIdx, size_t endIdx) {
        ThreadPool threadPool(numThreads);

        for (const auto& dir : directories) {
            // find file with specified indexes
            for (const auto& entry : fs::directory_iterator(dir)) {
                const std::string filename = entry.path().filename().string();

                // Extract the sequence number before '_'
                size_t underscorePos = filename.find('_');
                if (underscorePos == std::string::npos) {
                    continue; // Skip files with unexpected format
                }

                size_t sequenceNum;
                try {
                    sequenceNum = std::stoull(filename.substr(0, underscorePos));
                }
                catch (const std::exception&) {
                    continue; // Skip files with invalid sequence number
                }
                // Check if the sequence number is within the desired range
                if (sequenceNum >= startIdx && sequenceNum < endIdx) {
                    threadPool.enqueue([this, sequenceNum, path = entry.path()]() {
                        std::ifstream file(path);
                        if (file.is_open()) {
                            std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
                            this->addDocument(sequenceNum, content);
                        }
                        });
                }
            }
        }
    }

    // Пошук документів за терміном
    std::vector<int> search(const std::string& term) {
        std::shared_lock lock(mutex);
        std::vector<int> result;
        auto it = index.find(term);
        if (it != index.end()) {
            for (const auto& [docId, _] : it->second) {
                result.push_back(docId);
            }
        }
        return result;
    }


    std::vector<int> searchPhrase(const std::string& phrase) {
        auto tokens = tokenize(phrase);
        if (tokens.empty()) {
            return {};
        }

        std::shared_lock lock(mutex);
        std::vector<int> result;

        // Отримуємо ідентифікатори документів для першого терміну
        auto it = index.find(tokens[0]);
        if (it != index.end()) {
            for (const auto& [docId, _] : it->second) {
                result.push_back(docId);
            }
        }
        else {
            return {}; // Якщо перше слово відсутнє, результат - порожній
        }
        
        // Для кожного наступного терміну обчислюємо перетин
        for (size_t i = 1; i < tokens.size(); ++i) {
            it = index.find(tokens[i]);
            if (it == index.end()) {
                return {}; // Якщо якесь слово не знайдено, результат - порожній
            }

            for (auto iterate = result.begin(); iterate != result.end();) {
                // Шукаємо ідентифікатор документа у списку поточного слова
                auto found = std::find_if(it->second.begin(), it->second.end(), [&](const std::pair<int, int>& entry) {
                    return entry.first == *iterate;
                    });

                // Якщо ідентифікатор документа не знайдено, видаляємо його з результатів
                if (found == it->second.end()) {
                    iterate = result.erase(iterate);
                }
                else {
                    ++iterate;
                }
            }
        }

        return result;
    }


    std::string printIndex(const std::string& term = "") {
        std::shared_lock lock(mutex);
        std::ostringstream output;

        if (term.empty()) {
            // Виведення всього індексу
            for (const auto& [key, value] : index) {
                output << key << ": ";

                for (const auto& [docId, freq] : value) {
                    output << "(doc " << docId << ", freq " << freq << ") ";
                }
                output << "\n";
            }
        }
        else {
            // Виведення для конкретного терміну
            auto it = index.find(term);
            if (it != index.end()) {
                output << term << ": ";
                for (const auto& [docId, freq] : it->second) {
                    output << "(doc " << docId << ", freq " << freq << ") ";
                }
            }
            else {
                output << "Term '" << term << "' not found in index.\n";
            }
        }
        return output.str();
    }


    // Очищення індексу
    void clearIndex() {
        std::unique_lock lock(mutex);
        index.clear();
    }

    void saveIndex(const std::string& filePath) {

        std::ofstream file(filePath);
        if (!file.is_open()) {
            throw std::runtime_error("Failed to open file for writing: " + filePath);
        }

        ThreadPool threadPool(std::thread::hardware_concurrency());
        json j;
        {
            std::shared_lock lock(mutex);
            for (const auto& [term, postings] : index) {
                threadPool.enqueue([this, &j, &term, &postings]() {
                    json termPostings = json::array();
                    for (const auto& [docId, freq] : postings) {
                        termPostings.push_back({ {"docId", docId}, {"freq", freq} });
                    }
                    std::unique_lock jsonLock(j_mutex);
                    j[term] = termPostings;
                    });
            }
        }
        threadPool.~ThreadPool();
        
        file << j.dump(4);
    }

    void loadIndex(const std::string& filePath) {
        ThreadPool threadPool(std::thread::hardware_concurrency());
        json j;
        {
            std::ifstream file(filePath);
            if (!file.is_open()) {
                throw std::runtime_error("Failed to open file: " + filePath);
            }
            file >> j;
        }

        
        {
            std::unique_lock lock(mutex);
            index.clear();
            for (auto& [term, postings] : j.items()) {
                threadPool.enqueue([this, &term, &postings]() {
                    std::vector<std::pair<int, int>> termPostings;
                    for (auto& posting : postings) {
                        termPostings.emplace_back(posting["docId"], posting["freq"]);
                    }
                    std::unique_lock indexLock(mutex);
                    index[term] = termPostings;
                    });
            }
        }
        threadPool.~ThreadPool();
    }
};
