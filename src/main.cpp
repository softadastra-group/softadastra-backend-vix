#include <vix.hpp>
#include <vector>
#include <softadastra/commerce/products/ProductController.hpp>
using namespace Vix;

int main()
{
    App app;

    // GET /
    app.get("/", [](auto &, auto &res)
            { res.json({"message", "Hello world"}); });

    softadastra::commerce::products::ProductController(app);

    app.run(8080);
}
