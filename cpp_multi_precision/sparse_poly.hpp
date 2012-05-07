#ifndef HPP_CPP_MULTI_PRECISION_SPARSE_POLY
#define HPP_CPP_MULTI_PRECISION_SPARSE_POLY

#include <utility>
#include <map>
#include <type_traits>
#include "ns_aux.hpp"

namespace cpp_multi_precision{
    template<
        class OrderType,
        class CoefficientType,
        bool CommutativeRing = std::is_integral<CoefficientType>::value,
        class Alloc = std::allocator<int>
    > class sparse_poly{
        class container_type : public std::map<
            OrderType, CoefficientType, std::less<OrderType>,
            typename Alloc::template rebind<std::pair<const OrderType, CoefficientType> >::other
        >{
        private:
            typedef std::map<
                OrderType, CoefficientType, std::less<OrderType>,
                typename Alloc::template rebind<std::pair<const OrderType, CoefficientType> >::other
            > base_type;
            typedef typename base_type::key_type order_type;
            typedef typename base_type::mapped_type coefficient_type;

        public:
            container_type() : base_type(){}
            template<class Iter>
            container_type(const Iter &first, const Iter &last) : base_type(first, last){}

            struct ref_value_type{
                ref_value_type(const order_type &first_, const coefficient_type &second_) : first(first_), second(second_){}
                ref_value_type(const ref_value_type &other) : first(other.first), second(other.second){}
                ref_value_type(const std::pair<const order_type&, const coefficient_type&> &pair) : first(pair.first), second(pair.second){}
                const order_type &first;
                const coefficient_type &second;
            };

            typename base_type::iterator assign(const ref_value_type &v){
                if(v.second == 0){
                    typename base_type::iterator iter = base_type::find(v.first);
                    if(iter != base_type::end()){
                        base_type::erase(iter);
                    }
                    return base_type::end();
                }else{
                    typename base_type::iterator iter = base_type::find(v.first);
                    if(iter != base_type::end()){
                        iter->second = v.second;
                    }else{
                        base_type::insert(iter, std::make_pair(v.first, v.second));
                    }
                    return iter;
                }
            }

            typename base_type::iterator add(const ref_value_type &v){
                std::pair<typename base_type::iterator, bool> result = base_type::insert(std::make_pair(v.first, v.second));
                coefficient_type &coe(result.first->second);
                if(!result.second){
                    coe += v.second;
                }
                if(coe == 0){
                    base_type::erase(result.first);
                    return base_type::end();
                }else{
                    return result.first;
                }
            }

            typename base_type::iterator sub(const ref_value_type &v){
                std::pair<typename base_type::iterator, bool> result = base_type::insert(std::make_pair(v.first, v.second));
                coefficient_type &coe(result.first->second);
                if(result.second){
                    set_sign(coe, get_sign(result.first->second));
                }else{
                    coe -= v.second;
                }
                if(coe == 0){
                    base_type::erase(result.first);
                    return base_type::end();
                }else{
                    return result.first;
                }
            }

            typename base_type::iterator multi(const ref_value_type &v){
                typename base_type::iterator iter = base_type::find(v.first);
                if(iter == base_type::end()){ return base_type::end(); }
                coefficient_type &coe(iter.first->second), lhs(coe);
                aux::multi_dispatch(coe, lhs, v.second);
                if(coe == 0){
                    base_type::erase(iter.first);
                    return base_type::end();
                }else{
                    return iter.first;
                }
            }

            typename base_type::iterator div(const ref_value_type &v){
                typename base_type::iterator iter = base_type::find(v.first);
                if(iter == base_type::end()){ return base_type::end(); }
                coefficient_type &coe(iter.first->second), lhs(coe);
                coe = lhs / v.second;
                if(coe == 0){
                    base_type::erase(iter.first);
                    return base_type::end();
                }else{
                    return iter.first;
                }
            }
        };

    public:
        typedef typename container_type::key_type order_type;
        typedef typename container_type::mapped_type coefficient_type;
        static const bool commutative_ring = CommutativeRing;

        sparse_poly() : container(){}
        sparse_poly(const sparse_poly &other) : container(other.container){}
        sparse_poly(sparse_poly &&other) : container(other.container){}
        sparse_poly(const coefficient_type &coe) : container(){
            if(coe != 0){
                container.insert(typename container_type::value_type(0, coe));
            }
        }

    private:
        sparse_poly(const container_type &other_container) : container(other_container){}
        sparse_poly(typename container_type::const_iterator first, typename container_type::const_iterator last) : container(first, last){}

    public:
        virtual ~sparse_poly(){}

        const container_type &get_container() const{ return container; }
        void assign(const sparse_poly &other){ container = other.container; }
        void assign(sparse_poly &&other){ container = other.container; }

    private:
        void assign(const container_type &other_container){ container = other_container; }
        void assign(typename container_type::const_iterator first, typename container_type::const_iterator last){
            container.clear();
            container.insert(first, last);
        }

    private:
        struct coefficient_proxy{
            coefficient_proxy(){}
            const coefficient_type &get() const{ return *coe; }
            coefficient_proxy(const coefficient_proxy &other) : container(other.container), order(other.order), coe(other.coe){}
            const coefficient_proxy &operator =(const coefficient_type &v) const{
                if(v == 0){
                    container->clear();
                }else{
                    container->assign(typename container_type::ref_value_type(order, v));
                }
                return *this;
            }

            const coefficient_proxy &operator +=(const coefficient_type &v) const{
                container->add(typename container_type::ref_value_type(order, v));
                return *this;
            }

            const coefficient_proxy &operator -=(const coefficient_type &v) const{
                container->sub(typename container_type::ref_value_type(order, v));
                return *this;
            }

            const coefficient_proxy &operator *=(const coefficient_type &v) const{
                container->multi(typename container_type::ref_value_type(order, v));
                return *this;
            }

            const coefficient_proxy &operator /=(const coefficient_type &v) const{
                container->div(typename container_type::ref_value_type(order, v));
                return *this;
            }

            const coefficient_proxy &operator ()(const coefficient_type &v) const{
                operator =(v);
                return *this;
            }

            coefficient_proxy operator [](const order_type &v) const{
                coefficient_proxy ret;
                ret.container = container;
                ret.order = v;
                return std::move(ret);
            }

            container_type *container;
            order_type order;
            coefficient_type *coe;
        };

    public:
        sparse_poly &operator =(const coefficient_type &rhs){
            assign(rhs);
            return *this;
        }

        sparse_poly &operator =(const coefficient_type &&rhs){
            assign(rhs);
            return *this;
        }

        sparse_poly &operator =(const sparse_poly &rhs){
            assign(rhs);
            return *this;
        }

        sparse_poly &operator =(const sparse_poly &&rhs){
            assign(rhs);
            return *this;
        }

        const sparse_poly &operator +() const{
            return *this;
        }

        sparse_poly operator -() const{
            sparse_poly r;
            r.sub_iterator(container.begin(), container.end());
            return std::move(r);
        }

        sparse_poly &operator +=(const sparse_poly &rhs){
            add_order_n(rhs, 0);
            return *this;
        }

        sparse_poly &operator +=(const coefficient_type &rhs){
            if(container.empty()){
                assign(rhs);
            }else{
                typename container_type::iterator iter = container.find(0);
                if(iter != container.end()){
                    container.insert(typename container_type::value_type(order_type(0), rhs));
                }else{
                    iter->second += rhs;
                    if(iter->second == 0){ container.erase(iter); }
                }
            }
            return *this;
        }

        sparse_poly operator +(const sparse_poly &rhs) const{
            sparse_poly result(*this);
            result += rhs;
            return std::move(result);
        }

        sparse_poly operator +(const coefficient_type &rhs){
            sparse_poly result(*this);
            result += rhs;
            return std::move(result);
        }

        sparse_poly &operator -=(const sparse_poly &rhs){
            sub_iterator(rhs.container.begin(), rhs.container.end());
            return *this;
        }

        sparse_poly &operator -=(const coefficient_type &rhs){
            if(container.empty()){
                assign(rhs);
            }else{
                typename container_type::iterator iter = container.find(0);
                if(iter != container.end()){
                    container.insert(typename container_type::value_type(order_type(0), rhs));
                }else{
                    iter->second -= rhs;
                    if(iter->second == 0){ container.erase(iter); }
                }
            }
            return *this;
        }

        sparse_poly operator -(const sparse_poly &rhs) const{
            sparse_poly result(*this);
            result -= rhs;
            return std::move(result);
        }

        sparse_poly operator -(const coefficient_type &rhs) const{
            sparse_poly result(*this);
            result -= rhs;
            return std::move(result);
        }

        sparse_poly operator *(const sparse_poly &rhs) const{
            sparse_poly r;
            kar_multi(r, *this, rhs);
            return std::move(r);
        }

        sparse_poly operator *(const coefficient_type &rhs) const{
            sparse_poly r(*this);
            r *= rhs;
            return std::move(r);
        }

        sparse_poly &operator *=(const sparse_poly &rhs){
            assign(*this * rhs);
            return *this;
        }

        sparse_poly &operator *=(const coefficient_type &rhs){
            if(container.empty()){ return *this; }
            for(typename container_type::iterator iter = container.begin(); ; ){
                coefficient_type &coe(iter->second);
                coe *= rhs;
                if(coe == 0){
                    iter = container.erase(iter);
                }else{
                    ++iter;
                }
                if(iter == container.end()){ break; }
            }
            return *this;
        }

        sparse_poly operator /(const sparse_poly &rhs) const{
            sparse_poly r;
            if(container.empty()){
                return std::move(r);
            }
            monic_div(r, *this, rhs);
            return std::move(r);
        }

        sparse_poly operator /(const coefficient_type &rhs) const{
            sparse_poly r(*this);
            r /= rhs;
            return std::move(r);
        }

        sparse_poly &operator /=(const sparse_poly &rhs){
            assign(*this / rhs);
            return *this;
        }

        sparse_poly &operator /=(const coefficient_type &rhs){
            if(container.empty()){ return *this; }
            for(typename container_type::iterator iter = container.begin(); ; ){
                coefficient_type &coe(iter->second);
                coe /= rhs;
                if(coe == 0){
                    iter = container.erase(iter);
                }else{
                    ++iter;
                }
                if(iter == container.end()){ break; }
            }
            return *this;
        }

        sparse_poly operator %(const sparse_poly &rhs) const{
            sparse_poly r, rem;
            monic_div(r, rem, *this, rhs);
            return std::move(rem);
        }

        sparse_poly operator %(const coefficient_type &rhs) const{
            sparse_poly r, rem;
            monic_div(r, rem, *this, sparse_poly(rhs));
            return std::move(rem);
        }

        sparse_poly &operator %=(const sparse_poly &rhs){
            assign(*this % rhs);
            return *this;
        }

        sparse_poly &operator %=(const coefficient_type &rhs){
            assign(*this % rhs);
            return *this;
        }

        bool operator <(const sparse_poly &rhs) const{
            return base_less_eq(false, rhs.container);
        }

        bool operator <(const coefficient_type &rhs) const{
            return base_less_eq(false, sparse_poly(rhs).container);
        }

        bool operator >(const sparse_poly &rhs) const{
            return base_greater_eq(false, rhs.container);
        }

        bool operator >(const coefficient_type &rhs) const{
            return base_greater_eq(false, sparse_poly(rhs).container);
        }

        bool operator <=(const sparse_poly &rhs) const{
            return base_less_eq(true, rhs.container);
        }

        bool operator <=(const coefficient_type &rhs) const{
            return base_less_eq(true, sparse_poly(rhs).container);
        }

        bool operator >=(const sparse_poly &rhs) const{
            return base_greater_eq(true, rhs.container);
        }

        bool operator >=(const coefficient_type &rhs) const{
            return base_greater_eq(true, sparse_poly(rhs).container);
        }

        bool operator ==(const sparse_poly &rhs) const{
            return container == rhs.container;
        }

        bool operator ==(const coefficient_type &rhs) const{
            return container == sparse_poly(rhs).container;
        }

        bool operator !=(const sparse_poly &rhs) const{
            return container != rhs.container;
        }

        bool operator !=(const coefficient_type &rhs) const{
            return container != sparse_poly(rhs).container;
        }

        coefficient_proxy operator [](const order_type &order){
            coefficient_proxy ret;
            ret.container = &container;
            ret.order = order;
            ret.coe = &container[order];
            return ret;
        }

        static sparse_poly &kar_multi(sparse_poly &result, const sparse_poly &lhs, const sparse_poly &rhs){
            return result = kar_multi_impl_n(lhs, rhs);
        }

        static sparse_poly &eea(sparse_poly &result, sparse_poly &c_lhs, sparse_poly &c_rhs, const sparse_poly &lhs, const sparse_poly &rhs){
            if(lhs.container.rbegin()->first < rhs.container.rbegin()->first){
                eea_default_impl(result, c_rhs, c_lhs, rhs, lhs);
            }else{
                eea_default_impl(result, c_lhs, c_rhs, lhs, rhs);
            }
            return result;
        }

        const order_type &deg() const{ return container.rbegin()->first; }
        const coefficient_type &lc() const{ return container.rbegin()->second; }

        static sparse_poly &normal(sparse_poly &result, const sparse_poly &x){
            if(x.container.empty()){
                result = 0;
                return result;
            }
            result.container = x.container;
            result /= x.lc();
            return result;
        }

        void normalize(){
            sparse_poly r(*this);
            normal(r, *this);
            *this = std::move(r);
        }

        void lu(){
            if(container.empty()){
                container.insert(typename container_type::value_type(0, 1));
                return;
            }
            *this = sparse_poly(lc());
        }

        coefficient_type infinity_norm() const{
            coefficient_type r = 0;
            for(typename container_type::const_iterator iter = container.begin(), end = container.end(); iter != end; ++iter){
                const coefficient_type &coe(iter->second);
                max_dispatch(r, coe);
            }
            set_sign_dispatch(r, true);
            return std::move(r);
        }

        bool is_monic() const{
            return !container.empty() && container.rbegin()->second == 1;
        }

        template<class Variable>
        std::string to_string(const Variable &v) const{
            return to_string_impl<std::string, char, Variable, std::ostringstream>(
                v,
                '*', '^', '+', '-', '0', '(', ')'
            );
        }

        std::string to_string() const{
            return to_string("x");
        }

        template<class Variable>
        std::wstring to_wstring(const Variable &v) const{
            return to_string_impl<std::wstring, wchar_t, Variable, std::wostringstream>(
                v,
                L'*', L'^', L'+', L'-', L'0', L'(', L')'
            );
        }

        std::wstring to_wstring() const{
            return to_wstring(L"x");
        }

    private:
#define CPP_MULTI_PRECISION_AUX_SIGNATURE_SPARSE_POLY_ABS_MAX template<class T, const typename T::base_type& (*Func)(const typename T::base_type&, const typename T::base_type&)>
        CPP_MULTI_PRECISION_AUX_HAS_MEM_FN(max, CPP_MULTI_PRECISION_AUX_SIGNATURE_SPARSE_POLY_ABS_MAX);
        template<class T>
        static void max_dispatch(T &a, const T &b, typename boost::enable_if<has_max<T>>::type* = nullptr){
            const typename T::base_type *ptr = &((T::base_type::max)(a, b));
            if(ptr != &a){ a = b; }
        }

        template<class T>
        static void max_dispatch(T &a, const T &b, typename boost::disable_if<has_max<T>>::type* = nullptr){
            a = (std::max)(std::abs(a), std::abs(b));
        }

#define CPP_MULTI_PRECISION_AUX_SIGNATURE_SPARSE_POLY_CEIL_POW2 template<class T, void (T::base_type::*Func)()>
        CPP_MULTI_PRECISION_AUX_HAS_MEM_FN(ceil_pow2, CPP_MULTI_PRECISION_AUX_SIGNATURE_SPARSE_POLY_CEIL_POW2);
        template<class T>
        static void ceil_pow2_dispatch(T &x, const T &y, typename boost::enable_if<has_ceil_pow2<T>>::type* = nullptr){
            x = y;
            x.ceil_pow2();
        }

        template<class T>
        static void ceil_pow2_dispatch(T &x, const T &y, typename boost::disable_if<has_ceil_pow2<T>>::type* = nullptr){
            x = aux::ceil_pow2(y);
        }

#define CPP_MULTI_PRECISION_AUX_SIGNATURE_SPARSE_POLY_CEIL_LOG2 template<class T, std::size_t (T::base_type::*Func)() const>
        CPP_MULTI_PRECISION_AUX_HAS_MEM_FN(ceil_log2, CPP_MULTI_PRECISION_AUX_SIGNATURE_SPARSE_POLY_CEIL_LOG2);
        template<class T>
        static std::size_t ceil_log2_dispatch(const T &x, typename boost::enable_if<has_ceil_log2<T>>::type* = nullptr){
            return x.ceil_log2();
        }

        template<class T>
        static std::size_t ceil_log2_dispatch(const T &x, typename boost::disable_if<has_ceil_log2<T>>::type* = nullptr){
            return aux::ceil_log2(x);
        }

#define CPP_MULTI_PRECISION_AUX_SIGNATURE_SPARSE_POLY_SIGN template<class T, bool Sign>
        CPP_MULTI_PRECISION_AUX_HAS_MEM_FN(sign, CPP_MULTI_PRECISION_AUX_SIGNATURE_SPARSE_POLY_SIGN);
        template<class T>
        static bool get_sign_dispatch(const T &x, typename boost::enable_if<has_sign<T>>::type* = nullptr){
            return x.sign;
        }

        template<class T>
        static bool get_sign_dispatch(const T &x, typename boost::disable_if<has_sign<T>>::type* = nullptr){
            return x >= 0;
        }

        template<class T>
        static void set_sign_dispatch(T &x, bool b, typename boost::enable_if<has_sign<T>>::type* = nullptr){
            x.sign = b;
        }

        template<class T>
        static void set_sign_dispatch(T &x, bool b, typename boost::disable_if<has_sign<T>>::type* = nullptr){
            if(b){
                if(x < 0){ x = -x; }
            }else{
                if(x > 0){ x = -x; }
            }
        }

        template<class T>
        static void negate_dispatch(T &x, typename boost::enable_if<has_sign<T>>::type* = 0){
            x.sign = !x.sign;
        }

        template<class T>
        static void negate_dispatch(T &x, typename boost::disable_if<has_sign<T>>::type* = 0){
            x = -x;
        }

        template<class T>
        static void assign_reverse_dispatch(T &x, const T &y, typename boost::enable_if<has_sign<T>>::type* = 0){
            x.sign = !y.sign;
        }

        template<class T>
        static void assign_reverse_dispatch(T &x, const T &y, typename boost::disable_if<has_sign<T>>::type* = 0){
            if(y > 0){
                if(x > 0){ x = -x; }
            }else{
                if(x < 0){ x = -x; }
            }
        }

        template<class T>
        static bool get_sign(const T &x, typename boost::enable_if<has_sign<T>>::type* = 0){
            return x.sign;
        }

        template<class T>
        static bool get_sign(const T &x, typename boost::disable_if<has_sign<T>>::type* = 0){
            return x >= 0;
        }

        template<class T>
        static void set_sign(T &x, bool b, typename boost::enable_if<has_sign<T>>::type* = 0){
            x.sign = b;
        }

        template<class T>
        static void set_sign(T &x, bool b, typename boost::disable_if<has_sign<T>>::type* = 0){
            if(b){
                if(x < 0){ x = -x; }
            }else{
                if(x > 0){ x = -x; }
            }
        }

        static sparse_poly &square_multi(sparse_poly &result, const sparse_poly &lhs, const sparse_poly &rhs){
            square_multi_impl(result, lhs.container.begin(), lhs.container.end(), rhs.container.begin(), rhs.container.end());
            return result;
        }

        static sparse_poly &square_div(sparse_poly &result, sparse_poly &rem, const sparse_poly &lhs, const sparse_poly &rhs){
            rem = lhs;
            result = 0;
            const order_type &rhs_order(rhs.container.rbegin()->first);
            const coefficient_type &rhs_coe(rhs.container.rbegin()->second);
            for(; !rem.container.empty(); ){
                const order_type &rem_order(rem.container.rbegin()->first);
                const coefficient_type &rem_coe(rem.container.rbegin()->second);
                if(rem_order >= rhs_order){
                    order_type n = rem_order;
                    n -= rhs_order;
                    coefficient_type q = rem_coe / rhs_coe;
                    if(q == 0){
                        break;
                    }
                    rem.sub_n_q(rhs, n, q);
                    result.addition_order_coe(n, q);
                }else{ break; }
            }
            return result;
        }

        static sparse_poly &square_div(sparse_poly &result, const sparse_poly &lhs, const sparse_poly &rhs){
            sparse_poly rem;
            return square_div(result, rem, lhs, rhs);
        }

        static sparse_poly &square_mod(sparse_poly &rem, const sparse_poly &lhs, const sparse_poly &rhs){
            sparse_poly result;
            square_div(result, rem, lhs, rhs);
            return rem;
        }

        static sparse_poly &monic_div(sparse_poly &result, sparse_poly &rem, const sparse_poly &lhs, const sparse_poly &rhs){
            return monic_div_impl<true>(result, rem, lhs, rhs);
        }
        
        static sparse_poly &monic_div(sparse_poly &result, const sparse_poly &lhs, const sparse_poly &rhs){
            sparse_poly rem;
            return monic_div_impl<false>(result, rem, lhs, rhs);
        }

        static sparse_poly &monic_mod(sparse_poly &rem, const sparse_poly &lhs, const sparse_poly &rhs){
            sparse_poly result;
            monic_div_impl<true>(result, rem, lhs, rhs);
            return rem;
        }

        bool base_less_eq(bool final, const container_type &rhs_container) const{
            typename container_type::const_reverse_iterator
                lhs_iter = container.rbegin(),
                lhs_end = container.rend(),
                rhs_iter = rhs_container.rbegin(),
                rhs_end = rhs_container.rend();
            bool end_l = lhs_iter == lhs_end, end_r = rhs_iter == rhs_end;
            if(!end_l || !end_r){
                if(end_l && !end_r){ return false; }
                if(!end_l && end_r){ return true; }
                for(; lhs_iter != lhs_end && rhs_iter != rhs_end; ++lhs_iter, ++rhs_iter){
                    const order_type &lhs_order(lhs_iter->first), &rhs_order(rhs_iter->first);
                    if(lhs_order == rhs_order){
                        const coefficient_type &lhs_coe(lhs_iter->second), &rhs_coe(rhs_iter->second);
                        if(lhs_coe < rhs_coe){
                            return true;
                        }else if(lhs_coe > rhs_coe){
                            return false;
                        }
                    }else if(lhs_order < rhs_order){
                        return true;
                    }else{
                        return false;
                    }
                }
                if(lhs_iter == lhs_end && rhs_iter != rhs_end){ return false; }
                if(lhs_iter != lhs_end && rhs_iter == rhs_end){ return true; }
            }
            return final;
        }

        bool base_greater_eq(bool final, const container_type &rhs_container) const{
            typename container_type::const_reverse_iterator
                lhs_iter = container.rbegin(),
                lhs_end = container.rend(),
                rhs_iter = rhs_container.rbegin(),
                rhs_end = rhs_container.rend();
            bool end_l = lhs_iter == lhs_end, end_r = rhs_iter == rhs_end;
            if(!end_l || !end_r){
                if(end_l && !end_r){ return true; }
                if(!end_l && end_r){ return false; }
                for(; lhs_iter != lhs_end && rhs_iter != rhs_end; ++lhs_iter, ++rhs_iter){
                    const order_type &lhs_order(lhs_iter->first), &rhs_order(rhs_iter->first);
                    if(lhs_order == rhs_order){
                        const coefficient_type &lhs_coe(lhs_iter->second), &rhs_coe(rhs_iter->second);
                        if(lhs_coe > rhs_coe){
                            return true;
                        }else if(lhs_coe < rhs_coe){
                            return false;
                        }
                    }else if(lhs_order > rhs_order){
                        return true;
                    }else{
                        return false;
                    }
                }
                if(lhs_iter == lhs_end && rhs_iter != rhs_end){ return true; }
                if(lhs_iter != lhs_end && rhs_iter == rhs_end){ return false; }
            }
            return final;
        }

        void add_order_n(const sparse_poly &rhs, const order_type &n){
            for(typename container_type::const_iterator rhs_iter = rhs.container.begin(), rhs_end = rhs.container.end(); rhs_iter != rhs_end; ++rhs_iter){
                order_type order = rhs_iter->first;
                order += n;
                const coefficient_type &coe(rhs_iter->second);
                typename container_type::iterator iter = container.find(order);
                if(iter == container.end()){
                    iter = container.insert(iter, std::make_pair(order, coe));
                }else{
                    coefficient_type &lhs_coe(iter->second);
                    lhs_coe += coe;
                    if(lhs_coe == 0){ container.erase(iter); }
                }
            }
        }

        static void square_multi_impl(
            sparse_poly &result,
            const typename container_type::const_iterator &lhs_first, const typename container_type::const_iterator &lhs_last,
            const typename container_type::const_iterator &rhs_first, const typename container_type::const_iterator &rhs_last
        ){
            result.container.clear();
            order_type temp_order;
            coefficient_type temp_coefficient;
            for(typename container_type::const_iterator lhs_iter = lhs_first, lhs_end = lhs_last; lhs_iter != lhs_end; ++lhs_iter){
                const order_type &lhs_order(lhs_iter->first);
                const coefficient_type &lhs_coe(lhs_iter->second);
                for(typename container_type::const_iterator rhs_iter = rhs_first, rhs_end = rhs_last; rhs_iter != rhs_end; ++rhs_iter){
                    const order_type &rhs_order(rhs_iter->first);
                    const coefficient_type &rhs_coe(rhs_iter->second);
                    temp_order = rhs_order;
                    temp_order += lhs_order;
                    temp_coefficient = lhs_coe * rhs_coe;
                    result.addition_order_coe(temp_order, temp_coefficient);
                }
            }
        }

        void sub_order(typename container_type::iterator iter, typename container_type::iterator end, const order_type &a){
            for(; iter != end; ++iter){
                const_cast<order_type&>(iter->first) -= a;
            }
        }

        static sparse_poly kar_multi_impl_n(const sparse_poly &f, const sparse_poly &g){
            if(f.container.empty() || g.container.empty()){
                return sparse_poly();
            }
            const order_type &order_f(f.container.rbegin()->first), &order_g(g.container.rbegin()->first);
            order_type n;
            ceil_pow2_dispatch(n, order_f > order_g ? order_f : order_g);
            if(n < 2){
                sparse_poly result;
                square_multi(result, f, g);
                return std::move(result);
            }
            n >>= 1;
            sparse_poly f_0, f_1, g_0, g_1, ff, gg;
            typename container_type::const_iterator
                upper_bound_f = f.container.upper_bound(n),
                upper_bound_g = g.container.upper_bound(n);
            kar_multi_add_ffgg(f_0, f.container.begin(), upper_bound_f, 0);
            kar_multi_add_ffgg(g_0, g.container.begin(), upper_bound_g, 0);
            kar_multi_add_ffgg(f_1, upper_bound_f, f.container.end(), n);
            kar_multi_add_ffgg(g_1, upper_bound_g, g.container.end(), n);
            ff = f_0 + f_1, gg = g_0 + g_1;
            sparse_poly
                fg_0 = kar_multi_impl_n(f_0, g_0),
                fg_1 = kar_multi_impl_n(f_1, g_1),
                ffgg = kar_multi_impl_n(ff, gg);
            f_0.container.clear(), f_1.container.clear();
            g_0.container.clear(), g_1.container.clear();
            ff.container.clear(), gg.container.clear();
            ffgg -= fg_0, ffgg -= fg_1;
            sparse_poly result;
            result += fg_0;
            ffgg.radix_shift(n);
            result += ffgg;
            n <<= 1;
            fg_1.radix_shift(n);
            result += fg_1;
            return std::move(result);
        }

        static void kar_multi_add_ffgg(
            sparse_poly &result,
            typename container_type::const_iterator first,
            typename container_type::const_iterator last,
            const order_type &n
        ){
            for(; first != last; ++first){
                result.container[first->first - n] = first->second;
            }
        }

        void sub_iterator(typename container_type::const_iterator rhs_iter, typename container_type::const_iterator rhs_end){
            sub_iterator(rhs_iter, rhs_end, 0);
        }

        void sub_iterator(typename container_type::const_iterator rhs_iter, typename container_type::const_iterator rhs_end, const order_type &xrhs){
            for(; rhs_iter != rhs_end; ++rhs_iter){
                order_type order(rhs_iter->first);
                order -= xrhs;
                const coefficient_type &coe(rhs_iter->second);
                typename container_type::iterator iter = container.find(order);
                if(iter == container.end()){
                    iter = container.add(typename container_type::ref_value_type(order, coe));
                    if(iter != container.end()){ negate_dispatch(iter->second); }
                }else{
                    coefficient_type &lhs_coe(iter->second);
                    lhs_coe -= coe;
                    if(lhs_coe == 0){ container.erase(iter); }
                }
            }
        }

        void radix_shift(const order_type &n){
            for(typename container_type::iterator iter = container.begin(), end = container.end(); iter != end; ++iter){
                const_cast<order_type&>(iter->first) += n;
            }
        }

        template<bool Rem>
        static sparse_poly &monic_div_impl(sparse_poly &result, sparse_poly &rem, const sparse_poly &lhs, const sparse_poly &rhs){
            if(!commutative_ring || !rhs.is_monic()){
                return square_div(result, rem, lhs, rhs);
            }
            result.container.clear();
            if(rhs.container.rbegin()->first > lhs.container.rbegin()->first){
                if(Rem){ rem.assign(lhs); }
                return result;
            }
            order_type m(lhs.container.rbegin()->first);
            m -= rhs.container.rbegin()->first;
            sparse_poly inv_rev_rhs;
            {
                sparse_poly rev_rhs;
                sparse_poly::rev(rev_rhs, rhs);
                sparse_poly::inverse(inv_rev_rhs, rev_rhs, m + 1);
            }
            {
                sparse_poly rev_lhs;
                sparse_poly::rev(rev_lhs, lhs);
                result = rev_lhs * inv_rev_rhs;
            }
            result.container.erase(result.container.upper_bound(m), result.container.end());
            result.rev(m);
            if(Rem){ rem = lhs - rhs * result; }
            return result;
        }

        static void rev(sparse_poly &result, const sparse_poly &a){
            result.container.clear();
            const order_type &order(a.container.rbegin()->first);
            for(
                typename container_type::const_reverse_iterator iter = a.container.rbegin(),
                end = a.container.rend();
                iter != end; ++iter
            ){
                order_type x(order);
                x -= iter->first;
                result.container.insert(std::make_pair(std::move(x), iter->second));
            }
        }

        void rev(){
            sparse_poly result;
            rev(result, *this);
            assign(std::move(result));
        }

        void rev(const order_type &n){
            if(container.find(n) != container.end()){
                sparse_poly result;
                rev(result, *this);
                assign(std::move(result));
            }else{
                sparse_poly result;
                container.insert(std::make_pair(n, 0));
                for(
                    typename container_type::const_iterator iter = container.begin(), end = container.lower_bound(n);
                    iter != end;
                    ++iter
                ){
                    order_type x(n);
                    x -= iter->first;
                    result.container.insert(std::make_pair(std::move(x), iter->second));
                }
                assign(std::move(result));
            }
        }

        static sparse_poly &gcd_default_impl(sparse_poly &result, sparse_poly lhs, sparse_poly rhs){
            if(rhs.container.empty()){
                result.container.clear();
                return result;
            }
            sparse_poly *operands[3] = { &lhs, &rhs, &result };
            for(; ; ){
                mod(*operands[2], *operands[0], *operands[1]);
                if(operands[2]->size() == 0){ break; }
                sparse_poly *ptr = operands[0];
                for(int i = 0; i < 2; ++i){ operands[i] = operands[i + 1]; }
                operands[2] = ptr;
            }
            if(operands[1] != result){ result.assign(*operands[1]); }
            return result;
        }

        static sparse_poly &eea_default_impl(
            sparse_poly &result,
            sparse_poly &c_lhs,
            sparse_poly &c_rhs,
            const sparse_poly &lhs,
            const sparse_poly &rhs
        ){
            if(rhs.container.empty()){
                result.container.clear();
                c_lhs.container.clear();
                c_rhs.container.clear();
                return result;
            }
            sparse_poly r_0, r_1, s_0 = 1 / lhs.lc(), s_1 = coefficient_type(0), t_0 = coefficient_type(0), t_1 = 1 / rhs.lc();
            normal(r_0, lhs), normal(r_1, rhs);
            for(; !r_1.container.empty(); ){
                sparse_poly q = r_0 / r_1, rho = r_0 - q * r_1, r_m = r_1, s_m = s_1, t_m = t_1;
                rho.lu();
                r_1 = (r_0 - q * r_1) / rho;
                s_1 = (s_0 - q * s_1) / rho;
                t_1 = (t_0 - q * t_1) / rho;
                r_0 = std::move(r_m);
                s_0 = std::move(s_m);
                t_0 = std::move(t_m);
            }
            result = std::move(r_0);
            c_lhs = std::move(s_0);
            c_rhs = std::move(t_0);
            return result;
        }

        static sparse_poly &primitive_part(sparse_poly &result, const sparse_poly &x){
            result = x;
            coefficient_type c, t; x.cout(c);
            for(typename container_type::iterator iter = result.container.begin(), end = result.container.end(); iter != end; ++iter){
                t.assign(iter->second);
                coefficient_type &coe(iter->second);
                coefficient_type::div(coe, t, c);
                if(coe == 0){ iter = result.container.erase(iter); }
            }
            return result;
        }

        static sparse_poly &inverse(sparse_poly &result, const sparse_poly &f, const order_type &l){
            result.container.clear();
            sparse_poly &g(result);
            g[0] = 1;
            std::size_t r = ceil_log2_dispatch(l);
            order_type rem(1);
            for(std::size_t i = 0; i < r; ++i){
                rem <<= 1;
                sparse_poly next_g(g);
                for(typename container_type::iterator double_g_iter = next_g.container.begin(), double_g_end = next_g.container.end(); double_g_iter != double_g_end; ++double_g_iter){
                    coefficient_type coe_double_g(double_g_iter->second);
                    double_g_iter->second = coe_double_g * 2;
                }
                next_g -= f * g * g;
                if(next_g.container.rbegin()->first >= rem){
                    next_g.container.erase(next_g.container.find(rem), next_g.container.end());
                }
                g = std::move(next_g);
            }
            return result;
        }

        coefficient_type &cout(coefficient_type &result) const{
            if(container.empty()){
                result.container.clear();
                return result;
            }
            coefficient_type temp;
            result.assign(container.begin()->second);
            typename container_type::const_iterator iter = container.begin(), end = container.end(); ++iter;
            for(; iter != end; ++iter){
                coefficient_type::gcd(temp, result, iter->second);
                result.assign(temp);
                if(result == 1){ break; }
            }
            return result;
        }

        void sub_n_q(const sparse_poly &rhs, const order_type &n, const coefficient_type &q){
            for(typename container_type::const_iterator rhs_iter = rhs.container.begin(), rhs_end = rhs.container.end(); rhs_iter != rhs_end; ++rhs_iter){
                const order_type &order(rhs_iter->first);
                order_type new_order(order);
                new_order += n;
                const coefficient_type &coe(rhs_iter->second);
                coefficient_type new_coe = coe * q;
                typename container_type::iterator iter = container.find(new_order);
                if(iter == container.end()){
                    iter = container.add(typename container_type::ref_value_type(new_order, new_coe));
                    if(iter != container.end()){ assign_reverse_dispatch(iter->second, new_coe); }
                }else{
                    coefficient_type &lhs_coe(iter->second);
                    lhs_coe -= new_coe;
                    if(lhs_coe == 0){ container.erase(iter); }
                }
            }
        }

        void addition_order_coe(const order_type &order, const coefficient_type &coe){
            typename container_type::iterator iter = container.find(order);
            if(iter == container.end()){
                container.add(typename container_type::ref_value_type(order, coe));
            }else{
                coefficient_type &lhs_coe(iter->second);
                lhs_coe += coe;
                if(lhs_coe == 0){ container.erase(iter); }
            }
        }

        void subtraction_order_coe(const order_type &order, const coefficient_type &coe){
            typename container_type::iterator iter = container.find(order);
            if(iter == container.end()){
                container.sub(typename container_type::ref_value_type(order, coe));
            }else{
                coefficient_type &lhs_coe(iter->second);
                lhs_coe -= coe;
                if(lhs_coe == 0){ container.erase(iter); }
            }
        }

        template<class Str, class Char, class Variable, class OStringStream>
        Str to_string_impl(
            const Variable &v,
            Char multi,
            Char pow,
            Char plus,
            Char mn,
            Char zero,
            Char l_pare,
            Char r_pare
        ) const{
            Str result;
            if(container.empty()){
                result += zero;
                return std::move(result);
            }
            typename container_type::const_reverse_iterator first = container.rbegin();
            for(typename container_type::const_reverse_iterator iter = container.rbegin(), end = container.rend(); iter != end; ++iter){
                const order_type &order(iter->first);
                const coefficient_type &coe(iter->second);
                if(coe == 0){ continue; }
                if(first != iter && coe > 0){
                    result += plus;
                }
                if(coe == 1){
                    if(order == 0){
                        OStringStream ostringstream;
                        ostringstream << coe;
                        result += ostringstream.str();
                    }
                }else if(coe == -1){
                    if(order == 0){
                        OStringStream ostringstream;
                        ostringstream << coe;
                        result += ostringstream.str();
                    }else{
                        result += mn;
                    }
                }else if(coe > 0 || coe < 0){
                    OStringStream ostringstream;
                    ostringstream << coe;
                    result += ostringstream.str();
                }
                if(order != 0){
                    result += v;
                    if(order != 1){
                        result += pow;
                        if(order < 0){ result += l_pare; }
                        OStringStream ostringstream;
                        ostringstream << order;
                        result += ostringstream.str();
                        if(order < 0){ result += r_pare; }
                    }
                }
            }
            return std::move(result);
        }

    private:
        container_type container;
    };

