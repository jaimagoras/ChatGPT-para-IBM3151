#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

const char* ssid = "your_SSID";                                     // Cambia esto con tu WiFi
const char* password = "your_password";

const char* apiKey = "your_API_key";
const char* apiUrl = "https://api.openai.com/v1/chat/completions";
const String systemprompt = "Eres un asistente útil. No utilices emojis. Cuando te pida que cambies de modelo, responde exclusivamente @ + el texto del modelo al que quiero que cambies. Cuando te pida que cambies de temperatura, responde exclusivamente # + la temperatura. Considera que tienes memoria y que lo que hemos hablado antes es el contenido de este historial:\n ";
String pregunta = "Hola, ChatGPT";                                          // Almacena la pregunta recibida
String historial = " ";                                                     // historial de preguntas y respuestas
long int tokens;                                                            // tokens consumidos
String model="gpt-4o-mini";                                                 // modelo
float temperature = 0.7 ;                                                   // temperatura del modelo
 
void setup() 
    {
    Serial.begin(115200);                                                   // Puerto serie principal para depuración
    Serial2.begin(9600, SERIAL_8N1, 16, 17); // Puerto Serial2 a 9600 bps (RX=16, TX=17) // Puerto serie secundario para comunicarse con el IBM
    IBM3151_init();                                                         // inicializamos el IBM3151
    Serial2.print("\033 qA");                                               // pasamos al viewport 1
    Serial.print("\r\nConectando a WiFi");
    Serial2.print("\r\nConectando a WiFi");
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED)                                   // conectamos al WiFi
        {
        delay(500);
        Serial.print(".");
        Serial2.print(".");
        }
    Serial.print("\rConectado a IP " );
    Serial2.print("\rConectado a IP ");
    Serial2.println(WiFi.localIP());                                        // Muestra la IP en Serial2
    Serial.println(WiFi.localIP());                                         // Muestra la IP en Serial
    Serial2.print("modelo: "+ model );                                      // Muestra el modelo
    Serial2.printf("\rtemperatura: %*f",1,temperature );                    // Muestra la temperatura
    for(int i=1;i<3;i++) Serial2.print("\r");                               // nos vamos al fondo
    String respuesta = obtenerRespuesta(pregunta, historial);
    imprimirConPausa(respuesta);                                            // rutina de imprimir en pantalla
    Serial2.print("\033 qB");                                               // pasamos al viewport 2
    Serial.println("Pregunta: \r\n");
    }                                                                       // fin setup

void loop()  
    {
    pregunta="";                                                            // reseteamos la pregunta 
    delay(200);
    pregunta=recibirMensaje();
    if (pregunta!="")                                                       // si hay pregunta
      {
         Serial2.print("\033L");                                            // borramos la página del viewport 2
         Serial2.print("Procesando");
         Serial2.print("\033:");                                            // bloqueo de teclado
         Serial2.print("\033 qA");                                          // pasamos al viewport 1
         Serial2.print("\r\n"+ pregunta);
         pregunta = iso8859ToUtf8(pregunta);                                // convertimos la pregunta de ISO a UTF8
         historial = historial + pregunta + "\n";                           // añadimos la pregunta al historial despues de pasarla a UTF
         String respuesta = obtenerRespuesta(pregunta, historial);
         if (char(respuesta[0]) == 64)                                      // si empieza por una @, cambio de modo
             { 
             model="";
             for (int i = 1; i < respuesta.length(); i++) model+=respuesta[i];// extraemos el modelo
             Serial2.printf("\rCambio de modelo a: %s \r" ,model);
             }
         else if (char(respuesta[0]) == 35)                                 // si empieza por una #, cambio de temperatura
             {
             String Tstring = "";
             for (int i = 1; i < respuesta.length(); i++) Tstring+=respuesta[i];// extraemos la temperatura
             temperature = Tstring.toFloat();                               // convertimos la temperatura en float
             Serial2.printf("\rCambio de temperatura a: %*f \r",1 ,temperature);
             }
         else 
            {
            historial = historial + "ChatGPT responde: " + respuesta + "\r\n";// añadimos la respuesta al historial antes de pasarla a ISO
            Serial2.print("\r\nRespuesta de ChatGPT: \r");
            imprimirConPausa(respuesta);                                    // rutina de imprimir en pantalla
            }
         Serial2.print("\033 qB");                                          // pasamos al viewport 2
         Serial2.print("\033L");                                            // borramos página
         Serial2.print("Pregunta: "); 
         Serial2.print("\033;");                                            // desbloqueo de teclado      
      }  
    }                                                                       // fin loop

