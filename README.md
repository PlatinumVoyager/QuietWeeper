## QuietWeeper v1.2.2 - Weeping HTTP Server

Generic HTTP server operational GUI aimed at providing server operators with an easier toolset for general use case scenarios.

![quietweeper](https://github.com/PlatinumVoyager/QuietWeeper/assets/116006542/4d58bded-f822-4828-a865-41592e22b989)


</br>

### Get to Building:
In order to utilize the graphical side of the QuietWeeping server you must download initial dependencies. This is limited to a singular library needed. You can follow the required build source for *Raylib* from <a href="https://github.com/raysan5/raylib/wiki/Working-on-GNU-Linux" title="Raylib - The official Github repository">here</a>.

NOTE: You will also need to download `Civetweb` which is partially responsible for the backend server logic from <a href="https://github.com/civetweb/civetweb">here<a/>.


After downloading the essential packages with `apt` and building after `git clone` make sure to set `make PLATFORM=PLATFORM_DESKTOP RAYLIB_LIBTYPE=SHARED` when calling the `make` command for the first iteration.

This will let declaring the following define directive...
```c
#include "raylib.h"
```
...to work correctly, as quietweeper does not rely on static compilation pre-runtime.

There is no associated `Makefile` within QuietWeeper's main directory as of version `1.2.2`. For now you will have to manually start the build process yourself via executing:
  * `gcc -std=c11 quietweeper.c -o quietweeper -I./include -lraylib -lGL -lm -lpthread -ldl -lrt -lX11 -lcivetweb -s`

</br>

### Launching the Application Interface:

Usage: `./quietweeper`
</br>

![image](https://github.com/PlatinumVoyager/QuietWeeper/assets/116006542/b702c583-65f5-4bcd-b1ba-e4c1b5a08042)

</br>

Upon pressing <i>`ENTER`</i> the backend will spin up the Graphical User Interface. This interface accepts 3 **MANDATORY** arguments:
  * HOSTNAME - The TCP/IPv4 address or hostname "ALIAS" pointing to an Internet (local and/or remote) host
  * PORT - The port that will be bound/set for the internal AF_INET server socket
  * PATH - The absolute or "." (current directory of `weepingserver` executable) direct path for the document root

</br>

### Graphical Interface:

![image](https://github.com/PlatinumVoyager/PlatinumVoyager/assets/116006542/7d8a2960-e792-4d9f-9acf-a66e53e49885)
</br>
</br>

### Note: *This version is simply a "beta" public testing release. The official anticipated features that will make this a real viable toolset are still being developed. Stand by.*
