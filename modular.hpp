#ifndef HPP_CPP_MULTI_PRECISION_MODULAR
#define HPP_CPP_MULTI_PRECISION_MODULAR

#include <sstream>
#include "ns_aux.hpp"

namespace cpp_multi_precision{
    template<class ValueType>
    class modular{
    public:
        typedef ValueType value_type;

    public:
        modular() :
           value_(), modulus_(), modulus_1_5_()
        {}

       modular(const value_type &other_value) :
           value_(other_value), modulus_(), modulus_1_5_()
       {}

        modular(const value_type &other_value, const value_type &other_modulus) :
            value_(other_value), modulus_(other_modulus)
        {
            set_modulus_1_5();
            normalize();
        }

        modular(const modular &other) :
            value_(other.value_), modulus_(other.modulus_), modulus_1_5_(other.modulus_1_5_)
        { normalize(); }

        modular(modular &&other) :
            value_(std::move(other.value_)), modulus_(std::move(other.modulus_)), modulus_1_5_(std::move(other.modulus_1_5_))
        { normalize(); }

    public:
        std::string to_string() const{
            std::string r;
            r += to_string_dispatch(value_);
            r += " mod ";
            r += to_string_dispatch(modulus_);
            return r;
        }

        std::wstring to_wstring() const{
            std::wstring r;
            r += to_wstring_dispatch(value_);
            r += L" mod ";
            r += to_wstring_dispatch(modulus_);
            return r;
        }

        void normalize(){
            if(value_ >= modulus_1_5_){
                force_normalize();
            }
        }

        void force_normalize(){
            value_ = std::move(value_ % modulus_);
        }

    public:
        static modular &pow(modular &result, const modular &x, const modular &y){
            pow_dispatch(result, x, y);
            result.set_modulus(x.modulus_);
            result.normalize();
            return result;
        }

    public:
        modular &operator =(const modular &other){
            value_ = other.value_;
            modulus_ = other.modulus_;
            modulus_1_5_ = other.modulus_1_5_;
            return *this;
        }

        modular &operator =(modular &&other){
            value_ = std::move(other.value_);
            modulus_ = std::move(other.modulus_);
            modulus_1_5_ = std::move(other.modulus_1_5_);
            return *this;
        }

        modular &operator +=(const modular &rhs){
            value_ += rhs.value_;
            normalize();
            return *this;
        }

        modular operator +(const modular &rhs) const{
            modular r(*this);
            r += rhs;
            return std::move(r);
        }

        modular &operator +=(const value_type &rhs){
            value_ += rhs;
            normalize();
            return *this;
        }

        modular operator +(const value_type &rhs){
            modular r(*this);
            r += rhs;
            return std::move(r);
        }

        modular &operator -=(const modular &rhs){
            value_ -= rhs.value_;
            normalize();
            return *this;
        }

        modular operator -(const modular &rhs) const{
            modular r(*this);
            r -= rhs;
            return std::move(r);
        }

        modular &operator -=(const value_type &rhs){
            value_ -= rhs;
            normalize();
            return *this;
        }

        modular operator -(const value_type &rhs){
            modular r(*this);
            r -= rhs;
            return std::move(r);
        }

        modular &operator *=(const modular &rhs){
            value_ *= rhs.value_;
            normalize();
            return *this;
        }

        modular operator *(const modular &rhs) const{
            modular r(*this);
            r *= rhs;
            return std::move(r);
        }

        modular &operator *=(const value_type &rhs){
            value_ *= rhs;
            normalize();
            return *this;
        }

        modular operator *(const value_type &rhs){
            modular r(*this);
            r *= rhs;
            return std::move(r);
        }

        modular &operator /=(const modular &rhs){
            value_ /= rhs.value_;
            normalize();
            return *this;
        }

        modular operator /(const modular &rhs) const{
            modular r(*this);
            r /= rhs;
            return std::move(r);
        }

