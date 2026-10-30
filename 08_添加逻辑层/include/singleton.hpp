#pragma once
#include <memory>
#include <mutex>
#include <iostream>
#include <ostream>

template<typename T>
class singleton{
protected:
    singleton()=default;
    singleton(const singleton<T>&)=delete;  //禁止拷贝
    singleton& operator=(const singleton<T>&)=delete; //禁止复制
    inline static std::shared_ptr<T> _instance = nullptr;  //cpp17
    // static std::shared_ptr<T> _instance;

public:
    static std::shared_ptr<T> get_instance(){
        static std::once_flag _flag;
        std::call_once(_flag, [&]{
            _instance=std::make_shared<T>(new T);
        });
        return _instance;
    }
    void print_adddress(){
        std::cout<<_instance.get()<<std::endl;
    }
    ~singleton(){
        std::cout<<"singleton has destroied"<<std::endl;
    }
};

// template<typename T>
// std::shared_ptr<T> singleton<T>::_instance=nullptr;  //cpp17使用内联