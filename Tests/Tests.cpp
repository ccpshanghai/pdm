#include "gtest/gtest.h"
#include "Metrics.h"

TEST(MetricsCollection, CanGetMetrics)
{
	auto metrics = Metrics::RetrieveMetricsData();
	EXPECT_FALSE(metrics.items.empty());
}

TEST(MetricsCollection, GetEqualMetrics)
{
	auto metrics1 = Metrics::RetrieveMetricsData();
	auto metrics2 = Metrics::RetrieveMetricsData();
	EXPECT_EQ(metrics1, metrics2);
}

int main(int argc, char** argv)
{
	::testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}
