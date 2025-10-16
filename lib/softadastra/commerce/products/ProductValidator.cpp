#include <softadastra/commerce/products/ProductValidator.hpp>
#include <stdexcept>
#include <iostream>

namespace softadastra::commerce::products
{
    void ProductValidator::validate(const nlohmann::json &item, const std::string &source)
    {
        auto require_string_nonempty = [&](const std::string &key)
        {
            if (!item.contains(key) || !item[key].is_string() || item[key].get<std::string>().empty())
                throw std::runtime_error("Produit invalide : clé '" + key + "' manquante/vide (string) dans : " + source);
        };

        auto require_number_min = [&](const std::string &key, double min_inclusive, bool integer_only = false)
        {
            if (!item.contains(key) || !(item[key].is_number()))
                throw std::runtime_error("Produit invalide : clé '" + key + "' manquante ou invalide (number) dans : " + source);
            double v = item[key].get<double>();
            if (integer_only && !item[key].is_number_integer())
                throw std::runtime_error("Produit invalide : clé '" + key + "' doit être un entier dans : " + source);
            if (v < min_inclusive)
                throw std::runtime_error("Produit invalide : '" + key + "' doit être >= " + std::to_string(min_inclusive) + " dans : " + source);
        };

        auto ensure_array_if_present = [&](const std::string &key)
        {
            if (item.contains(key) && !item[key].is_array())
                throw std::runtime_error("Produit invalide : '" + key + "' présent mais non-array dans : " + source);
        };

        auto ensure_string_if_present = [&](const std::string &key)
        {
            if (item.contains(key) && !item[key].is_string())
                throw std::runtime_error("Produit invalide : '" + key + "' présent mais non-string dans : " + source);
        };

        auto ensure_number_if_present = [&](const std::string &key)
        {
            if (item.contains(key) && !item[key].is_number())
                throw std::runtime_error("Produit invalide : '" + key + "' présent mais non-number dans : " + source);
        };

        auto ensure_bool_like = [&](const std::string &key)
        {
            if (!item.contains(key))
                return; // optionnel
            if (!(item[key].is_boolean() ||
                  (item[key].is_number_integer() && (item[key] == 0 || item[key] == 1))))
            {
                throw std::runtime_error("Produit invalide : '" + key + "' doit être bool/0/1 dans : " + source);
            }
        };

        // --- Contexte ------------------------------------------------------------
        const bool isCreate = (source == "createFromJson");
        const bool isUpdate = (source == "update");
        const bool isInternal = (source == "createFromInternalJson");

        // id : requis UNIQUEMENT hors create
        if (isUpdate || isInternal)
        {
            require_number_min("id", 1, /*integer_only=*/true);
        }

        // Champs MINIMAUX pour une création
        if (isCreate)
        {
            require_string_nonempty("title");
            require_string_nonempty("currency");
            require_number_min("category_id", 1, /*integer_only=*/true);
        }

        // Le reste : optionnel mais typé si présent
        ensure_string_if_present("image_url");
        ensure_string_if_present("city_name");
        ensure_string_if_present("country_image_url");
        ensure_string_if_present("formatted_price");
        ensure_string_if_present("converted_price");
        ensure_string_if_present("condition_name");
        ensure_string_if_present("brand_name");
        ensure_string_if_present("package_format_name");

        ensure_number_if_present("views");
        ensure_number_if_present("review_count");

        ensure_number_if_present("converted_price_value");
        ensure_number_if_present("price_with_shipping_value");

        // average_rating : number ou null si présent
        if (item.contains("average_rating") && !item["average_rating"].is_null() && !item["average_rating"].is_number())
        {
            throw std::runtime_error("Produit invalide : 'average_rating' présent mais invalide (number|null) dans : " + source);
        }

        ensure_array_if_present("sizes");
        ensure_array_if_present("colors");
        ensure_array_if_present("images");
        ensure_array_if_present("custom_fields");
        ensure_array_if_present("similar_products");

        ensure_bool_like("boost");
    }

    bool ProductValidator::isValid(const nlohmann::json &item)
    {
        try
        {
            validate(item, "isValid() check");
            return true;
        }
        catch (const std::exception &e)
        {
            std::cerr << "[ProductValidator] ⚠ Produit rejeté : " << e.what() << std::endl;
            return false;
        }
    }
}
