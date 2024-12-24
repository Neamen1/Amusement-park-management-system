#include <iostream>
#include <unordered_map>
#include <vector>
#include <string>
#include <shared_mutex>
#include <sstream>
#include <mutex>
#include <algorithm>

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

    // Очищення індексу
    void clearIndex() {
        std::unique_lock lock(mutex);
        index.clear();
    }
};
