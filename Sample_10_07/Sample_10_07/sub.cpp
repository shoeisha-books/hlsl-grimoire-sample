#include "stdafx.h"
#include "sub.h"


void MoveCamera()
{
    g_camera3D->MoveForward(g_pad[0]->GetLStickYF() * 10.0f );
    g_camera3D->MoveRight(g_pad[0]->GetLStickXF() * 10.0f );
}