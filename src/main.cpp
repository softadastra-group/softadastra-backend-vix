#include <vix.hpp>
#include <softadastra/commerce/products/ProductController.hpp>
#include <vix/json/Simple.hpp>
#include <vix/utils/Validation.hpp>

#include <string>
#include <mutex>
#include <unordered_map>
#include <vector>

using namespace Vix;

struct User
{
    std::string id;
    std::string name;
    std::string email;
    int age{};
};

static std::mutex g_mtx;
static std::unordered_map<std::string, User> g_users;

static Vix::json::kvs to_json(const User &u)
{
    return Vix::json::obj({"id", u.id,
                           "name", u.name,
                           "email", u.email,
                           "age", u.age});
}

static std::string j_to_string(const nlohmann::json &j, const char *k)
{
    if (!j.contains(k))
    {
        return {};
    }
    const auto &v = j.at(k);
    if (v.is_string())
    {
        return v.get<std::string>();
    }
    if (v.is_number_integer())
    {
        return std::to_string(v.get<long long>());
    }
    if (v.is_number_unsigned())
    {
        return std::to_string(v.get<unsigned long long>());
    }
    if (v.is_number_float())
    {
        return std::to_string(v.get<double>());
    }
    if (v.is_boolean())
    {
        return v.get<bool>() ? "true" : "false";
    }
    return v.dump();
}

static bool parse_user(const nlohmann::json &j, User &out)
{
    try
    {
        out.name = j.value("name", std::string{});
        out.email = j.value("email", std::string{});

        if (j.contains("age"))
        {
            if (j["age"].is_string())
            {
                out.age = std::stoi(j["age"].get<std::string>());
            }
            else if (j["age"].is_number_integer())
            {
                out.age = static_cast<int>(j["age"].get<long long>());
            }
            else if (j["age"].is_number_unsigned())
            {
                out.age = static_cast<int>(j["age"].get<unsigned long long>());
            }
            else if (j["age"].is_number_float())
            {
                out.age = static_cast<int>(j["age"].get<double>());
            }
            else
            {
                out.age = 0;
            }
        }
        else
        {
            out.age = 0;
        }
        return true;
    }
    catch (...)
    {
        return false;
    }
}

int main()
{
    App app;

    app.get("/", [](auto &, auto &res)
            { res.json({"message", "Hello world"}); });

    app.post("/users", [](auto &req, auto &res)
             {
                 nlohmann::json body;
                 try
                 {
                     body = nlohmann::json::parse(req.body());
                 }
                 catch (...)
                 {
                     res.status(http::status::bad_request).json({"error", "Invalid JSON"});
                     return;
                 }

                 std::unordered_map<std::string, std::string> data{
                     {"name", j_to_string(body, "name")},
                     {"email", j_to_string(body, "email")},
                     {"age", j_to_string(body, "age")}};

                 Vix::utils::Schema schema{
                     {"name", Vix::utils::required("name")},
                     {"age", Vix::utils::num_range(1, 150, "Age")},
                     {"email", Vix::utils::match(R"(^[^@\s]+@[^@\s]+\.[^@\s]+$)", "Invalid email")}};

                 auto r = Vix::utils::validate_map(data, schema);
                 if (r.is_err())
                 {
                     std::vector<Vix::json::token> flat;
                     flat.reserve(r.error().size() * 2);

                     for (const auto &kv : r.error())
                     {
                         flat.emplace_back(kv.first);
                         flat.emplace_back(kv.second);
                     }

                     res.status(http::status::bad_request).json({"errors", Vix::json::obj(std::move(flat))});

                     return;
                 }

                 User u;
                 if(!parse_user(body, u)){
                    res.status(http::status::bad_request).json({"error", "Invalid fields"});
                    return;
                 }

                 u.id = std::to_string(std::hash<std::string>{}(u.email) & 0xFFFFFF);
                 {
                    std::lock_guard<std::mutex> lock(g_mtx);
                    g_users[u.id] = u;
                 }

                 res.status(http::status::created).json({
                    "status", "created",
                    "user", to_json(u)
                 }); });

    softadastra::commerce::products::ProductController(app);

    app.run(8080);
}
