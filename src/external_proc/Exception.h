#pragma once
#include <string>

// Global exceptions
class PyException : public std::exception {
public:
    explicit PyException(std::string str) : message(str) {}
    const char * what() const noexcept override {return message.c_str();}
private:
    std::string message;
};

// Ptr.h exceptions
class RequiredValueSizeArgument : public std::exception {
public:
    explicit RequiredValueSizeArgument() {}
    const char * what() const noexcept override {return message.c_str();}
private:
    std::string message = "'value_size' for size type not specified";
};

class UnknownPtrType : public std::exception {
public:
    explicit UnknownPtrType() {}
    const char * what() const noexcept override {return message.c_str();}
private:
    std::string message = "unknown ptr type";
};
