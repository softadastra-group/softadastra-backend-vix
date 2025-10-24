#include <softadastra/commerce/products/ProductController.hpp>
#include <softadastra/commerce/products/ProductCache.hpp>
#include <softadastra/commerce/products/ProductService.hpp>
#include <softadastra/commerce/products/ProductRecommender.hpp>
#include <softadastra/commerce/products/ProductValidator.hpp>
#include <softadastra/commerce/products/ProductFactory.hpp>

#include <adastra/config/env/EnvLoader.hpp>
#include <adastra/utils/json/JsonUtils.hpp>

#include <cstdlib>
#include <memory>
#include <mutex>
#include <iostream>

#include <vix.hpp>
#include <algorithm>
#include <cctype>

#include <filesystem>
#include <unordered_set>

#ifndef SA_BACKEND_ROOT
#define SA_BACKEND_ROOT ""
#endif

using namespace adastra::utils::json;
using namespace Vix::json;

namespace softadastra::commerce::products
{
    static std::unique_ptr<ProductCache> g_productCache;
    static std::once_flag init_flag;
    [[maybe_unused]] static std::once_flag dotenv_flag;
    [[maybe_unused]] constexpr int DEFAULT_LIMIT = 10;
    [[maybe_unused]] constexpr int DEFAULT_OFFSET = 0;

    static Json product_to_json(const Product &p)
    {
        return p.toJson();
    }

    static inline std::string to_lower_copy(std::string s)
    {
        std::transform(s.begin(), s.end(), s.begin(),
                       [](unsigned char c)
                       { return std::tolower(c); });
        return s;
    }

    static inline bool looks_like_bool_key(std::string key)
    {
        key = to_lower_copy(std::move(key));
        return key.rfind("is_", 0) == 0 || key.rfind("has_", 0) == 0 || key.rfind("can_", 0) == 0 || key.find("enable") != std::string::npos || key.find("enabled") != std::string::npos || key.find("active") != std::string::npos || key.find("visible") != std::string::npos || key.find("featured") != std::string::npos || key.find("boost") != std::string::npos;
    }

    static inline void coerce_product_json(Vix::json::Json &obj)
    {
        using Json = Vix::json::Json;
        if (!obj.is_object())
            return;

        static const std::unordered_set<std::string> price_keys = {
            "price", "price_with_shipping", "shipping_cost_usd", "original_price"};

        for (auto &[key, value] : obj.items())
        {
            if (value.is_object())
            {
                coerce_product_json(value);
                continue;
            }
            if (value.is_array())
            {
                for (auto &element : value)
                    if (element.is_object())
                        coerce_product_json(element);
                continue;
            }

            if ((value.is_number_integer() || value.is_number_unsigned()) && looks_like_bool_key(key))
            {
                value = Json(value.get<int64_t>() != 0);
                continue;
            }

            if (key == "converted_price")
            {
                if (value.is_null())
                {
                    value = Json(std::string{});
                }
                else if (value.is_number())
                {
                    value = Json(std::to_string(value.get<double>()));
                }
                continue;
            }

            if (value.is_string() && price_keys.find(key) != price_keys.end())
            {
                try
                {
                    value = Json(std::stod(value.get<std::string>()));
                }
                catch (const std::exception &)
                {
                }
                continue;
            }

            if (key == "original_price" && value.is_null())
            {
                value = Json(std::string{});
                continue;
            }

            if (key == "average_rating" && value.is_null())
            {
                value = Json(0);
                continue;
            }
        }
    }

    static std::string resolveProductPath(std::string p)
    {
        std::filesystem::path pp(p);
        if (pp.is_relative())
            pp = std::filesystem::path(SA_BACKEND_ROOT) / pp;
        return pp.lexically_normal().string();
    }

