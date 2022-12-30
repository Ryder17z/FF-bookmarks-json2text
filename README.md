# FF-bookmarks-json2text
Takes a Firefox (json format) bookmark backup file and converts it to a text file of urls

Tested on Linux, should cross-compile just fine for Win/Mac\
Example C++17 makefile is available.

![image](https://user-images.githubusercontent.com/2000703/210113268-70e1f8d4-bd0b-4dcc-940e-4f392eab302d.png)


Usage: drag-n-drop file onto the built executable or run from cmd/terminal with a file path as the single argument

<br/>

Tested using a bookmarks file with ~8000 entries, with partial support for special urls created by extensions.

Tested with special urls provided by: https://addons.mozilla.org/en-US/firefox/addon/tab-session-manager/

Also supports bookmarks of local files (i.e. file://c:/mydoc.pdf)
