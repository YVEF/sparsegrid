#include <filesystem>
#include <iostream>

namespace common {

std::string getNNGzipFile() {
    auto execDir = std::filesystem::current_path();
    for (const auto& entry : std::filesystem::directory_iterator(execDir)) {
        if (entry.is_regular_file() && entry.path().string().ends_with(".nn"))
            return entry.path().string();
    }
    return std::string{""};
}

} // namespace common