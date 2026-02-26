#pragma once

/// @brief Marks class as static
#define SE_STATIC_CLASS(Class) \
    Class() = delete; \
    ~Class() = delete;

/// @brief Marks class as a singleton
#define SE_SINGLETON_CLASS(Class) \
    Class(Class& Copy) = delete; \
    Class operator=(Class& copy) = delete;

/// @brief Forces noexcept on move constructor
#define SE_MOVE_NOEXCEPT(Class) \
    Class(Class&&) noexcept; \
    Class& operator=(Class&&) noexcept;

/// @brief Removes copy semantics from class
#define SE_NO_COPY(Class) \
    Class(const Class&) = delete; \
    Class& operator=(const Class&) = delete;

/// @brief Removes move semantics from class
#define SE_NO_MOVE(Class) \
    Class(Class&&) = delete; \
    Class& operator=(Class&&) = delete;

/// @brief Removes both copy and move semantics from class
#define SE_NO_COPY_MOVE(Class) SE_NO_COPY(Class) SE_NO_MOVE(Class)
