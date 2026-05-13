#pragma once

template <typename T>
class CustomList {
private:
    T* data;
    int capacity;
    int count;

    void Expand() {
        capacity = capacity == 0 ? 4 : capacity * 2;
        T* newData = new T[capacity];
        for (int i = 0; i < count; i++) newData[i] = data[i];
        delete[] data;
        data = newData;
    }

public:
    CustomList() : data(nullptr), capacity(0), count(0) {}
    ~CustomList() { delete[] data; }

    void Add(T item) {
        if (count == capacity) Expand();
        data[count++] = item;
    }

    int Count() { return count; }

    T& operator[](int index) { return data[index]; }
};