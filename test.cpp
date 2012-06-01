#include <iostream>
#include <vector>
#include "cpp_multi_precision/modular.hpp"
#include "cpp_multi_precision/sparse_poly.hpp"
#include "cpp_multi_precision/rational.hpp"
#include "cpp_multi_precision/integer.hpp"

void test_simple_sparse_poly(){
    typedef cpp_multi_precision::sparse_poly<int, int> sparse_poly;

    std::cout << "start test_simple_sparse_poly\n";

    sparse_poly poly_a, poly_b, poly_c;

    // x^2 - x - 1
    poly_a [2](1) [1](-1) [0](-1);

    // x^5 + 2x^4 - x^3 + x^2 + 2x
    poly_b [5](1) [4](2) [3](-1) [2](1) [1](2);

    std::cout << poly_a << " * " << poly_b << " = "<< poly_a * poly_b << "\n";

    poly_c [10](-10) [5](5) [0](-20);
    std::cout << "infinity_norm(" << poly_c << ") = " << poly_c.infinity_norm() << "\n";

    sparse_poly a(2), f(a + a), g;
    bool ret_comp;

    g = 1 + a;
    g = 1 - a;
    g = 1 * a;
    g = 1 / a;
    ret_comp = 1 < a;
    ret_comp = 1 > a;
    ret_comp = 1 <= a;
    ret_comp = 1 >= a;
    ret_comp = 1 == a;
    ret_comp = 1 != a;

    g = a + 1;
    g = a - 1;
    g = a * 1;
    g = a / 1;
    ret_comp = a < 1;
    ret_comp = a > 1;
    ret_comp = a <= 1;
    ret_comp = a >= 1;
    ret_comp = a == 1;
    ret_comp = a != 1;

    g = f + a;
    g = f - a;
    g = f * a;
    g = f / a;
    ret_comp = f < a;
    ret_comp = f > a;
    ret_comp = f <= a;
    ret_comp = f >= a;
    ret_comp = f == a;
    ret_comp = f != a;

    std::cout << "end of test_simple_sparse_poly\n\n";
}

void test_sparse_poly(){
    // 次数となる整数型
    typedef cpp_multi_precision::integer<
        unsigned int,
        32,
        long long,
        unsigned long long,
        std::vector<unsigned int>
    > integer;

    // 係数となる有理数型
    typedef cpp_multi_precision::rational<
        unsigned int,
        32,
        long long,
        unsigned long long,
        std::vector<unsigned int>
    > rational;

    // 上記の二つをあわせて疎な多項式型を作る
    typedef cpp_multi_precision::sparse_poly<integer, rational> sparse_poly;
    typedef cpp_multi_precision::sparse_poly<integer, integer> z_field_sparse_poly;

    std::cout << "start test_sparse_poly\n";

    z_field_sparse_poly za, zb, zs, zt;

    // modulus == 7
    za [2](1) [1](2) [0](1);
    zb [2](1) [1](3) [0](2);
    std::cout << "modular gcd = " << z_field_sparse_poly::modular_eea(zs, zt, za, zb, 7) << "\n";
    std::cout << "bezout coefficient = " << zs << " : " << zt << "\n";
    std::cout << "modular gcd (linear combination) = " << (za * zs + zb * zt).mod_coefficient(7) << "\n";

    //za = 0, zb = 0;
    //za [100](5) [3](2);
    //zb [2](2);
    //std::cout << z_field_sparse_poly::primitive_gcd(za, zb) << "\n";

    za = 0, zb = 0;
    za [8](8161) [7](8167) [6](8171) [5](8179);
    zb [3](8171) [2](8179);
    std::cout << z_field_sparse_poly::primitive_gcd(za, zb) << "\n";

    sparse_poly poly_a, poly_b, poly_c, poly_r, poly_cl, poly_cr;
    // a += 18 * x^3
    poly_a[3] = 18;
    // a -= 42 * x^2
    poly_a[2] -= 42;
    // a += 30 * x
    poly_a[1] = 30;
    // a -= 6
    poly_a[0] -= 6;

    // b -= 12 * x^2
    poly_b[2] -= 12;
    // b += 10 * x
    poly_b[1] = 10;
    // b -= 2
    poly_b[0] -= 2;

    //poly_c = poly_a * poly_b;
    //sparse_poly::eea(poly_r, poly_cl, poly_cr, poly_a, poly_b);

    //// a, b の式は入力
    //// c = a * b
    //std::cout << "a  : " << poly_a << "\nb  : " << poly_b << "\nc  : " << poly_c << "\n";

    //// r は GCD
    //// cl, cr はそれぞれ、 r = a * cl + b * cr となる係数
    //std::cout << "r  : " << poly_r << "\ncl : " << poly_cl << "\ncr : " << poly_cr << "\n";

    //sparse_poly a(poly_a), f(a + a), g;
    //bool ret_comp;

    //g = 1 + a;
    //g = 1 - a;
    //g = 1 * a;
    //g = 1 / a;
    //ret_comp = 1 < a;
    //ret_comp = 1 > a;
    //ret_comp = 1 <= a;
    //ret_comp = 1 >= a;
    //ret_comp = 1 == a;
    //ret_comp = 1 != a;

    //g = a + 1;
    //g = a - 1;
    //g = a * 1;
    //g = a / 1;
    //ret_comp = a < 1;
    //ret_comp = a > 1;
    //ret_comp = a <= 1;
    //ret_comp = a >= 1;
    //ret_comp = a == 1;
    //ret_comp = a != 1;

    //g = f + a;
    //g = f - a;
    //g = f * a;
    //g = f / a;
    //ret_comp = f < a;
    //ret_comp = f > a;
    //ret_comp = f <= a;
    //ret_comp = f >= a;
    //ret_comp = f == a;
    //ret_comp = f != a;

    //std::cout << "end of test_sparse_poly\n\n";
}

