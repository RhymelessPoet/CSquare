#pragma once

#define CS_DEFAULT_COPY(class_name)                                                                                    \
    class_name(const class_name&) = default;                                                                           \
    class_name& operator=(const class_name&) = default;

#define CS_DEFAULT_MOVE(class_name)                                                                                    \
    class_name(class_name&&) noexcept = default;                                                                       \
    class_name& operator=(class_name&&) noexcept = default;

#define CS_DELETE_COPY(class_name)                                                                                     \
    class_name(const class_name&) = delete;                                                                            \
    class_name& operator=(const class_name&) = delete;

#define CS_DELETE_MOVE(class_name)                                                                                     \
    class_name(class_name&&) noexcept = delete;                                                                        \
    class_name& operator=(class_name&&) noexcept = delete;

#define CS_DELETE_NEW_OPERATORS                                                                                        \
    void* operator new(size_t) = delete;                                                                               \
    void* operator new[](size_t) = delete;

#define CS_DEFAULT_COPY_MOVE(class_name)                                                                               \
    CS_DEFAULT_COPY(class_name)                                                                                        \
    CS_DEFAULT_MOVE(class_name)

#define CS_DELETE_COPY_MOVE(class_name)                                                                                \
    CS_DELETE_COPY(class_name)                                                                                         \
    CS_DELETE_MOVE(class_name)
