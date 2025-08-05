# 0.3.5 - UTF-8 & terminal colors
    ADDITIONS
    [+] UTF-8 submodule
    [+] UTF-8 compatibility
    [+] Style class

    MISC
    [-] Input optimizations
    [-] The server module is now more verbose
    [-] Added startup tests
    [-] The server module now displays terminal colors

# 0.3.4 - Server updates
    CHANGES
    [-] The server now uses epoll() instead of threads
    [-] Switched the server to an event based system.
    [-] Refactored parts of the server for efficiency

    MISC
    [-] The server no longer takes in an IP as an argument
        - Instead, you should only specify `./server <port = 8080>`
    [-] Updated the usage to reflect changes
    [-] Updated README.md to reflect changes

# 0.3.3 - Network optimization
    REMOVED
    [-] common.cpp became redundant after network_session was added
    
    MISC
    [-] Network packets sent have been reduced by half!
    [-] Switched port from int to uint16_t

# 0.3.2 - Stability improvements
    MISC
    [-] Improved code readability
    [-] Improved error handling
    [-] Debugger overloaded functions for boolean logging
    [-] Debugger improved performance
    [-] Improved thread safety
    [-] Further checks to prevent memory leaks
    [-] Improved user input stability
    [-] Minor refactors

# 0.3.1 - Debugger update
    ADDITIONS
    [+] Added a debugger class to allow for more robust logging

    BUG FIXES
    [-] Segmentation fault on exit has been fixed (again)
    [-] Fixed all known bugs

    MISC
    [-] Stability improvements

# 0.3.0 - Refactor and expansion
    ADDITIONS
    [+] vcpkg package manager
    [+] libsodium for future message encryption
    [+] Reintroduced seg fault on exit (TODO lol)

    BUG FIXES
    [-] Fixed a bug with ncurses window refreshing
    
    MISC
    [-] Added common/headers and common/src for consistency
    [-] Added network_session as a network wrapper
    [-] Added documentation in certain files (WIP)
    [-] Switched remaining class function names to CamelCase for consistency
    [-] Renamed remaining member variables to reflect the 'm_varName' style
    [-] Refactored server code to reflect the new NetworkSession class.
    [-] Implemented unique_ptr for improved memory management
    [-] server/HandleClient was replaced with HandleClientSession

# 0.2.1 - Hotfix
    BUG FIXES
    [-] Fixed segmentation fault on /exit
    [-] Fixed all known bugs
    [-] Stability improvements

# 0.2.0 - UI is here!

    ADDITIONS
    [+] Added ncurses UI!
    [+] Input is no longer linked to message updates
    [+] Input is now in a separate window from the message screen

    BUG FIXES
    [-] Many problems with socket creation and closing
    [-] Sockets now shutdown instead of closing abruptly

    MISC
    [-] More descriptive errors
    [-] Stability improvements
    [-] The server now supports up to 10 clients instead of 5. That's a 100% increase!

    KNOWN ISSUES
    [-] Seg fault when /exit is run (the program still exits, just in a crash and burn kind of way instead of a graceful bird gently landing on a branch)

# 0.1.1

    CHANGES
    [-] You can now specify the IP and port when launching the server and/or client
    [-] Updated the README to reflect these changes

# 0.1.0 - Initial release

    ADDITIONS
    [+] A server that can handle client connections
    [+] A client that can connect to a server
