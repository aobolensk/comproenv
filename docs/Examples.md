#### Warning! This document is generated automatically by using 'generate_docs' executable

# Examples of commands usage

### Scope: GLOBAL

#### ?
```
help <- get list of commands
help <command> <- get examples of using this command
```
#### about
```
about <- get information about comproenv executable and environment
```
#### alias
```
alias se cd <- create alias 'cd' for command 'se'
alias q quit <- create alias 'quit' for command 'q'
```
#### autosave
```
autosave <- toggle autosave (if it was 'on' it will be 'off' and vice versa)
```
#### ce
```
ce e1 <- create an environment with name 'e1'
```
#### clear
```
clear <- clear the console screen
```
#### delete-alias
```
delete-alias se <- delete all aliases for command 'se'
```
#### docs
```
docs <- get link to online documentation
```
#### exit
```
q <- exit
```
#### help
```
help <- get list of commands
help <command> <- get examples of using this command
```
#### history
```
history <- show commands history
Commands history length can be set using: set max_history_size <new_history_size>
```
#### le
```
le <- show list of all available environments
```
#### lef
```
lef <- show list of all available environments
```
#### les
```
les <- show list of all available environments
```
#### py-shell
```
py-shell <- launch Python shell
```
#### q
```
q <- exit
```
#### re
```
re e1 <- remove the environment with name 'e1'
```
#### reload-envs
```
reload-envs <- reload environments from environments.yaml file
```
#### reload-settings
```
reload-settings <- reload settings from config.yaml file
```
#### s
```
s <- save settings
```
#### se
```
se e1 <- enter the environment with name 'e1'
```
#### set
```
set compiler_cpp g++ @name@.@lang@ -o @name@ -std=c++17 -O3 <- set compiler command for C++
set editor notepad @name@.@lang@ & <- set editor to notepad
set max_history_size 32 <- set history size buffer to 32 entries
set python_interpreter python <- set path to python interpreter
set runner_py python @name@.@lang@ <- set runner for Python
set template_cpp templates/cpp <- set path to template file for C++
```
#### sets
```
sets <- print all settings in key:value format
```
#### unset
```
unset runner_py <- delete runner for Python
unset template_cpp <- delete template for C++
```


### Scope: ENVIRONMENT

#### ?
```
help <- get list of commands
help <command> <- get examples of using this command
```
#### alias
```
alias se cd <- create alias 'cd' for command 'se'
alias q quit <- create alias 'quit' for command 'q'
```
#### autosave
```
autosave <- toggle autosave (if it was 'on' it will be 'off' and vice versa)
```
#### clear
```
clear <- clear the console screen
```
#### ct
```
ct t1 <- create a task with name 't1' in C++ (default language)
ct t1 py <- create a task with name 't1' in Python
```
#### delete-alias
```
delete-alias se <- delete all aliases for command 'se'
```
#### docs
```
docs <- get link to online documentation
```
#### ee
```
ee <- edit environment
You will get a list of editable settings, where you can either edit option or leave it as is (by pressing Enter)
```
#### exit
```
q <- exit from the environment
```
#### help
```
help <- get list of commands
help <command> <- get examples of using this command
```
#### history
```
history <- show commands history
Commands history length can be set using: set max_history_size <new_history_size>
```
#### lt
```
lt <- show list of all available tasks
```
#### py-shell
```
py-shell <- launch Python shell
```
#### q
```
q <- exit from the environment
```
#### reload-envs
```
reload-envs <- reload environments from environments.yaml file
```
#### reload-settings
```
reload-settings <- reload settings from config.yaml file
```
#### rt
```
rt t1 <- remove the task with name 't1'
```
#### set
```
set compiler_cpp g++ @name@.@lang@ -o @name@ -std=c++17 -O3 <- set compiler command for C++
set editor notepad @name@.@lang@ & <- set editor to notepad
set max_history_size 32 <- set history size buffer to 32 entries
set python_interpreter python <- set path to python interpreter
set runner_py python @name@.@lang@ <- set runner for Python
set template_cpp templates/cpp <- set path to template file for C++
```
#### sets
```
sets <- print all settings in key:value format
```
#### st
```
st t1 <- enter the task with name 't1'
```
#### unset
```
unset runner_py <- delete runner for Python
unset template_cpp <- delete template for C++
```


### Scope: TASK

