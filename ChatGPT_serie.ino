#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

const char* ssid = "your_SSID";       // Cambia esto con tu WiFi
const char* password = "your_password";

const char* apiKey = "your_API_key";
const char* apiUrl = "https://api.openai.com/v1/chat/completions";
String pregunta = "";  // Almacena la pregunta recibida
String historial="";
long int tokens;

void setup() 
    {
    Serial.begin(115200);  // Puerto serie principal para depuración
    Serial2.begin(9600, SERIAL_8N1, 16, 17); // Puerto Serial2 a 9600 bps (RX=16, TX=17)
    //comprobación de caracteres del display
    /*int c=0;  
    for(int i=1;i<255;i++){
      c++;
      Serial2.printf("%i,%c; ", i, c);
    }*/
    // inicializar el terminal
    //Serial2.print("\033 r!! 9$D"); //132 columnas 25 filas. 1 Viewport
    Serial2.print("\033 r\"! 6$$\" #$D"); //132 columnas 22 filas wiewport 1 3 filas viewport 2. 
    linea_status("modelo: haz una pregunta",0);
    Serial2.print("\033L"); //borra la pantalla
    //Serial.println("ChatGPT for IBM3151 V1.0. ");
    //delay(100);
    //Serial2.print(0x0E);
    Serial2.print("▀▀▀▀▀▀▀ ▀▀▀▀▀▀▀▀▀▀   ▀▀▀▀▀       ▀▀▀▀▀\r");
   // Serial2.print(0x0F);
    Serial2.print("▀▀▀▀▀▀▀ ▀▀▀▀▀▀▀▀▀▀▀▀ ▀▀▀▀▀▀     ▀▀▀▀▀▀\r");
    Serial2.print("  ▀▀▀     ▀▀▀    ▀▀▀   ▀▀▀▀▀   ▀▀▀▀▀\r");
    Serial2.print("  ▀▀▀     ▀▀▀▀▀▀▀▀     ▀▀▀▀▀▀ ▀▀▀▀▀▀\r");
    Serial2.print("  ▀▀▀     ▀▀▀▀▀▀▀▀     ▀▀▀ ▀▀▀▀▀ ▀▀▀\r");
    Serial2.print("  ▀▀▀     ▀▀▀    ▀▀▀   ▀▀▀  ▀▀▀  ▀▀▀\r");
    Serial2.print("▀▀▀▀▀▀▀ ▀▀▀▀▀▀▀▀▀▀▀▀ ▀▀▀▀▀   ▀   ▀▀▀▀▀\r");
    Serial2.print("▀▀▀▀▀▀▀ ▀▀▀▀▀▀▀▀▀▀   ▀▀▀▀▀       ▀▀▀▀▀\r");
    Serial2.print("(logo como homenaje)");
    Serial.print("\r\nConectando a WiFi");
    Serial2.print("\r\nConectando a WiFi");
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) 
        {
        delay(500);
        Serial.print(".");
        Serial2.print(".");
        }
    Serial.print("\rConectado a IP " );
    Serial2.print("\rConectado a IP ");
    Serial.print(WiFi.localIP());  // Muestra la IP en el puerto serie
    Serial2.print(WiFi.localIP()); // Muestra la IP en Serial2
    Serial.print("\r\nEscriba una pregunta y presione Enter:");
    for(int i=1;i<4;i++) Serial2.println("\r");//nos vamos al fondo
    Serial2.print("Escriba una pregunta y presione Enter:");
    Serial2.print("\033+:"); //crea la linea de separacion entre viewports
    Serial2.print("\033!qB"); //pasamos al viewport 2 y traspasa alli el cursor
    Serial2.print("\033 qB"); //pasamos al viewport 2
    Serial.println("Pregunta: \r\n");
    Serial2.print("Pregunta: \r");
    //Serial2.print("\033 Z"); //pone en modo cursor
    //Serial2.print("\033Z"); //inserta el cursor en el viewport 2
    //Serial2.print("\033H"); //manda el cursor al origen
    } //fin setup

void loop()  
    {
    recibirPreguntaSerial2();  // Escucha datos del puerto serie 2
    }