        modular &operator /=(const value_type &rhs){
            value_ /= rhs;
            normalize();
            return *this;
        }

        modular operator /(const value_type &rhs){
            modular r(*this);
            r /= rhs;
            return std::move(r);
        }

        modular &operator %=(const modular &rhs){
            value_ %= rhs.value_;
            normalize();
            return *this;
        }

        modular operator %(const modular &rhs) const{
            modular r(*this);
            r %= rhs;
            return std::move(r);
        }

        modular &operator %=(const value_type &rhs){
            value_ %= rhs;
            normalize();
            return *this;
        }

        modular operator %(const value_type &rhs){
            modular r(*this);
            r %= rhs;
            return std::move(r);
        }

        modular &operator <<=(const std::size_t n){
            value_ <<= n;
            normalize();
            return *this;
        }

        modular operator <<(const std::size_t n) const{
            modular r(*this);
            r <<= n;
            return std::move(r);
        }

        modular &operator >>=(const std::size_t n){
            value_ >>= n;
            normalize();
            return *this;
        }

        modular operator >>(const std::size_t n) const{
            modular r(*this);
            r >>= n;
            return std::move(r);
        }

        modular &operator +(){
            return *this;
        }

        modular operator -() const{
            modular r(*this);
            r.value_ = std::move(-r.value_);
            return std::move(r);
        }

        bool operator <(const modular &rhs) const{
            return value_ < rhs.value_;
        }

        bool operator <(const value_type &rhs) const{
            return value_ < rhs;
        }

        bool operator >(const modular &rhs) const{
            return value_ > rhs.value_;
        }

        bool operator >(const value_type &rhs) const{
            return value_ > rhs;
        }

        bool operator <=(const modular &rhs) const{
            return value_ <= rhs.value_;
        }

        bool operator <=(const value_type &rhs) const{
            return value_ <= rhs;
        }

        bool operator >=(const modular &rhs) const{
            return value_ >= rhs.value_;
        }

        bool operator >=(const value_type &rhs) const{
            return value_ >= rhs;
        }

        bool operator ==(const modular &rhs) const{
            return value_ == rhs.value_;
        }

        bool operator ==(const value_type &rhs) const{
            return value_ == rhs;
        }

        bool operator !=(const modular &rhs) const{
            return value_ != rhs.value_;
        }

        bool operator !=(const value_type &rhs) const{
            return value_ != rhs;
        }

    public:
        const value_type &value() const{
            return value_;
        }

        const value_type  &modulus() const{
            return modulus_;
        }

    private:
        void set_modulus(const value_type &value){
            modulus_ = value;;
        }

        void set_modulus_1_5(){
            modulus_1_5_ = modulus_;
            modulus_1_5_ += modulus_ / 2;
        }

    private:
#define CPP_MULTI_PRECISION_SIGUNATURE_TO_STRING template<class T, std::string (T::*Func)() const>
        CPP_MULTI_PRECISION_AUX_HAS_MEM_FN(to_string, CPP_MULTI_PRECISION_SIGUNATURE_TO_STRING);
        template<class T>
        static std::string to_string_dispatch(const T &value, typename boost::enable_if<has_to_string<T>>::type* = 0){
            return value.to_string();
        }

        template<class T>
        static std::string to_string_dispatch(const T &value, typename boost::disable_if<has_to_string<T>>::type* = 0){
            std::ostringstream os;
            os << value;
            return os.str();
        }

#define CPP_MULTI_PRECISION_SIGUNATURE_TO_WSTRING template<class T, std::wstring (T::*Func)() const>
        CPP_MULTI_PRECISION_AUX_HAS_MEM_FN(to_wstring, CPP_MULTI_PRECISION_SIGUNATURE_TO_WSTRING);
        template<class T>
        static std::wstring to_wstring_dispatch(const T &value, typename boost::enable_if<has_to_string<T>>::type* = 0){
            return value.to_wstring();
        }