#### ?
```
help <- get list of commands
help <command> <- get examples of using this command
```
#### alias
```
alias se cd <- create alias 'cd' for command 'se'
alias q quit <- create alias 'quit' for command 'q'
```
#### autosave
```
autosave <- toggle autosave (if it was 'on' it will be 'off' and vice versa)
```
#### c
```
ct <- compile task
You can setup compiler using set compiler_<language> <compile_command>
```
#### cat
```
cat <- compile and test
```
#### catf
```
catf <- compile and test (stop testing after first failure)
```
#### cg
```
cg <- create test generator in C++ (default language)
cg py <- create test generator in Python
```
#### clear
```
clear <- clear the console screen
```
#### co
```
co test_1 <- create expected result for test with name 'test_1'
After typing this command you need to provide expected result for test.
After that you need to enter empty line to save expected result.
```
#### cr
```
cr <- compile and run
```
#### ct
```
ct <- create test
ct test_1 <- create test with name 'test_1'
After typing this command you need to provide input for test.
After that you need to enter empty line to save input.
```
#### cte
```
cte <- create test
cte test_1 <- create test with name 'test_1'
After typing this command you need to provide input for test.
After that you need to enter empty line to save input.
Same for expected output.
```
#### ctr
```
ctr <- compile, test and run
```
#### delete-alias
```
delete-alias se <- delete all aliases for command 'se'
```
#### docs
```
docs <- get link to online documentation
```
#### edit
```
edit <- edit task in specified text editor
```
#### ee
```
ee <- edit task
You will get a list of editable settings, where you can either edit option or leave it as is (by pressing Enter)
```
#### eo
```
eo test_1 <- edit expected result for test with name 'test_1'
Expected result will be opened in specified editor.
```
#### et
```
et test_1 <- edit test with name 'test_1'
Test will be opened in specified editor.
```
#### exit
```
q <- exit from task
```
#### help
```
help <- get list of commands
help <command> <- get examples of using this command
```
#### history
```
history <- show commands history
Commands history length can be set using: set max_history_size <new_history_size>
```
#### lt
```
lt <- print list of tests
```
#### lts
```
lts <- print list of tests
```
#### parse
```
parse <link> <- parse tests from website
```
#### py-shell
```
py-shell <- launch Python shell
```
#### q
```
q <- exit from task
```
#### r
```
r <- run task
You can setup custom runner (if you need) using set runner_<language> <compile_command>
```
#### rat
```
rat <- remove all tests
```
#### reload-envs
```
reload-envs <- reload environments from environments.yaml file
```
#### reload-settings
```
reload-settings <- reload settings from config.yaml file
```
#### rg
```
rg <- remove test generator
```
#### ro
```
ro test_1 <- remove expected result for test with name 'test_1'
```
#### rt
```
rt test_1 <- remove test with name 'test_1'
```
#### set
```
set compiler_cpp g++ @name@.@lang@ -o @name@ -std=c++17 -O3 <- set compiler command for C++
set editor notepad @name@.@lang@ & <- set editor to notepad
set max_history_size 32 <- set history size buffer to 32 entries
set python_interpreter python <- set path to python interpreter
set runner_py python @name@.@lang@ <- set runner for Python
set template_cpp templates/cpp <- set path to template file for C++
```
#### sets
```
sets <- print all settings in key:value format
```
#### sg
```
sg <- enter test generator
```
#### t
```
t <- test task
This command launches all available tests and report results of testing
```
#### unset
```
unset runner_py <- delete runner for Python
unset template_cpp <- delete template for C++
```


### Scope: GENERATOR

#### ?
```
help <- get list of commands
help <command> <- get examples of using this command
```
#### alias
```
alias se cd <- create alias 'cd' for command 'se'
alias q quit <- create alias 'quit' for command 'q'
```
#### autosave
```
autosave <- toggle autosave (if it was 'on' it will be 'off' and vice versa)
```
#### cg
```
cg <- compile generator
You can setup compiler using set compiler_<language> <compile_command>
```
#### clear
```
clear <- clear the console screen
```
#### delete-alias
```
delete-alias se <- delete all aliases for command 'se'
```
#### docs
```
docs <- get link to online documentation
```
#### edit
```
eg <- edit generator in specified text editor
```
#### eg
```
eg <- edit generator in specified text editor
```
#### exit
```
q <- exit from generator
```
#### help
```
help <- get list of commands
help <command> <- get examples of using this command
```
#### history
```
history <- show commands history
Commands history length can be set using: set max_history_size <new_history_size>
```
#### lt
```
lt <- print list of tests
```
#### lts
```
lts <- print list of tests
```
#### py-shell
```
py-shell <- launch Python shell
```
#### q
```
q <- exit from generator
```
#### reload-envs
```
reload-envs <- reload environments from environments.yaml file
```
#### reload-settings
```
reload-settings <- reload settings from config.yaml file
```
#### rg
```
rg <- run task
You can setup custom runner (if you need) using set runner_<language> <compile_command>
```
