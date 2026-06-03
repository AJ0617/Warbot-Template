// Warbot Sim — Viewer. Read-only field view that subscribes to the host's
// localhost table server. No robot code, no PROS shims, no raygui.
#include "field_renderer.hpp"
#include "net/table_client.hpp"
#include <raylib.h>

static const int TABLE_PORT = 5805;

int main() {
    const int WIN_W = 820;
    const int WIN_H = 860;
    InitWindow(WIN_W, WIN_H, "Warbot Sim — Viewer");
    SetTargetFPS(60);

    TableClient client;
    client.start("127.0.0.1", TABLE_PORT);

    FieldRenderer renderer(WIN_W, WIN_H);
    RenderState rs;

    while (!WindowShouldClose()) {
        bool haveData = client.latest(rs);
        renderer.drawFrame(rs, haveData && client.connected());
    }

    client.stop();
    CloseWindow();
    return 0;
}
