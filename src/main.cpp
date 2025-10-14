#include <vix.hpp>
using namespace Vix;

int main()
{
    App app;

    // GET /
    app.get("/", [](auto&, auto& res) {
        res.json({"message", "Hello world"});
    });

    app.run(8080);
}
