# 0.2.1
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
