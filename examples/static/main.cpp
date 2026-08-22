#include <vermell/vermell.h>

int main() {

    Router router;
    router.setPort(8080);

    // ---- static directories (Node's express.static equivalent) ----
    //
    // Serve a Vue/React/Angular dist folder at the site root:
    //   - explicit routes always win over static mounts
    //   - .spa = true: any unknown path answers index.html, so the
    //     client-side router keeps working on refresh/deep links
    //   - cache is on by default (Cache-Control + ETag + 304); hashed
    //     build assets can be cached hard, index.html revalidates (max_age 0)
    router.staticX("/", "./dist", {
        .spa     = true,
        .max_age = std::chrono::days{30},   // Cache-Control: public, max-age=2592000
    });

    // A classic mount: only files under ./public/assets are served at
    // /assets. No SPA fallback here: a missing asset is a 404, never HTML.
    router.staticX("/assets", "./public/assets");

    // The API keeps working alongside the static files.
    router.get("/api/health", { [](Query &web) {
        web.json(R"({"ok":true})");
    }});

    router.listen();
}
