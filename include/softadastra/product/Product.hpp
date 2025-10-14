#ifndef SOFTADASTRA_PRODUCT_HPP
#define SOFTADASTRA_PRODUCT_HPP

#include <string>
#include <vector>
#include <cstdint>
#include <stdexcept>
#include <optional>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

namespace Softadastra::product
{
    class Product
    {
    public:
        Product() : id_(0), converted_price_(0), price_with_shipping_value_(0), category_id_(0), views_(0), review_count_(0), boost_(false) {}

        Product(const Product &rhs) = default;
        Product &operator=(const Product &rhs);

        Product(Product &&rhs) noexcept;
        Product &operator=(Product &&rhs) noexcept;
        virtual ~Product() = default;

        uint32_t getId() const { return id_; }
        const std::string &getTitle() const { return title_; }
        const std::string &getImageUrl() const { return image_url_; }
        const std::string &getCityName() const { return city_name_; }
        const std::string &getCountryImageUrl() const { return country_image_url_; }
        const std::string &getFormattedPrice() const { return formated_price_; }
        const std::string &getConvertedPrice() const { return converted_price_; }
        const std::vector<std::string> &getSizes() const { return sizes_; }
        const std::vector<std::string> &getColors() const { return colors_; }
        const std::string &getConditionName() const { return condition_name_; }
        const std::string &getBrandName() const { return brand_name_; }
        const std::string &getPackageFormatName() const { return package_format_name_; }
        const std::string &getCurrency() const { return currency_; }
        float getConvertedPriceValue() const { return converted_price_value__; }
        float getPriceWithShipping() const { return price_with_shipping_value_; }
        uint32_t getCategoryId() const { return category_id_; }
        uint32_t getViews() const { return views_; }
        std::optional<float> getAverageRating() const { return average_rating_; }
        std::optional<uint32_t> getBrandId() const { return brand_id_; }
        std::optional<std::string> getOriginalPrice() const { return original_price_; }
        uint32_t getReviewCount() const { return review_count_; }
        bool isBoosted() const { return boost_; }

        const std::vector<std::uint32_t> &getSimilarProducts() const { return similar_products_; }
        const std::vector<std::pair<std::string, std::string>> &getCustomFields() const { return custom_fields_; }
        const std::vector<std::string> &getImages() const { return images_; }

        void setId(uint32_t value) { id_ = value; }
        void setTitle(const std::string &value) { title_ = value; }
        void setImageUrl(const std::string &value) { image_url_ = value; }
        void setCityName(const std::string &value) { city_name_ = value; }
        void setCountryImageUrl(const std::string &value) { country_image_url_ = value; }
        void setFormattedPrice(const std::string &value) { formated_price_ = value; }
        void setConvertedPrice(const std::string &value) { converted_price_ = value; }
        void setConvertedPriceValue(float value) { converted_price_ = value; }
        void setPriceWithShipping(float value) { price_with_shipping_value_ = value; }
        void setSizes(const std::vector<std::string> &value) { sizes_ = value; }
        void setColors(const std::vector<std::string> &value) { colors_ = value; }
        void setConditionName(const std::string &value) { condition_name_ = value; }
        void setBrandName(const std::string &value) { brand_name_ = value; }
        void setPackageFormatName(const std::string &value) { package_format_name_ = value; }
        void setCurrency(const std::string &value) { currency_ = value; }
        void setCategoryId(uint32_t value) { category_id_ = value; }
        void setViews(uint32_t value) { views_ = value; }
        void setAverageRating(std::optional<float> value) { average_rating_ = value; }
        void setBrandId(std::optional<uint32_t> value) { brand_id_ = value; }
        void setOriginalPrice(std::optional<std::string> value) { original_price_ = value; }
        void setReviewCount(uint32_t value) { review_count_ = value; }
        void setBoost(bool value) { boost_ = value; }

        void setSimilarProducts(const std::vector<uint32_t> &value) { similar_products_ = value; }
        void setCustomFields(const std::vector<std::pair<std::string, std::string>> &value) { custom_fields_ = value; }
        void setImages(const std::vector<std::string> &value) { images_ = value; }

        nlohmann::json toJson() const
        {
            nlohmann::json j;
            j["id"] = getId();
            j["title"] = getTitle();
            j["image_url"] = getImageUrl();
            j["city_name"] = getCityName();
            j["country_image_url"] = getCountryImageUrl();
            j["currency"] = getCurrency();
            j["formatted_price"] = getFormattedPrice();
            j["converted_price"] = getConvertedPrice();

            if (getConvertedPriceValue() > 0.0f)
                j["converted_price_value"] = getConvertedPriceValue();
            if (getPriceWithShipping() > 0.0f)
                j["price_with_shipping_value"] = getPriceWithShipping();
            if (getOriginalPrice().has_value())
                j["original_price"] = getOriginalPrice().value();
            if (getBrandId().has_value())
                j["brand_id"] = getBrandId().value();
            if (getAverageRating().has_value())
                j["average_rating"] = getAverageRating().value();

            if (!getSizes().empty())
                j["sizes"] = getSizes();
            if (!getColors().empty())
                j["colors"] = getColors();
            if (!getConditionName().empty())
                j["condition_name"] = getConditionName();
            if (!getBrandName().empty())
                j["brand_name"] = getBrandName();
            if (!getPackageFormatName().empty())
                j["package_format_name"] = getFormattedPrice();
            if (getCategoryId() != 0)
                j["category_id"] = getCategoryId();
            if (getViews() > 0)
                j["views"] = getViews();
            if (getReviewCount() > 0)
                j["review_count"] = getReviewCount();

            j["boost"] = isBoosted();

            if (!getSimilarProducts().empty())
                j["similar_products"] = getSimilarProducts();

            if (!getImages().empty())
                j["images"] = getImages();

            if (!getCustomFields().empty())
            {
                j["custom_fields"] = nlohmann::json::array();
                for (const auto &field : getCustomFields())
                {
                    j["custom_fields"].push_back({{"name", field.first},
                                                  {"value", field.second}});
                }
            }

            return j;
        }

    private:
        std::uint32_t id_;
        std::string title_;
        std::string image_url_;
        std::string city_name_;
        std::string country_image_url_;
        std::string currency_;
        std::string formated_price_;
        std::string converted_price_;
        float converted_price_value__;
        float price_with_shipping_value_;
        std::optional<std::string> original_price_;
        std::optional<std::uint32_t> brand_id_;
        std::optional<float> average_rating_;
        std::vector<std::string> sizes_;
        std::vector<std::string> colors_;
        std::string condition_name_;
        std::string brand_name_;
        std::string package_format_name_;
        std::uint32_t category_id_;
        std::uint32_t views_;
        std::uint32_t review_count_;
        bool boost_;
        std::vector<std::uint32_t> similar_products_;
        std::vector<std::pair<std::string, std::string>> custom_fields_;
        std::vector<std::string> images_;
    };
}

#endif