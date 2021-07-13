#include <iostream>
#include <string>
#include <cstdlib>
#include <ctime>
#include <thread>

#include "unsure_ownership_ptr.hpp"
using namespace std;

unsure_ownership_ptr<string> get_str(bool arg)
{
    static int magic_number=123123;
    static string bigStr = "I am very very long.";//a string has enough lifetime, is too big to copy, and shouldn't be moved
    string tempStr = std::to_string(magic_number);//a temporary string
    if (arg)
    {
        //create an unsure_ownership_ptr by arguments: bool is_owned,T* address
        return {false, &bigStr};
        //the pointer doesnt have ownership for this string
    }
    else 
    {
        //unique_ptr always has ownership
        return make_unique<string>(std::move(tempStr));
        //the pointer has ownership for this string
    }
}
int main(void)
{
    srand(time(nullptr));
    while (true)
    {
        int choose=rand()%2;

        //str_ptr is an unsure_ownership_ptr
        auto str_ptr = get_str(choose);
        
        //sizeof(unsure_ownership_ptr<T>)==sizeof(unique_ptr<T>) 
#ifdef __x86_64__
        static_assert(sizeof(unsure_ownership_ptr<string>)==sizeof(unique_ptr<string>));
#endif 

        cout << "if str_ptr has ownership: " << (str_ptr.has_ownership()? "true" : "false") << '\n';
        cout << "string content: " << *str_ptr <<endl;

        unsure_ownership_ptr<char> char_ptr{false,(char *)(str_ptr.get()->data())};
        cout << char_ptr.get() << endl;

        char_ptr.reset(false,(char *)(str_ptr.get()->data()+1));
        cout << char_ptr.get() << endl;

        //string will be destructed automatically if pointer has ownership

        this_thread::sleep_for(100ms);
    }
    return 0;
}