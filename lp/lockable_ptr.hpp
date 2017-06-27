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

#include "lockable_ptr_base.hpp"


namespace lp
{

template<typename T>
class lockable_ptr {
  private:
    friend class reader;
    friend class writer;

  private:
    struct wrapper {
      typedef std::shared_ptr<struct wrapper> pointer_type;

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

  public:
    lockable_ptr() {
    }

    lockable_ptr(T *t) :
      wrapper_(new wrapper(t)) {
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
    
  public:
    class reader {
      private:
        friend class lockable_ptr<T>;

      private:
        explicit reader(typename wrapper::pointer_type& parent_wrapper) :
          wrapper_(parent_wrapper),
          lock_(wrapper_->mutex_) {
        }

      public:

      private:
        std::shared_ptr<struct wrapper> wrapper_;

      private:
        std::shared_lock<std::shared_timed_mutex> lock_;
    };

    class writer {
      private:
        friend class lockable_ptr<T>;

      private:
        explicit writer(typename wrapper::pointer_type& parent_wrapper) :
          wrapper_(parent_wrapper),
          lock_(wrapper_->mutex_) {
        }

      public:

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
    lockable_ptr<T> lockable_from_this() {
      return weak_ref_.lock();
    }

  protected:
    std::weak_ptr<T> weak_ref_;
};

}; // namespace lp

#endif // LP_LOCKABLE_PTR_HPP
