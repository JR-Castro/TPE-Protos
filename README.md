# TPE - Protocolos de Comunicación - POP3

Implementación de un servidor para el protocolo POP3 (Post Office Protocol versión 3) que pueda ser usado
por Mail User Agents tales como Mozilla Thunderbird, Microsoft Outlook y Evolution para la recepción de correos electrónicos.

## Requerimientos

* atender a múltiples clientes en forma concurrente y simultánea (al menos 500)
* soportar autenticación usuario / contraseña y pipelining

## Construcción y Ejecución

Desde el directorio principal ingrese el siguiente comando:

```bash
cmake -S . -B bin/
```

Luego, debe cambiar de directorio y dirigirse a bin:

```bash
cd bin
```

Para compilar el servidor pop3, usar:

```bash
make pop3server
```

y para compilar el cliente:

```bash
make managerclient
```

Una vez ejecutado, debe ejecutar el siguiente comando para compilar el servidor pop3:

```bash
./stc/pop3server -d [path]/.mails -u [user]:[password]
```

Donde:
* [path] es el path donde se encuentran los mails a enviar.
* [user] es el usuario que se desea autenticar.
* [password] es la contraseña del usuario que se desea autenticar.
* Por default, escucha en la address :: y puerto 1110.

Para ejecutar el manager, usar:

```bash
./src/managerclient
```

## Comandos soportados
| Comandos         | Descripción                                         |
|------------------|-----------------------------------------------------|
| -h               | Imprime la ayuda y termina.                         |
| -l \<pop3 addr>  | Dirección donde servirá  el proxy POP3.             |
| -L \<conf  addr> | Dirección donde servirá  el servicio de management. |
| -p \<pop3 port>  | Puerto entrante conexiones POP3.                    |
| -o \<conf port>  | Puerto entrante conexiones management.              |
| -u \<user:pass>  | Registra usuario y contraseña de un usuario válido. |
| -d \<directory>  | Directorio donde se almacenarán los mails.          |
| -v               | Imprime información sobre la versión y termina.     |

## Comandos soportados por el cliente de management
Tener en consideración que estos comandos son case sensitive.
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
