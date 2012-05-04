#ifndef HPP_CPP_MULTI_PRECISION_NS_AUX
#define HPP_CPP_MULTI_PRECISION_NS_AUX

#include <string>
#include <ostream>
#include <algorithm>
#include <utility>
#include <type_traits>
#include <exception>
#include <stdexcept>
#include <cmath>
#include <cassert>
#include <limits.h>
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

        template<class Type>
        struct prime{
            typedef Type value_type;
            static value_type n_th(std::size_t n){
                static const value_type table[] = {
#include "prime32_262144.hpp"
                };
                struct size_init{ size_init(std::size_t n){ prime::size() = n; } };
                static size_init init(sizeof(table) / sizeof(value_type));
                return table[n];
            }

            static std::size_t &size(){
                static std::size_t n;
                return n;
            }
        };
    }
}

#endif
