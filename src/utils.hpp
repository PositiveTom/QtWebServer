#pragma once

#include <cstring>

template <typename T>
void saveDataToCharArray(const T& data, char* buffer) {
    std::memcpy(buffer, &data, sizeof(data));
}

template <typename T>
void recoveryDataFromCharArray(const char* buffer, T& data) {
    std::memcpy(&data, buffer, sizeof(T));
}