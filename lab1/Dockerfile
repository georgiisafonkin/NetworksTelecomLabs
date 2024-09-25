FROM debian:latest

WORKDIR /app

COPY . .

RUN apt-get update -y && apt-get upgrade -y && apt-get install gcc -y
RUN gcc -o main main.c -pthread

CMD ["./main", "239.255.255.250", "214.255.1.3", "8080"]