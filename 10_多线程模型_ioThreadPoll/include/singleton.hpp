#pragma once
#include <memory>
#include <mutex>
#include <iostream>
#include <ostream>

template<typename T>
class singleton{
protected:
    singleton()=default; //默认构造
    singleton(const singleton<T>&)=delete;  //禁止拷贝   singleton t1(t2);
    singleton& operator=(const singleton<T>&)=delete; //禁止复制 singleton t2=t1;
    inline static std::shared_ptr<T> _instance = nullptr;  //cpp17
    // static std::shared_ptr<T> _instance;

public:
    static std::shared_ptr<T> get_instance(){
        static std::once_flag _flag;
        std::call_once(_flag, []{
            _instance=std::shared_ptr<T>(new T);   //保证只创建一次
        });
        return _instance;
    }
    void print_adddress(){
        std::cout<<_instance.get()<<std::endl;  //打印单例对象地址
    }
    ~singleton(){
        std::cout<<"singleton has destroied"<<std::endl;
    }
};

// template<typename T>
// std::shared_ptr<T> singleton<T>::_instance=nullptr;  //cpp17使用内联