void test_rational(){
    typedef cpp_multi_precision::rational<
        unsigned int,
        sizeof(unsigned int) * 8,
        long long,
        unsigned long long,
        std::vector<int>
    > rational;
    rational zero, a(2), b(a), c(std::move(zero)), d(10), e(rational::integer_type(29)), f(a + a), g;
    bool ret_comp;

    std::cout << "start test_rational\n";

    g = 1 + a;
    g = 1 - a;
    g = 1 * a;
    g = 1 / a;
    ret_comp = 1 < a;
    ret_comp = 1 > a;
    ret_comp = 1 <= a;
    ret_comp = 1 >= a;
    ret_comp = 1 == a;
    ret_comp = 1 != a;

    g = a + 1;
    g = a - 1;
    g = a * 1;
    g = a / 1;
    ret_comp = a < 1;
    ret_comp = a > 1;
    ret_comp = a <= 1;
    ret_comp = a >= 1;
    ret_comp = a == 1;
    ret_comp = a != 1;

    g = f + a;
    g = f - a;
    g = f * a;
    g = f / a;
    ret_comp = f < a;
    ret_comp = f > a;
    ret_comp = f <= a;
    ret_comp = f >= a;
    ret_comp = f == a;
    ret_comp = f != a;

    std::cout << "end of test_rational\n\n";
}

void test_integer(){
    typedef cpp_multi_precision::integer<
        unsigned int,
        sizeof(unsigned int) * 8,
        long long,
        unsigned long long,
        std::vector<int>
    > integer;
    integer a(2), f(a + a), g;
    bool ret_comp;

    std::cout << "start test_integer\n";

    g = 1 + a;
    g = 1 - a;
    g = 1 * a;
    g = 1 / a;
    ret_comp = 1 < a;
    ret_comp = 1 > a;
    ret_comp = 1 <= a;
    ret_comp = 1 >= a;
    ret_comp = 1 == a;
    ret_comp = 1 != a;

    g = a + 1;
    g = a - 1;
    g = a * 1;
    g = a / 1;
    ret_comp = a < 1;
    ret_comp = a > 1;
    ret_comp = a <= 1;
    ret_comp = a >= 1;
    ret_comp = a == 1;
    ret_comp = a != 1;

    g = f + a;
    g = f - a;
    g = f * a;
    g = f / a;
    ret_comp = f < a;
    ret_comp = f > a;
    ret_comp = f <= a;
    ret_comp = f >= a;
    ret_comp = f == a;
    ret_comp = f != a;

    std::cout << "end of test_integer\n\n";
}

