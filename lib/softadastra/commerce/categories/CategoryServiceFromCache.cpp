#include <softadastra/commerce/categories/CategoryServiceFromCache.hpp>
#include <unordered_set>
#include <algorithm>

namespace softadastra::commerce::categories
{
    CategoryServiceFromCache::CategoryServiceFromCache(const std::vector<Category> &categories)
        : data_(categories) {}

    std::vector<Category> CategoryServiceFromCache::getAllCategories() const
    {
        return data_;
    }

    std::vector<Category> CategoryServiceFromCache::getTopLevelCategories() const
    {
        if (!isTopLevelCacheBuilt_)
        {
            topLevelCache_.clear();
            for (const auto &c : data_)
            {
                if (!c.getParentId().has_value())
                {
                    topLevelCache_.push_back(c);
                }
            }
            isTopLevelCacheBuilt_ = true;
        }
        return topLevelCache_;
    }

    std::vector<Category> CategoryServiceFromCache::getLeafCategories(std::size_t offset, std::size_t limit) const
    {
        if (!isLeafCacheBuilt_)
        {
            std::unordered_set<std::uint32_t> parentIds;
            for (const auto &c : data_)
            {
                if (c.getParentId().has_value())
                {
                    parentIds.insert(c.getParentId().value());
                }
            }

            leafCache_.clear();
            for (const auto &c : data_)
            {
                if (parentIds.count(c.getId()) == 0)
                {
                    leafCache_.push_back(c);
                }
            }

            isLeafCacheBuilt_ = true;
        }

        const std::size_t n = leafCache_.size();
        if (offset >= n)
            return {};

        const std::size_t start_u = offset;
        const std::size_t end_u = std::min(n, start_u + limit);

        using diff_t = std::vector<Category>::difference_type; // généralement std::ptrdiff_t
        const diff_t start = static_cast<diff_t>(start_u);
        const diff_t stop = static_cast<diff_t>(end_u);

        auto first = leafCache_.cbegin() + start;
        auto last = leafCache_.cbegin() + stop;

        return std::vector<Category>(first, last);
    }

    void CategoryServiceFromCache::reloadData(const std::vector<Category> &newData)
    {
        data_ = newData;

        // Invalider les caches
        topLevelCache_.clear();
        leafCache_.clear();
        isTopLevelCacheBuilt_ = false;
        isLeafCacheBuilt_ = false;
    }
}
