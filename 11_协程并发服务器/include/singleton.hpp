#pragma once
#include <iostream>
#include <iterator>
#include <memory>
#include <mutex>
#include <ostream>

// C++11 起，函数局部静态变量只初始化一次，且初始化过程线程安全，无需 call_once。

//旧版写法
// template<typename T>
// class singleton{
// protected:
//     singleton()=default;
//     singleton(const singleton&)=delete;
//     singleton& operator=(const singleton&)=delete;
// public:
//     inline static std::shared_ptr<T> get_instance(){
//         static std::once_flag flag;
//         std::call_once(flag, [](){
//             _instance=std::make_shared<T>();
//         });
//         return _instance;
//     }
//     ~singleton(){
//         std::cout<<"destruct singleton"<<std::endl;
//     }
//     static void get_instance_addr(){
//         std::cout<<_instance.get()<<std::endl;
//     }
// private:
//     inline static std::shared_ptr<T> _instance=nullptr;
// };

template<typename T>
class singleton{
protected:
    singleton()=default;
    singleton(const singleton<T>&)=delete;
    singleton& operator=(const singleton<T>&)=delete;
public:
    ~singleton(){
        std::cout<<"destruct singleton"<<std::endl;
    }

    static std::shared_ptr<T> get_instance(){   
        // static auto _instance=std::make_shared<T>(new T);
        return _instance;
    }
private:
    // inline static std::shared_ptr<T> _instance=std::make_shared<T>();
    inline static std::shared_ptr<T> _instance=std::shared_ptr<T>(new T);
};