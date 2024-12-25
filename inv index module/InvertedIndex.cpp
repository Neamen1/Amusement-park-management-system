#include <iostream>
#include <unordered_map>
#include <vector>
#include <string>
#include <shared_mutex>
#include <sstream>
#include <mutex>
#include <algorithm>
#include <fstream>
#include <nlohmann/json.hpp>
using json = nlohmann::json;

class InvertedIndex {
private:
    std::unordered_map<std::string, std::vector<std::pair<int, int>>> index;
    std::shared_mutex mutex;

    // Приватна функція для токенізації тексту
    std::vector<std::string> tokenize(const std::string& text) {
        std::vector<std::string> tokens;
        std::istringstream stream(text);
        std::string word;
        while (stream >> word) {
            // Видалення пунктуації та переведення в нижній регістр
            word.erase(std::remove_if(word.begin(), word.end(), ::ispunct), word.end());
            std::transform(word.begin(), word.end(), word.begin(), ::tolower);
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
        std::unique_lock lock(mutex);
        json j;
        for (const auto& [term, postings] : index) {
            j[term] = json::array();
            for (const auto& [docId, freq] : postings) {
                j[term].push_back({ {"docId", docId}, {"freq", freq} });
            }
        }
        std::ofstream file(filePath);
        if (!file.is_open()) {
            throw std::runtime_error("Failed to open file for writing: " + filePath);
        }
        file << j.dump(4);
    }

    void loadIndex(const std::string& filePath) {
        std::unique_lock lock(mutex);
        std::ifstream file(filePath);
        if (!file.is_open()) {
            throw std::runtime_error("Failed to open file: " + filePath);
        }
        json j;
        file >> j;
        index.clear();
        for (auto& [term, postings] : j.items()) {
            for (auto& posting : postings) {
                index[term].emplace_back(posting["docId"], posting["freq"]);
            }
        }
    }
};
