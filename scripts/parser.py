import sys
from bs4 import BeautifulSoup
import requests
import os

def run():
    path = sys.argv[2]
    link = sys.argv[3]
    if link.startswith("https://codeforces.com/"):
        page = requests.get(link).text
        soup = BeautifulSoup(page, "html.parser")
        inputs = soup.find_all("div", class_="input")
        outputs = soup.find_all("div", class_="output")
        if len(inputs) != len(outputs):
            print("Numbers of inputs and outputs are not equal")
            exit(-1)
        for i in range(len(inputs)):
            with open(os.path.join(path, "cf_sample_" + str(i + 1)) + ".in", "w") as f:
                f.write(inputs[i].contents[-1].contents[0])
            with open(os.path.join(path, "cf_sample_" + str(i + 1)) + ".out", "w") as f:
                f.write(outputs[i].contents[-1].contents[0])
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
    