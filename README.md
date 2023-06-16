# TPE - Protocolos de Comunicación - POP3

Implementación de un servidor para el protocolo POP3 (Post Office Protocol versión 3) que pueda ser usado
por Mail User Agents tales como Mozilla Thunderbird, Microsoft Outlook y Evolution para la recepción de correos electrónicos.

## Requerimientos

* atender a múltiples clientes en forma concurrente y simultánea (al menos 500)
* soportar autenticación usuario / contraseña y pipelining

## Construcción y Ejecución

Para construir el proyecto, ejecute el siguiente comando en el directorio principal:

```bash
user@machine:~/TPE-Protos$ make all
```