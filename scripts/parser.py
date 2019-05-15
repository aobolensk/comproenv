import sys
from bs4 import BeautifulSoup
import requests
import os

def remove_empty_lines(s):
    i = 0
    while i < len(s) and s[i] == '\n':
        i += 1
    s = s[i:]
    i = len(s) - 1
    while i >= 0 and s[i] == '\n':
        i -= 1
    s = s[:i+1]
    i = len(s)
    while True:
        s = s.replace('\n\n', '\n')
        if i == len(s):
            break
        i = len(s)
    return s

def run():
    path = sys.argv[2]
    link = sys.argv[3]
    if (link.startswith("http://codeforces.com/") or
        link.startswith("https://codeforces.com/")):
        page = requests.get(link).text
        soup = BeautifulSoup(page, "html.parser")
        inputs = soup.find_all("div", class_="input")
        outputs = soup.find_all("div", class_="output")
        if len(inputs) != len(outputs):
            print("Numbers of inputs and outputs are not equal")
            exit(-1)
        for i in range(len(inputs)):
            with open(os.path.join(path, "cf_sample_" + str(i + 1)) + ".in", "w") as f:
                content = inputs[i].contents[-1].contents
                result = ""
                for x in content:
                    if isinstance(x, str):
                        result += x
                    else:
                        result += '\n'
                result = remove_empty_lines(result)
                f.write(result)
            with open(os.path.join(path, "cf_sample_" + str(i + 1)) + ".out", "w") as f:
                content = outputs[i].contents[-1].contents
                result = ""
                for x in content:
                    if isinstance(x, str):
                        result += x
                    else:
                        result += '\n'
                result = remove_empty_lines(result)
                f.write(result)
    elif (link.startswith("http://acmp.ru/") or
        link.startswith("https://acmp.ru/")):
        page = requests.get(link).text
        soup = BeautifulSoup(page, "html.parser")
        tables = soup.find_all("table", {"class" : "main",
                                        "cellpadding" : "2",
                                        "cellspacing" : "1"})
        for table in tables:
            rows = table.find_all("tr")
            cells = rows[0].find_all("th")
            check = 2
            for cell in cells:
                if cell.contents[0].find("INPUT.TXT") != -1:
                    check -= 1
                elif cell.contents[0].find("OUTPUT.TXT") != -1:
                    check -= 1
            if (check == 0):
                for i in range(1, len(rows)):
                    cells = rows[i].find_all("td")
                    with open(os.path.join(path, "acmp_sample_" + str(i)) + ".in", "w") as f:
                        result = ""
                        for x in cells[1].contents:
                            if (isinstance(x, str)):
                                result += x
                            else:
                                result += '\n'
                        result = remove_empty_lines(result)
                        f.write(result)
                    with open(os.path.join(path, "acmp_sample_" + str(i)) + ".out", "w") as f:
                        result = ""
                        for x in cells[2].contents:
                            if (isinstance(x, str)):
                                result += x
                            else:
                                result += '\n'
                        result = remove_empty_lines(result)
                        f.write(result)
    elif (link.startswith("http://acm.timus.ru/") or
        link.startswith("https://acm.timus.ru/") or
        link.startswith("http://timus.online/") or
        link.startswith("https://timus.online/")):
        page = requests.get(link).text
        soup = BeautifulSoup(page, "html.parser")
        tables = soup.find_all("table", class_="sample")
        for table in tables:
            rows = table.find_all("tr")
            cells = rows[0].find_all("th")
            check = 2
            for cell in cells:
                if cell.contents[0].find("input") != -1:
                    check -= 1
                elif cell.contents[0].find("output") != -1:
                    check -= 1
            if (check == 0):
                for i in range(1, len(rows)):
                    cells = rows[i].find_all("td")
                    with open(os.path.join(path, "timus_sample_" + str(i)) + ".in", "w") as f:
                        f.write(cells[0].get_text())
                    with open(os.path.join(path, "timus_sample_" + str(i)) + ".out", "w") as f:
                        f.write(cells[1].get_text())
    else:
        print("Couldn't parse contents of this page")
        exit(-1)

def test():
    raise NotImplementedError

def parse_args():
    if sys.argv[1] == "run":
        if len(sys.argv) <= 3:
            print("Not enough args to execute 'run'")
            exit(-1)
        run()
    elif sys.argv[1] == "test":
        test()

if __name__ == "__main__":
    print(sys.argv)
    if len(sys.argv) == 1:
        print("Not enough args")
        exit(-1)
    parse_args()