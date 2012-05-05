#ifndef HPP_CPP_MULTI_PRECISION_NS_AUX
#define HPP_CPP_MULTI_PRECISION_NS_AUX

#include <string>
#include <ostream>
#include <queue>
#include <vector>
#include <algorithm>
#include <utility>
#include <type_traits>
#include <exception>
#include <stdexcept>
#include <cmath>
#include <cassert>
#include <climits>
#include <boost/utility/enable_if.hpp>

namespace cpp_multi_precision{
    namespace aux{
        static_assert(CHAR_BIT == 8, "cpp_multi_precision::aux - 'CHAR_BIT == 8'");

        template<class Container>
        struct rebind_container;

        template<template<class, class> class Container, class Elem, class Alloc>
        struct rebind_container<Container<Elem, Alloc>>{
            template<class NewType, class NewAlloc> struct rebind{ typedef Container<NewType, NewAlloc> other; };
        };

        template<class Container, class NewType>
        struct wrapped_rebind_container{
            typedef typename rebind_container<Container>::template rebind<NewType, typename Container::allocator_type::template rebind<NewType>::other>::other type;
        };

        template<class Container, class Alloc>
        class bit_array{
        public:
            typedef typename aux::rebind_container<Container>::template rebind<
                unsigned int,
                typename Alloc::template rebind<unsigned int>::other
            >::other container_type;

            bool get(std::size_t i) const{
                std::size_t idx = (i / 32), rem = i % 32;
                return container.size() >= idx + 1 && ((container[idx] >> rem) & 1) == 1;
            }

            void set(std::size_t i, bool flag){
                std::size_t idx = i / 32, rem = i % 32;
                if(!flag && container.size() < idx + 1){ return; }
                if(flag){
                    if(container.size() < idx + 1){ container.resize(idx + 1); }
                    container[idx] |= 1 <<  rem;
                }else{
                    if(((container[idx] >> rem) & 1) == 1){
                        container[idx] -= 1 << rem;
                    }
                }
            }

        private:
            container_type container;
        };

        template<class T>
        T ceil_pow2(T n){
            --n;
            for(std::size_t i = 1; i < sizeof(T) * 8; i <<= 1){
                n = n | (n >> i);
            }
            return n + 1;
        }

        template<class T>
        std::size_t index_of_leftmost_flag(T x){
            T y;
            std::size_t n = sizeof(T) * 8, m = n / 2;
            while(m > 0){
                y = x >> m;
                if(y != 0){
                    n -= m;
                    x = y;
                }
                m /= 2;
            }
            return 1 + sizeof(T) * 8 - n - x;
        }

        std::size_t ceil_log2(std::size_t n){
            std::size_t m = n, r = 0;
            for(std::size_t i = 0; i < sizeof(std::size_t)  * 8; ++i, m >>= 1){
                if((m & 1) == 1){
                    ++r;
                    if(r > 1){ break; }
                }
            }
            return index_of_leftmost_flag(n) + (r > 1 ? 1 : 0);
        }

#define CPP_MULTI_PRECISION_AUX_HAS_MEM_FN(fn_name, signature) \
        template<class Type> \
        class has_ ## fn_name{ \
        private: \
            typedef char yes; \
            typedef struct{ char a[2]; } no; \
            signature \
            struct ptmf_helper{}; \
            template<class T> \
            static yes check(ptmf_helper<T, &T::fn_name>* p); \
            template<class T> \
            static no check(...); \
        public: \
            BOOST_STATIC_CONSTANT(bool, value = sizeof(check<Type>(0)) == sizeof(yes)); \
        }

#define CPP_MULTI_PRECISION_AUX_SIGNATURE_RESERVE template<class T, void (T::*Func)(typename T::size_type)>
        CPP_MULTI_PRECISION_AUX_HAS_MEM_FN(reserve, CPP_MULTI_PRECISION_AUX_SIGNATURE_RESERVE);
        template<class T>
        void reserve_dispatch(T &container, std::size_t n, typename boost::enable_if<has_reserve<T>>::type* = 0){
            container.reserve(n);
        }

        template<class T>
        void reserve_dispatch(T &container, std::size_t, typename boost::disable_if<has_reserve<T>>::type* = 0){
            return;
        }

#define CPP_MULTI_PRECISION_AUX_SIGNATURE_MULTI template<class T, T& (*Func)(T&, const T&, const T&)>
        CPP_MULTI_PRECISION_AUX_HAS_MEM_FN(square_multi, CPP_MULTI_PRECISION_AUX_SIGNATURE_MULTI);
        CPP_MULTI_PRECISION_AUX_HAS_MEM_FN(kar_multi, CPP_MULTI_PRECISION_AUX_SIGNATURE_MULTI);

        template<class T>
        T &multi_dispatch_2(T &result, const T &lhs, const T &rhs, typename boost::enable_if<has_square_multi<T>>::type* = 0){
            return T::square_multi(result, lhs, rhs);
        }

        template<class T>
        T &multi_dispatch_2(T &result, const T &lhs, const T &rhs, typename boost::disable_if<has_square_multi<T>>::type* = 0){
            return T::multi(result, lhs, rhs);
        }

