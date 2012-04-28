#ifndef HPP_CPP_MULTI_PRECISION_MODULAR
#define HPP_CPP_MULTI_PRECISION_MODULAR

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
        void set_modulus_1_5(){
            modulus_1_5_ = modulus_;
            modulus_1_5_ += modulus_ / 2;
        }

        void force_normalize(){
            value_ = std::move(value_ % modulus_);
        }

        void normalize(){
            if(value_ >= modulus_1_5_){
                force_normalize();
            }
        }

    private:
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
