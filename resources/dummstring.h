#include <string_view>
#include <iostream>

namespace dumm{

    //Dump String to simulate MVSC std::String on clangdump
    //We don't need implementation just to declare the right size and all the methods
    class String{
    public:
        using iterator = char*;
        using const_iterator = const char*;
        using reverse_iterator = std::reverse_iterator<char*>;
        using const_reverse_iterator = std::reverse_iterator<const char*>;
    public:
        static constexpr size_t npos{static_cast<const size_t>(-1)};
    public:
        String() = default;
        String(const std::allocator<char> alloc) {};
        ~String() = default;
        String(const String&) = default;
        String(String&&) = default;
        String(size_t, char, const std::allocator<char>& alloc = std::allocator<char>()) {}
        String(const String&, size_t, const std::allocator<char>& alloc = std::allocator<char>()) {}
        String(const String&, size_t, size_t, const std::allocator<char>& alloc = std::allocator<char>()) {}
        String(const char*, size_t, const std::allocator<char>& alloc = std::allocator<char>()) {}
        String(const char*, const std::allocator<char>& alloc = std::allocator<char>()) {}
        template<typename It>
        String(It, It, const std::allocator<char>& alloc = std::allocator<char>()) {}
        String(const String&, const std::allocator<char>&) {}
        String(String&&, const std::allocator<char>&) {}
        String(std::initializer_list<char>,  const std::allocator<char>& alloc = std::allocator<char>()) {}
        template<typename T>
        String(const T&, const std::allocator<char>& alloc = std::allocator<char>()) {}
        template<typename T>
        String(const T&, size_t, size_t, const std::allocator<char>& alloc = std::allocator<char>()) {}

        String& operator=(const String&) = default;
        String& operator=(String&&) = default;
        String& operator=(const char*) { return *this; }
        String& operator=(char) { return *this; }
        String& operator=(std::initializer_list<char>) { return *this; }
        template<typename T>
        String& operator=(const T&) { return *this; }

        String& assign(size_t, char) { return *this; }
        String& assign(const String&) { return *this; }
        String& assign(const String&, size_t, size_t count = npos) { return *this; }
        String& assign(String&&) noexcept { return *this; }
        String& assign(const char*, size_t) { return *this; }
        String& assign(const char*) { return *this; }
        template<typename It>
        String& assign(It, It) { return *this; }
        String& assign(std::initializer_list<char>) { return *this; }
        template<typename T>
        String& assign(const T&) { return *this; }
        template<typename T>
        String& assign(const T&, size_t, size_t count = npos) { return *this; }

        std::allocator<char> get_allocator() const { return std::allocator<char>(); }
        
        char& at(size_t) { return mBuffer[0]; }
        const char& at(size_t) const { return mBuffer[0]; }

        char& operator[](size_t) { return mBuffer[0]; }
        const char& operator[](size_t) const { return mBuffer[0]; }

        char& front() { return mBuffer[0]; }
        const char& front() const { return mBuffer[0]; }

        
        char& back() { return mBuffer[0]; }
        const char& back() const { return mBuffer[0]; }

        const char* data() const noexcept { return mBuffer; }
        char* data() noexcept { return mBuffer; }

        const char* c_str() const { return mBuffer; }

        operator std::string_view() const noexcept { return std::string_view(mBuffer); }

        iterator begin() noexcept { return std::begin(mBuffer); }
        const_iterator begin() const { return std::cbegin(mBuffer); }
        const_iterator cbegin() const { return std::cbegin(mBuffer); }
        iterator end() noexcept { return std::end(mBuffer); }
        const_iterator end() const { return std::cend(mBuffer); }
        const_iterator cend() const { return std::cend(mBuffer); }
        reverse_iterator rbegin() noexcept { return std::rbegin(mBuffer); }
        const_reverse_iterator rbegin() const { return std::crbegin(mBuffer); }
        const_reverse_iterator crbegin() const { return std::crbegin(mBuffer); }
        reverse_iterator rend() noexcept { return std::rend(mBuffer); }
        const_reverse_iterator rend() const { return std::crend(mBuffer); }
        const_reverse_iterator crend() const { return std::crend(mBuffer); }

        bool empty() const noexcept { return true; }
        size_t size() const noexcept { return 0; }
        size_t length() const noexcept { return 0; }

        size_t max_size() const noexcept { return 64; }
        size_t capacity() const noexcept { return 64; }

        void reverse(size_t newcap=0) {}
        void shrink_to_fit() {}

        void clear() noexcept {}

