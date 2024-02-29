
//          Copyright Oliver Kowalke 2016.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <cstdio>
#include <cstdlib>
#include <exception>
#include <functional>
#include <iostream>
#include <memory>
#include <sstream>

#include <boost/context/continuation.hpp>

namespace ctx = mars_boost::context;

/*
 * grammar:
 *   P ---> E '\0'
 *   E ---> T {('+'|'-') T}
 *   T ---> S {('*'|'/') S}
 *   S ---> digit | '(' E ')'
 */
class Parser{
   char next;
   std::istream& is;
   std::function<void(char)> cb;

   char pull(){
        return std::char_traits<char>::to_char_type(is.get());
   }

   void scan(){
       do{
           next=pull();
       }
       while(isspace(next));
   }

public:
   Parser(std::istream& is_,std::function<void(char)> cb_) :
      next(), is(is_), cb(cb_)
    {}

   void run() {
      scan();
      E();
   }

private:
   void E(){
      T();
      while (next=='+'||next=='-'){
         cb(next);
         scan();
         T();
      }
   }

   void T(){
      S();
      while (next=='*'||next=='/'){
         cb(next);
         scan();
         S();
      }
   }

   void S(){
      if (isdigit(next)){
         cb(next);
         scan();
      }
      else if(next=='('){
         cb(next);
         scan();
         E();
         if (next==')'){
             cb(next);
             scan();
         }else{
             throw std::runtime_error("parsing failed");
         }
      }
      else{
         throw std::runtime_error("parsing failed");
      }
   }
};

int main() {
    try {
        std::istringstream is("1+1");
        // execute parser in new execution context
        ctx::continuation source;
        // user-code pulls parsed data from parser
        // invert control flow
        char c;
        bool done = false;
        source=ctx::callcc(
                [&is,&c,&done](ctx::continuation && sink){
                    // create parser with callback function
                    Parser p( is,
                              [&sink,&c](char c_){
                                    // resume main execution context
                                    c = c_;
                                    sink=sink.resume();
                            });
                    // start recursive parsing
                    p.run();
                    // signal termination
                    done = true;
                    // resume main execution context
                    return std::move(sink);
                });
        while(!done){
            printf("Parsed: %c\n",c);
            source=source.resume();
        }
        std::cout << "main: done" << std::endl;
        return EXIT_SUCCESS;
    } catch (std::exception const& e) {
        std::cerr << "exception: " << e.what() << std::endl;
    }
    return EXIT_FAILURE;
}
