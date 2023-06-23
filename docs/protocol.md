# Introducción

Este protocolo permite a un administrador monitorear métricas, modificar algunos valores de configuración y detener el servidor. Es binario, no orientado a sesión el cual funciona sobre UDP.

Para poder usarlo, se debe pasar como argumento al iniciar el servidor el token que se va a utilizar para la autenticación. Debe ocupar 4 bytes.

## Especificaciones

### Requests al servidor

El protocolo usa un header de 10 bytes al principio del datagrama, donde se especifica la información del mensaje, y luego tiene la información del mensaje en sí, de tamaño variable. Todo debe ser codificado en big-endian.

| **HEADER** | **DATA** |
|------------|----------|
| 10         | Variable |

El header tiene el siguiente formato:

| **VER** | **TYPE** | **CMD** | **ID** | **TOKEN** |
|---------|----------|---------|--------|-----------|
| 1       | 1        | 2       | 2      | 4         |

Procedemos a explicar los distintos campos:

- **VER**: Versión del protocolo en uso.
- **TYPE**: Tipo de mensaje: 0x00 para una consulta, 0x01 para modificar.
- **CMD**: Comando a ejecutar.
- **ID**: Identificador del mensaje, debe coincidir con el de la respuesta.
- **TOKEN**: Token de autenticación.
- **DATA**: Datos del mensaje: Usado para los argumentos necesarios por el comando usado, puede estar vació, estar formado por enteros sin signo de 8, 16 o 32 bits, o por strings null-terminated. Si el comando no necesita argumentos, este campo será ignorado.

Los valores válidos para CMD con ***TYPE*** 0x00 son:
  - **0x00**: Obtener lista de usuarios.
  - **0x01**: Obtener la cantidad de conexiones históricas al servidor.
  - **0x02**: Obtener la cantidad de conexiones actuales al servidor.
  - **0x03**: Obtener la cantidad de bytes enviados.
  - **0x04**: Obtener el tamaño de página actual del listado de usuarios.

Los valores válidos para CMD con ***TYPE*** 0x01 son:
  - **0x00**: Detener el servidor.
  - **0x01**: Agregar un usuario.
  - **0x02**: Borrar un usuario.
  - **0x03**: Cambiar el tamaño de página del listado de usuarios, debe estar entre 1 y 200.

Al pedir un listado de usuarios, la página por defecto contiene hasta 200 usuarios. En el caso de que todos estos sean usados, la longitud máxima del argumento de un comando POP3 es 40 caracteres, y dado que el nombre y la contraseña de un usuario son enviados por comando estos también deben seguir esta restricción. Así, el tamaño de esta página sería de 200 * (40 + 1) = 8200 bytes. Esto se nota que es muy por debajo del tamaño máximo de datos soportados por un datagrama UDP, que es de 64 KB.

### Operaciones para pedidos al servidor

| **Comando**                             | **TYPE** | **CMD** | **DATA**                                |
|-----------------------------------------|----------|---------|-----------------------------------------|
| Ver página de lista de usuarios         | 0x00     | 0x00    | uint8 de la pagina solicitada           |
| Ver cantidad de conexiones históricas   | 0x00     | 0x01    | -                                       |
| Ver cantidad de conexiones concurrentes | 0x00     | 0x02    | -                                       |
| Obtener la cantidad de bytes enviados   | 0x00     | 0x03    | -                                       |
| Ver tamaño de pagina de usuarios        | 0x00     | 0x04    | -                                       |
| Detener el servidor                     | 0x01     | 0x00    | -                                       |
| Agregar un usuario                      | 0x01     | 0x01    | "nombre:contraseña"                     |
| Borrar un usuario                       | 0x01     | 0x02    | "nombre"                                |
| Cambiar tamaño de pagina de usuarios    | 0x01     | 0x03    | uint8<br/> 1 - mínimo<br/> 200 - máximo |

### Respuestas del servidor

Los datagramas de respuesta del servidor tienen la siguiente estructura:

| **VER** | **STATUS** | **TYPE** | **CMD** | **ID** | **DATA** |
|---------|------------|----------|---------|--------|----------|
| 1       | 1          | 1        | 2       | 2      | VARIABLE |

El valor VER debe ser el mismo explicitado en la sección de datagrama de pedido, a menos que la version no sea la soportada por el servidor, en cuyo caso debe responder con la version que soporta y el código de ***STATUS*** correcto para versión inválida.

Valores posibles de ***STATUS***:

- 0x00: OK
- 0x01: Versión del pedido inválida.
- 0x02: Token inválido.
- 0x03: ***TYPE*** inválido en el pedido
- 0x04: ***CMD*** inválido en el pedido
- 0x05: Argumento inválido en el pedido
- 0x06: No se aceptan nuevos usuarios
- 0x07: El usuario ya se encuentra registrado
- 0x08: El usuario a eliminar no se encontró
- 0x09: Fallo general en el servidor

Los valores ***TYPE*** y ***CMD*** deben coincidir con los valores indicados en el pedido. El valor ***ID*** debe coincidir con el valor indicado en el pedido. El valor DATA se utiliza para enviar la información adicional de respuesta, de ser necesaria. En el caso de no necesitar enviar información adicional al ***STATUS***, se deja vacío.

### Operaciones para respuestas del servidor

| **Comando**                             | **TYPE** | **CMD** | **DATA**                                                  |
|-----------------------------------------|----------|---------|-----------------------------------------------------------|
| Ver lista de usuarios                   | 0x00     | 0x00    | Secuencia de strings separados por '\n' terminada en '\0' |
| Ver cantidad de conexiones históricas   | 0x00     | 0x01    | Uint64                                                    |
| Ver cantidad de conexiones concurrentes | 0x00     | 0x02    | Uint16                                                    |
| Ver cantidad de bytes transferidos      | 0x00     | 0x03    | Uint64                                                    |
| Ver tamaño de pagina de usuarios        | 0x00     | 0x04    |                                                           |
| Detener el servidor                     | 0x01     | 0x00    | -                                                         |
| Agregar un usuario                      | 0x01     | 0x01    | -                                                         |
| Borrar un usuario                       | 0x01     | 0x02    | -                                                         |
| Cambiar tamaño de pagina de usuarios    | 0x01     | 0x03    | -                                                         |