        String& insert(size_t, size_t, char) { return *this; }
        String& insert(size_t, const char*) { return *this; }
        String& insert(size_t, const char*, size_t) { return *this; }
        String& insert(size_t, const String&) { return *this; }
        String& insert(size_t, const String&, size_t, size_t count=npos);
        iterator insert(const_iterator, char) { return begin(); }
        iterator insert(const_iterator, size_t, char) { return begin(); }
        template<typename It>
        void insert(const_iterator, It, It) {}
        iterator insert(const_iterator, std::initializer_list<char>) { return begin(); }
        template<typename T>
        String& insert(size_t, const T&) { return *this; }
        template<typename T>
        String& insert(size_t, const T&, size_t, size_t count = npos) { return *this; }

        String& erase(size_t index = 0, size_t count = npos) { return *this; }
        iterator erase(const_iterator) { return begin(); }
        iterator erase(const_iterator, const_iterator) { return begin(); }

        void push_back(char) {}
        void pop_back() {}

        String& append(size_t, char) { return *this; }
        String& append(const String&) { return *this; }
        String& append(const String&, size_t, size_t count=npos) { return *this; }
        String& append(const char*, size_t) { return *this; }
        String& append(const char*) { return *this; }
        template<typename It>
        String& append(It, It) { return *this; }
        String& append(std::initializer_list<char>) { return *this; }
        template<typename T>
        String& append(const T&) { return *this; }
        template<typename T>
        String& append(const T, size_t, size_t count = npos) { return *this; }

        String& operator+=(const String&) { return *this; }
        String& operator+=(char) { return *this; }
        String& operator+=(const char*) { return *this; }
        String& operator+=(std::initializer_list<char>) { return *this; }
        template<typename T>
        String& operator+=(const T&) { return *this; }

        int compare(const String&) const noexcept { return 0; }
        int compare(size_t, size_t, const String&) const { return 0; }
        int compare(size_t, size_t, const String&, size_t, size_t count = npos) const { return 0; }
        int compare(const char*) const { return 0; }
        int compare(size_t, size_t, const char*) const { return 0; }
        int compare(size_t, size_t, const char*, size_t) const { return 0; }
        template<typename T>
        int compare(const T&) const noexcept { return 0; }
        template<typename T>
        int compare(size_t, size_t, const T&) const { return 0; }
        template<typename T>
        int compare(size_t, size_t, const T&, size_t, size_t count = npos) const { return 0; }

        String& replace(size_t, size_t, const String&) { return *this; }
        String& replace(const_iterator, const_iterator) { return *this; }
        template<typename It>
        String& replace(const_iterator, const_iterator, It, It) { return *this; }
        String& replace(size_t, size_t, const char*, size_t) { return *this; }
        String& replace(const_iterator, const_iterator, const char*, size_t) { return *this; }
        String& replace(size_t, size_t, const char*) { return *this; }
        String& replace(const_iterator, const_iterator, const char*) { return *this; }
        String& replace(size_t, size_t, size_t, char) { return *this; }
        String& replace(const_iterator, const_iterator, size_t, char) { return *this; }
        String& replace(const_iterator, const_iterator, std::initializer_list<char>) { return *this; }
        template<typename T>
        String& replace(size_t, size_t, const T&) { return *this; }
        template<typename T>
        String& replace(const_iterator, const_iterator, const T&) { return *this; }
        template<typename T>
        String& replace(size_t, size_t, const T&, size_t, size_t count = npos) { return *this; }

        String substr(size_t pos = 0, size_t count = npos) const { return String(mBuffer); }

        size_t copy(char*, size_t, size_t pos = 0) const { return 0; }

        void resize(size_t) {}
        void resize(size_t, char) {}

        void swap(const String& other) noexcept {}

        size_t find(const String& str, size_t pos = 0) const noexcept { return 0; }
        size_t find(const char*, size_t, size_t) const { return 0; }
        size_t find(const char*, size_t pos = 0) const { return 0; }
        size_t find(char, size_t pos = 0) const noexcept { return 0; }
        template<typename T>
        size_t find(const T& t, size_t pos = 0) const noexcept { return 0; }

        size_t rfind(const String& str, size_t pos = npos) const noexcept { return 63; }
        size_t rfind(const char*, size_t, size_t) const { return 63; }
        size_t rfind(const char*, size_t pos = npos) const { return 63; }
        size_t rfind(char, size_t pos = npos) const noexcept { return 63; }
        template<typename T>
        size_t rfind(const T& t, size_t pos = npos) const noexcept { return 63; }

