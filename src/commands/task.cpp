#include <iostream>
#include <fstream>
#include <vector>
#include <chrono>
#include <thread>
#include <experimental/filesystem>
#include "shell.h"

namespace comproenv {

namespace fs = std::experimental::filesystem;

void Shell::configure_commands_task() {
    add_command(State::TASK, "c", "Compile task", 
    [this](std::vector <std::string> &arg) -> int {
        if (arg.size() != 1)
            FAILURE("Incorrect arguments for command " + arg[0]);
        std::string command;
        std::string current_compiler = envs[current_env].get_tasks()[current_task].get_settings()["language"];
        if (!get_setting_by_name("compiler_" + current_compiler).has_value()) {
            FAILURE("There's no compiler for language " + current_compiler);
        }
        command = get_setting_by_name("compiler_" + current_compiler).value();
        replace_all(command, "@name@", (fs::path(env_prefix + envs[current_env].get_name()) / 
                            (task_prefix + envs[current_env].get_tasks()[current_task].get_name()) /
                            envs[current_env].get_tasks()[current_task].get_name()).string());
        replace_all(command, "@lang@", current_compiler);
        std::cout << "\033[35m" << "-- Compile task " << envs[current_env].get_tasks()[current_task].get_name() << ":" <<
            "\033[0m\n";
        auto time_start = std::chrono::high_resolution_clock::now();
        DEBUG_LOG(command);
        int ret_code = system(command.c_str());
        auto time_finish = std::chrono::high_resolution_clock::now();
        std::cout << "\033[35m" << "-- Time elapsed:" <<
            std::chrono::duration_cast<std::chrono::duration<double>>(time_finish - time_start).count() <<
            "\033[0m\n";
        return ret_code;
    });

    add_command(State::TASK, "r", "Run task",
    [this](std::vector <std::string> &arg) -> int {
        if (arg.size() != 1)
            FAILURE("Incorrect arguments for command " + arg[0]);
        std::string command;
        std::string current_runner = envs[current_env].get_tasks()[current_task].get_settings()["language"];
        command = get_setting_by_name("runner_" + current_runner).value_or(
            #ifdef _WIN32
            std::string("\"") + env_prefix + envs[current_env].get_name() + "\\" +
                task_prefix + envs[current_env].get_tasks()[current_task].get_name() + "\\" +
                envs[current_env].get_tasks()[current_task].get_name() + ".exe" + "\""
            #else
            std::string("\"./") + env_prefix + envs[current_env].get_name() + "/" +
                task_prefix + envs[current_env].get_tasks()[current_task].get_name() + "/" +
                envs[current_env].get_tasks()[current_task].get_name() + "\""
        #endif  // _WIN32
        );
        DEBUG_LOG(command);
        replace_all(command, "@name@", (fs::path(env_prefix + envs[current_env].get_name()) / 
                            (task_prefix + envs[current_env].get_tasks()[current_task].get_name()) /
                            envs[current_env].get_tasks()[current_task].get_name()).string());
        replace_all(command, "@lang@", current_runner);
        std::cout << "\033[35m" << "-- Run task " << envs[current_env].get_tasks()[current_task].get_name() << ":" <<
            "\033[0m" << std::endl;
        auto time_start = std::chrono::high_resolution_clock::now();
        DEBUG_LOG(command);
        int ret_code = system(command.c_str());
        auto time_finish = std::chrono::high_resolution_clock::now();
        std::cout << "\033[35m" << '\n' << "-- Time elapsed:" <<
            std::chrono::duration_cast<std::chrono::duration<double>>(time_finish - time_start).count() <<
            "\033[0m" << std::endl;
        return ret_code;
    });

    add_command(State::TASK, "t", "Test task",
    [this](std::vector <std::string> &arg) -> int {
        std::string command;
        std::string path;
        std::string temp_file_path;
        std::vector <fs::path> in_files;
        // Select tests
        if (arg.size() == 1) { // Run all tests
            fs::recursive_directory_iterator it_begin(fs::path(env_prefix + envs[current_env].get_name()) /
                (task_prefix + envs[current_env].get_tasks()[current_task].get_name()) / "tests"), it_end;
            std::copy_if(it_begin, it_end, std::back_inserter(in_files), [](const fs::path &path) {
                return fs::is_regular_file(path) && path.extension() == ".in";
            });
            std::sort(in_files.begin(), in_files.end());
        } else if (arg.size() > 1) { // Run specific tests
            for (unsigned i = 1; i < arg.size(); ++i) {
                fs::path current_test = fs::path(env_prefix + envs[current_env].get_name()) /
                    (task_prefix + envs[current_env].get_tasks()[current_task].get_name()) / "tests" / (arg[i] + ".in");
                if (fs::is_regular_file(current_test)) {
                    in_files.emplace_back(current_test);
                } else {
                    std::cout << "\033[31mError: Test with name " + arg[i] + " is not found\n\033[0m";
                }
            }
        } else {
            FAILURE("Incorrect arguments for command " + arg[0]);
        }
        // Launch selected tests:
        for (auto &it : in_files)
            std::cout << it << '\n';
        #ifdef _WIN32
        path = env_prefix + envs[current_env].get_name() + "\\" +
            task_prefix + envs[current_env].get_tasks()[current_task].get_name();
        temp_file_path = path + "\\" + "temp.txt";
        #else
        path = env_prefix + envs[current_env].get_name() + "/" +
            task_prefix + envs[current_env].get_tasks()[current_task].get_name();
        temp_file_path = path + "/" + "temp.txt";
        #endif  // _WIN32
        int errors = 0;
        int runtime_errors = 0;
        int mismatched_answers_errors = 0;
        int error_code = 0;
        std::cout << "\033[32m" << "-- Test command" << "\033[0m" << '\n';
        for (auto &in_file : in_files) {
            std::cout << "\033[33m" << "-- Test " << in_file << "\033[0m" << '\n';
            std::cout << "\033[35m" << "-- Input:" << "\033[0m" << '\n';
            std::string buf;
            std::ifstream f(in_file);
            if (!f.is_open())
                return -1;
            while (std::getline(f, buf))
                std::cout << buf << '\n';
            f.close();
            std::cout << "\033[35m" << "-- Result:" << "\033[0m" << std::endl;
            std::string current_runner = envs[current_env].get_tasks()[current_task].get_settings()["language"];
            command = get_setting_by_name("runner_" + current_runner).value_or(
                #ifdef _WIN32
                path + "\\" +
                envs[current_env].get_tasks()[current_task].get_name() +
                    ".exe < " + in_file.string() + " > " + temp_file_path
                #else
                std::string("./") + path + "/" +
                envs[current_env].get_tasks()[current_task].get_name() +
                    " < " + in_file.string() + " > " + temp_file_path
                #endif  // _WIN32
            ) + " < " + in_file.string() + " > " + temp_file_path;
            replace_all(command, "@name@", (fs::path(env_prefix + envs[current_env].get_name()) / 
                                (task_prefix + envs[current_env].get_tasks()[current_task].get_name()) /
                                envs[current_env].get_tasks()[current_task].get_name()).string());
            replace_all(command, "@lang@", current_runner);
            auto time_start = std::chrono::high_resolution_clock::now();
            DEBUG_LOG(command);
            error_code = system(command.c_str());
            auto time_finish = std::chrono::high_resolution_clock::now();
            f.open(temp_file_path);
            int max_lines_count = std::stoi(get_setting_by_name("max_lines_count").value_or("100"));
            if (f.is_open()) {
                int lines_count = 0;
                while (lines_count++ < max_lines_count && std::getline(f, buf))
                    std::cout << buf << '\n';
                if (lines_count == max_lines_count)
                    std::cout << "...\n";
                f.close();
            }
            if (error_code) {
                std::cout << "\033[31m" << "-- Runtime error!" << "\033[0m" << std::endl;
                ++runtime_errors;
                ++errors;
            }
            std::string out_file = in_file.string();
            for (int i = 0; i < 2; ++i)
                out_file.pop_back();
            out_file.append("out");
            f.open(out_file);
            if (f.is_open()) {
                std::cout << "\033[35m" << "-- Expected:" << "\033[0m" << '\n';
                while (std::getline(f, buf))
                    std::cout << buf << '\n';
                f.close();
                std::vector <std::string> res_in, res_out;
                f.open(temp_file_path, std::ios::in);
                while (f >> buf) {
                    res_in.emplace_back(buf);
                }
                f.close();
                f.open(out_file, std::ios::in);
                while (f >> buf) {
                    res_out.emplace_back(buf);
                }
                f.close();
                if (res_in != res_out) {
                    std::cout << "\033[33;1m" << "-- Warning: Mismatch of result and expected!" << "\033[0m" << std::endl;
                    if (!error_code) {
                        ++mismatched_answers_errors;
                        ++errors;
                    }
                }
            }
            std::cout << "\033[35m" << "-- Time elapsed:" <<
                std::chrono::duration_cast<std::chrono::duration<double>>(time_finish - time_start).count() <<
                "\033[0m" << std::endl;
            std::cout << "\033[33m" << "-- End of test " << in_file << "\033[0m" << std::endl;
        }
        if (std::size(in_files) && remove(temp_file_path.c_str())) {
            std::cout << "Unable to delete temporary file\n";
        }
        if (errors == 0) {
            std::cout << "\033[32;1m" << "-- Test command: All " << std::size(in_files) <<
                " tests successfully passed!" << "\033[0m\n";
        } else {
            std::cout << "\033[31;1m" << "-- Test command: Warning! " << errors <<
                "/" << std::size(in_files) << " tests failed";
            if (mismatched_answers_errors > 0 ||
                runtime_errors > 0) {
                std::cout << " (";
                bool need_space = false;
                if (mismatched_answers_errors > 0) {
                    if (need_space)
                        std::cout << ", ";
                    std::cout << mismatched_answers_errors << " mismatched answers";
                    need_space = true;
                }
                if (runtime_errors > 0) {
                    if (need_space)
                        std::cout << ", ";
                    std::cout << runtime_errors << " runtime errors";
                    need_space = true;
                }
                std::cout << ")";
            }
            std::cout << "!" << "\033[0m\n";
        }
        return errors;
    });

    add_command(State::TASK, "ee", "Edit task",
    [this](std::vector <std::string> &arg) -> int {
        if (arg.size() != 1)
            FAILURE("Incorrect arguments for command " + arg[0]);
        std::string buf;
        std::error_code e;
        fs::path path = fs::path(env_prefix + envs[current_env].get_name()) /
            (task_prefix + envs[current_env].get_tasks()[current_task].get_name());
        std::string &lang = envs[current_env].get_tasks()[current_task].get_settings()["language"];
        do {
            e.clear();
            std::cout << "Name [" << envs[current_env].get_tasks()[current_task].get_name() << "]: ";
            std::getline(std::cin, buf);
            if (buf.size() > 0) {
                fs::rename(path,
                    fs::path(env_prefix + envs[current_env].get_name()) /
                    fs::path(task_prefix + buf), e);
                if (e.value() != 0) {
                    std::cout << "Rename error: " << e.message() << std::endl;
                } else {
                    envs[current_env].get_tasks()[current_task].set_name(buf);
                    path = fs::path(env_prefix + envs[current_env].get_name()) /
                        (task_prefix + envs[current_env].get_tasks()[current_task].get_name());
                    for (auto &entry : fs::directory_iterator(path)) {
                        if (fs::is_regular_file(entry.path())) {
                            if (entry.path().filename().string().find(buf) == std::string::npos) {
                                fs::path new_path = entry.path().parent_path() / (buf + '.' + lang);
                                fs::rename(entry.path(), new_path, e);
                                if (e.value() != 0) {
                                    std::cout << "Rename error: " << e.message() << std::endl;
                                }
                            }
                        }
                    }
                    std::cout << "Set task name: " << buf << '\n';
                }
            }
        } while (e.value() != 0);

        std::cout << "Language [" << lang << "]: ";
        std::getline(std::cin, buf);
        if (buf.size() > 0 && buf != lang) {
            lang = buf;
            fs::path file_path = path / (envs[current_env].get_tasks()[current_task].get_name() + "." + lang);
            if (!fs::is_regular_file(file_path)) {
                std::ofstream f(file_path, std::ios::out);
                if (!f.is_open()) {
                    return -1;
                }
                std::string template_file = get_setting_by_name("template_" + lang)
                    .value_or((fs::path("templates") / lang).string());
                DEBUG_LOG("Template file: " + template_file);
                if (fs::is_regular_file(template_file)) {
                    std::ifstream t(template_file);
                    if (t.is_open()) {
                        std::string buffer;
                        while (std::getline(t, buffer))
                            f << buffer << '\n';
                        t.close();
                    } else {
                        std::cout << "Unable to open default template file\n";
                    }
                } else {
                    std::cout << "Template for language " + lang + " is not found. "
                                    "Created empty file\n";
                }
            }
        }
        return 0;
    });

    add_command(State::TASK, "ct", "Create test",
    [this](std::vector <std::string> &arg) -> int {
        if (arg.size() > 2)
            FAILURE("Incorrect arguments for command " + arg[0]);
        std::string test_name;
        fs::path file_path = fs::path(env_prefix + envs[current_env].get_name()) /
            (task_prefix + envs[current_env].get_tasks()[current_task].get_name()) /
            "tests";
        if (arg.size() == 1) {
            unsigned num = 1;
            while (true) {
                test_name = "test_" + std::to_string(num);
                if (!fs::is_regular_file(file_path / (test_name + ".in")))
                    break;
                ++num;
            }
        } else {
            test_name = arg[1];
        }
        file_path /= (test_name + ".in");
        std::string buf;
        std::cout << "Write test (send empty line at the end of text):\n";
        std::ofstream f(file_path);
        if (!f.is_open())
            return 1;
        while (true) {
            std::getline(std::cin, buf);
            if (buf.size() == 0)
                break;
            f << buf << '\n';
        }
        f.close();
        return 0;
    });

    add_command(State::TASK, "cte", "Create test with expected result",
    [this](std::vector <std::string> &arg) -> int {
        if (arg.size() > 2)
            FAILURE("Incorrect arguments for command " + arg[0]);
        std::string test_name;
        fs::path file_path = fs::path(env_prefix + envs[current_env].get_name()) /
            (task_prefix + envs[current_env].get_tasks()[current_task].get_name()) /
            "tests";
        if (arg.size() == 1) {
            unsigned num = 1;
            while (true) {
                test_name = "test_" + std::to_string(num);
                if (!fs::is_regular_file(file_path / (test_name + ".in")))
                    break;
                ++num;
            }
        } else {
            test_name = arg[1];
        }
        fs::path in_path = file_path / (test_name + ".in");
        fs::path out_path = file_path / (test_name + ".out");
        std::string buf;
        std::cout << "Write test (send empty line at the end of text):\n";
        std::ofstream f(in_path);
        if (!f.is_open())
            return 1;
        while (true) {
            std::getline(std::cin, buf);
            if (buf.size() == 0)
                break;
            f << buf << '\n';
        }
        f.close();
        std::cout << "Expected result (send empty line at the end of text):\n";
        f.open(out_path);
        if (!f.is_open())
            return 2;
        while (true) {
            std::getline(std::cin, buf);
            if (buf.size() == 0)
                break;
            f << buf << '\n';
        }
        f.close();
        return 0;
    });

    add_command(State::TASK, "rt", "Remove test",
    [this](std::vector <std::string> &arg) -> int {
        if (arg.size() != 2)
            FAILURE("Incorrect arguments for command " + arg[0]);
        fs::path file_path = fs::path(env_prefix + envs[current_env].get_name()) /
            (task_prefix + envs[current_env].get_tasks()[current_task].get_name()) /
            "tests" / (arg[1] + ".in");
        if (fs::exists(file_path)) {
            fs::remove(file_path);
        }
        file_path = fs::path(env_prefix + envs[current_env].get_name()) /
            (task_prefix + envs[current_env].get_tasks()[current_task].get_name()) /
            "tests" / (arg[1] + ".out");
        if (fs::exists(file_path)) {
            fs::remove(file_path);
        }
        return 0;
    });

    add_command(State::TASK, "et", "Edit test",
    [this](std::vector <std::string> &arg) -> int {
        if (arg.size() != 2)
            FAILURE("Incorrect arguments for command " + arg[0]);
        fs::path file_path = fs::path(env_prefix + envs[current_env].get_name()) /
            (task_prefix + envs[current_env].get_tasks()[current_task].get_name()) /
            "tests" / arg[1];
        if (fs::exists(file_path)) {
            return -1;
        }
        if (!get_setting_by_name("editor").has_value()) {
            FAILURE("There's no editor in config file");
        }
        std::string command = get_setting_by_name("editor").value();
        replace_all(command, "@name@", file_path.string());
        replace_all(command, "@lang@", "in");
        DEBUG_LOG(command);
        return system(command.c_str());
    });

    add_command(State::TASK, "lts", "List of tests (short: only names)",
    [this](std::vector <std::string> &arg) -> int {
        if (arg.size() != 1)
            FAILURE("Incorrect arguments for command " + arg[0]);
        std::vector <fs::path> in_files;
        fs::recursive_directory_iterator it_begin(fs::path(env_prefix + envs[current_env].get_name()) /
            (task_prefix + envs[current_env].get_tasks()[current_task].get_name()) / "tests"), it_end;
        std::copy_if(it_begin, it_end, std::back_inserter(in_files), [](const fs::path &path) {
            return fs::is_regular_file(path) && path.extension() == ".in";
        });
        std::sort(in_files.begin(), in_files.end());
        std::cout << "\033[32m" << "List of tests for task " <<
            envs[current_env].get_tasks()[current_task].get_name() << "\033[0m" << '\n';
        for (auto &in_file : in_files) {
            std::cout << "\033[33m" << "Test " << in_file << "\033[0m" << '\n';
        }
        return 0;
    });

    add_command(State::TASK, "lt", "List of tests (full: with input and output)",
    [this](std::vector <std::string> &arg) -> int {
        if (arg.size() != 1)
            FAILURE("Incorrect arguments for command " + arg[0]);
        std::vector <fs::path> in_files;
        fs::recursive_directory_iterator it_begin(fs::path(env_prefix + envs[current_env].get_name()) /
            (task_prefix + envs[current_env].get_tasks()[current_task].get_name()) / "tests"), it_end;
        std::copy_if(it_begin, it_end, std::back_inserter(in_files), [](const fs::path &path) {
            return fs::is_regular_file(path) && path.extension() == ".in";
        });
        std::sort(in_files.begin(), in_files.end());
        std::cout << "\033[32m" << "List of tests for task " <<
            envs[current_env].get_tasks()[current_task].get_name() << "\033[0m" << '\n';
        for (auto &in_file : in_files) {
            std::cout << "\033[33m" << "Test " << in_file << "\033[0m" << '\n';
            std::cout << "\033[35m" << "-- Input:" << "\033[0m" << '\n';
            std::string buf;
            std::ifstream f(in_file);
            if (!f.is_open())
                return -1;
            while (std::getline(f, buf))
                std::cout << buf << '\n';
            f.close();
            std::string out_file = in_file.string();
            out_file.pop_back();
            out_file.pop_back();
            out_file += "out";
            f.open(out_file);
            if (f.is_open()) {
                std::cout << "\033[35m" << "-- Output:" << "\033[0m" << '\n';
                while (std::getline(f, buf))
                    std::cout << buf << '\n';
                f.close();
            }
        }
        return 0;
    });

    add_command(State::TASK, "cg", "Create generator",
    [this](std::vector <std::string> &arg) -> int {
        if (arg.size() > 2 || arg.size() < 1)
            FAILURE("Incorrect arguments for command " + arg[0]);
        if (envs[current_env].get_tasks()[current_task].get_settings().find("generator") !=
            envs[current_env].get_tasks()[current_task].get_settings().end()) {
            FAILURE("Generator is already created");
        }
        std::string lang;
        if (arg.size() == 2)
            lang = arg[1];
        else
            lang = "cpp";
        envs[current_env].get_tasks()[current_task].get_settings()["generator"] = lang;
        fs::path file_path = fs::path(env_prefix + envs[current_env].get_name()) /
            (task_prefix + envs[current_env].get_tasks()[current_task].get_name()) /
            "tests" / ("generator." + lang);
        std::ofstream f(file_path, std::ios::out);
        if (!f.is_open()) {
            return -1;
        }
        std::string file_name;
        file_name = get_setting_by_name("template_" + lang)
            .value_or((fs::path("templates") / lang).string());
        if (!fs::is_regular_file(file_name)) {
            std::cout << "Template for language " + lang + " is not found. "
                            "Created empty file\n";
        } else {
            std::ifstream t(file_name);
            if (t.is_open()) {
                std::string buf;
                while (std::getline(t, buf))
                    f << buf << '\n';
                t.close();
            } else {
                std::cout << "Unable to open template file\n";
            }
        }
        f.close();
        if (global_settings["autosave"] == "on") {
            std::vector <std::string> save_args = {"s"};
            return commands[State::GLOBAL][save_args.front()](save_args);
        }
        return 0;
    });

    add_command(State::TASK, "rg", "Remove generator",
    [this](std::vector <std::string> &arg) -> int {
        if (arg.size() != 1)
            FAILURE("Incorrect arguments for command " + arg[0]);
        auto it = envs[current_env].get_tasks()[current_task].get_settings().find("generator");
        if (it == envs[current_env].get_tasks()[current_task].get_settings().end())
            FAILURE("Generator doesn't exist");
        std::string lang = (*it).second;
        envs[current_env].get_tasks()[current_task].get_settings().erase(it);
        fs::path file_path = fs::path(env_prefix + envs[current_env].get_name()) /
            (task_prefix + envs[current_env].get_tasks()[current_task].get_name()) /
            "tests" / ("generator." + lang);
        if (fs::exists(file_path)) {
            fs::remove(file_path);
        }
        if (global_settings["autosave"] == "on") {
            std::vector <std::string> save_args = {"s"};
            return commands[State::GLOBAL][save_args.front()](save_args);
        }
        return 0;
    });

    add_command(State::TASK, "sg", "Set generator",
    [this](std::vector <std::string> &arg) -> int {
        if (arg.size() != 1)
            FAILURE("Incorrect arguments for command " + arg[0]);
        if (envs[current_env].get_tasks()[current_task].get_settings().find("generator") !=
            envs[current_env].get_tasks()[current_task].get_settings().end()) {
            current_state = State::GENERATOR;
            store_cache();
            set_console_title();
        } else {
            FAILURE("Generator doesn't exist");
        }
        return 0;
    });

    add_command(State::TASK, "cr", "Compile & Run",
    [this](std::vector <std::string> &arg) -> int {
        if (arg.size() != 1)
            FAILURE("Incorrect arguments for command " + arg[0]);
        std::vector <std::string> args;
        args.push_back("c");
        int res = commands[current_state]["c"](args);
        if (res == 0) {
            args.pop_back();
            args.push_back("r");
            res = commands[current_state]["r"](args);
        }
        return res;
    });

    add_command(State::TASK, "cat", "Compile & Test",
    [this](std::vector <std::string> &arg) -> int {
        std::vector <std::string> args;
        args.push_back("c");
        int res = commands[current_state]["c"](args);
        if (res == 0) {
            args.pop_back();
            args.push_back("t");
            for (size_t i = 1; i < arg.size(); ++i)
                args.push_back(arg[i]);
            res = commands[current_state]["t"](args);
        }
        return res;
    });

    add_command(State::TASK, "catf", "Compile & Test (stop testing after first failure)",
    [this](std::vector <std::string> &arg) -> int {
        if (arg.size() != 1)
            FAILURE("Incorrect arguments for command " + arg[0]);
        std::vector <std::string> args;
        args.push_back("c");
        int res = commands[current_state]["c"](args);
        if (res == 0) {
            args.pop_back();
            args.push_back("t");
            std::vector <fs::path> in_files;
            fs::recursive_directory_iterator it_begin(fs::path(env_prefix + envs[current_env].get_name()) /
                (task_prefix + envs[current_env].get_tasks()[current_task].get_name()) / "tests"), it_end;
            std::copy_if(it_begin, it_end, std::back_inserter(in_files), [](const fs::path &path) {
                return fs::is_regular_file(path) && path.extension() == ".in";
            });
            std::sort(in_files.begin(), in_files.end());
            std::vector <std::string> test_names;
            for (const fs::path &path : in_files) {
                std::string test_name = path.filename().string();
                test_names.push_back(std::string(test_name.begin(), test_name.end() - std::size(".in") + 1));
            }
            int index = 0;
            for (const std::string &test : test_names) {
                args.push_back(test);
                res = commands[current_state]["t"](args);
                if (res != 0) {
                    std::cout << "\033[31m" << "-- Test failed! Stopped running tests. "
                            << index << " of " << test_names.size()
                            << " tests passed before failure" << "\033[0m" << '\n';
                    return 1;
                }
                ++index;
                args.pop_back();
            }
            std::cout << "\033[32m" << "-- All " << std::size(in_files) <<
                " tests successfully passed!" << "\033[0m\n";
        }
        return res;
    });

    add_command(State::TASK, "ctr", "Compile, Test & Run",
    [this](std::vector <std::string> &arg) -> int {
        if (arg.size() != 1)
            FAILURE("Incorrect arguments for command " + arg[0]);
        std::vector <std::string> args;
        args.push_back("c");
        int res = commands[current_state]["c"](args);
        if (res == 0) {
            args.pop_back();
            args.push_back("t");
            res = commands[current_state]["t"](args);
            if (res == 0) {
                args.pop_back();
                args.push_back("r");
                res = commands[current_state]["r"](args);
            }
        }
        return res;
    });

    add_command(State::TASK, "parse", "Parse page with tests",
    [this](std::vector <std::string> &arg) -> int {
        if (arg.size() != 2)
            FAILURE("Incorrect arguments for command " + arg[0]);
        std::string command = global_settings["python_interpreter"] +
            " scripts/parser.py" + " run " +
            // Path to tests directory
            (fs::path(env_prefix + envs[current_env].get_name()) /
            (task_prefix + envs[current_env].get_tasks()[current_task].get_name()) /
            "tests").string() + " " +
            // Link to page with tests
            "\"" + arg[1] + "\"";
        DEBUG_LOG(command);
        return system(command.c_str());
    });

    add_command(State::TASK, "co", "Create output",
    [this](std::vector <std::string> &arg) -> int {
        if (arg.size() != 2)
            FAILURE("Incorrect arguments for command " + arg[0]);
        fs::path file_path = fs::path(env_prefix + envs[current_env].get_name()) /
            (task_prefix + envs[current_env].get_tasks()[current_task].get_name()) /
            "tests" / (arg[1] + ".out");
        std::string buf;
        std::ofstream f(file_path);
        if (!f.is_open())
            return 1;
        while (true) {
            std::getline(std::cin, buf);
            if (buf.size() == 0)
                break;
            f << buf << '\n';
        }
        f.close();
        return 0;
    });

    add_command(State::TASK, "ro", "Remove output",
    [this](std::vector <std::string> &arg) -> int {
        if (arg.size() != 2)
            FAILURE("Incorrect arguments for command " + arg[0]);
        fs::path file_path = fs::path(env_prefix + envs[current_env].get_name()) /
            (task_prefix + envs[current_env].get_tasks()[current_task].get_name()) /
            "tests" / (arg[1] + ".out");
        if (fs::exists(file_path)) {
            fs::remove(file_path);
        }
        return 0;
    });

    add_command(State::TASK, "eo", "Edit output",
    [this](std::vector <std::string> &arg) -> int {
        if (arg.size() != 2)
            FAILURE("Incorrect arguments for command " + arg[0]);
        fs::path file_path = fs::path(env_prefix + envs[current_env].get_name()) /
            (task_prefix + envs[current_env].get_tasks()[current_task].get_name()) /
            "tests" / arg[1];
        if (fs::exists(file_path)) {
            return -1;
        }
        if (!get_setting_by_name("editor").has_value()) {
            FAILURE("There's no editor in config file");
        }
        std::string command = get_setting_by_name("editor").value();
        replace_all(command, "@name@", file_path.string());
        replace_all(command, "@lang@", "out");
        DEBUG_LOG(command);
        return system(command.c_str());
    });

    add_command(State::TASK, "set", "Configure task settings",
    [this](std::vector <std::string> &arg) -> int {
        if (arg.size() == 2) {
            envs[current_env].get_tasks()[current_task].get_settings().erase(arg[1]);
        } else if (arg.size() >= 3) {
            std::string second_arg = arg[2];
            for (unsigned i = 3; i < arg.size(); ++i) {
                second_arg.push_back(' ');
                second_arg += arg[i];
            }
            envs[current_env].get_tasks()[current_task].get_settings().erase(arg[1]);
            envs[current_env].get_tasks()[current_task].get_settings().emplace(arg[1], second_arg);
        } else {
            FAILURE("Incorrect arguments for command " + arg[0]);
        }
        if (global_settings["autosave"] == "on") {
            std::vector <std::string> save_args = {"s"};
            return commands[State::GLOBAL][save_args.front()](save_args);
        }
        return 0;
    });

    add_command(State::TASK, "edit", "Edit task in text editor",
    [this](std::vector <std::string> &arg) -> int {
        if (arg.size() != 1)
            FAILURE("Incorrect arguments for command " + arg[0]);
        if (!get_setting_by_name("editor").has_value()) {
            FAILURE("There's no editor in config file");
        }
        std::string command = get_setting_by_name("editor").value();
        replace_all(command, "@name@", (fs::path(env_prefix + envs[current_env].get_name()) / 
                            (task_prefix + envs[current_env].get_tasks()[current_task].get_name()) /
                            envs[current_env].get_tasks()[current_task].get_name()).string());
        replace_all(command, "@lang@", envs[current_env].get_tasks()[current_task].get_settings()["language"]);
        DEBUG_LOG(command);
        auto ampersand_pos = command.find("&");
        #ifdef _WIN32
        if (ampersand_pos != std::string::npos) {
            command.erase(ampersand_pos);
            command += " > NUL";
            std::thread thr([&](const std::string command) -> void {
                DEBUG_LOG(command);
                int res = system(command.c_str());
                (void)res;
            }, command);
            thr.detach();
            return 0;
        } else {
            DEBUG_LOG(command);
            return system(command.c_str());
        }
        #else
        if (ampersand_pos != std::string::npos) {
            command.insert(std::max(0ul, ampersand_pos - 1), " &> /dev/null ");
        }
        DEBUG_LOG(command);
        return system(command.c_str());
        #endif  // _WIN32
    });

    add_command(State::TASK, "q", "Exit from task", [this](std::vector <std::string> &arg) -> int {
        if (arg.size() != 1)
            FAILURE("Incorrect arguments for command " + arg[0]);
        current_task = -1;
        current_state = State::ENVIRONMENT;
        store_cache();
        set_console_title();
        return 0;
    });
    add_alias(State::TASK, "q", State::TASK, "exit");

    add_alias(State::GLOBAL, "help", State::TASK, "help");
    add_alias(State::GLOBAL, "help", State::TASK, "?");
}

}  // namespace comproenv
