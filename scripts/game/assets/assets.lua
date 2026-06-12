DefineTexture.Cursor = "images/cursor.png"

-- A plain white square. Tinted via materials to draw bullets, walls and pillars.
-- 64x64 px with a couple of transparent edge pixels so the outline shader has room.
DefineTexture.Rectangle = "images/rectangle.png"

DefineTexture.PlayerIdle01 = "images/player/HJ_idle01.png"
DefineTexture.PlayerIdle02 = "images/player/HJ_idle02.png"
DefineTexture.PlayerIdle03 = "images/player/HJ_idle03.png"
DefineTexture.PlayerIdle04 = "images/player/HJ_idle04.png"

DefineTexture.PlayerRun01 = "images/player/HJ_run01.png"
DefineTexture.PlayerRun02 = "images/player/HJ_run02.png"
DefineTexture.PlayerRun03 = "images/player/HJ_run03.png"
DefineTexture.PlayerRun04 = "images/player/HJ_run04.png"
DefineTexture.PlayerRun05 = "images/player/HJ_run05.png"
DefineTexture.PlayerRun06 = "images/player/HJ_run06.png"
DefineTexture.PlayerRun07 = "images/player/HJ_run07.png"
DefineTexture.PlayerRun08 = "images/player/HJ_run08.png"
DefineTexture.PlayerRun09 = "images/player/HJ_run09.png"
DefineTexture.PlayerRun10 = "images/player/HJ_run010.png"

DefineShader.Psychedelic = "shaders/psychedelic"

DefineMaterial.Psychedelic = {
    shader = Shaders.Psychedelic,
}

DefineMaterial.WhiteOutline = {
    outline_color = Color.white(),
    outline_width = 2.0,
}

DefineMaterial.PlayerBullet = {
    tint = Color.yellow(),
    outline_color = Color.orange(),
    outline_width = 1.0,
}

DefineMaterial.EnemyBullet = {
    tint = Color.red(),
    outline_color = Color(0.4, 0.0, 0.0, 1.0),
    outline_width = 1.0,
}

-- Walls and pillars: slate blue-gray fill with a dark outline.
DefineMaterial.Wall = {
    tint = Color(0.45, 0.50, 0.62, 1.0),
    outline_color = Color(0.14, 0.17, 0.24, 1.0),
    outline_width = 2.0,
}

-- Enemies: white body with a red outline. The body briefly tints red when hit.
DefineMaterial.EnemyBody = {
    tint = Color.white(),
    outline_color = Color.red(),
    outline_width = 2.0,
}