        template<class T>
        static std::wstring to_wstring_dispatch(const T &value, typename boost::disable_if<has_to_string<T>>::type* = 0){
            std::wostringstream os;
            os << value;
            return os.str();
        }

#define CPP_MULTI_PRECISION_SIGNATURE_POW template<class T, T &(*Func)(T&, const T&, const T&)>
        CPP_MULTI_PRECISION_AUX_HAS_MEM_FN(pow, CPP_MULTI_PRECISION_SIGNATURE_POW);
        template<class T>
        static T &pow_dispatch(
            T &result,
            const T &x,
            const T &y,
            typename boost::enable_if<has_pow<typename T::value_type>>::type* = 0
        ){
            T::value_type::pow(result.value_, x.value_, y.value_);
            return result;
        }

        template<class T>
        static T &pow_dispatch(
            T &result,
            const T &x,
            const T &y,
            typename boost::disable_if<has_pow<typename T::value_type>>::type* = 0
        ){
            std::size_t k = sizeof(T) * 8;
            result.value_ = x.value_;
            for(std::size_t i = 0; i < k; ++i){
                std::size_t j = k - i - 1;
                if(((y.value_ >> j) & 1) == 1){
                    result.value_ = (result.value_ * result.value_ * x.value_) % x.modulus_;
                }else{
                    result.value_ = (result.value_ * result.value_) % x.modulus_;
                }
            }
            return result;
        }

        value_type value_, modulus_, modulus_1_5_;
    };

    template<class ValueType>
    modular<ValueType> operator +(const typename modular<ValueType>::value_type &lhs, const modular<ValueType> &rhs){
        modular<ValueType> r(lhs, rhs.modulus());
        r += rhs;
        return std::move(r);
    }

    template<class ValueType>
    modular<ValueType> operator -(const typename modular<ValueType>::value_type &lhs, const modular<ValueType> &rhs){
        modular<ValueType> r(lhs, rhs.modulus());
        r -= rhs;
        return std::move(r);
    }

    template<class ValueType>
    modular<ValueType> operator *(const typename modular<ValueType>::value_type &lhs, const modular<ValueType> &rhs){
        modular<ValueType> r(lhs, rhs.modulus());
        r *= rhs;
        return std::move(r);
    }

    template<class ValueType>
    modular<ValueType> operator /(const typename modular<ValueType>::value_type &lhs, const modular<ValueType> &rhs){
        modular<ValueType> r(lhs, rhs.modulus());
        r /= rhs;
        return std::move(r);
    }

    template<class ValueType>
    modular<ValueType> operator %(const typename modular<ValueType>::value_type &lhs, const modular<ValueType> &rhs){
        modular<ValueType> r(lhs, rhs.modulus());
        r %= rhs;
        return std::move(r);
    }

    template<class ValueType>
    bool operator <(const typename modular<ValueType>::value_type &lhs, const modular<ValueType> &rhs){
        return lhs < rhs.value();
    }

    template<class ValueType>
    bool operator >(const typename modular<ValueType>::value_type &lhs, const modular<ValueType> &rhs){
        return lhs > rhs.value();
    }

    template<class ValueType>
    bool operator <=(const typename modular<ValueType>::value_type &lhs, const modular<ValueType> &rhs){
        return lhs <= rhs.value();
    }

    template<class ValueType>
    bool operator >=(const typename modular<ValueType>::value_type &lhs, const modular<ValueType> &rhs){
        return lhs >= rhs.value();
    }

    template<class ValueType>
    bool operator ==(const typename modular<ValueType>::value_type &lhs, const modular<ValueType> &rhs){
        return lhs == rhs.value();
    }

    template<class ValueType>
    bool operator !=(const typename modular<ValueType>::value_type &lhs, const modular<ValueType> &rhs){
        return lhs != rhs.value();
    }
}

#endif
