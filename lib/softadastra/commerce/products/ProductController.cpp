#include <softadastra/commerce/products/ProductController.hpp>
#include <softadastra/commerce/products/ProductCache.hpp>
#include <softadastra/commerce/products/ProductService.hpp>
#include <softadastra/commerce/products/ProductRecommender.hpp>
#include <softadastra/commerce/products/ProductValidator.hpp>
#include <softadastra/commerce/products/ProductFactory.hpp>

#include <adastra/config/env/EnvLoader.hpp>
#include <adastra/utils/json/JsonUtils.hpp>
#include <adastra/database/Database.hpp>

#include <cstdlib>
#include <memory>
#include <mutex>
#include <iostream>
#include <nlohmann/json.hpp>

#include <vix.hpp>

#include <filesystem>

#ifndef SA_BACKEND_ROOT
#define SA_BACKEND_ROOT ""
#endif

using namespace adastra::utils::json;

namespace softadastra::commerce::products
{
    static std::unique_ptr<ProductCache> g_productCache;
    static std::once_flag init_flag;
    static std::once_flag dotenv_flag;
    [[maybe_unused]] constexpr int DEFAULT_LIMIT = 10;
    [[maybe_unused]] constexpr int DEFAULT_OFFSET = 0;

    void ProductController(Vix::App &app)
    {
        static std::once_flag dotenv_flag;
        std::call_once(dotenv_flag, []
                       { adastra::config::env::EnvLoader::loadDotenv(std::string(SA_BACKEND_ROOT) + "/.env"); });

        // Lire PRODUCT_JSON_PATH, sinon fallback
        std::string path = adastra::config::env::EnvLoader::get("PRODUCT_JSON_PATH", "");
        if (path.empty())
        {
            std::filesystem::path def = std::filesystem::path(SA_BACKEND_ROOT) / "config" / "data" / "products.json";
            path = def.string();
            std::cerr << "[ProductController] PRODUCT_JSON_PATH non défini, fallback: " << path << "\n";
        }

        // Résoudre les chemins RELATIFS par rapport à SA_BACKEND_ROOT
        {
            std::filesystem::path p(path);
            if (p.is_relative())
            {
                p = std::filesystem::path(SA_BACKEND_ROOT) / p;
            }
            path = p.lexically_normal().string();
        }

        // Logs
        std::cerr << "[ProductController] SA_BACKEND_ROOT=" << SA_BACKEND_ROOT << "\n";
        std::cerr << "[ProductController] Resolved PRODUCT_JSON_PATH=" << path << "\n";

        if (!std::filesystem::exists(path))
        {
            throw std::runtime_error(std::string("PRODUCT_JSON_PATH introuvable: ") + path);
        }

        std::call_once(
            init_flag, [&]()
            {
                auto deserializer = [](const nlohmann::json &json) -> std::vector<Product>
                {
                    if (!json.contains("data") || !json["data"].is_array())
                        throw std::runtime_error("key 'data' is missing");

                    std::vector<Product> products;
                    for (const auto &item : json["data"])
                    {
                        try {
                            products.push_back(ProductFactory::fromJsonOrThrow(item));
                        } catch (const std::exception &e) {
                            std::cerr << "Product ignoré: " << e.what() << std::endl;
                        }
                    }
                    return products;
                };

                auto serializer = [](const std::vector<Product> &products) -> nlohmann::json
                {
                    nlohmann::json j;
                    j["data"] = nlohmann::json::array();
                    for (const auto &p : products) {
                        j["data"].push_back(p.toJson());
                    }
                    return j;
                };

                g_productCache = std::make_unique<ProductCache>(
                    path,
                    []() -> std::vector<Product> { return {}; },
                    serializer,
                    deserializer
                ); });

        app.post("/api/products/create", [](auto &req, auto &res)
                 {
            auto body = nlohmann::json::parse(req.body());

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

        app.get("/api/products", [](auto &, auto &res)
                { res.json({"message", "Product 1"}); });
    }
}