// Función para recibir la pregunta desde Serial2
void recibirPreguntaSerial2() 
    {
    Serial2.print("\033 qB"); //pasamos al viewport 2
    while (Serial2.available()) 
        {
        char c = Serial2.read();  // Leer carácter recibido
        Serial.print(c);
        Serial2.print(c);
        
        if (c == 13) {  // Si el usuario presiona Enter (código ASCII 13), procesa la pregunta
            pregunta.trim();  // Elimina espacios en blanco y saltos de línea
            if (pregunta.length() > 0) {
                Serial2.print("\0338"); //descarga toda la pagina
                
                Serial2.print("\033 qA"); //pasamos al viewport 1
                Serial2.print("\rPregunta:" + pregunta);
                pregunta = iso8859ToUtf8(pregunta);   //la convertimos de ISO a UTF8
                historial = historial + "\nPregunta: " + pregunta + "\n";    //añadimos la pregunta al historial despues de pasarla a UTF
                Serial2.print("\033 qB"); //pasamos al viewport 2 
                Serial2.print("\033L"); //borra la pantalla (viewport 2)
                Serial.print("Pregunta: ");
                Serial2.print("\rPregunta: \r\n");
             
                //String respuesta = obtenerRespuesta(pregunta);
                String respuesta = obtenerRespuesta(historial);
                historial = historial + "ChatGPT responde: " + respuesta + "\n";    //añadimos la respuesta al historial antes de pasarla a ISO
               //Serial2.print("\033I"); //borra toda la linea
                Serial2.print("\033 qA"); //pasamos al viewport 1
                Serial.println("\r\nChatGPT:");
                Serial2.print("\r\nChatGPT responde:\r");
                imprimirConPausa(respuesta);
                pregunta = "";  // Reiniciar para la siguiente entrada
                //Serial2.print("\033Z"); //inserta cursor
                //Serial2.print("\033!qB"); //salta de particioninserta cursor
                //Serial2.print("\033M"); //mueve el cursor a la siguiente linea
                //Serial2.print("\033M"); //mueve el cursor a la siguiente linea
            }
        } 
        else if (c == 8 || c == 127) {  // Manejar retroceso (ASCII 8 y 127)
            if (pregunta.length() > 0) {
                pregunta.remove(pregunta.length() - 1);
                Serial.print("\b"); // Borrar un carácter en terminal
            }
        
        } 
        else {
            pregunta += c;  // Acumular caracteres en la pregunta
        }
        
    }
}

// Función para obtener respuesta de ChatGPT
String obtenerRespuesta(String prompt) 
    {
    const char* chatGPTResponse = "";
    WiFiClientSecure client;
    client.setInsecure(); // Desactiva la verificación SSL (usar solo si es necesario)
    HTTPClient http;
    http.begin(client, apiUrl);
    http.addHeader("Content-Type", "application/json");
    http.addHeader("Authorization", String("Bearer ") + apiKey);
    Serial.print(pregunta);
    DynamicJsonDocument doc(4096);  //configuramos la pregunta
    doc["model"] = "gpt-4o-mini";  // 
    doc["messages"][0]["role"] = "system";
    doc["messages"][0]["content"] = "Eres un asistente útil. No utilices emojis. Considera que tienes memoria y que lo que hemos hablado antes es el contenido de este historial\n" +historial;
    doc["messages"][1]["role"] = "user";
    doc["messages"][1]["content"] = pregunta;
    doc["temperature"] = 0.7;
    String requestBody;
    serializeJson(doc, requestBody);
    int httpResponseCode = http.POST(requestBody);    //lanzamos la pregunta
    String response = "{}"; // Respuesta por defecto
    if (httpResponseCode > 0) response = http.getString(); //recibimos la respuesta
    http.end();
    DynamicJsonDocument responseDoc(4096);    //configuramos la respuesta
    DeserializationError error = deserializeJson(responseDoc, response);
    if (error) 
        {
        Serial.println("\nError al analizar JSON");
        Serial2.println("\nError al analizar JSON");
        delay(1000);
        
        if (responseDoc.containsKey("message")) response = responseDoc["message"].as<String>();
        else return response;
        }
    else       //si no hay error
      {  
       String modelo = "modelo: "+ responseDoc["model"].as<String>();
       tokens = tokens + responseDoc["usage"]["total_tokens"].as<long int>();
      linea_status(modelo,tokens);
      //responseDoc["choices"][0]["message"]["content"].as<String>()
      //return response;    //respuesta sin procesar
      return responseDoc["choices"][0]["message"]["content"].as<String>();
      }
}