String recibirMensaje()                                                     // Función para leer del terminal del IBM3151
    {
    String Texto = "";
    while (Serial2.available()) 
      {
      delay(5);                                                             // para darle tiempo 
      char c = Serial2.read();
      if ((int)c == 4)                                                      // Si se recibe el final del mensaje (código ASCII 04), procesa la pregunta
         {
          Texto.trim();                                                     // Elimina espacios en blanco y saltos de línea
          return Texto;
         } 
      else 
            {
              if ((int)c > 31||(int)c == 13) Texto += c;                    // eliminamos caracteres de control menos el retrono de linea
            }
      }
      return "";
    }

String obtenerRespuesta(String prompt, String historico)                    // Función para obtener respuesta de ChatGPT
    {
    WiFiClientSecure client;
    client.setInsecure();                                                   // Desactiva la verificación SSL (usar solo si es necesario)
    delay (200);
    HTTPClient http;                                                        // Conectamos con el servidor y le mandamos la llave
    http.begin(client, apiUrl);
    http.addHeader("Content-Type", "application/json");
    http.addHeader("Authorization", String("Bearer ") + apiKey);
    delay(200);
    DynamicJsonDocument doc(4096);                                          // Pregunta en JSON con configuración
    doc.clear();
    doc["model"] = model;
    doc["temperature"] = temperature;
    JsonArray messages = doc.createNestedArray("messages");
    JsonObject systemMessage = messages.createNestedObject();               // array de prompt de sistema
      systemMessage["role"] = "system";
      systemMessage["content"] = systemprompt + historico;
    JsonObject userMessage = messages.createNestedObject();                 // array de prompt de usuario
      userMessage["role"] = "user";
      userMessage["content"] = prompt;
    String requestBody;
    delay(200);
    serializeJson(doc, requestBody);                                        // serializamos al pregunta
    int httpResponseCode = http.POST(requestBody);                          // lanzamos la pregunta
    String response = "{}";                                                 // respuesta por defecto
    if (httpResponseCode > 0) response = http.getString();                  // recibimos la respuesta
    if (httpResponseCode != 200)                                            // si hay error
      {
      Serial2.println (response);
      }
    http.end();
    DynamicJsonDocument responseDoc(4096);                                  // configuramos la respuesta
    DeserializationError error = deserializeJson(responseDoc, response);    // si hay error
    if (error) 
        {
        Serial.println("\nError al analizar JSON");
        Serial2.println("\nError al analizar JSON");
        delay(500);
        return response;
        }
    else                                                                    // si no hay error
      {  
      String modelo = "modelo: "+ responseDoc["model"].as<String>();        // recuperamos la información del JSON
      tokens = tokens + responseDoc["usage"]["total_tokens"].as<long int>();
      linea_status(modelo,tokens);
      return responseDoc["choices"][0]["message"]["content"].as<String>();  // devolvemos la respuesta
      }
    }                                                                       // fin función obtenerRespuesta

