#include <string>
#include <fstream>
    #include <thread>
#include <chrono>
#include <experimental/filesystem>
#include "shell.h"
#include "task.h"

namespace fs = std::experimental::filesystem;

Task::Task(const std::string &task_name) : name(task_name) {

}

std::string Task::get_name() const {
    return name;
}

void Task::add_setting(const std::string &key, const std::string &value) {
    settings.emplace(key, value);
}

std::unordered_map <std::string, std::string> &Task::get_settings() {
    return settings;
}

void Shell::configure_commands_task() {
    add_command(State::TASK, "c", "Compile task", 
    [this](std::vector <std::string> &arg) -> int {
        if (arg.size() != 1)
            throw std::runtime_error("Incorrect arguments for command " + arg[0]);
        std::string command;
        std::string current_compiler = envs[current_env].get_tasks()[current_task].get_settings()["language"];
        if (envs[current_env].get_tasks()[current_task].get_settings().find("compiler_" + current_compiler) !=
            envs[current_env].get_tasks()[current_task].get_settings().end())
            command = envs[current_env].get_tasks()[current_task].get_settings()["compiler_" + current_compiler];
        else if (envs[current_env].get_settings().find("compiler_" + current_compiler) !=
                envs[current_env].get_settings().end())
            command = envs[current_env].get_settings()["compiler_" + current_compiler];
        else if (global_settings.find("compiler_" + current_compiler) != global_settings.end())
            command = global_settings["compiler_" + current_compiler];
        else {
            std::cout << "There is no compiler rule for language " <<
                envs[current_env].get_tasks()[current_task].get_settings()["language"] << '\n';
            return -1;
        }
        size_t pos = std::string::npos;
        while ((pos = command.find("@name@")) != std::string::npos) {
            command.replace(command.begin() + pos, command.begin() + pos + std::size("@name@") - 1,
                            (fs::current_path() / ("env_" + envs[current_env].get_name()) / 
                            ("task_" + envs[current_env].get_tasks()[current_task].get_name()) /
                            envs[current_env].get_tasks()[current_task].get_name()).string());
        }
        std::cout << "\033[35m" << "-- Compile task " << envs[current_env].get_tasks()[current_task].get_name() << ":" <<
            "\033[0m\n";
        auto time_start = std::chrono::high_resolution_clock::now();
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
            throw std::runtime_error("Incorrect arguments for command " + arg[0]);
        std::string command;
        std::string current_runner = envs[current_env].get_tasks()[current_task].get_settings()["language"];
        if (envs[current_env].get_tasks()[current_task].get_settings().find("runner_" + current_runner) !=
            envs[current_env].get_tasks()[current_task].get_settings().end())
            command = envs[current_env].get_tasks()[current_task].get_settings()["runner_" + current_runner];
        else if (envs[current_env].get_settings().find("runner_" + current_runner) !=
                envs[current_env].get_settings().end())
            command = envs[current_env].get_settings()["runner_" + current_runner];
        else if (global_settings.find("runner_" + current_runner) != global_settings.end())
            command = global_settings["runner_" + current_runner];
        else {
            #ifdef _WIN32
            command = "env_" + envs[current_env].get_name() + "\\" +
                "task_" + envs[current_env].get_tasks()[current_task].get_name() + "\\" +
                envs[current_env].get_tasks()[current_task].get_name() + ".exe";
            #else
            command = std::string("./") + "env_" + envs[current_env].get_name() + "/" +
                "task_" + envs[current_env].get_tasks()[current_task].get_name() + "/" +
                envs[current_env].get_tasks()[current_task].get_name();
            #endif  // _WIN32
        }
        std::cout << "cmd: " << command << '\n';
        size_t pos = std::string::npos;
        while ((pos = command.find("@name@")) != std::string::npos) {
            command.replace(command.begin() + pos, command.begin() + pos + std::size("@name@") - 1,
                            (fs::current_path() / ("env_" + envs[current_env].get_name()) / 
                            ("task_" + envs[current_env].get_tasks()[current_task].get_name()) /
                            envs[current_env].get_tasks()[current_task].get_name()).string());
        }
        std::cout << "\033[35m" << "-- Run task " << envs[current_env].get_tasks()[current_task].get_name() << ":" <<
            "\033[0m" << std::endl;
        auto time_start = std::chrono::high_resolution_clock::now();
        int ret_code = system(command.c_str());
        auto time_finish = std::chrono::high_resolution_clock::now();
        std::cout << "\033[35m" << '\n' << "-- Time elapsed:" <<
            std::chrono::duration_cast<std::chrono::duration<double>>(time_finish - time_start).count() <<
            "\033[0m" << std::endl;
        return ret_code;
    });

    add_command(State::TASK, "t", "Test task",
    [this](std::vector <std::string> &arg) -> int {
        if (arg.size() != 1)
            throw std::runtime_error("Incorrect arguments for command " + arg[0]);
        std::string command;
        std::string path;
        std::string temp_file_path;
        std::vector <fs::path> in_files;
        fs::recursive_directory_iterator it_begin(fs::current_path() / ("env_" + envs[current_env].get_name()) /
            ("task_" + envs[current_env].get_tasks()[current_task].get_name()) / "tests"), it_end;
        std::copy_if(it_begin, it_end, std::back_inserter(in_files), [](const fs::path &path) {
            return fs::is_regular_file(path) && path.extension() == ".in";
        });
        std::sort(in_files.begin(), in_files.end());
        for (auto &it : in_files)
            std::cout << it << '\n';
        #ifdef _WIN32
        path = std::string("env_") + envs[current_env].get_name() + "\\" +
            "task_" + envs[current_env].get_tasks()[current_task].get_name();
        temp_file_path = path + "\\" + "temp.txt";
        #else
        path = "env_" + envs[current_env].get_name() + "/" +
            "task_" + envs[current_env].get_tasks()[current_task].get_name();
        temp_file_path = path + "/" + "temp.txt";
        #endif  // _WIN32
        int errors = 0, error_code = 0;
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
            if (envs[current_env].get_tasks()[current_task].get_settings().find("runner_" + current_runner) !=
                envs[current_env].get_tasks()[current_task].get_settings().end())
                command = envs[current_env].get_tasks()[current_task].get_settings()["runner_" + current_runner] +
                    " < " + in_file.string() + " > " + temp_file_path;
            else if (envs[current_env].get_settings().find("runner_" + current_runner) !=
                    envs[current_env].get_settings().end())
                command = envs[current_env].get_settings()["runner_" + current_runner] +
                    " < " + in_file.string() + " > " + temp_file_path;
            else if (global_settings.find("runner_" + current_runner) != global_settings.end())
                command = global_settings["runner_" + current_runner] + " < " + in_file.string() + " > " + temp_file_path;
            else {
                #ifdef _WIN32
                command = path + "\\" +
                envs[current_env].get_tasks()[current_task].get_name() + " < " + in_file.string() + " > " + temp_file_path;
                #else
                command = std::string("./") + path + "/" +
                envs[current_env].get_tasks()[current_task].get_name() + " < " + in_file.string() + " > " + temp_file_path;
                #endif  // _WIN32
            }
            size_t pos = std::string::npos;
            while ((pos = command.find("@name@")) != std::string::npos) {
                command.replace(command.begin() + pos, command.begin() + pos + std::size("@name@") - 1,
                                (fs::current_path() / ("env_" + envs[current_env].get_name()) / 
                                ("task_" + envs[current_env].get_tasks()[current_task].get_name()) /
                                envs[current_env].get_tasks()[current_task].get_name()).string());
            }
            auto time_start = std::chrono::high_resolution_clock::now();
            error_code = system(command.c_str());
            auto time_finish = std::chrono::high_resolution_clock::now();
            f.open(temp_file_path);
            if (f.is_open()) {
                while (std::getline(f, buf))
                    std::cout << buf << '\n';
                f.close();
            }
            if (error_code) {
                std::cout << "\033[31m" << "-- Runtime error!" << "\033[0m" << std::endl;
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
                    ++errors;
                }
            }
            std::cout << "\033[35m" << "-- Time elapsed:" <<
                std::chrono::duration_cast<std::chrono::duration<double>>(time_finish - time_start).count() <<
                "\033[0m" << std::endl;
            std::cout << "\033[33m" << "-- End of test " << in_file << "\033[0m" << std::endl;
        }
        if (errors == 0) {
            std::cout << "\033[32;1m" << "-- Test command: All " << std::size(in_files) <<
                " tests successfully passed!" << "\033[0m\n";
        } else {
            std::cout << "\033[31;1m" << "-- Test command: Warning! " << errors <<
                "/" << std::size(in_files) << " tests failed!" << "\033[0m\n";
        }
        return errors;
    });

    add_command(State::TASK, "ct", "Create test",
    [this](std::vector <std::string> &arg) -> int {
        if (arg.size() != 2)
            throw std::runtime_error("Incorrect arguments for command " + arg[0]);
        fs::path file_path = fs::current_path() / ("env_" + envs[current_env].get_name()) /
            ("task_" + envs[current_env].get_tasks()[current_task].get_name()) /
            "tests" / (arg[1] + ".in");
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
        if (arg.size() != 2)
            throw std::runtime_error("Incorrect arguments for command " + arg[0]);
        fs::path in_path = fs::current_path() / ("env_" + envs[current_env].get_name()) /
            ("task_" + envs[current_env].get_tasks()[current_task].get_name()) /
            "tests" / (arg[1] + ".in");
        fs::path out_path = fs::current_path() / ("env_" + envs[current_env].get_name()) /
            ("task_" + envs[current_env].get_tasks()[current_task].get_name()) /
            "tests" / (arg[1] + ".out");
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
            throw std::runtime_error("Incorrect arguments for command " + arg[0]);
        fs::path file_path = fs::current_path() / ("env_" + envs[current_env].get_name()) /
            ("task_" + envs[current_env].get_tasks()[current_task].get_name()) /
            "tests" / (arg[1] + ".in");
        if (fs::exists(file_path)) {
            fs::remove(file_path);
        }
        file_path = fs::current_path() / ("env_" + envs[current_env].get_name()) /
            ("task_" + envs[current_env].get_tasks()[current_task].get_name()) /
            "tests" / (arg[1] + ".out");
        if (fs::exists(file_path)) {
            fs::remove(file_path);
        }
        return 0;
    });

    add_command(State::TASK, "et", "Edit test",
    [this](std::vector <std::string> &arg) -> int {
        if (arg.size() != 2)
            throw std::runtime_error("Incorrect arguments for command " + arg[0]);
        fs::path file_path = fs::current_path() / ("env_" + envs[current_env].get_name()) /
            ("task_" + envs[current_env].get_tasks()[current_task].get_name()) /
            "tests" / arg[1];
        if (fs::exists(file_path)) {
            return -1;
        }
        std::string command = "";
        std::cout << "gce: " << global_settings["editor"] << '\n';
        if (global_settings.find("editor") != global_settings.end())
            command = global_settings["editor"];
        if (envs[current_env].get_settings().find("editor") != envs[current_env].get_settings().end())
            command = envs[current_env].get_settings()["editor"];
        size_t pos = std::string::npos;
        while ((pos = command.find("@name@")) != std::string::npos) {
            command.replace(command.begin() + pos, command.begin() + pos + std::size("@name@") - 1, file_path.string());
        }
        pos = std::string::npos;
        while ((pos = command.find("@lang@")) != std::string::npos) {
            command.replace(command.begin() + pos, command.begin() + pos + std::size("@lang@") - 1, "in");
        }
        std::cout << "cmd: " << command << '\n';
        return system(command.c_str());
    });

    add_command(State::TASK, "lt", "List of tests",
    [this](std::vector <std::string> &arg) -> int {
        if (arg.size() != 1)
            throw std::runtime_error("Incorrect arguments for command " + arg[0]);
        std::vector <fs::path> in_files;
        fs::recursive_directory_iterator it_begin(fs::current_path() / ("env_" + envs[current_env].get_name()) /
            ("task_" + envs[current_env].get_tasks()[current_task].get_name()) / "tests"), it_end;
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
            throw std::runtime_error("Incorrect arguments for command " + arg[0]);
        if (envs[current_env].get_tasks()[current_task].get_settings().find("generator") !=
            envs[current_env].get_tasks()[current_task].get_settings().end()) {
            throw std::runtime_error("Generator is already created");
        }
        std::string lang;
        if (arg.size() == 2)
            lang = arg[1];
        else
            lang = "cpp";
        envs[current_env].get_tasks()[current_task].get_settings()["generator"] = lang;
        fs::path file_path = fs::current_path() / ("env_" + envs[current_env].get_name()) /
            ("task_" + envs[current_env].get_tasks()[current_task].get_name()) /
            "tests" / ("generator." + lang);
        std::string buf;
        std::ofstream f(file_path, std::ios::out);
        if (!f.is_open()) {
            return -1;
        }
        if (envs[current_env].get_settings().find("template_" + lang) != envs[current_env].get_settings().end()) {
            std::ifstream t(envs[current_env].get_settings()["template_" + lang]);
            if (t.is_open()) {
                std::string buf;
                while (std::getline(t, buf))
                    f << buf << '\n';
                t.close();
            } else {
                std::cout << "Unable to open template file\n";
            }
        } else if (global_settings.find("template_" + lang) != global_settings.end()) {
            std::ifstream t(global_settings["template_" + lang]);
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
        return 0;
    });

    add_command(State::TASK, "rg", "Remove generator",
    [this](std::vector <std::string> &arg) -> int {
        if (arg.size() != 1)
            throw std::runtime_error("Incorrect arguments for command " + arg[0]);
        auto it = envs[current_env].get_tasks()[current_task].get_settings().find("generator");
        if (it == envs[current_env].get_tasks()[current_task].get_settings().end())
            throw std::runtime_error("Generator doesn't exist");
        std::string lang = (*it).second;
        envs[current_env].get_tasks()[current_task].get_settings().erase(it);
        fs::path file_path = fs::current_path() / ("env_" + envs[current_env].get_name()) /
            ("task_" + envs[current_env].get_tasks()[current_task].get_name()) /
            "tests" / ("generator." + lang);
        if (fs::exists(file_path)) {
            fs::remove(file_path);
        }
        return 0;
    });

    add_command(State::TASK, "sg", "Set generator",
    [this](std::vector <std::string> &arg) -> int {
        if (arg.size() != 1)
            throw std::runtime_error("Incorrect arguments for command " + arg[0]);
        if (envs[current_env].get_tasks()[current_task].get_settings().find("generator") !=
            envs[current_env].get_tasks()[current_task].get_settings().end()) {
            current_state = State::GENERATOR;
        } else {
            throw std::runtime_error("Generator doesn't exist");
        }
        return 0;
    });

    add_command(State::TASK, "cr", "Compile & Run",
    [this](std::vector <std::string> &arg) -> int {
        if (arg.size() != 1)
            throw std::runtime_error("Incorrect arguments for command " + arg[0]);
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
        if (arg.size() != 1)
            throw std::runtime_error("Incorrect arguments for command " + arg[0]);
        std::vector <std::string> args;
        args.push_back("c");
        int res = commands[current_state]["c"](args);
        if (res == 0) {
            args.pop_back();
            args.push_back("t");
            res = commands[current_state]["t"](args);
        }
        return res;
    });

    add_command(State::TASK, "ctr", "Compile, Test & Run",
    [this](std::vector <std::string> &arg) -> int {
        if (arg.size() != 1)
            throw std::runtime_error("Incorrect arguments for command " + arg[0]);
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
            throw std::runtime_error("Incorrect arguments for command " + arg[0]);
        std::string command = global_settings["python_interpreter"] +
            " scripts/parser.py" + " run " +
            // Path to tests directory
            (fs::current_path() / ("env_" + envs[current_env].get_name()) /
            ("task_" + envs[current_env].get_tasks()[current_task].get_name()) /
            "tests").string() + " " +
            // Link to page with tests
            "\"" + arg[1] + "\"";
        return system(command.c_str());
    });

    add_command(State::TASK, "co", "Create output",
    [this](std::vector <std::string> &arg) -> int {
        if (arg.size() != 2)
            throw std::runtime_error("Incorrect arguments for command " + arg[0]);
        fs::path file_path = fs::current_path() / ("env_" + envs[current_env].get_name()) /
            ("task_" + envs[current_env].get_tasks()[current_task].get_name()) /
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
            throw std::runtime_error("Incorrect arguments for command " + arg[0]);
        fs::path file_path = fs::current_path() / ("env_" + envs[current_env].get_name()) /
            ("task_" + envs[current_env].get_tasks()[current_task].get_name()) /
            "tests" / (arg[1] + ".out");
        if (fs::exists(file_path)) {
            fs::remove(file_path);
        }
        return 0;
    });

    add_command(State::TASK, "eo", "Edit output",
    [this](std::vector <std::string> &arg) -> int {
        if (arg.size() != 2)
            throw std::runtime_error("Incorrect arguments for command " + arg[0]);
        fs::path file_path = fs::current_path() / ("env_" + envs[current_env].get_name()) /
            ("task_" + envs[current_env].get_tasks()[current_task].get_name()) /
            "tests" / arg[1];
        if (fs::exists(file_path)) {
            return -1;
        }
        std::string command = "";
        std::cout << "gce: " << global_settings["editor"] << '\n';
        if (global_settings.find("editor") != global_settings.end())
            command = global_settings["editor"];
        if (envs[current_env].get_settings().find("editor") != envs[current_env].get_settings().end())
            command = envs[current_env].get_settings()["editor"];
        size_t pos = std::string::npos;
        while ((pos = command.find("@name@")) != std::string::npos) {
            command.replace(command.begin() + pos, command.begin() + pos + std::size("@name@") - 1, file_path.string());
        }
        pos = std::string::npos;
        while ((pos = command.find("@lang@")) != std::string::npos) {
            command.replace(command.begin() + pos, command.begin() + pos + std::size("@lang@") - 1, "out");
        }
        std::cout << "cmd: " << command << '\n';
        return system(command.c_str());
    });

    add_command(State::TASK, "set", "Configure settings",
    [this](std::vector <std::string> &arg) -> int {
        if (arg.size() == 2) {
            envs[current_env].get_tasks()[current_task].get_settings().erase(arg[1]);
            return 0;
        }
        if (arg.size() >= 3) {
            std::string second_arg = arg[2];
            for (unsigned i = 3; i < arg.size(); ++i) {
                second_arg.push_back(' ');
                second_arg += arg[i];
            }
            envs[current_env].get_tasks()[current_task].get_settings().erase(arg[1]);
            envs[current_env].get_tasks()[current_task].get_settings().emplace(arg[1], second_arg);
            return 0;
        }
        throw std::runtime_error("Incorrect arguments for command " + arg[0]);
    });

    add_command(State::TASK, "edit", "Edit task in text editor",
    [this](std::vector <std::string> &arg) -> int {
        if (arg.size() != 1)
            throw std::runtime_error("Incorrect arguments for command " + arg[0]);
        std::string command = "";
        std::cout << "gce: " << global_settings["editor"] << '\n';
        if (global_settings.find("editor") != global_settings.end())
            command = global_settings["editor"];
        if (envs[current_env].get_settings().find("editor") != envs[current_env].get_settings().end())
            command = envs[current_env].get_settings()["editor"];
        size_t pos = std::string::npos;
        while ((pos = command.find("@name@")) != std::string::npos) {
            command.replace(command.begin() + pos, command.begin() + pos + std::size("@name@") - 1,
                            (fs::current_path() / ("env_" + envs[current_env].get_name()) / 
                            ("task_" + envs[current_env].get_tasks()[current_task].get_name()) /
                            envs[current_env].get_tasks()[current_task].get_name()).string());
        }
        pos = std::string::npos;
        while ((pos = command.find("@lang@")) != std::string::npos) {
            command.replace(command.begin() + pos, command.begin() + pos + std::size("@lang@") - 1,
                            envs[current_env].get_tasks()[current_task].get_settings()["language"]);
        }
        std::cout << "cmd: " << command << '\n';
        auto ampersand_pos = command.find("&");
        #ifdef _WIN32
        if (ampersand_pos != std::string::npos) {
            command.erase(ampersand_pos);
            command += " > NUL";
            std::thread thr([&](const std::string command) -> void {
                int res = system(command.c_str());
                (void)res;
            }, command);
            thr.detach();
            return 0;
        } else {
            return system(command.c_str());
        }
        #else
        if (ampersand_pos != std::string::npos) {
            command.insert(std::max(0ul, ampersand_pos - 1), " &> /dev/null ");
        }
        return system(command.c_str());
        #endif  // _WIN32
    });

    add_command(State::TASK, "q", "Exit from task", [this](std::vector <std::string> &arg) -> int {
        if (arg.size() != 1)
            throw std::runtime_error("Incorrect arguments for command " + arg[0]);
        current_task = -1;
        current_state = State::ENVIRONMENT;
        if (global_settings["autosave"] == "on") {
            std::vector <std::string> save_args = {"s"};
            return commands[State::GLOBAL][save_args.front()](save_args);
        } else {
            return 0;
        }
    });
    add_alias(State::TASK, "q", State::TASK, "exit");

    add_alias(State::GLOBAL, "help", State::TASK, "help");
    add_alias(State::GLOBAL, "help", State::TASK, "?");
}
