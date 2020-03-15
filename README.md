# NZTsh
Simple shell written in C based on [LSH shell tutorial](https://brennan.io/2015/01/16/write-a-shell-in-c/). 

* Limitations of shell:
    * Commands must be on a single line.
    * Arguments must be separated by whitespace.
    * No piping or redirection. (to be implemented)
    * Only builtins are: `cd`, `help`, `exit`.