El objetivo es programar un ESP32 en Arduino para poder utilizar un IBM 3151 como terminal para hacer preguntas a ChatGPT y obtener sus respuestas en texto.

El programa genera su propia memoria, con lo que se recuerda el contenido de cada sesión. Se pierde al reiniciarlo.

El modelo por defecto es GPT-4o-mini y temperatura 0.7, pero el programa permite elegir el modelo y la temperatura pidiéndiselo directamente a ChatGPT, lo que cambia la configuración para la próxima pregunta.

Se envían los comandos de configuración de pantalla del IBM3151 para tener dos viewports, uno con el cursor y otro con las respuestas. Presionar la tecla "enviar" envía a chatGPT la pregunta y muestra la respuesta en la parte superior, con scroll suave. En la ventana de status se muestran el modelo y los tokens consumidos 

Si la respuesta ocupa más del contenido de la pantalla, se pide presionar "enviar" para cada pantalla extra de texto.

Se puede modificar para otros terminales. Una primera versión arcaica (de pruebas) sirve para cualquier terminal serie, pero la definitiva se personalizó para aprovechar todos los recursos del IBM3151.

El programa realiza internamente la conversión de UTF-8 (ChatGPT) a ISO 8859 (IBM) para poderse usar con caracteres internacionales (mi terminal dispone del paquete español).

Espero que lo disfrutéis
