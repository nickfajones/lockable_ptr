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

#include <shared_ptr>
#include <shared_mutex>


namespace lp
{

template<typename T>
class lockable_ptr {
  private:
    friend class reader;
    friend class writer;

  public:
    lockable_ptr() {
    }

    lockable_ptr(T *t) :
      inner_(new inner(t)) {
    }

    lockable_ptr(const lockable_ptr<T>& rvalue) :
      inner_(rvalue.inner_) {
    }

    lockable_ptr(const lockable_ptr<T>&& rvalue) :
      inner_(rvalue.inner_) {
      rvalue.inner_.reset();
    }

  public:
    lockable_ptr<T>& operator=(const lockable_ptr<T>& rvalue) {
      inner_ = rvalue.inner_;

      return *this;
    }

  public:
    class reader {
      private:
        template <typename T>
        friend class lockable_ptr<T>;

      private:
        reader(lockable_ptr<T>& parent) {
        }
    };

    class writer {
      private:
        template <typename T>
        friend class lockable_ptr<T>;

      private:
        writer(lockable_ptr<T>& parent) {
        }
    };

  public:
    reader get_reader() {
      return reader(*this);
    }

    writer get_writer() {
      return writer(*this);
    }

  private:
    struct inner {
      inner(T *t) :
        t_(t) {
      }
      ~inner() {
        delete t_;
        t_ = 0;
      }

      std::shared_mutex mutex_;

      T *t_;
    };

  private:
    std::shared_ptr<struct inner> inner_;
};

}; // namespace lp

#endif // LP_LOCKABLE_PTR_HPP
