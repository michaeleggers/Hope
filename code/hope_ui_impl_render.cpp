#include "hope_ui.h"
#include "hope_draw.h"
#include "ref.h"

void createRenderCmdFromHopeUIDrawList(HopeUIDrawList * in_drawList,
                                       RenderCommand * out_renderCmd)

{
    out_renderCmd->type = RENDER_CMD_FILLED_RECT;
    out_renderCmd->tint = {0,0,1};
    // TODO(Michael): we need to separate the creation of rendercommands and
    // the global draw list, so we can just make a new command independently...
}