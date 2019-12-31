FROM ubuntu:latest

WORKDIR /app
ADD . /app

RUN apt-get update -y
RUN apt-get install software-properties-common -y
RUN add-apt-repository ppa:ubuntu-toolchain-r/test
RUN apt-get update -y
RUN apt-get install python3 python3-pip git cmake g++-9 -y
RUN update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-9 100 --slave /usr/bin/g++ g++ /usr/bin/g++-9
RUN pip3 install bs4

RUN ["python3", "launch.py", "--f", "build"]

CMD ["python3", "launch.py", "--f", "run", "--r", "config/config.yaml", "config/environments.yaml"]