void test_modular(){
    typedef cpp_multi_precision::integer<
        unsigned int,
        sizeof(unsigned int) * 8,
        long long,
        unsigned long long,
        std::vector<int>
    > integer;
    
    // typedef cpp_multi_precision::modular<int> modular; // こっちも使えます
    typedef cpp_multi_precision::modular<integer> modular;

    std::cout << "start test_modular\n";

    modular a(4, 5), f(a + a), g;
    bool ret_comp;

    f.force_normalize();
    std::cout << f << "\n";
    std::cout << -f << "\n";
    std::cout << f + -f << "\n";

    std::cout << modular::pow(g, a, f) << "\n";

    g = 1 + a;
    g = 1 - a;
    g = 1 * a;
    g = 1 / a;
    ret_comp = 1 == a;
    ret_comp = 1 != a;

    g = a + 1;
    g = a - 1;
    g = a * 1;
    g = a / 1;
    ret_comp = a == 1;
    ret_comp = a != 1;

    g = f + a;
    g = f - a;
    g = f * a;
    g = f / a;
    ret_comp = f == a;
    ret_comp = f != a;

    std::cout << "end of test_modular\n\n";
}

void test_modular_and_poly(){
    typedef cpp_multi_precision::integer<
        unsigned int,
        32,
        long long,
        unsigned long long,
        std::vector<unsigned int>
    > integer;

    typedef cpp_multi_precision::sparse_poly<integer, integer> sparse_poly;
    typedef cpp_multi_precision::modular<sparse_poly> modular;
    
    std::cout << "start test_modular_and_poly\n";

    sparse_poly a, b, c, modulus;

    c = 0;
    c [10](-10) [5](5) [0](-20);
    std::cout << "infinity_norm(" << c << ") = " << c.infinity_norm() << "\n";

    // modulus = x^2 - x - 1
    modulus [2](1) [1](-1) [0](-1);
    std::cout << "modulus = " << modulus << "\n";

    // a = x^3 ≡ 2x + 1 mod modulus
    a = 0;
    a [3](1);
    std::cout << a << " === " << modular(a, modulus) << "\n";

    // a = x^2 + 2x ≡ 3x + 1 mod modulus
    a = 0;
    a [2](1) [1](2);
    std::cout << a << " === " << modular(a, modulus) << "\n";

    // a = 6x^2 + 8x + 2 ≡ 14x + 8 mod modulus
    a = 0;
    a [2](6) [1](8) [0](2);
    std::cout << a << " === " << modular(a, modulus) << "\n";

    // a = x^5 + 2x^4 - x^3 + x^2 + 2x ≡ 12x + 7 mod modulus
    a = 0;
    a [5](1) [4](2) [3](-1) [2](1) [1](2);
    std::cout << a << " === " << modular(a, modulus) << "\n";

    std::cout << "end of test_modular_and_poly\n\n";
}

void test_prime_list(){
    std::cout << "start test_prime_list\n";

    typedef cpp_multi_precision::aux::prime_list<unsigned int> prime_list32_type;
    std::vector<unsigned int> r32 = prime_list32_type::get_prime_set(0x382b6b + 2, 10);
    for(auto iter = r32.begin(), end = r32.end(); iter != end; ++iter){
        std::cout << *iter << "\n";
    }
    std::cout << "\n";
    r32 = prime_list32_type::get_prime_set(0x382b6b + 9999, 5);
    for(auto iter = r32.begin(), end = r32.end(); iter != end; ++iter){
        std::cout << *iter << "\n";
    }
    std::cout << "\n";
    typedef cpp_multi_precision::aux::prime_list<unsigned long long> prime_list64_type;
    std::vector<unsigned long long> r64 = prime_list64_type::get_prime_set(0x10058b727 + 100, 10);
    for(auto iter = r64.begin(), end = r64.end(); iter != end; ++iter){
        std::cout << *iter << "\n";
    }

    std::cout << "end of test_prime_list\n\n";
}

int main(){
    //test_prime_list();
    //test_modular();
    //test_simple_sparse_poly();
    test_sparse_poly();
    //test_rational();
    //test_integer();
    //test_modular_and_poly();

    return 0;
}