        template<class T>
        T &multi_dispatch(T &result, const T &lhs, const T &rhs, typename boost::enable_if<has_kar_multi<T>>::type* = 0){
            return T::kar_multi(result, lhs, rhs);
        }

        template<class T>
        T &multi_dispatch(T &result, const T &lhs, const T &rhs, typename boost::disable_if<has_kar_multi<T>>::type* = 0){
            return multi_dispatch_2(result, lhs, rhs);
        }

        template<class Ptr, class Char>
        Ptr reading_str_rbegin(Ptr str, Char zero, Char nine, Ptr delim){
            for(; str != delim && *str >= zero && *str <= nine; ++str);
            return --str;
        }

        class sign{
        public:
            sign() : value(false){}
            sign(const sign &other) : value(other.value){}
            sign(bool value_) : value(value_){}

            sign &operator =(const sign &other){
                value = other.value;
                return *this;
            }

            sign &operator =(const bool value){
                this->value = value;
                return *this;
            }

            bool operator ==(const sign &other){
                return (value && other.value) || (!value && !other.value);
            }

            bool operator ==(const bool other_value){
                return (value && other_value) || (!value && !other_value);
            }

            bool operator !=(const sign &other){
                return (value && !other.value) || (!value && other.value);
            }
            
            bool operator !=(const bool other_value){
                return (value && !other_value) || (!value && other_value);
            }

            operator bool() const{
                return value;
            }

        private:
            bool value;
        };

        template <class T, class Container = std::vector<T>, class Comp = std::greater<T>>
        class random_access_priority_queue{
        public:
            random_access_priority_queue(){}

            random_access_priority_queue(const Container& container){
                container_ = container;
                std::make_heap(container_.begin(), container_.end(), Comp());
            }

            random_access_priority_queue<T, Container, Comp> &operator =(
                const random_access_priority_queue<T, Container, Comp> &other
            ){
                if(this != &other){ container_ = other.container_; }
                return *this;
            }

            void push(const T &x){
                container_.push_back(x);
                std::push_heap(container_.begin(), container_.end(), Comp());
            }

            void pop(){
                std::pop_heap(container_.begin(), container_.end(), Comp());
                container_.pop_back();
            }

            const T &top(){
                return container_.front();
            }

            const Container &get_container() const{
                return container_;
            }

            T &operator [](size_t n){
                return container_[n];
            }

            typename Container::const_iterator begin() const{
                return container_.begin();
            }

            typename Container::const_iterator end() const{
                return container_.end();
            }

            size_t size() const{
                return container_.size();
            }

            T &base(){
                return container_.back();
            }

            typename Container::iterator erase(typename Container::iterator position) {
                return container_.erase(position);
            }

        private:
            Container container_;
        };

        template<class Type>
        bool prime_div_test(Type n){
            for(Type i = 3; i * i <= n; i += 2){
                if(n % i == 0){ return false; }
            }
            return true;
        }

        template<class Type, std::size_t N = sizeof(Type) * 8>
        struct prime;

        template<class Type>
        struct prime<Type, 32>{
            typedef Type value_type;
            typedef random_access_priority_queue<
                value_type,
                std::vector<value_type>,
                std::greater<value_type>
            > out_of_range_prime_queue_type;

            static value_type table(std::size_t n){
                static const value_type table_[] = {
#include "prime32_262144.hpp"
                };

                struct init_type{
                    init_type(std::size_t n, value_type begin, value_type end){
                        prime::table_size() = n;
                        prime::range_begin() = begin;
                        prime::range_end() = end;
                    }
                };

                static init_type init(
                    sizeof(table_) / sizeof(value_type),
                    table_[0],
                    table_[sizeof(table_) / sizeof(value_type) - 1]
                );

                return table_[n];
            }

            static std::size_t &table_size(){
                static std::size_t n;
                return n;
            }

            static value_type &range_begin(){
                static value_type n;
                return n;
            }

            static value_type &range_end(){
                static value_type n;
                return n;
            }

            static out_of_range_prime_queue_type &out_of_range_prime_queue(){
                static out_of_range_prime_queue_type queue;
                return queue;
            }
        };

        template<class Type>
        struct prime<Type, 64>{
            typedef Type value_type;

            static value_type table(std::size_t n){
                static const value_type table_[] = {
#include "prime64_262144.hpp"
                };

                struct init_type{
                    init_type(std::size_t n, value_type begin, value_type end){
                        prime::table_size() = n;
                        prime::range_begin() = begin;
                        prime::range_end() = end;
                    }
                };

                static init_type init(
                    sizeof(table_) / sizeof(value_type),
                    table_[0],
                    table_[sizeof(table_) / sizeof(value_type) - 1]
                );

                return table[n];
            }

            static std::size_t &table_size(){
                static std::size_t n;
                return n;
            }

            static value_type &range_begin(){
                static value_type n;
                return n;
            }

            static value_type &range_end(){
                static value_type n;
                return n;
            }
        };
    }
}

#endif
