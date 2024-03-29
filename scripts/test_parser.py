import base64
import os
import parser
import shutil
import tempfile


def assert_eq(expected, result):
    if expected == result:
        print(f"\033[32massert_eq({expected}, {result}): OK\033[0m")
        return 0
    else:
        print(f"\033[31massert_eq({expected}, {result}): FAILED\033[0m")
        return 1


def test():
    temp_dir = tempfile.mkdtemp()

    link = base64.b64decode("aHR0cDovL2NvZGVmb3JjZXMuY29tL2NvbnRlc3QvMTE4MS9wcm9ibGVtL0E=").decode("utf-8")
    print("Test: " + link)
    parser.run(temp_dir, link)
    errors = 0
    file_name = "cf_sample_1.in"
    with open(os.path.join(temp_dir, file_name), "r") as f:
        print("Checking " + file_name)
        errors += assert_eq("5 4 3".split(), f.read().split())
    file_name = "cf_sample_1.out"
    with open(os.path.join(temp_dir, file_name), "r") as f:
        print("Checking " + file_name)
        errors += assert_eq("3 1".split(), f.read().split())
    file_name = "cf_sample_2.in"
    with open(os.path.join(temp_dir, file_name), "r") as f:
        print("Checking " + file_name)
        errors += assert_eq("6 8 2".split(), f.read().split())
    file_name = "cf_sample_2.out"
    with open(os.path.join(temp_dir, file_name), "r") as f:
        print("Checking " + file_name)
        errors += assert_eq("7 0".split(), f.read().split())

    link = base64.b64decode("aHR0cDovL2NvZGVmb3JjZXMuY29tL2NvbnRlc3QvMS9wcm9ibGVtL0I=").decode("utf-8")
    print("Test: " + link)
    parser.run(temp_dir, link)
    file_name = "cf_sample_1.in"
    with open(os.path.join(temp_dir, file_name), "r") as f:
        print("Checking " + file_name)
        errors += assert_eq("2\nR23C55\nBC23".split(), f.read().split())
    file_name = "cf_sample_1.out"
    with open(os.path.join(temp_dir, file_name), "r") as f:
        print("Checking " + file_name)
        errors += assert_eq("BC23\nR23C55".split(), f.read().split())

    link = base64.b64decode("aHR0cDovL2NvZGVmb3JjZXMuY29tL2NvbnRlc3QvNzAwL3Byb2JsZW0vRA==").decode("utf-8")
    print("Test: " + link)
    parser.run(temp_dir, link)
    file_name = "cf_sample_1.in"
    with open(os.path.join(temp_dir, file_name), "r") as f:
        print("Checking " + file_name)
        errors += assert_eq("7\n1 2 1 3 1 2 1\n5\n1 7\n1 3\n3 5\n2 4\n4 4".split(), f.read().split())
    file_name = "cf_sample_1.out"
    with open(os.path.join(temp_dir, file_name), "r") as f:
        print("Checking " + file_name)
        errors += assert_eq("10\n3\n3\n5\n0".split(), f.read().split())

    link = base64.b64decode("aHR0cHM6Ly9jb2RlZm9yY2VzLmNvbS9jb250ZXN0LzE3NDIvcHJvYmxlbS9B").decode("utf-8")
    print("Test: " + link)
    parser.run(temp_dir, link)
    file_name = "cf_sample_1.in"
    with open(os.path.join(temp_dir, file_name), "r") as f:
        print("Checking " + file_name)
        errors += assert_eq("7\n1 4 3\n2 5 8\n9 11 20\n0 0 0\n20 20 20\n4 12 3\n15 7 8".split(), f.read().split())
    file_name = "cf_sample_1.out"
    with open(os.path.join(temp_dir, file_name), "r") as f:
        print("Checking " + file_name)
        errors += assert_eq("YES\nNO\nYES\nYES\nNO\nNO\nYES".split(), f.read().split())

    link = base64.b64decode("aHR0cDovL2FjbXAucnUvaW5kZXguYXNwP21haW49dGFzayZpZF90YXNrPTc=").decode("utf-8")
    print("Test: " + link)
    parser.run(temp_dir, link)
    file_name = "acmp_sample_1.in"
    with open(os.path.join(temp_dir, file_name), "r") as f:
        print("Checking " + file_name)
        errors += assert_eq("5 7 3".split(), f.read().split())
    file_name = "acmp_sample_1.out"
    with open(os.path.join(temp_dir, file_name), "r") as f:
        print("Checking " + file_name)
        errors += assert_eq("7".split(), f.read().split())
    file_name = "acmp_sample_2.in"
    with open(os.path.join(temp_dir, file_name), "r") as f:
        print("Checking " + file_name)
        errors += assert_eq("987531 234 86364".split(), f.read().split())
    file_name = "acmp_sample_2.out"
    with open(os.path.join(temp_dir, file_name), "r") as f:
        print("Checking " + file_name)
        errors += assert_eq("987531".split(), f.read().split())
    file_name = "acmp_sample_3.in"
    with open(os.path.join(temp_dir, file_name), "r") as f:
        print("Checking " + file_name)
        errors += assert_eq("189285 283 4958439238923098349024".split(), f.read().split())
    file_name = "acmp_sample_3.out"
    with open(os.path.join(temp_dir, file_name), "r") as f:
        print("Checking " + file_name)
        errors += assert_eq("4958439238923098349024".split(), f.read().split())

    link = base64.b64decode("aHR0cDovL2FjbXAucnUvaW5kZXguYXNwP21haW49dGFzayZpZF90YXNrPTQzMg==").decode("utf-8")
    print("Test: " + link)
    parser.run(temp_dir, link)
    file_name = "acmp_sample_1.in"
    with open(os.path.join(temp_dir, file_name), "r") as f:
        print("Checking " + file_name)
        errors += assert_eq(
            "5 10\n##......#.\n.#..#...#.\n.###....#.\n..##....#.\n........#.".split(), f.read().split())
    file_name = "acmp_sample_1.out"
    with open(os.path.join(temp_dir, file_name), "r") as f:
        print("Checking " + file_name)
        errors += assert_eq("3".split(), f.read().split())
    file_name = "acmp_sample_2.in"
    with open(os.path.join(temp_dir, file_name), "r") as f:
        print("Checking " + file_name)
        errors += assert_eq(
            "5 10\n##..#####.\n.#.#.#....\n###..##.#.\n..##.....#\n.###.#####".split(), f.read().split())
    file_name = "acmp_sample_2.out"
    with open(os.path.join(temp_dir, file_name), "r") as f:
        print("Checking " + file_name)
        errors += assert_eq("5".split(), f.read().split())

    link = base64.b64decode("aHR0cDovL2FjbXAucnUvaW5kZXguYXNwP21haW49dGFzayZpZF90YXNrPTIzOQ==").decode("utf-8")
    print("Test: " + link)
    parser.run(temp_dir, link)
    file_name = "acmp_sample_1.in"
    with open(os.path.join(temp_dir, file_name), "r") as f:
        print("Checking " + file_name)
        errors += assert_eq("4 3 3\n2 2 2\n2 0 0\n2 1 2\n2 2 2\n2 10 0 0\n1 5 1\n4 6 0 0 1".split(), f.read().split())
    file_name = "acmp_sample_1.out"
    with open(os.path.join(temp_dir, file_name), "r") as f:
        print("Checking " + file_name)
        errors += assert_eq("15".split(), f.read().split())

    link = base64.b64decode("aHR0cDovL2FjbS50aW11cy5ydS9wcm9ibGVtLmFzcHg/c3BhY2U9MSZudW09MTAwMQ==").decode("utf-8")
    print("Test: " + link)
    parser.run(temp_dir, link)
    file_name = "timus_sample_1.in"
    with open(os.path.join(temp_dir, file_name), "r") as f:
        print("Checking " + file_name)
        errors += assert_eq(" 1427  0   \n   876652098643267843 \n5276538\n  \n   ".split(), f.read().split())
    file_name = "timus_sample_1.out"
    with open(os.path.join(temp_dir, file_name), "r") as f:
        print("Checking " + file_name)
        errors += assert_eq("2297.0716\n936297014.1164\n0.0000\n37.7757".split(), f.read().split())

    link = base64.b64decode("aHR0cDovL2FjbS50aW11cy5ydS9wcm9ibGVtLmFzcHg/c3BhY2U9MSZudW09MTIzMg==").decode("utf-8")
    print("Test: " + link)
    parser.run(temp_dir, link)
    file_name = "timus_sample_1.in"
    with open(os.path.join(temp_dir, file_name), "r") as f:
        print("Checking " + file_name)
        errors += assert_eq("11 5 2".split(), f.read().split())
    file_name = "timus_sample_1.out"
    with open(os.path.join(temp_dir, file_name), "r") as f:
        print("Checking " + file_name)
        errors += assert_eq("3\n0 3 7\n3 3 3\n3 –1 0".split(), f.read().split())

    link = base64.b64decode("aHR0cDovL2FjbS50aW11cy5ydS9wcm9ibGVtLmFzcHg/c3BhY2U9MSZudW09MTQxMg==").decode("utf-8")
    print("Test: " + link)
    parser.run(temp_dir, link)
    file_name = "timus_sample_1.in"
    with open(os.path.join(temp_dir, file_name), "r") as f:
        print("Checking " + file_name)
        errors += assert_eq("10 10\n2\n0.5 2\n2 10.1".split(), f.read().split())
    file_name = "timus_sample_1.out"
    with open(os.path.join(temp_dir, file_name), "r") as f:
        print("Checking " + file_name)
        errors += assert_eq("99.666".split(), f.read().split())

    link = base64.b64decode("aHR0cHM6Ly9hdGNvZGVyLmpwL2NvbnRlc3RzL2FiYzE0MS90YXNrcy9hYmMxNDFfYQ==").decode("utf-8")
    print("Test: " + link)
    parser.run(temp_dir, link)
    file_name = "atcoder_sample_1.in"
    with open(os.path.join(temp_dir, file_name), "r") as f:
        print("Checking " + file_name)
        errors += assert_eq("Sunny".split(), f.read().split())
    file_name = "atcoder_sample_1.out"
    with open(os.path.join(temp_dir, file_name), "r") as f:
        print("Checking " + file_name)
        errors += assert_eq("Cloudy".split(), f.read().split())
    file_name = "atcoder_sample_2.in"
    with open(os.path.join(temp_dir, file_name), "r") as f:
        print("Checking " + file_name)
        errors += assert_eq("Rainy".split(), f.read().split())
    file_name = "atcoder_sample_2.out"
    with open(os.path.join(temp_dir, file_name), "r") as f:
        print("Checking " + file_name)
        errors += assert_eq("Sunny".split(), f.read().split())

    link = base64.b64decode("aHR0cHM6Ly9hdGNvZGVyLmpwL2NvbnRlc3RzL2FiYzE0MS90YXNrcy9hYmMxNDFfYw==").decode("utf-8")
    print("Test: " + link)
    parser.run(temp_dir, link)
    file_name = "atcoder_sample_1.in"
    with open(os.path.join(temp_dir, file_name), "r") as f:
        print("Checking " + file_name)
        errors += assert_eq("6 3 4\n3\n1\n3\n2".split(), f.read().split())
    file_name = "atcoder_sample_1.out"
    with open(os.path.join(temp_dir, file_name), "r") as f:
        print("Checking " + file_name)
        errors += assert_eq("No\nNo\nYes\nNo\nNo\nNo".split(), f.read().split())
    file_name = "atcoder_sample_2.in"
    with open(os.path.join(temp_dir, file_name), "r") as f:
        print("Checking " + file_name)
        errors += assert_eq("6 5 4\n3\n1\n3\n2".split(), f.read().split())
    file_name = "atcoder_sample_2.out"
    with open(os.path.join(temp_dir, file_name), "r") as f:
        print("Checking " + file_name)
        errors += assert_eq("Yes\nYes\nYes\nYes\nYes\nYes".split(), f.read().split())
    file_name = "atcoder_sample_3.in"
    with open(os.path.join(temp_dir, file_name), "r") as f:
        print("Checking " + file_name)
        errors += assert_eq("10 13 15\n3\n1\n4\n1\n5\n9\n2\n6\n5\n3\n5\n8\n9\n7\n9".split(), f.read().split())
    file_name = "atcoder_sample_3.out"
    with open(os.path.join(temp_dir, file_name), "r") as f:
        print("Checking " + file_name)
        errors += assert_eq("No\nNo\nNo\nNo\nYes\nNo\nNo\nNo\nYes\nNo".split(), f.read().split())

    shutil.rmtree(temp_dir)
    exit(errors)
