// Tencent is pleased to support the open source community by making Mars available.
// Copyright (C) 2016 THL A29 Limited, a Tencent company. All rights reserved.

// Licensed under the MIT License (the "License"); you may not use this file except in 
// compliance with the License. You may obtain a copy of the License at
// http://opensource.org/licenses/MIT

// Unless required by applicable law or agreed to in writing, software distributed under the License is
// distributed on an "AS IS" basis, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND,
// either express or implied. See the License for the specific language governing permissions and
// limitations under the License.



#ifndef RUNNABLE_H_
#define RUNNABLE_H_

struct Runnable {
    virtual ~Runnable() {}
    virtual void run() = 0;
};

namespace detail {

template <class T>
class RunnableFunctor : public Runnable {
  public:
    RunnableFunctor(const T& f) : func_(f) {}
    virtual void run() { func_(); }
  private:
    T func_;
};

template <class T>
class RunnableFunctor<T*> : public Runnable {
  public:
    RunnableFunctor(T* f) : func_(f) {}
    virtual void run() { (*func_)(); }

  private:
    RunnableFunctor(const RunnableFunctor&);
    RunnableFunctor& operator=(const RunnableFunctor&);

  private:
    T* func_;
};

template <>
class RunnableFunctor<Runnable> : public Runnable {
    RunnableFunctor();
};

template <>
class RunnableFunctor<Runnable*> : public Runnable {
  public:
    RunnableFunctor(Runnable* f) : func_(f) {}
    virtual void run() { static_cast<Runnable*>(func_)->run();}

  private:
    RunnableFunctor(const RunnableFunctor&);
    RunnableFunctor& operator=(const RunnableFunctor&);

  private:
    Runnable* func_;
};

// base template for no argument functor
template <class T>
struct TransformImplement {
    static Runnable* transform(const T& t) {
        return new RunnableFunctor<T>(t);
    }
};

template <class T>
inline Runnable* transform(const T& t) {
    return TransformImplement<T>::transform(t);
}

}  // namespace detail


#endif /* RUNNABLE_H_ */
