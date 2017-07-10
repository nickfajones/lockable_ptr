/*******************************************************************************
 *  Copyright 2017 Nick Jones <nick.fa.jones@gmail.com>
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 ******************************************************************************/

#ifndef LP_LOCKABLE_PTR_HPP
#define LP_LOCKABLE_PTR_HPP

#include <memory>
#include <shared_mutex>

#include <lp/lockable_ptr_base.hpp>


namespace lp
{

template<typename T>
class lockable_ptr {
  private:
    friend class enable_lockable_from_this<T>;

  private:
    friend class reader;
    friend class writer;

  private:
    struct wrapper {
      wrapper(T *t) :
        t_(t) {
      }
      ~wrapper() {
        delete t_;
        t_ = 0;
      }

      std::shared_timed_mutex mutex_;

      T *t_;
    };

  private:
    void enable_lockable_setter(
        enable_lockable_from_this<T>* t,
        std::shared_ptr<struct wrapper>& wrapper) {
      t->weak_wrapper_ = wrapper;
    }
    void enable_lockable_setter(
        void* t,
        std::shared_ptr<struct wrapper>& wrapper) {
    }

  public:
    lockable_ptr() {
    }

    lockable_ptr(T* t) :
      wrapper_(new wrapper(t)) {
      enable_lockable_setter(t, wrapper_);
    }

    lockable_ptr(const lockable_ptr<T>& rvalue) :
      wrapper_(rvalue.wrapper_) {
    }

    lockable_ptr(const lockable_ptr<T>&& rvalue) :
      wrapper_(rvalue.wrapper_) {
      rvalue.wrapper_.reset();
    }

  public:
    lockable_ptr<T>& operator=(const lockable_ptr<T>& rvalue) {
      wrapper_ = rvalue.wrapper_;

      return *this;
    }

  public:
    void reset() {
      wrapper_.reset();
    }

    void reset(T* t) {
      wrapper_.reset(new wrapper(t));
      enable_lockable_setter(t, wrapper_);
    }

    void reset(const lockable_ptr<T>& rvalue) {
      wrapper_ = rvalue.wrapper_;
    }

  public:
    class reader {
      private:
        friend class lockable_ptr<T>;
        friend class enable_lockable_from_this<T>;

      private:
        explicit reader(std::shared_ptr<struct wrapper>& parent_wrapper) :
          wrapper_(std::atomic_load(&parent_wrapper)),
          lock_(wrapper_->mutex_) {
        }

      public:
        const T* get() const {
          return wrapper_->t_;
        }

        const T* operator->() const {
          return wrapper_->t_;
        }

      private:
        std::shared_ptr<struct wrapper> wrapper_;

      private:
        std::shared_lock<std::shared_timed_mutex> lock_;
    };

    class writer {
      private:
        friend class lockable_ptr<T>;
        friend class enable_lockable_from_this<T>;

      private:
        explicit writer(std::shared_ptr<struct wrapper>& parent_wrapper) :
          wrapper_(std::atomic_load(&parent_wrapper)),
          lock_(wrapper_->mutex_) {
        }

      public:
        T* get() const {
          return wrapper_->t_;
        }

        T* operator->() const {
          return wrapper_->t_;
        }

      private:
        std::shared_ptr<struct wrapper> wrapper_;

      private:
        std::unique_lock<std::shared_timed_mutex> lock_;
    };

  public:
    reader get_reader() {
      return reader(wrapper_);
    }

    writer get_writer() {
      return writer(wrapper_);
    }

  private:
    std::shared_ptr<struct wrapper> wrapper_;
};

template<typename T>
class enable_lockable_from_this {
  public:
    friend class lockable_ptr<T>;

  public:
    typename lockable_ptr<T>::reader reader_from_this() {
      std::shared_ptr<typename lockable_ptr<T>::wrapper> wrapper =
        weak_wrapper_.lock();
      return typename lockable_ptr<T>::reader(wrapper);
    }

    typename lockable_ptr<T>::writer writer_from_this() {
      std::shared_ptr<typename lockable_ptr<T>::wrapper> wrapper =
        weak_wrapper_.lock();
      return typename lockable_ptr<T>::writer(wrapper);
    }

  protected:
    std::weak_ptr<typename lockable_ptr<T>::wrapper> weak_wrapper_;
};

}; // namespace lp

#endif // LP_LOCKABLE_PTR_HPP
