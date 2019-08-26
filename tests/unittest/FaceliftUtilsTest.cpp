#include <gtest/gtest.h>
#include "FaceliftUtils.h"

namespace {

using namespace facelift;

class MostRecentlyUsedCacheTest : public ::testing::Test
{
public:
    const unsigned int cacheSize = 3;
    const int key1 = 1;
    const int val1 = 100;
    const int key2 = 2;
    const int val2 = 200;
    const int key3 = 3;
    const int val3 = 300;
    const int key4 = 4;
    const int val4 = 400;

    MostRecentlyUsedCache<int, int> cache;

    MostRecentlyUsedCacheTest() : cache(cacheSize) {}

    /**
     * @brief insert inserts pair key-value and verifies if added
     * @param key
     * @param val
     */
    void insert(int key, int val)
    {
        EXPECT_FALSE(cache.exists(key));
        cache.insert(key, val);
        ASSERT_TRUE(cache.exists(key));
        EXPECT_EQ(cache.get(key), val);
    }
};

TEST_F(MostRecentlyUsedCacheTest, instertExistsAndGet)
{
    insert(key1, val1);
    insert(key2, val2);
    insert(key3, val3);
    // key1 should be removed
    insert(key4, val4);

    EXPECT_FALSE(cache.exists(key1));
    EXPECT_TRUE(cache.exists(key2));
    EXPECT_TRUE(cache.exists(key3));
    EXPECT_TRUE(cache.exists(key4));
}

TEST_F(MostRecentlyUsedCacheTest, mostRecent)
{
    insert(key1, val1);
    insert(key2, val2);
    insert(key3, val3);
    // change key1 to be most recent
    EXPECT_EQ(cache.get(key1), val1);
    // key2 should be removed
    insert(key4, val4);

    EXPECT_TRUE(cache.exists(key1));
    EXPECT_FALSE(cache.exists(key2));
    EXPECT_TRUE(cache.exists(key3));
    EXPECT_TRUE(cache.exists(key4));
}

TEST_F(MostRecentlyUsedCacheTest, remove)
{
    insert(key1, val1);
    insert(key2, val2);
    insert(key3, val3);
    EXPECT_TRUE(cache.exists(key1));
    EXPECT_TRUE(cache.exists(key2));
    EXPECT_TRUE(cache.exists(key3));

    cache.remove(key2);

    EXPECT_TRUE(cache.exists(key1));
    EXPECT_FALSE(cache.exists(key2));
    EXPECT_TRUE(cache.exists(key3));
}

TEST_F(MostRecentlyUsedCacheTest, clear)
{
    insert(key1, val1);
    insert(key2, val2);
    insert(key3, val3);
    EXPECT_TRUE(cache.exists(key1));
    EXPECT_TRUE(cache.exists(key2));
    EXPECT_TRUE(cache.exists(key3));

    cache.clear();

    EXPECT_FALSE(cache.exists(key1));
    EXPECT_FALSE(cache.exists(key2));
    EXPECT_FALSE(cache.exists(key3));
}

} // end namespace
