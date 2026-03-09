#include <cassert>
#include <cstddef>
#include <iostream>
#include <limits>

namespace
{
    constexpr int kExpectedTypeSize = 4;

    constexpr unsigned int kShiftAmount = 1u;
    constexpr unsigned int kShiftThreshold = 1u;

    constexpr unsigned int kExponentMask = 0xFFu;
    constexpr unsigned int kMantissaMask = 0x7FFFFFu;
    constexpr unsigned int kExponentShift = 23u;

    constexpr int kExponentBias = 127;
    constexpr int kDenormalizedAdjust = 149;

    constexpr unsigned int kZeroExponent = 0u;
    constexpr unsigned int kMaxExponent = 255u;

    union FloatBits
    {
        float f_value;
        unsigned int u_value;
    };

    auto calc_log2_unsigned(unsigned int value) -> int
    {
        int result = 0;

        assert(value != 0u);

        while (value > kShiftThreshold)
        {
            value >>= kShiftAmount;
            ++result;
        }

        return result;
    }

    auto calc_log2_int(int value) -> int
    {
        unsigned int u_value = 0u;

        assert(sizeof(int) == kExpectedTypeSize);
        assert(sizeof(unsigned int) == kExpectedTypeSize);
        assert(value > 0);

        u_value = static_cast<unsigned int>(value);

        return calc_log2_unsigned(u_value);
    }

    auto calc_log2_float(float value) -> int
    {
        FloatBits converter = {};
        unsigned int u_value = 0u;
        unsigned int exponent = 0u;
        unsigned int mantissa = 0u;

        assert(sizeof(float) == kExpectedTypeSize);
        assert(sizeof(unsigned int) == kExpectedTypeSize);
        assert(value > 0.0f);

        converter.f_value = value;
        u_value = converter.u_value;

        exponent = (u_value >> kExponentShift) & kExponentMask;
        mantissa = u_value & kMantissaMask;

        assert(exponent < kMaxExponent);

        if (exponent == kZeroExponent)
        {
            assert(mantissa != 0u);
            return calc_log2_unsigned(mantissa) - kDenormalizedAdjust;
        }

        return static_cast<int>(exponent) - kExponentBias;
    }

    void run_all_tests()
    {
        FloatBits denorm_converter = {};
        int actual = 0;

        assert(sizeof(int) == kExpectedTypeSize);
        assert(sizeof(float) == kExpectedTypeSize);
        assert(sizeof(unsigned int) == kExpectedTypeSize);

        actual = calc_log2_int(1);
        assert(actual == 0);

        actual = calc_log2_int(2);
        assert(actual == 1);

        actual = calc_log2_int(3);
        assert(actual == 1);

        actual = calc_log2_int(4);
        assert(actual == 2);

        actual = calc_log2_int(1000);
        assert(actual == 9);

        actual = calc_log2_int(1024);
        assert(actual == 10);

        actual = calc_log2_int(std::numeric_limits<int>::max());
        assert(actual == 30);

        actual = calc_log2_float(1.0f);
        assert(actual == 0);

        actual = calc_log2_float(1.5f);
        assert(actual == 0);

        actual = calc_log2_float(2.0f);
        assert(actual == 1);

        actual = calc_log2_float(6.0f);
        assert(actual == 2);

        actual = calc_log2_float(0.5f);
        assert(actual == -1);

        actual = calc_log2_float(0.25f);
        assert(actual == -2);

        denorm_converter.u_value = 0x00000001u;
        actual = calc_log2_float(denorm_converter.f_value);
        assert(actual == -149);

        denorm_converter.u_value = 0x00000002u;
        actual = calc_log2_float(denorm_converter.f_value);
        assert(actual == -148);

        denorm_converter.u_value = 0x007FFFFFu;
        actual = calc_log2_float(denorm_converter.f_value);
        assert(actual == -127);

        actual = calc_log2_float(std::numeric_limits<float>::min());
        assert(actual == -126);
    }
}

int main()
{
    int int_value = 37;
    float float_value = 10.5f;
    FloatBits denorm_value = {};

    run_all_tests();

    denorm_value.u_value = 0x00000001u;

    std::cout << "floor(log2(" << int_value << ")) = "
              << calc_log2_int(int_value) << '\n';

    std::cout << "floor(log2(" << float_value << "f)) = "
              << calc_log2_float(float_value) << '\n';

    std::cout << "floor(log2(min positive denormal float)) = "
              << calc_log2_float(denorm_value.f_value) << '\n';

    std::cout << "floor(log2(min normal float)) = "
              << calc_log2_float(std::numeric_limits<float>::min()) << '\n';
}