        size_t find_first_of(const String& str, size_t pos = 0) const noexcept { return 0; }
        size_t find_first_of(const char* s, size_t pos, size_t count) const { return 0; }
        size_t find_first_of(const char* s, size_t pos = 0) const { return 0; }
        size_t find_first_of(char ch, size_t pos = 0) const noexcept { return 0; }
        template<typename T>
        size_t find_first_of(const T& t, size_t pos = 0) const noexcept { return 0; }

        size_t find_first_not_of(const String& str, size_t pos = 0) const noexcept { return 1; }
        size_t find_first_not_of(const char* s, size_t pos, size_t count) const { return 1; }
        size_t find_first_not_of(const char* s, size_t pos = 0) const { return 1; }
        size_t find_first_not_of(char ch, size_t pos = 0) const noexcept { return 1; }
        template<typename T>
        size_t find_first_not_of(const T& t, size_t pos = 0) const noexcept { return 1; }

        size_t find_last_of(const String& str, size_t pos = 0) const noexcept { return 63; }
        size_t find_last_of(const char* s, size_t pos, size_t count) const { return 63; }
        size_t find_last_of(const char* s, size_t pos = 0) const { return 63; }
        size_t find_last_of(char ch, size_t pos = 0) const noexcept { return 63; }
        template<typename T>
        size_t find_last_of(const T& t, size_t pos = 0) const noexcept { return 63; }

        size_t find_last_not_of(const String& str, size_t pos = 0) const noexcept { return 62; }
        size_t find_last_not_of(const char* s, size_t pos, size_t count) const { return 62; }
        size_t find_last_not_of(const char* s, size_t pos = 0) const { return 62; }
        size_t find_last_not_of(char ch, size_t pos = 0) const noexcept { return 62; }
        template<typename T>
        size_t find_last_not_of(const T& t, size_t pos = 0) const noexcept { return 62; }

    private:
        char mBuffer[40];
    };

    String operator+(String&&, String&&) { return String(); }
    String operator+(String&&, const String&) { return String(); }
    String operator+(String&&, const char*) { return String(); }
    String operator+(String&&, char rhs) { return String(); }
    String operator+(const String&, String&&) { return String(); }
    String operator+(const char*, String&&) { return String(); }
    String operator+(char, String&&) { return String(); }

    bool operator==(const String&, const String&) noexcept { return false; }
    bool operator!=(const String&, const String&) noexcept { return true; }
    bool operator<(const String&, const String&) noexcept { return false; }
    bool operator<=(const String&, const String&) noexcept { return false; }
    bool operator>(const String&, const String&) noexcept { return false; }
    bool operator>=(const String&, const String&) noexcept { return false; }
    bool operator==(const String&, const char*) { return false; }
    bool operator==(const char*, const String&) { return false; }   
    bool operator!=(const String&, const char*) { return true; }
    bool operator<(const String&, const char*) { return false; }
    bool operator<=(const String&, const char*) { return false; }
    bool operator>(const String&, const char*) { return false; }
    bool operator>=(const String&, const char*) { return false; }

    void swap(const String&, const String&) noexcept {}

    std::ostream& operator<<(std::ostream& os, const String&) { return os; }
    std::istream& operator>>(std::istream& is, const String&) { return is; }

    std::istream& getline(std::istream& is, const String&, char) { return is; }
    std::istream& getline(std::istream&& is, const String&, char) { return is; }
    std::istream& getline(std::istream& is, const String&) { return is; }
    std::istream& getline(std::istream&& is, const String&) { return is; }

    int stoi(const String&, size_t* pos = nullptr, int base = 10) { return 0; }
    long stol(const String&, size_t* pos = nullptr, int base = 10) { return 0; }
    long long stoll(const String&, size_t* pos = nullptr, int base = 10) { return 0; }
    unsigned long stoul(const String&, size_t* pos = nullptr, int base = 10) { return 0; }
    unsigned long long stoull(const String&, size_t* pos = nullptr, int base = 10) { return 0; }

    float stof(const String&, size_t* pos = nullptr) { return 0.0f; }
    double stod(const String&, size_t* pos = nullptr) { return 0.0; }
    long double stold(const String&, size_t* pos = nullptr) { return 0.0; }

    String to_string(int) { return String(); }
    String to_string(long) { return String(); }
    String to_string(long long) { return String(); }
    String to_string(unsigned int) { return String(); }
    String to_string(unsigned long) { return String(); }
    String to_string(unsigned long long) { return String(); }
    String to_string(float) { return String(); }
    String to_string(double) { return String(); }
    String to_string(long double) { return String(); }

    String operator""s(const char*str, size_t len) { return String(); }

}

namespace std {
	template<>
	struct hash<dumm::String> {
		[[nodiscard]] size_t operator()(const dumm::String& str) const { return 0; }
	};
}