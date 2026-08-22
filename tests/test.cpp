//#include "../include/vermell/vermell.h"
#include <vermell/vermell.h>


int main() {

    Router router;
    router.setPort(8080);

    std::string hello = "hello world";

    router.get("/", {[&](Query& web) {

        web.send(hello, [&]{
            hello = "";
        });

    }});

    router.listen();
}