    template<class OrderType, class CoefficientType, bool CoefficientIsIntegral, class Alloc>
    sparse_poly<OrderType, CoefficientType, CoefficientIsIntegral, Alloc>
    operator +(
        const typename sparse_poly<OrderType, CoefficientType, CoefficientIsIntegral, Alloc>::coefficient_type &lhs,
        const sparse_poly<OrderType, CoefficientType, CoefficientIsIntegral, Alloc> &rhs
    ){ return sparse_poly<OrderType, CoefficientType, CoefficientIsIntegral, Alloc>(lhs) + rhs; }

    template<class OrderType, class CoefficientType, bool CoefficientIsIntegral, class Alloc>
    sparse_poly<OrderType, CoefficientType, CoefficientIsIntegral, Alloc>
    operator -(
        const typename sparse_poly<OrderType, CoefficientType, CoefficientIsIntegral, Alloc>::coefficient_type &lhs,
        const sparse_poly<OrderType, CoefficientType, CoefficientIsIntegral, Alloc> &rhs
    ){ return sparse_poly<OrderType, CoefficientType, CoefficientIsIntegral, Alloc>(lhs) - rhs; }

    template<class OrderType, class CoefficientType, bool CoefficientIsIntegral, class Alloc>
    sparse_poly<OrderType, CoefficientType, CoefficientIsIntegral, Alloc>
    operator *(
        const typename sparse_poly<OrderType, CoefficientType, CoefficientIsIntegral, Alloc>::coefficient_type &lhs,
        const sparse_poly<OrderType, CoefficientType, CoefficientIsIntegral, Alloc> &rhs
    ){ return sparse_poly<OrderType, CoefficientType, CoefficientIsIntegral, Alloc>(lhs) * rhs; }

