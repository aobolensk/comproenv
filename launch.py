import argparse
import datetime
import multiprocessing
import os
import shutil
import signal
import subprocess


def executable(filename):
    if os.name == "nt":
        return filename + ".exe"
    return filename


def enumerate_args(args):
    if args is None:
        return ''
    return ' '.join(args)


def clean(args):
    if os.path.exists("build"):
        shutil.rmtree("build")
    return 0


def backup(args):
    dst_directory = enumerate_args(args.directory)
    if len(dst_directory) == 0:
        print("Failed backup (please, specify destination directory)")
        return 1
    dst_directory = os.path.join(
        os.path.abspath(dst_directory),
        "comproenv_data_backup_%s" % datetime.datetime.now().strftime("%Y-%m-%d_%H-%M-%S"))
    try:
        shutil.copytree("data", dst_directory)
    except IOError as e:
        print("Failed backup: %s" % e)
        return 1
    return 0


def build(args):
    subprocess.call("git submodule update --init --recursive", shell=True)
    if not os.path.exists("build"):
        os.mkdir("build")
    os.chdir("build")
    if args.build_args is not None:
        args.build_args = ["-D" + x for x in args.build_args]
    ret_code = subprocess.call(
        "cmake .. " + enumerate_args(args.build_args),
        shell=True)
    if ret_code != 0:
        print("Failed build (cmake)")
        return 1
    if os.name == "posix":
        ret_code = subprocess.call(
            "cmake --build . --config Release -- -j" + str(multiprocessing.cpu_count()),
            shell=True)
    elif os.name == "nt":
        ret_code = subprocess.call("cmake --build . --config Release")
    if ret_code != 0:
        print("Failed build (cmake --build)")
        return 1
    os.chdir("..")
    return 0


def generate_docs(args):
    with open(os.devnull, 'w+') as devnull:
        proc = subprocess.Popen(
            os.path.join(
                ".", "build", "bin",
                executable("generate_docs")) + " docs",
            shell=True, stdout=devnull, stdin=devnull)
        return proc.wait()


def run(args):
    signal.signal(signal.SIGINT, signal.SIG_IGN)
    if os.name == "posix":
        signal.signal(signal.SIGTSTP, signal.SIG_IGN)
    ret_code = subprocess.call(
        os.path.join(
            ".", "build", "bin",
            executable("comproenv") + ' ' + enumerate_args(args.run_args)),
        shell=True)
    signal.signal(signal.SIGINT, signal.SIG_DFL)
    if os.name == "posix":
        signal.signal(signal.SIGTSTP, signal.SIG_DFL)
    return ret_code


def parse_args():
    parser = argparse.ArgumentParser()
    parser.add_argument("--function", nargs='*', help="Provide function name")
    parser.add_argument("--build_args", nargs='*', help="Args for build system")
    parser.add_argument("--run_args", nargs='*', help="Args for application")
    parser.add_argument("--directory", nargs='*', help="Directory for backup")
    args = parser.parse_args()
    if args.function is None:
        parser.print_help()
        exit(1)
    print(args)
    return args


def main():
    args = parse_args()
    ret_code = 0
    for arg in args.function:
        if arg == "backup":
            ret_code = backup(args)
        elif arg == "build":
            ret_code = build(args)
        elif arg == "clean":
            ret_code = clean(args)
        elif arg == "docs":
            ret_code = generate_docs(args)
        elif arg == "run":
            ret_code = run(args)
        else:
            print("Unknown function argument: " + arg)
            continue
        print("Function " + arg + " returned exit code " + str(ret_code))
        if (ret_code != 0):
            exit(1)
    exit(0)


if __name__ == "__main__":
    main()