    void ProductController(Vix::App &app)
    {
        static std::once_flag dotenv_flag;
        std::call_once(dotenv_flag, []
                       { adastra::config::env::EnvLoader::loadDotenv(std::string(SA_BACKEND_ROOT) + "/.env"); });

        std::string path = adastra::config::env::EnvLoader::get("PRODUCT_JSON_PATH", "");
        path = resolveProductPath(path);

        if (path.empty())
        {
            std::filesystem::path def = std::filesystem::path(SA_BACKEND_ROOT) / "config" / "data" / "products.json";
            path = def.string();
            std::cerr << "[ProductController] PRODUCT_JSON_PATH non défini, fallback: " << path << "\n";
        }

        {
            std::filesystem::path p(path);
            if (p.is_relative())
            {
                p = std::filesystem::path(SA_BACKEND_ROOT) / p;
            }
            path = p.lexically_normal().string();
        }

        if (!std::filesystem::exists(path))
        {
            throw std::runtime_error(std::string("PRODUCT_JSON_PATH introuvable: ") + path);
        }

        std::call_once(
            init_flag, [&]()
            {
        auto deserializer = [](const Vix::json::Json &json) -> std::vector<Product>
        {
            using Vix::json::Json;

            // --- 1) RACINE stringifiée : "{ \"data\": [...] }" en texte brut
            Json root = json;
            if (root.is_string()) {
                try {
                    root = Json::parse(root.get<std::string>());
                } catch (...) {
                    throw std::runtime_error("Top-level JSON is a string but cannot be parsed");
                }
            }

            // --- 2) data peut être un tableau ou ... une chaîne JSON
            auto parse_maybe_stringified = [](const Json& j) -> Json {
                if (j.is_string()) {
                    try { return Json::parse(j.get<std::string>()); }
                    catch (...) {}
                }
                return j;
            };

            // --- 3) coercitions (0/1 -> bool, prix string -> double) inchangé
            auto to_lower_copy = [](std::string s) {
                std::transform(s.begin(), s.end(), s.begin(),
                            [](unsigned char c){ return std::tolower(c); });
                return s;
            };
            auto looks_like_bool_key = [&](std::string key) {
                key = to_lower_copy(std::move(key));
                return key.rfind("is_",0)==0 || key.rfind("has_",0)==0 || key.rfind("can_",0)==0
                    || key.find("enable")!=std::string::npos || key.find("enabled")!=std::string::npos
                    || key.find("active")!=std::string::npos || key.find("visible")!=std::string::npos
                    || key.find("featured")!=std::string::npos || key.find("boost")!=std::string::npos;
            };
            std::function<void(Json&)> coerce = [&](Json& obj){
                if (!obj.is_object()) return;
                for (auto& [k, v] : obj.items()) {
                    if ((v.is_number_integer() || v.is_number_unsigned()) && looks_like_bool_key(k)) {
                        v = Json(v.get<int64_t>() != 0);
                        continue;
                    }
                    if (v.is_string()) {
                        if (k=="price" || k=="price_with_shipping" || k=="shipping_cost_usd"
                        || k=="original_price" || k=="converted_price") {
                            try { v = Json(std::stod(v.get<std::string>())); } catch (...) {}
                        }
                    }
                    if (v.is_object()) coerce(v);
                    if (v.is_array())  for (auto& e : v) if (e.is_object()) coerce(e);
                }
            };

            // --- 4) détecte le tableau de produits
            const Json* arrPtr = nullptr;

            if (root.is_object() && root.contains("data")) {
                Json dataNode = parse_maybe_stringified(root["data"]);
                if (dataNode.is_array()) { root = std::move(dataNode); arrPtr = &root; }
            }
            if (!arrPtr && root.is_array()) arrPtr = &root;

            if (!arrPtr) {
                throw std::runtime_error("Unsupported JSON schema: expected {data:[...]}, {data:\"[...]\"} or [...]");
            }

            // --- 5) construit les produits
            std::vector<Product> products;
            products.reserve(arrPtr->size());

            std::size_t ok = 0, bad = 0;
            for (auto item : *arrPtr) {
                try {
                    if (item.is_string()) item = Json::parse(item.get<std::string>());
                    if (item.is_object()) coerce(item);
                    products.push_back(ProductFactory::fromJsonOrThrow(item));
                    ++ok;
                } catch (const std::exception& e) {
                    ++bad;
                    std::cerr << "[ProductCache] Ignored product: " << e.what() << "\n";
                }
            }
            std::cerr << "[ProductCache] Loaded products ok=" << ok << " bad=" << bad << "\n";
            return products;
        };



            auto serializer = [](const std::vector<Product>& products) -> Json
            {
                Json arr = Json::array();
                for (const auto& p : products)
                    arr.push_back(p.toJson()); 

                return o("data", arr);
            };

            g_productCache = std::make_unique<ProductCache>(
                path,
                []() -> std::vector<Product> { return {}; },
                serializer,
                deserializer
            ); });

        app.post("/api/products/create", [](auto &req, auto &res)
                 {
            auto body = Json::parse(req.body());

            std::string title   = body.value("title", "");
            std::string content = body.value("content", "");
            double      price   = body.value("price", 0.0);

            res.status(http::status::created).json({
                "action", "created",
                "status", "created",
                "user", Vix::json::obj({
                    "title",   title,
                    "content", content,
                    "price",   price
                })
            }); });

        app.get("/api/products/status", [](auto &, auto &res)
                {
    try {
        const auto& items = g_productCache->getAll();
        res.json(Vix::json::o(
            "path",  adastra::config::env::EnvLoader::get("PRODUCT_JSON_PATH", ""),
            "count", items.size()
        ));
    } catch (const std::exception& e) {
        res.status(http::status::internal_server_error)
           .json(Vix::json::o("error", e.what()));
    } });

        app.get("/api/products/all", [](auto &, auto &res)
                {
            try {
                const auto& items = g_productCache->getAll();
                Json arr = Json::array();

                for(const auto& p: items)
                    arr.push_back(product_to_json(p));

                res.json(o(
                    "count", items.size(),
                    "data", arr
                ));
            } catch (const std::exception& e) {
                res.json(o("error", std::string("Invalid cache JSON: ") + e.what()));
            } });

        app.get("/api/products/first", [](auto &, auto &res)
                {
        const auto& items = g_productCache->getAll();
        if (items.empty()) {
            res.json(Vix::json::o("empty", true));
            return;
        }
        res.json(Vix::json::o("sample", product_to_json(items.front()))); });

        app.post("/api/products/reload", [](auto &, auto &res)
                 {
        try {
            g_productCache->reload(); // force loadFromFile()
            const auto& items = g_productCache->getAll();
            res.json(Vix::json::o("reloaded", true, "count", items.size()));
        } catch (const std::exception& e) {
            res.status(http::status::internal_server_error)
            .json(Vix::json::o("error", e.what()));
        } });

        app.get("/api/products/raw", [path](auto &, auto &res)
                {
    try {
        std::ifstream in(path);
        std::string s((std::istreambuf_iterator<char>(in)), {});
        std::string head = s.substr(0, 400);
        res.json(Vix::json::o("path", path, "head", head, "size", (long long)s.size()));
    } catch (const std::exception& e) {
        res.status(http::status::internal_server_error)
           .json(Vix::json::o("error", e.what(), "path", path));
    } });
    }

}