    template<class OrderType, class CoefficientType, bool CoefficientIsIntegral, class Alloc>
    sparse_poly<OrderType, CoefficientType, CoefficientIsIntegral, Alloc>
    operator /(
        const typename sparse_poly<OrderType, CoefficientType, CoefficientIsIntegral, Alloc>::coefficient_type &lhs,
        const sparse_poly<OrderType, CoefficientType, CoefficientIsIntegral, Alloc> &rhs
    ){ return sparse_poly<OrderType, CoefficientType, CoefficientIsIntegral, Alloc>(lhs) / rhs; }

    template<class OrderType, class CoefficientType, bool CoefficientIsIntegral, class Alloc>
    sparse_poly<OrderType, CoefficientType, CoefficientIsIntegral, Alloc>
    operator %(
        const typename sparse_poly<OrderType, CoefficientType, CoefficientIsIntegral, Alloc>::coefficient_type &lhs,
        const sparse_poly<OrderType, CoefficientType, CoefficientIsIntegral, Alloc> &rhs
    ){ return sparse_poly<OrderType, CoefficientType, CoefficientIsIntegral, Alloc>(lhs) % rhs; }

    template<class OrderType, class CoefficientType, bool CoefficientIsIntegral, class Alloc>
    bool operator <(
        const typename sparse_poly<OrderType, CoefficientType, CoefficientIsIntegral, Alloc>::coefficient_type &lhs,
        const sparse_poly<OrderType, CoefficientType, CoefficientIsIntegral, Alloc> &rhs
    ){ return sparse_poly<OrderType, CoefficientType, CoefficientIsIntegral, Alloc>(lhs) < rhs; }

