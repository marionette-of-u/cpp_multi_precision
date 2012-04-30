#ifndef HPP_CPP_MULTI_PRECISION_SPARSE_POLY
#define HPP_CPP_MULTI_PRECISION_SPARSE_POLY

#include <utility>
#include <string>
#include "ns_aux.hpp"

namespace cpp_multi_precision{
    template<class OrderType, class CoefficientType, class Alloc = std::allocator<int>>
    class sparse_poly{
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
                    bool sign = result.first->second.sign;
                    coe.sign = !sign;
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

        sparse_poly() : container(){}
        sparse_poly(const sparse_poly &other) : container(other.container){}
        sparse_poly(sparse_poly &&other) : container(other.container){}
        sparse_poly(const coefficient_type &coe) : container(){
            if(coe != 0){
                container.insert(typename container_type::value_type(order_type(0), coe));
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
            square_div(r, rem, *this, sparse_poly(rhs));
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

        static sparse_poly &square_multi(sparse_poly &result, const sparse_poly &lhs, const sparse_poly &rhs){
            square_multi_impl(result, lhs.container.begin(), lhs.container.end(), rhs.container.begin(), rhs.container.end());
            return result;
        }

    public:
        static sparse_poly &kar_multi(sparse_poly &result, const sparse_poly &lhs, const sparse_poly &rhs){
            return result = kar_multi_impl_n(lhs, rhs);
        }

        static sparse_poly &square_div(sparse_poly &result, sparse_poly &rem, const sparse_poly &lhs, const sparse_poly &rhs){
            rem.container = lhs.container;
            result.container.clear();
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
            result.container = x.container;
            const coefficient_type &lc_value(x.lc());
            for(typename container_type::iterator iter = result.container.begin(), end = result.container.end(); iter != end; ++iter){
                iter->second.assign(iter->second / lc_value);
            }
            return result;
        }

        void normalize(){
            if(container.size() == 0){ return; }
            coefficient_type lc_value(lc())/* , temp */;
            for(typename container_type::iterator iter = container.begin(), end = container.end(); iter != end; ++iter){
                iter->second.assign(iter->second / lc_value);
            }
        }

        void lu(){
            if(container.size() == 0){
                container.add(container_type::value_type(order_type(0), coefficient_type(1)));
                return;
            }
            sparse_poly a(*this), na;
            normal(na, a);
            monic_div(*this, a, na);
        }

        coefficient_type infinity_norm() const{
            const coefficient_type *r = 0;
            for(typename container_type::const_iterator iter = container.begin(), end = container.end(); iter != end; ++iter){
                const coefficient_type &iter_coe(iter->second);
                if(r == 0){ r = &iter_coe; }else{
                    if(iter_coe.sign){
                        if(r->sign){
                            if(iter_coe > *r){ r = &iter_coe; }
                        }else{
                            iter_coe.sign = false;
                            if(iter_coe < *r){ r = &iter_coe; }
                            iter_coe.sign = true;
                        }
                    }else{
                        if(!r->sign){
                            if(iter_coe < *r){ r = &iter_coe; }
                        }else{
                            iter_coe.sign = true;
                            if(iter_coe > *r){ r = &iter_coe; }
                            iter_coe.sign = false;
                        }
                    }
                }
            }
            return r == 0 ? coefficient_type(0) : *r;
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

        void negate(){
            for(typename container_type::iterator iter = container.begin(), end = container.end(); iter != end; ++iter){
                coefficient_type &coe(iter->second);
                coe.sign = !coe.sign;
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
                    temp_order.assign(rhs_order);
                    temp_order += lhs_order;
                    aux::multi_dispatch<coefficient_type>(temp_coefficient, lhs_coe, rhs_coe);
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
            order_type::ceil_pow2(n, order_f > order_g ? order_f : order_g);
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
                    if(iter != container.end()){ iter->second.sign = !iter->second.sign; }
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
            if(!rhs.is_monic()){
                return square_div(result, rem, lhs, rhs);
            }
            result.container.clear();
            if(rhs.container.rbegin()->first > lhs.container.rbegin()->first){
                rem.assign(lhs);
                return result;
            }
            order_type m(lhs.container.rbegin()->first);
            m -= rhs.container.rbegin()->first;
            m += 1;
            sparse_poly inv_rev_rhs;
            {
                sparse_poly rev_rhs;
                sparse_poly::rev(rev_rhs, rhs);
                sparse_poly::inverse(inv_rev_rhs, rev_rhs, m);
            }
            {
                sparse_poly rev_lhs;
                sparse_poly::rev(rev_lhs, lhs);
                sparse_poly::kar_multi(result, rev_lhs, inv_rev_rhs);
            }
            if(result.container.rbegin()->first >= m){
                result.container.erase(result.container.find(m), result.container.end());
            }
            result.rev();
            if(Rem){
                rem.assign(lhs);
                sparse_poly multi_result;
                kar_multi(multi_result, rhs, result);
                rem -= multi_result;
            }
            return result;
        }

        static void rev(sparse_poly &result, const sparse_poly &a){
            const order_type order(a.container.rbegin()->first);
            for(typename container_type::const_reverse_iterator iter = a.container.rbegin(), end = a.container.rend(); iter != end; ++iter){
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

        static sparse_poly &gcd_default_impl(sparse_poly &result, sparse_poly lhs, sparse_poly rhs){
            if(rhs.container.size() == 0){ result = 0; return result; }
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

        static sparse_poly &eea_default_impl(sparse_poly &result, sparse_poly &c_lhs, sparse_poly &c_rhs, const sparse_poly &lhs, const sparse_poly &rhs){
            if(rhs.container.empty()){
                result.container.clear();
                c_lhs.container.clear();
                c_rhs.container.clear();
                return result;
            }
            sparse_poly r_0(rhs), r_1(lhs), s_0, s_1, t_0, t_1;
            s_0[0] = 1;
            t_1[0] = 1;
            while(!r_1.container.empty()){
                sparse_poly q;
                monic_div(q, r_0, r_1);
                {
                    sparse_poly next_r(r_0);
                    sparse_poly result_multi;
                    next_r -= kar_multi(result_multi, q, r_1);
                    r_0 = std::move(r_1);
                    r_1 = std::move(next_r);
                }
                {
                    sparse_poly next_s(s_0);
                    sparse_poly result_multi;
                    next_s -= kar_multi(result_multi, q, s_1);
                    s_0 = std::move(s_1);
                    s_1 = std::move(next_s);
                }
                {
                    sparse_poly next_t(t_0);
                    sparse_poly result_multi;
                    next_t -= kar_multi(result_multi, q, t_1);
                    t_0 = std::move(t_1);
                    t_1 = std::move(next_t);
                }
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
            std::size_t r = l.ceil_log2();
            order_type rem(1);
            for(std::size_t i = 0; i < r; ++i){
                rem <<= 1;
                sparse_poly next_g(g);
                for(typename container_type::iterator double_g_iter = next_g.container.begin(), double_g_end = next_g.container.end(); double_g_iter != double_g_end; ++double_g_iter){
                    coefficient_type coe_double_g(double_g_iter->second);
                    coefficient_type::multi(double_g_iter->second, coe_double_g, 2);
                }
                {
                    sparse_poly f_g_square;
                    {
                        sparse_poly g_square;
                        kar_multi(g_square, g, g);
                        kar_multi(f_g_square, f, g_square);
                    }
                    next_g -= f_g_square;
                }
                if(next_g.container.rbegin()->first >= rem){
                    next_g.container.erase(next_g.container.find(rem), next_g.container.end());
                }
                g = std::move(next_g);
            }
            return result;
        }

        coefficient_type &cout(coefficient_type &result) const{
            if(container.size() == 0){ result.assign(0); return result; }
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
                coefficient_type new_coe;
                aux::multi_dispatch<coefficient_type>(new_coe, coe, q);
                typename container_type::iterator iter = container.find(new_order);
                if(iter == container.end()){
                    iter = container.add(typename container_type::ref_value_type(new_order, new_coe));
                    if(iter != container.end()){ iter->second.sign = !new_coe.sign; }
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
            if(container.size() == 0){ result += zero; return std::move(result); }
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

    template<class OrderType, class CoefficientType, class Alloc>
    sparse_poly<OrderType, CoefficientType, Alloc>
    operator +(
        const typename sparse_poly<OrderType, CoefficientType, Alloc>::coefficient_type &lhs,
        const sparse_poly<OrderType, CoefficientType, Alloc> &rhs
    ){ return sparse_poly<OrderType, CoefficientType, Alloc>(lhs) + rhs; }

    template<class OrderType, class CoefficientType, class Alloc>
    sparse_poly<OrderType, CoefficientType, Alloc>
    operator -(
        const typename sparse_poly<OrderType, CoefficientType, Alloc>::coefficient_type &lhs,
        const sparse_poly<OrderType, CoefficientType, Alloc> &rhs
    ){ return sparse_poly<OrderType, CoefficientType, Alloc>(lhs) - rhs; }

    template<class OrderType, class CoefficientType, class Alloc>
    sparse_poly<OrderType, CoefficientType, Alloc>
    operator *(
        const typename sparse_poly<OrderType, CoefficientType, Alloc>::coefficient_type &lhs,
        const sparse_poly<OrderType, CoefficientType, Alloc> &rhs
    ){ return sparse_poly<OrderType, CoefficientType, Alloc>(lhs) * rhs; }

    template<class OrderType, class CoefficientType, class Alloc>
    sparse_poly<OrderType, CoefficientType, Alloc>
    operator /(
        const typename sparse_poly<OrderType, CoefficientType, Alloc>::coefficient_type &lhs,
        const sparse_poly<OrderType, CoefficientType, Alloc> &rhs
    ){ return sparse_poly<OrderType, CoefficientType, Alloc>(lhs) / rhs; }

    template<class OrderType, class CoefficientType, class Alloc>
    sparse_poly<OrderType, CoefficientType, Alloc>
    operator %(
        const typename sparse_poly<OrderType, CoefficientType, Alloc>::coefficient_type &lhs,
        const sparse_poly<OrderType, CoefficientType, Alloc> &rhs
    ){ return sparse_poly<OrderType, CoefficientType, Alloc>(lhs) % rhs; }

    template<class OrderType, class CoefficientType, class Alloc>
    bool operator <(
        const typename sparse_poly<OrderType, CoefficientType, Alloc>::coefficient_type &lhs,
        const sparse_poly<OrderType, CoefficientType, Alloc> &rhs
    ){ return sparse_poly<OrderType, CoefficientType, Alloc>(lhs) < rhs; }

    template<class OrderType, class CoefficientType, class Alloc>
    bool operator >(
        const typename sparse_poly<OrderType, CoefficientType, Alloc>::coefficient_type &lhs,
        const sparse_poly<OrderType, CoefficientType, Alloc> &rhs
    ){ return sparse_poly<OrderType, CoefficientType, Alloc>(lhs) > rhs; }

    template<class OrderType, class CoefficientType, class Alloc>
    bool operator <=(
        const typename sparse_poly<OrderType, CoefficientType, Alloc>::coefficient_type &lhs,
        const sparse_poly<OrderType, CoefficientType, Alloc> &rhs
    ){ return sparse_poly<OrderType, CoefficientType, Alloc>(lhs) <= rhs; }

    template<class OrderType, class CoefficientType, class Alloc>
    bool operator >=(
        const typename sparse_poly<OrderType, CoefficientType, Alloc>::coefficient_type &lhs,
        const sparse_poly<OrderType, CoefficientType, Alloc> &rhs
    ){ return sparse_poly<OrderType, CoefficientType, Alloc>(lhs) >= rhs; }

    template<class OrderType, class CoefficientType, class Alloc>
    bool operator ==(
        const typename sparse_poly<OrderType, CoefficientType, Alloc>::coefficient_type &lhs,
        const sparse_poly<OrderType, CoefficientType, Alloc> &rhs
    ){ return sparse_poly<OrderType, CoefficientType, Alloc>(lhs) == rhs; }

    template<class OrderType, class CoefficientType, class Alloc>
    bool operator !=(
        const typename sparse_poly<OrderType, CoefficientType, Alloc>::coefficient_type &lhs,
        const sparse_poly<OrderType, CoefficientType, Alloc> &rhs
    ){ return sparse_poly<OrderType, CoefficientType, Alloc>(lhs) != rhs; }

    template<class OrderType, class CoefficientType, class Alloc>
    std::ostream &operator <<(
        std::ostream &ostream,
        const sparse_poly<OrderType, CoefficientType, Alloc> &value
    ){
        ostream << value.to_string();
        return ostream;
    }

    template<class OrderType, class CoefficientType, class Alloc>
    std::wostream &operator <<(
        std::wostream &ostream,
        const sparse_poly<OrderType, CoefficientType, Alloc> &value
    ){
        ostream << value.to_wstring();
        return ostream;
    }
}

#endif
