#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include"tcp_client.hpp"

#if defined(FORCE_STYLE)
#include <QQuickStyle>
#endif


int main(int argc, char *argv[]) {
    QGuiApplication app(argc, argv);

#if defined(FORCE_STYLE)
    QQuickStyle::setStyle(FORCE_STYLE);
#endif
    QQmlApplicationEngine engine;
    engine.addImageProvider(QLatin1String("wasm"), new WasmImageProvider());
    engine.addImportPath("qrc:/esterVtech.com/imports");
    engine.loadFromModule("SecurityCamera", "Client");

    return app.exec();
}
