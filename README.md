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

## Comandos soportados por el cliente
| Comandos         | Descripción                                         |
|------------------|-----------------------------------------------------|
| USER \<username> | Autentica al usuario con el nombre de usuario.      |
| PASS \<password> | Autentica al usuario con la contraseña.             |
| STAT             | Devuelve la cantidad de mails y el tamaño total.    |
| LIST             | Devuelve la cantidad de mails y el tamaño total.    |
| LIST \<id>       | Devuelve el tamaño del mail con el id especificado. |
| RETR \<id>       | Devuelve el mail con el id especificado.            |
| DELE \<id>       | Marca el mail con el id especificado para borrarlo. |
| NOOP             | No hace nada.                                       |
| RSET             | Desmarca todos los mails marcados para borrarse.    |
| QUIT             | Cierra la conexión.                                 |
| CAPA             | Devuelve la lista de comandos soportados.           |

## Comandos soportados por el cliente de management
| Comandos         | Descripción                             |
|------------------|-----------------------------------------|
| GET_USERS        | Lista de usuarios.                      |
| GET_HISTORIC     | Cantidad de conexiones históricas.      |
| GET_CONCURRENT   | Cantidad de conexiones concurrentes.    |
| GET_BYTES        | Cantidad de bytes transferidos.         |
| STOP             | Detiene el servidor.                    |
| ADD_USER         | Agrega un usuario.                      |
| DEL_USER         | Elimina un usuario.                     |
| CHANGE_PAGE_SIZE | Cambia el tamaño de página de usuarios. |
| HELP             | Lista de comandos.                      |
