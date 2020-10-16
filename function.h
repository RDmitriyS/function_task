#pragma once
#include <utility>

template <typename F>
struct function;

class bad_function_call : std::exception {

};

template <typename F>
void delete_func(void* obj) {
    delete static_cast<F*>(obj);
}

template <typename F, typename R, typename... Args>
R invoker_func(void* obj, Args... args) {
    return (*static_cast<F*>(obj))(std::forward<Args>(args)...);
}

template <typename T>
void *copy_obj(void* obj) {
    return new T(*static_cast<T*>(obj));
}

template <typename R, typename... Args>
struct function<R (Args...)>
{
    function() noexcept :
        copier(nullptr),
        obj(nullptr),
        deleter(nullptr),
        invoker(nullptr),
        id(typeid(void).name()) {
    }

    function(function const& other) :
        copier(other.copier),
        obj(other.obj ? copier(other.obj) : nullptr),
        deleter(other.deleter),
        invoker(other.invoker),
        id(other.id) {
    }

    function(function&& other) noexcept : function() {
        swap(std::move(other));
    }
        
    template <typename T>
    function(T val) :
        copier(&copy_obj<T>),
        obj(new T(std::move(val))),
        deleter(&delete_func<T>),
        invoker(&invoker_func<T, R, Args...>),
        id(typeid(T).name()) {
    }

    function& operator=(function const& rhs) {
        if (this != &rhs) {
            copier = rhs.copier;
            if (rhs.obj) {
                obj = copier(rhs.obj);
            } else {
                obj = nullptr;
            }
            deleter = rhs.deleter;
            invoker = rhs.invoker;
            id = rhs.id;
        }
        return *this;
    }

    function& operator=(function&& rhs) noexcept {
        swap(std::move(rhs));
        return *this;
    }

    ~function() {
        if (deleter) {
            deleter(obj);
        }
    }

    explicit operator bool() const noexcept {
        return obj != nullptr;
    }

    R operator()(Args... args) const {
        if (invoker == nullptr) {
            throw bad_function_call();
        }
        return invoker(obj, std::forward<Args>(args)...);
    }

    template <typename T>
    T* target() noexcept {
        if (typeid(T).name() == id) {
            return static_cast<T*>(obj);
        } else {
            return nullptr;
        }
    }

    template <typename T>
    T const* target() const noexcept {
        if (typeid(T).name() == id) {
            return static_cast<T*>(obj);
        } else {
            return nullptr;
        }
    }

private:
    void* (*copier)(void*);
    void* obj;
    void (*deleter)(void*);
    R (*invoker)(void*, Args...);
    std::string id;

    void swap(function&& rhs) {
        std::swap(copier, rhs.copier);
        std::swap(obj, rhs.obj);
        std::swap(deleter, rhs.deleter);
        std::swap(invoker, rhs.invoker);
        std::swap(id, rhs.id);
    }
};

