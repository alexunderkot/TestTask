1. Джедай может скопировать репозиторий через конольную команду в нужную папку: "git clone https://github.com/alexunderkot/TestTask.git"
2. После открыть корневую папку в VS code
3. Джедай нажмёт F1 ==> Dev Containers: Reopen in Container
4. Откройте консоль в корневой папке и напишите команду для компиляции: "docker build -t cpp-echo-server -f .devcontainer/Dockerfile ."
5. Затем введите команду, чтобы запустить сервер: "docker run -p 8080:8080 cpp-echo-server"
6. Джедай откроет новую консоль и введёт команду для подключения: "curl http://localhost:8080"