    template<class OrderType, class CoefficientType, bool CoefficientIsIntegral, class Alloc>
    bool operator >(
        const typename sparse_poly<OrderType, CoefficientType, CoefficientIsIntegral, Alloc>::coefficient_type &lhs,
        const sparse_poly<OrderType, CoefficientType, CoefficientIsIntegral, Alloc> &rhs
    ){ return sparse_poly<OrderType, CoefficientType, CoefficientIsIntegral, Alloc>(lhs) > rhs; }

    template<class OrderType, class CoefficientType, bool CoefficientIsIntegral, class Alloc>
    bool operator <=(
        const typename sparse_poly<OrderType, CoefficientType, CoefficientIsIntegral, Alloc>::coefficient_type &lhs,
        const sparse_poly<OrderType, CoefficientType, CoefficientIsIntegral, Alloc> &rhs
    ){ return sparse_poly<OrderType, CoefficientType, CoefficientIsIntegral, Alloc>(lhs) <= rhs; }

    template<class OrderType, class CoefficientType, bool CoefficientIsIntegral, class Alloc>
    bool operator >=(
        const typename sparse_poly<OrderType, CoefficientType, CoefficientIsIntegral, Alloc>::coefficient_type &lhs,
        const sparse_poly<OrderType, CoefficientType, CoefficientIsIntegral, Alloc> &rhs
    ){ return sparse_poly<OrderType, CoefficientType, CoefficientIsIntegral, Alloc>(lhs) >= rhs; }

