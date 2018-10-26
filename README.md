# Digital Rights Management Application
C# application to protect image files from getting screen captured by print scrn, windows snipping tool. 

The application uses windows desktop manager(aka DWM) api's to prevent screen capture. In cases (ex - on vista) where DWM is disabled the application depend on windows keyboard hook to prevent print screen. For preventing snipping tool action, the application depend on WM_GETMESSAGE (intercepts all the messages for snipping tool) and WH_MOUSE_LL (low level mouse hook) to intercept the action, blanks the image and when the action completes reloads the image.

#Technical

- Show cases - Native dll function invocation, use of image control, hooking M_GETMESSAGE< WM_KEYBOARD_LL, WM_MOUSE_LL techniques, WPF mainwindow window procedure to process custom windows message,  message passing between native dll and managed code, reverse engineering of snipping tool functionality.

- WPF provide the main window functionality. It also hosts a image control. User is also provided with file open funcitonality to open a image file for viewing. During initialization the window is marked to be protected using SetWindowDisplayAFfinity api. 

- In the cases where DWM api fails to protect, the application uses hook mechanism. Keyboard hooks monitor the key press event and on detecting print scrn combination it cancels out. The native dll also intercepts snipping tool WM_GETMESSAGE. This is done so that native dll can know when a user clicks a "New" menu item in the snipping tool. This is how a user does a screen capture. Since interception is done on a 3rd party app the hooking procedures need to be in native dll and exported as functions. Once a "New" screen capture workflow is detected by the native WM_GETMESSAGE hook procedure, it sends a custom windows message WM_SNIP_ALERT to WPF MainWIndow. Mainwindow on receiving the message unloads the image and sets it to null. Even if the user attempts to capture image, the image will be blank. Any further mouse click or left button would complete the "New" workflow; this event is again tracked by Mouse hook, sends a message to WPF MainWIndow, and the WPF main window can  load the image again.

#Assumptionsperformed.
- Under no DWM case, the applciation expects snipping tool to be running prior to this paplication running.
- The testing was performed on 64 bit OS, snippping tool is a 64 bit process. Inorder to intercept we need a 64 bit native dll.

#Execution
Run DRM>exe. 
Make sure hook.dll is along side the same directory. Open a image, Try Print screen keystroke. Observe nothing gets copied
For seeing hook.dll funcitonality the application need to be run on vista 64 bit. (alternatively the code could be modified to execute tat workflow, DWMWRapper.EnableDWM)
For snipping tool functionality the tool need to be opened prior to running DRM.exe. SUbsquently "NEW" workflow could be performed.

#Issues:
Under no DWM case, 
Snipping tool need to be running prior to DRM.exe
Image doesn't refresh when user does a cancel. 





