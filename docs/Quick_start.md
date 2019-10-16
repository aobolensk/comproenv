# Quick start guide

1. Build the project (follow instructions in [Building guide](docs/Building.md))  
2. Launch comproenv with sample configurations:  
`python launch.py --f run --r config/windows_sample.yaml`
3. Setup editor:  
`set editor <editor_executable> @name@.@lang@ &`  
- `<editor_executable>` - path to editor executable  
- `@name@.@lang@` - mask for file name and extension. It is better to use exactly this mask  
- `&` - that marker is needed if you don't want to block comproenv execution while you are using the editor  
Example:  
`set editor notepad @name@.@lang@ &`  
4. Setup compilers and runners:  
* Compilers:  
`set compiler_<language> <compiler_command>`  
If you are going to setup [compiled language](https://en.wikipedia.org/wiki/Compiled_language) then you need to provide compilation command for source code file to compile.  
Examples:  
`set compiler_cpp g++ @name@.@lang@ -o @name@ -std=c++17`  
`set compiler_c gcc @name@.@lang@ -o @name@ -std=c11`  
* Runners:  
`set runner_<language> <runner_command>`  
If you are going to setup language that require some custom way to run instead of just launching executable file (e.g. Python)then you need to setup custom runner.  
Example:  
`set runner_py python @name@.@lang@`  
5. Usage:  
* Creating the task:  
After finishing editor, compilers and runners configurations you can use comproenv to solve some tasks.  
All tasks are grouped to environments. First, you need to create environment:  
`ce test_env` command creates environment with name 'test_env'  
Then you need to enter that environment:  
`se test_env` command sets environment 'test_env'  
Now you finally can create the task:  
`ct my_task <language>` command creates the task (language is optional parameter, default: cpp)  
`st my_task` command sets this task  
* Manipulating with task:  
`edit` - edit task source code  
`c` - compile task  
`r` - run task  
`t` - test task  
`cat` - compile & test  
`cr` - compile and run  
`ctr` - compile, test and run  
* Tests:
In `t` (test) command you can specify your own test.
Note: end marker of multiline input (like tests, expected output and stuff like that) is double empty line  
`ct <test_name>` - create test  
`co <test_name>` - add expected answer for test  
`cte <test_name>` - create test with expected answer  
If you omit test_name parameter then test name will be generated automatically.  
Also you can parse tests from webpage using the following command:  
`parse <url>`  
To launch testing you need to call 't' command  
* Generator:  
You can create your own custom test generator using language that you prefer.  
`cg <language>` - create generator  
`sg` - select generator  
In generator menu:  
`cg` - compile generator  
`rg` - run generator  
Note: generator is launched within tests directory, so you can use `test_name.in` and `test_name.out` file names for your custom tests  

You can use `help` command to get the list of all commands that are available in current menu.
