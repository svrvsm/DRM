# DRM
C# application to protect image files from getting screen captured by print screen, snipping tool.

This branch just depends on desktop window manager api to primarily prevent screen capture. If its disabled the application does not prevent screen capture. Branch add_no_dwm_support addresses this problem extensively by hooking snipping tool and incorporating low level keyboard and mouse hooks. Please refer to that branch for extensive functionality.