void imprimirConPausa(String respuesta)                                     // Función para imprimir la respuesta con pausa cada 20 líneas 
    {
    int lineCount = 0;
    int charCount = 0;
    String respuestaiso;
    respuestaiso = utf8ToIso8859(respuesta);                                // creamos variable con la respuesta en ISO para mostrarla por pantalla
    for (int i = 0; i < respuesta.length(); i++) 
        {
        char c = respuesta[i];
        char ciso=respuestaiso[i];
        charCount++;
        if (Serial.availableForWrite() == 0)                                // si el terminal está muy ocupado, esperar antes de seguir imprimiendo
            {
            delay(10);  
            }
        if (ciso == '\n')                                                   // cambia \n por  \r, si no  los cambios de linea salen mal
            {
            lineCount++;
            Serial2.print("\r");
            }
        else  Serial2.print(ciso);
        Serial.print(c);
        if (charCount >= 130)                                               // si llega al final de una linea
              {
              lineCount++;
              charCount = 0;
              }
         if (lineCount >= 20)                                               // si hemos llegado al final de la página
                  {
                  Serial2.print("\033 qB");                                 // cambiamos el Vireport 2
                  delay (100);
                  Serial2.print("\033L");                                   // borra la pantalla (viewport 2)
                  Serial2.print("\033 qA");                                 // pasamos al viewport 1
                  //Serial2.print("\033!qA");                               // pasamos al viewport 1 y activamos el cursor
                  Serial.print("\r\nPresiona cualquier tecla para continuar...");
                  Serial2.print("\rPresiona enviar para continuar...");
                  Serial2.print("\033;");                                   // desbloqueo de teclado   
                  //while (Serial.available()) Serial.read();
                  while (Serial2.available()) Serial2.read();
                  while (!Serial.available() && !Serial2.available());
                  //while (Serial.available()) Serial.read();
                  while (Serial2.available()) Serial2.read();
                  Serial2.print("\033:"); // bloqueo de teclado    
                  lineCount = 0;
                  Serial2.print("\033L"); //borra la pantalla (viewport 2)
                  }
        }                                                                   
     }                                                                      // fin función imprimirConPausa

void IBM3151_init()                                                         // inicializar el terminal
    {
    delay(500);
    //setting control bits    
    Serial2.print("\033 9");                                                // control 1
    Serial2.write(0b01000010);                                              // modo block  
    Serial2.print("\033!9");                                                // control 2
    Serial2.write(0b00110001);                                              // inserta linea, enter return, crt saver
    Serial2.write(0b01011100);                                              // inserta caracter, no transparente, CRLF
    Serial2.print("\033\"9");                                               // control 3
    Serial2.write(0b00110111);                                              // jump scroll, crlf, auto LF, nueva linea
    Serial2.write(0b01010001);                                              // jump scroll, insert mode, tab campo, retorno nueva linea
    Serial2.print("\033#9");                                                // control 4
    Serial2.write(0b01000110);                                              // Send key works as Send key, send null, bloq teclado MBT, text LTA
    Serial2.print("\033$9");                                                // control 5
    Serial2.write(0b00101110);                                              // 1 stop bit, 9600bps
    Serial2.write(0b00111000);                                              // 8 bit, sin paridad
    Serial2.write(0b00110010);                                              // 100ms delay, line speed not extended?, line control IPTRS
    Serial2.write(0b01011010);                                              // break 170ms,retorno EOT 
    //control 6 puerto auxiliar, no lo configuro
    //control 7 impresion, no lo configuro
    Serial2.print("\033 r\"! 6$$\" #$D");                                   // 132 columnas 22 filas wiewport 1 3 filas viewport 2. 
    linea_status("modelo: ",0);
    Serial2.print("\033:");                                                 // bloqueo de teclado
    Serial2.print("\033L");                                                 // borra la pantalla
    Serial2.print("\033>A");                                                // seleccionamos caracteres graficos
    delay(2000);
    int c=0;  
    const String logo = "▀▀▀▀▀▀▀ ▀▀▀▀▀▀▀▀▀▀   ▀▀▀▀▀       ▀▀▀▀▀\n"  // contenido del logo
                  "▀▀▀▀▀▀▀ ▀▀▀▀▀▀▀▀▀▀▀▀ ▀▀▀▀▀▀     ▀▀▀▀▀▀\n"
                  "  ▀▀▀     ▀▀▀    ▀▀▀   ▀▀▀▀▀   ▀▀▀▀▀\n"
                  "  ▀▀▀     ▀▀▀▀▀▀▀▀     ▀▀▀▀▀▀ ▀▀▀▀▀▀\n"
                  "  ▀▀▀     ▀▀▀▀▀▀▀▀     ▀▀▀ ▀▀▀▀▀ ▀▀▀\n"
                  "  ▀▀▀     ▀▀▀    ▀▀▀   ▀▀▀  ▀▀▀  ▀▀▀\n"
                  "▀▀▀▀▀▀▀ ▀▀▀▀▀▀▀▀▀▀▀▀ ▀▀▀▀▀   ▀   ▀▀▀▀▀\n"
                  "▀▀▀▀▀▀▀ ▀▀▀▀▀▀▀▀▀▀   ▀▀▀▀▀       ▀▀▀▀▀";
    for (int i = 0; i < logo.length(); i++)                                 // dibujamos el logo
        {
        if (logo[i]==226) Serial2.write(243);                               // escribe "▀"
          else Serial2.write(logo[i]);
          delay(2);                                                         // para dar más sensación Oldschool
        }
    //Serial2.println();                                                      // nueva línea al final
    Serial2.print("\033>B");                                                // caracteres nacionales
    Serial2.print("\r(logo como homenaje, sin nexo con IBM)");
    Serial2.print("\033+:");                                                // crea la linea de separacion entre viewports
    Serial2.print("\033 qB");                                               // pasamos al viewport 2 
    Serial2.print("\033!qB");                                               // pasamos al viewport 2 y traspasa alli el cursor
    Serial2.print("\0338");                                                 // hacemos un dump para borrar el primer carácter
    delay(1000);
    while (Serial2.available())                                             // borramos buffer
      {
      char c = Serial2.read();
      delay(10);
      }
    Serial2.print("Pregunta: ");
    Serial2.print("\033;");                                                 // desbloqueo de teclado 
    }                                                                       // fin IBM3151_init
   
