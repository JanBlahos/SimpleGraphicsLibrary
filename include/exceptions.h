//---------------------------------------------------------------------------
// exceptions.h
// header file containing all custom exceptions
// (such as mismatching dimensions for matrix operations)
//---------------------------------------------------------------------------

#ifndef _EXCEPTIONS_H_
#define _EXCEPTIONS_H_

#include <exception>
#include <string>


/// <summary>
/// Exception used when either mismatching dimensions
/// appear during matrix/vector operations
/// or when 0 or negative dimensions matrices are attempted to be constructed
/// </summary>
class BadDimensionException : public std::exception {
private:
    std::string message;

public:
    BadDimensionException(std::string msg)
        : message(msg)
    {
    }

    // Override the what() method to return the message
    const char* what() const throw()
    {
        return message.c_str();
    }
};

/// <summary>
/// Exception used when attempting to access
/// a vector or a matrix at an invalid/out of range index.
/// </summary>
class BadIndexException : public std::exception {
private:
    std::string message;

public:
    BadIndexException(std::string msg)
        : message(msg)
    {
    }

    // Override the what() method to return the message
    const char* what() const throw()
    {
        return message.c_str();
    }
};

/// <summary>
/// Exception used when matrix
/// allocation failed due to insufficient memory
/// </summary>
class OutOfMemoryException : public std::exception {
private:
    std::string message;

public:
    OutOfMemoryException(std::string msg)
        : message(msg)
    {
    }

    // Override the what() method to return the message
    const char* what() const throw()
    {
        return message.c_str();
    }
};

/// <summary>
/// Exception used when matrix stack has
/// no matrices in it yet but top of the stack was requested
/// </summary>
class MatrixStackUnderflowException : public std::exception {
private:
    std::string message;

public:
    MatrixStackUnderflowException(std::string msg)
        : message(msg)
    {
    }

    // Override the what() method to return the message
    const char* what() const throw()
    {
        return message.c_str();
    }
};

/// <summary>
/// Exception used when the process to allocate another 
/// matrix on the stack has failed.
/// </summary>
class MatrixStackOverflowException : public std::exception {
private:
    std::string message;

public:
    MatrixStackOverflowException(std::string msg)
        : message(msg)
    {
    }

    // Override the what() method to return the message
    const char* what() const throw()
    {
        return message.c_str();
    }
};



#endif