    template<class OrderType, class CoefficientType, bool CoefficientIsIntegral, class Alloc>
    bool operator ==(
        const typename sparse_poly<OrderType, CoefficientType, CoefficientIsIntegral, Alloc>::coefficient_type &lhs,
        const sparse_poly<OrderType, CoefficientType, CoefficientIsIntegral, Alloc> &rhs
    ){ return sparse_poly<OrderType, CoefficientType, CoefficientIsIntegral, Alloc>(lhs) == rhs; }

    template<class OrderType, class CoefficientType, bool CoefficientIsIntegral, class Alloc>
    bool operator !=(
        const typename sparse_poly<OrderType, CoefficientType, CoefficientIsIntegral, Alloc>::coefficient_type &lhs,
        const sparse_poly<OrderType, CoefficientType, CoefficientIsIntegral, Alloc> &rhs
    ){ return sparse_poly<OrderType, CoefficientType, CoefficientIsIntegral, Alloc>(lhs) != rhs; }

    template<class OrderType, class CoefficientType, bool CoefficientIsIntegral, class Alloc>
    std::ostream &operator <<(
        std::ostream &ostream,
        const sparse_poly<OrderType, CoefficientType, CoefficientIsIntegral, Alloc> &value
    ){
        ostream << value.to_string();
        return ostream;
    }

    template<class OrderType, class CoefficientType, bool CoefficientIsIntegral, class Alloc>
    std::wostream &operator <<(
        std::wostream &ostream,
        const sparse_poly<OrderType, CoefficientType, CoefficientIsIntegral, Alloc> &value
    ){
        ostream << value.to_wstring();
        return ostream;
    }
}

#endif
