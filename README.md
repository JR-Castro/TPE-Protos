# TPE - Protocolos de Comunicación - POP3

Implementación de un servidor para el protocolo POP3 (Post Office Protocol versión 3) que pueda ser usado
por Mail User Agents tales como Mozilla Thunderbird, Microsoft Outlook y Evolution para la recepción de correos electrónicos.

## Requerimientos

* atender a múltiples clientes en forma concurrente y simultánea (al menos 500)
* soportar autenticación usuario / contraseña y pipelining

## Construcción y Ejecución

Desde el directorio principal ingrese el siguiente comando:

```bash
cd cmake-build-debug/
```

Luego, debe cambiar de directorio y dirigirse a src:

```bash
cd src/
```

Una vez allí, debe ejecutar el siguiente comando para compilar el servidor pop3:

```bash
./pop3server -d [path]/.mails -u [user]:[password]
```

Donde:
* [path] es el path donde se encuentran los mails a enviar.
* [user] es el usuario que se desea autenticar.
* [password] es la contraseña del usuario que se desea autenticar.

Por último, para conectarse al servidor pop3, debe ejecutar el siguiente comando:

```bash
nc -C -v localhost 1110
```

## Comandos soportados
| Comandos         | Descripción                                         |
|------------------|-----------------------------------------------------|
| -h               | Imprime la ayuda y termina.                         |
| -l \<pop3 addr>  | Dirección donde servirá  el proxy POP3              |
| -L \<conf  addr> | Dirección donde servirá  el servicio de management. |
| -p \<pop3 port>  | Puerto entrante conexiones POP3.                    |
| -o \<conf port>  | Puerto entrante conexiones management.              |
| -u \<user:pass>  | Registra usuario y contraseña de un usuario válido. |
| -d \<directory>  | Directorio donde se almacenarán los mails..         |
| -v               | Imprime información sobre la versión y termina.     |

## Comandos soportados por el cliente de management
| Comandos      | Descripción                                                  |
|---------------|--------------------------------------------------------------|
| help          | Imprime la info de los comandos.                             |
| list          | Retorna la página solicitada del directorio del usuario.     |
| get-page-size | Retorna el número de usuario por página \(max 200).          |
| connections   | Retorna el número de conexiones concurrentes en el servidor. |
| bytes         | Retorna el número bytes enviados por el servidor.            |
| historic      | Retorna el número de requests atendidos por el servidor.     |
| add-user      | Añade un usuario nuevo al servidor.                          |
| del-user      | Borra un usuario del servidor.                               |
| set-page-size | Setea un nuevo número de usuarios por página \(max 200).     |
| stop          | Detiene el servidor.                                         |
