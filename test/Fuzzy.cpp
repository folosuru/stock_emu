#include <gtest/gtest.h>

#include <fstream>
#include <stock_emu_lib/util/Fuzzy/Fuzzy.hpp>

TEST(Util, Fuzzy) {
    using fuzzy = ToyFuzzy<double, 512>;

    fuzzy::FuzzySetVector_t setVec;
    fuzzy::HightVector_t height;
    fuzzy::DiscreteArr_t rate;
    fuzzy::DiscreteArr_t weight;
    for (int i = 0; i < fuzzy::Discrete_count; i++) {
        rate[i] = (-2.56) + i * 0.01;
    }

    setVec.emplace_back(-2, -1.5, -1, 1);
    height.push_back(0.5);
    setVec.emplace_back(1, 1.5, 2, 2);
    height.push_back(1);
    setVec.emplace_back(-1, 0, 2, 6);
    height.push_back(0.3);

    fuzzy::BindAndDiscretization(setVec, height, rate, weight);
    std::ofstream f("a.csv");
    for (int i = 0; i < 512; i++) {
        f << rate[i] << "," << weight[i] << ",\n";
    }

    ASSERT_EQ(fuzzy::getCenter(rate, weight), -1.5);
}