void linea_status(String texto,long int numero)                             // escribe linea de status abajo de la pantalla
   {
    Serial2.print("\033#:");                                                // datos de status
    Serial2.print("\033*:");                                                // linea de status
    Serial2.print("\033=");                                                 // texto de la linea de status
    Serial2.print(texto);                                                   // modelo
    for(int i=1;i<(56-texto.length());i++) Serial2.print(" ");              // centramos el mensaje
    Serial2.print("ChatGPT IBM3151 V1.0");
    for(int i=1;i<37;i++) Serial2.print(" ");                               // tokens al final
    Serial2.printf("Tokens: %i", numero);
    Serial2.print("\033=");                                                 // texto de la linea de status 
   }
   
String utf8ToIso8859(String utf8Str) {                                      // Convierte una cadena UTF-8 a ISO-8859-1
    String result = "";
    for (size_t i = 0; i < utf8Str.length(); i++) {
        uint8_t c = (uint8_t)utf8Str[i];
        if (c ==13) {                                                       // si hay enter
            result += 10;                                                   // retorno de carro
            result += 13;                                                   // añadimos enter
          } else if (c < 128) {
            result += (char)c;                                              // Caracter ASCII directo
          } else if ((c & 0xE0) == 0xC0 && i + 1 < utf8Str.length()) {      // Caracter de 2 bytes en UTF-8
            uint8_t c2 = (uint8_t)utf8Str[i + 1];
            if ((c2 & 0xC0) == 0x80) {
                char isoChar = (char)(((c & 0x1F) << 6) | (c2 & 0x3F));
                result += isoChar;
                i++;                                                        // Saltar el siguiente byte
           }
        }
    }
    return result;
}                                                                           // fin función utf8ToIso8859


String iso8859ToUtf8(String isoStr) {                                       // Convierte una cadena ISO-8859-1 a UTF-8
    String result = "";
    for (size_t i = 0; i < isoStr.length(); i++) {
        uint8_t c = (uint8_t)isoStr[i];
        if (c < 128) {
            result += (char)c;                                              // Caracter ASCII directo
        } else {
            result += (char)(0xC0 | (c >> 6));
            result += (char)(0x80 | (c & 0x3F));
        }
    }
    return result;
}                                                                           // fin función iso8859ToUtf8
