# MiniHTTPd - Servidor Web EPN

Proyecto integrador de Redes de Computadoras. Es un servidor HTTP/1.1 desarrollado en C para Linux, utilizando sockets POSIX, E/S no bloqueante con `epoll` (*Edge-Triggered*) y soporte de ejecución en segundo plano (daemon).

## Estructura del proyecto

```text
minihttpd/
├── Makefile
├── README.md
├── include/
│   ├── files.h
│   ├── http.h
│   ├── mime.h
│   └── server.h
├── src/
│   ├── files.c
│   ├── http.c
│   ├── main.c
│   ├── mime.c
│   └── server.c
└── www/
    ├── image.png
    ├── index.html
    └── style.css
```

## Arquitectura de módulos

* **`server` (`server.h` / `server.c`):** Creación de sockets pasivos TCP (`passivesock`), demonización del proceso mediante doble `fork()` y `setsid()`, y bucle de eventos asíncrono con `epoll` (`EPOLLET`) sobre sockets no bloqueantes (`O_NONBLOCK`).
* **`http` (`http.h` / `http.c`):** Parseo de peticiones `GET`, mapeo de `/` a `index.html` y protección contra *Directory Traversal* mediante validación de rutas canónicas con `realpath()`.
* **`files` (`files.h` / `files.c`):** Obtención de tamaño de archivo (`stat()`), envío completo de bytes sobre sockets no bloqueantes (`send_all`) y generación de respuestas HTTP (`200`, `400`, `403`, `404`, `405`, `500`).
* **`mime` (`mime.h` / `mime.c`):** Resolución dinámica del encabezado `Content-Type` (`text/html`, `text/css`, `image/png`, etc.) según la extensión del archivo solicitado.

## Compilación y ejecución

1. **Compilar el proyecto:**
   ```bash
   make
   ```

2. **Ejecutar el servidor:**
   * Modo primer plano (*foreground*):
     ```bash
     ./minihttpd 8080
     ```
   * Modo demonio (*background*):
     ```bash
     ./minihttpd 8080 -d
     ```

3. **Probar en el navegador o terminal:**
   * Navegador: `http://localhost:8080/`
   * Terminal:
     ```bash
     curl -i http://localhost:8080/
     curl -i http://localhost:8080/style.css
     curl -i http://localhost:8080/inexistente
     ```

4. **Detener el demonio y limpiar binarios:**
   ```bash
   pkill minihttpd
   make clean
   ```
