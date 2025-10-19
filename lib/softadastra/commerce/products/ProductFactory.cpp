#include <softadastra/commerce/products/ProductFactory.hpp>
#include <softadastra/commerce/products/ProductBuilder.hpp>
#include <softadastra/commerce/products/ProductValidator.hpp>
#include <iostream>
#include <sstream>
#include <algorithm>

#include <limits>

namespace
{
    inline uint32_t json_u32(const nlohmann::json &j, const char *key, uint32_t def = 0)
    {
        try
        {
            if (!j.contains(key))
                return def;

            const auto &v = j.at(key);
            if (v.is_number_unsigned())
            {
                auto u = v.get<unsigned long long>();
                if (u > std::numeric_limits<uint32_t>::max())
                    return std::numeric_limits<uint32_t>::max();
                return static_cast<uint32_t>(u);
            }
            if (v.is_number_integer())
            {
                long long s = v.get<long long>();
                if (s <= 0)
                    return 0u;
                if (s > static_cast<long long>(std::numeric_limits<uint32_t>::max()))
                    return std::numeric_limits<uint32_t>::max();
                return static_cast<uint32_t>(s);
            }
            if (v.is_string())
            {
                // accepte "123" -> 123
                const auto s = v.get<std::string>();
                std::size_t pos = 0;
                unsigned long long u = std::stoull(s, &pos);
                if (pos != s.size())
                    return def; // contient des chars non-numériques
                if (u > std::numeric_limits<uint32_t>::max())
                    return std::numeric_limits<uint32_t>::max();
                return static_cast<uint32_t>(u);
            }
        }
        catch (...)
        {
            // tombe sur def
        }
        return def;
    }
}

namespace softadastra::commerce::products
{

    static std::vector<std::string> safeArray(const nlohmann::json &j, const std::string &key)
    {
        if (j.contains(key) && j[key].is_array())
        {
            return j.at(key).get<std::vector<std::string>>();
        }
        return {};
    }

    static std::vector<std::pair<std::string, std::string>> parseCustomFieldsFlexible(const nlohmann::json &customFields)
    {
        std::vector<std::pair<std::string, std::string>> result;

        for (const auto &item : customFields)
        {
            if (item.is_array() && item.size() == 2 && item[0].is_string() && item[1].is_string())
            {
                result.emplace_back(item[0], item[1]); // format API
            }
            else if (item.is_object() && item.contains("name") && item.contains("value") &&
                     item["name"].is_string() && item["value"].is_string())
            {
                result.emplace_back(item["name"], item["value"]); // format interne
            }
        }

        return result;
    }

    static inline std::string trim_copy(const std::string &s)
    {
        auto start = s.find_first_not_of(" \t\n\r");
        if (start == std::string::npos)
            return "";
        auto end = s.find_last_not_of(" \t\n\r");
        return s.substr(start, end - start + 1);
    }

    static std::vector<std::string> normalizeCsvArray(std::vector<std::string> in)
    {
        std::vector<std::string> out;
        out.reserve(in.size());
        for (auto &s : in)
        {
            if (s.find(',') == std::string::npos)
            {
                auto t = trim_copy(s);
                if (!t.empty())
                    out.push_back(t);
                continue;
            }
            std::stringstream ss(s);
            std::string token;
            while (std::getline(ss, token, ','))
            {
                auto t = trim_copy(token);
                if (!t.empty())
                    out.push_back(t);
            }
        }
        // déduplique éventuellement (optionnel)
        std::sort(out.begin(), out.end());
        out.erase(std::unique(out.begin(), out.end()), out.end());
        return out;
    }

