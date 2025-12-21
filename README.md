1. Скопировать репозиторий через конольную команду в нужную папку: "git clone https://github.com/alexunderkot/TestTask.git"
2. Открыть корневую папку в VS code
3. Нажмите F1 ==> Dev Containers: Reopen in Container
4. Откройте консоль в корневой папке и напишите команду для компиляции: "docker build -t cpp-echo-server -f .devcontainer/Dockerfile ."
5. Затем введите команду, чтобы запустить сервер: "docker run -p 8080:8080 cpp-echo-server"
6. Откройте новую консоль и введите команду для подключения: "curl http://localhost:8080"