// Función para imprimir la respuesta con pausa cada 20 líneas 
void imprimirConPausa(String respuesta) 
    {
    int lineCount = 0;
    int charCount = 0;
    String respuestaiso;
    respuestaiso = utf8ToIso8859(respuesta);
    for (int i = 0; i < respuesta.length(); i++) 
        {
        char c = respuesta[i];
        char ciso=respuestaiso[i];
        charCount++;
        // Si el terminal está muy ocupado, esperar antes de seguir imprimiendo
        if (Serial.availableForWrite() == 0) 
            {
            delay(10);  // Espera a que haya espacio en el buffer
            }
        if (ciso == '\n') //cambia \n por  \r, si no  los cambios de linea salen mal
            {
            lineCount++;
            Serial2.print("\r");
            }
        else  Serial2.print(ciso);
        Serial.print(c);
        if (charCount >= 130) //si llega al final de una linea
              {
              lineCount++;
              charCount = 0;
              }
         if (lineCount >= 18) //si hemos llegado al final de la página
                  {
                  Serial2.print("\033!qA"); //pasamos al viewport 1 y activamos el cursor
                  Serial.print("\r\nPresiona cualquier tecla para continuar...");
                  Serial2.print("\r\nPresiona cualquier tecla para continuar...");
                  while (Serial.available()) Serial.read();
                  while (Serial2.available()) Serial2.read();
                  while (!Serial.available() && !Serial2.available());
                  while (Serial.available()) Serial.read();
                  while (Serial2.available()) Serial2.read();
                  lineCount = 0;
                  Serial2.print("\033!qB"); //pasamos al viewport 2 y activamos el cursor
                  }
        } //fin for
     }  //fin función

void linea_status(String texto,long int numero)
   {
      //model, usage/total_tokens
    Serial2.print("\033#:"); //datos de status
    Serial2.print("\033*:"); //linea de status
    Serial2.print("\033="); //texto de la linea de status
    Serial2.print(texto);
    for(int i=1;i<(56-texto.length());i++) Serial2.print(" ");
    Serial2.print("ChatGPT for IBM3151 V1.0");
    for(int i=1;i<35;i++) Serial2.print(" ");
    Serial2.printf("Tokens: %i", numero);
    Serial2.print("\033="); //texto de la linea de status 
   }

// Convierte una cadena UTF-8 a ISO-8859-1
String utf8ToIso8859(String utf8Str) {
    String result = "";
    for (size_t i = 0; i < utf8Str.length(); i++) {
        uint8_t c = (uint8_t)utf8Str[i];
        if (c ==13) { //si hay enter
            result += 10; // retorno de carro
            result += 13; // 
           
          } else if (c < 128) {
            result += (char)c; // Caracter ASCII directo
          } else if ((c & 0xE0) == 0xC0 && i + 1 < utf8Str.length()) { // Caracter de 2 bytes en UTF-8
            uint8_t c2 = (uint8_t)utf8Str[i + 1];
            if ((c2 & 0xC0) == 0x80) {
                char isoChar = (char)(((c & 0x1F) << 6) | (c2 & 0x3F));
                result += isoChar;
                i++; // Saltar el siguiente byte
           }
        }
    }
    return result;
}

// Convierte una cadena ISO-8859-1 a UTF-8
String iso8859ToUtf8(String isoStr) {
    String result = "";
    for (size_t i = 0; i < isoStr.length(); i++) {
        uint8_t c = (uint8_t)isoStr[i];
        if (c < 128) {
            result += (char)c; // Caracter ASCII directo
        } else {
            result += (char)(0xC0 | (c >> 6));
            result += (char)(0x80 | (c & 0x3F));
        }
    }
    return result;
}
