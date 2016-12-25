# minicatcher
A simplistic command line-based podcatcher. Requires Qt 5.

### Usage:
 minicatcher [Action]

Actions:
* -f, --fetch            Downloads new episodes. Giving no action also does this
* -l, --list             Lists current sources
* -a, --add <URL> [MODE] Adds URL to sources
* -r, --rem <URL>        Removes URL from sources
* -m, --max-dl [NUM]     Displays (NUM not given) or sets number of parallel downloads
* -d, --dest [DEST]      Displays (DEST not given) or sets download location

Modes:
* last: Only most recent episode is considered new
* none: None of the episodes are considered new
* all: All episodes are consisdered new
