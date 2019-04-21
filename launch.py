import os, subprocess, argparse

def executable(filename):
    if os.name == "nt":
        return filename + ".exe"
    return filename

def build():
    if not os.path.exists("build"):
        os.mkdir("build")
    os.chdir("build")
    ret_code = subprocess.call("cmake ..", shell=True)
    if ret_code != 0:
        print("Failed build (cmake)")
        return 1
    if os.name == "posix":
        ret_code = subprocess.call("cmake --build . --config Release -- -j" +
                                    str(multiprocessing.cpu_count()))
    elif os.name == "nt":
        ret_code = subprocess.call("cmake --build . --config Release")
    if ret_code != 0:
        print("Failed build (cmake --build)")
        return 1
    os.chdir("..")
    return 0

def parse_args():
    parser = argparse.ArgumentParser()
    parser.add_argument("--function", nargs='*', help="Provide function name")
    args = parser.parse_args()
    print(args)
    return args

def main():
    args = parse_args()
    ret_code = 0
    for arg in args.function:
        if arg == "build":
            ret_code = build()
        if arg == "run":
            ret_code = subprocess.call(
                os.path.join(os.getcwd(), "build", "bin", executable("main")), shell=True)
        print("Function " + arg + " returned exit code " + str(ret_code))
        if (ret_code != 0):
            exit(1)
    exit(0)

if __name__ == "__main__":
    main()