    std::unique_ptr<Product> ProductFactory::createFromJson(const nlohmann::json &data)
    {
        try
        {
            // ✅ Validation en contexte "create" (pas d'id requis)
            ProductValidator::validate(data, "createFromJson");

            // 1) Normaliser sizes/colors (split CSV + trim + dédup)
            auto sizes = normalizeCsvArray(safeArray(data, "sizes"));
            auto colors = normalizeCsvArray(safeArray(data, "colors"));

            if (!sizes.empty())
                sizes.erase(sizes.begin());
            if (!colors.empty())
                colors.erase(colors.begin());

            ProductBuilder builder;
            builder.setTitle(data.value("title", ""))
                .setImageUrl(data.value("image_url", ""))
                .setCityName(data.value("city_name", ""))
                .setCountryImageUrl(data.value("country_image_url", ""))
                .setCurrency(data.value("currency", ""))
                .setFormattedPrice(data.value("formatted_price", ""))
                .setConvertedPrice(data.value("converted_price", ""))
                .setPriceWithShipping(data.value("price_with_shipping_value", 0.0f))
                .setSizes(sizes)
                .setColors(colors)
                .setConditionName(data.value("condition_name", ""))
                .setBrandName(data.value("brand_name", ""))
                .setPackageFormatName(data.value("package_format_name", ""))
                .setCategoryId(json_u32(data, "category_id"))
                .setViews(json_u32(data, "views"))
                .setReviewCount(json_u32(data, "review_count"))
                .setBoost(data.value("boost", false))
                .setConvertedPriceValue(data.value("converted_price_value", 0.0f))
                .setOriginalPrice(data.value("original_price", ""))
                .setAverageRating(data.value("average_rating", 0.0f));

            if (data.contains("similar_products") && data["similar_products"].is_array())
            {
                builder.setSimilarProducts(data["similar_products"].get<std::vector<uint32_t>>());
            }

            if (data.contains("custom_fields") && data["custom_fields"].is_array())
            {
                builder.setCustomFields(parseCustomFieldsFlexible(data["custom_fields"]));
            }

            if (data.contains("images") && data["images"].is_array())
            {
                builder.setImages(data["images"].get<std::vector<std::string>>());
            }

            return std::make_unique<Product>(builder.build());
        }
        catch (const std::exception &ex)
        {
            std::string msg = std::string("ProductFactory::createFromJson → ") + ex.what();
            std::cerr << msg << "\nJSON:\n"
                      << data.dump(2) << std::endl;
            throw std::runtime_error(msg);
        }
    }

    std::unique_ptr<Product> ProductFactory::createFromInternalJson(const nlohmann::json &data)
    {
        try
        {
            ProductBuilder builder;
            builder.setId(json_u32(data, "id"))
                .setTitle(data.value("title", ""))
                .setImageUrl(data.value("image_url", ""))
                .setCityName(data.value("city_name", ""))
                .setCountryImageUrl(data.value("country_image_url", ""))
                .setCurrency(data.value("currency", ""))
                .setFormattedPrice(data.value("formatted_price", ""))
                .setConvertedPrice(data.value("converted_price", ""))
                .setPriceWithShipping(data.value("price_with_shipping_value", 0.0f))
                .setSizes(safeArray(data, "sizes"))
                .setColors(safeArray(data, "colors"))
                .setConditionName(data.value("condition_name", ""))
                .setBrandName(data.value("brand_name", ""))
                .setPackageFormatName(data.value("package_format_name", ""))
                .setCategoryId(json_u32(data, "category_id"))
                .setViews(json_u32(data, "views"))
                .setReviewCount(json_u32(data, "review_count"))
                .setBoost(data.value("boost", false))
                .setConvertedPriceValue(data.value("converted_price_value", 0.0f))
                .setOriginalPrice(data.value("original_price", ""))
                .setAverageRating(data.value("average_rating", 0.0f));

            if (data.contains("similar_products") && data["similar_products"].is_array())
            {
                builder.setSimilarProducts(data["similar_products"].get<std::vector<uint32_t>>());
            }

            if (data.contains("custom_fields") && data["custom_fields"].is_array())
            {
                builder.setCustomFields(parseCustomFieldsFlexible(data["custom_fields"]));
            }

            if (data.contains("images") && data["images"].is_array())
            {
                builder.setImages(data["images"].get<std::vector<std::string>>());
            }

            return std::make_unique<Product>(builder.build());
        }
        catch (const std::exception &ex)
        {
            std::cerr << "Erreur dans ProductFactory::createFromInternalJson: " << ex.what() << std::endl;
            std::cerr << "🔎 JSON en erreur : " << data.dump(2) << std::endl;
            return nullptr;
        }
    }

    Product ProductFactory::fromJsonOrThrow(const nlohmann::json &data)
    {
        auto ptr = createFromJson(data);
        if (!ptr)
            throw std::runtime_error("ProductFactory::fromJsonOrThrow() → JSON invalide");
        return *ptr;
    }
}
