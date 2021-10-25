#include <WiFi.h>
#include <Servo.h>

Servo myservo;  // создаем экземпляр класса «Servo»,
                // чтобы с его помощью управлять сервоприводом;
                // большинство плат позволяют
                // создать 12 объектов класса «Servo»

// GPIO-контакт, к которому подключен сервопривод:
static const int servoPin = 13;

// вставьте здесь учетные данные своей сети:
const char* ssid     = "Mi";
const char* password = "panteon2";

// создаем веб-сервер на порте «80»:
WiFiServer server(80);

// переменная для хранения HTTP-запроса:
String header;

// несколько переменных для расшифровки значения в HTTP-запросе GET:
String valueString = String(5);
int pos1 = 0;
int pos2 = 0;

TaskHandle_t scheduleTask;
TaskHandle_t webServerTask;

volatile int workMode = 0; // 0 = по слайдеру, 1 = по распсианию

void setup() {
  setupServo();
  connectToWifi();
  startWebServerTask();
  startScheduleTask();
}

void setupServo() {
  Serial.begin(115200);
  myservo.attach(servoPin);  // привязываем сервопривод,
                             // подключенный к контакту «servoPin»,
                             // к объекту «myservo»
}

void connectToWifi() {
  // подключаемся к WiFi при помощи заданных выше SSID и пароля: 
  Serial.print("Connecting to ");  //  "Подключаемся к "
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  // печатаем локальный IP-адрес и запускаем веб-сервер:
  Serial.println("");
  Serial.println("WiFi connected.");  //  "WiFi подключен."
  Serial.println("IP address: ");     //  "IP-адрес: "
  Serial.println(WiFi.localIP());
  server.begin();
}

void startWebServerTask() {
    xTaskCreatePinnedToCore(
                  webServerWork,   /* Task function. */
                  "WebServer",     /* name of task. */
                  10000,       /* Stack size of task */
                  NULL,        /* parameter of the task */
                  10,           /* priority of the task */
                  &webServerTask,      /* Task handle to keep track of created task */
                  0);          /* pin task to core 0 */                  
  delay(500); 
}

void startScheduleTask() {
    xTaskCreatePinnedToCore(
                    scheduleWork,   /* Task function. */
                    "Schedule",     /* name of task. */
                    10000,       /* Stack size of task */
                    NULL,        /* parameter of the task */
                    10,           /* priority of the task */
                    &scheduleTask,      /* Task handle to keep track of created task */
                    1);          /* pin task to core 1 */
    delay(500);
}

void webServerWork(void * unused) {
  Serial.print("Web server running on core ");
  Serial.println(xPortGetCoreID());
  for(;;) {
  WiFiClient client = server.available();

  if (client) {                     // если подключился новый клиент,     
    Serial.println("New Client.");  // печатаем сообщение
                                    // «Новый клиент.»
                                    // в мониторе порта;
    String currentLine = "";        // создаем строку для хранения
                                    // входящих данных от клиента;
    while (client.connected()) {    // цикл while() будет работать
                                    // все то время, пока клиент
                                    // будет подключен к серверу;
      if (client.available()) {     // если у клиента есть данные,
                              
                          // которые можно прочесть, 
        char c = client.read();// считываем байт, а затем    
        Serial.write(c);            // печатаем его в мониторе порта
        header += c;
        if (c == '\n') {            // если этим байтом является
                                    // символ новой строки
          // если получили два символа новой строки подряд,
          // то это значит, что текущая строчка пуста;
          // это конец HTTP-запроса клиента,
          // а значит – пора отправлять ответ:
          if (currentLine.length() == 0) {
            // HTTP-заголовки всегда начинаются
            // с кода ответа (например, «HTTP/1.1 200 OK»)
            // и информации о типе контента
            // (чтобы клиент понимал, что получает);
            // в конце пишем пустую строчку:
            client.println("HTTP/1.1 200 OK");
            client.println("Content-type:text/html");
            client.println("Connection: close");
                       //  "Соединение: отключено"
            client.println();

            // показываем веб-страницу:
            client.println("<!DOCTYPE html><html>");
            client.println("<head><meta charset=\"utf-8\"> <meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">");
            client.println("<link rel=\"icon\" href=\"data:,\">");
            client.println("<style>html{min-height:100%;}body { text-align: center; font-family: \"Trebuchet MS\", Arial; margin-left:auto; margin-right:auto; background: rgb(0,234,234); background: -moz-linear-gradient(top, rgba(0,234,234,1) 0%, rgba(0,80,209,1) 29%, rgba(0,158,195,1) 100%); background: -webkit-linear-gradient(top, rgba(0,234,234,1) 0%,rgba(0,80,209,1) 29%,rgba(0,158,195,1) 100%); background: linear-gradient(to bottom, rgba(0,234,234,1) 0%,rgba(0,80,209,1) 29%,rgba(0,158,195,1) 100%); filter: progid:DXImageTransform.Microsoft.gradient( startColorstr=\'#00eaea\', endColorstr=\'#009ec3\',GradientType=0 );}");
            client.println(".slider { width: 300px; }");
            client.println(".disabled {"
                              "pointer-events: none;"
                              "opacity: 0.4;"
                          "}");
            client.println("</style>");
            client.println("<script src=\"https://ajax.googleapis.com/ajax/libs/jquery/3.3.1/jquery.min.js\"></script>");
            // веб-страница:
            client.println("</head><body><h1><u>МБОУ СОШ № 18</u></h1>");
            client.println("<h2><u>Большие вызовы</u></h2>");
            client.println("<h3>Автоматическое проветривание помещения</h3>");
            client.println("<p><i><strong><font color=\"#FFFFFF\" size=\"5\"> Умное окно</font></strong></i></p>");
                                        //  "Управление сервомотором
                                        //   с помощью платы ESP32"
            client.println("<div id=\"sliderBlock\">");
            client.println("<p><b><font size=\"3\">Угол открытия окна:</font></b><span id=\"servoPos\"></span></p>");          
            client.println("<input type=\"range\" min=\"0\" max=\"90\" class=\"slider\" id=\"servoSlider\" onchange=\"servo(this.value)\" value=\""+valueString+"\"/><br>");
            client.println("</div>");
            client.println("<input type=\"checkbox\" id=\"modeChecker\"/><label for=\"modeChecker\"><font size=\"4\"><i><b>По расписанию</b></i></font></label>");
            
            client.println("<br><br><br>");
            client.println("<table border=\"0\" width=\"400\" align=\"center\">");
            client.println("<caption><font size=\"4\"><i><b>Настроить расписание проветривания</b></i></font></caption>");
            client.println("<tr><th align=\"left\" width=\"30%\">День недели</th><th colspan=\"2\">Время проветривания</th></tr>");
            client.println("<tr><td align=\"left\" width=\"30%\">Понедельник</td><td align=\"right\"><label>Открыть:</label><input type=\"time\"><br><label>Закрыть:</label><input type=\"time\"></td><td align=\"center\"><label><font size=\"6\" color=\"#D3D3D3\">+</font></label></td></tr>");
            client.println("<tr><td align=\"left\" width=\"30%\">Вторник</td><td align=\"right\"><label>Открыть:</label><input type=\"time\"><br><label>Закрыть:</label><input type=\"time\"></td><td align=\"center\"><label><font size=\"6\" color=\"#D3D3D3\">+</font></label></td></tr>");
            client.println("<tr><td align=\"left\" width=\"30%\">Среда</td><td align=\"right\"><label>Открыть:</label><input type=\"time\"><br><label>Закрыть:</label><input type=\"time\"></td><td align=\"center\"><label><font size=\"6\" color=\"#D3D3D3\">+</font></label></td></tr>");
            client.println("<tr><td align=\"left\" width=\"30%\">Четверг</td><td align=\"right\"><label>Открыть:</label><input type=\"time\"><br><label>Закрыть:</label><input type=\"time\"></td><td align=\"center\"><label><font size=\"6\" color=\"#D3D3D3\">+</font></label></td></tr>");
            client.println("<tr><td align=\"left\" width=\"30%\">Пятница</td><td align=\"right\"><label>Открыть:</label><input type=\"time\"><br><label>Закрыть:</label><input type=\"time\"></td><td align=\"center\"><label><font size=\"6\" color=\"#D3D3D3\">+</font></label></td></tr>");
            client.println("<tr><td align=\"left\" width=\"30%\">Суббота</td><td align=\"right\"><label>Открыть:</label><input type=\"time\"><br><label>Закрыть:</label><input type=\"time\"></td><td align=\"center\"><label><font size=\"6\" color=\"#D3D3D3\">+</font></label></td></tr>");
            client.println("<tr><td align=\"left\" width=\"30%\">Воскресенье</td><td align=\"right\"><label>Открыть:</label><input type=\"time\"><br><label>Закрыть:</label><input type=\"time\"></td><td align=\"center\"><label><font size=\"6\" color=\"#D3D3D3\">+</font></label></td></tr>");
            client.println("</table>");
            client.println("<script>var slider = document.getElementById(\"servoSlider\");");
            client.println("var servoP = document.getElementById(\"servoPos\"); servoP.innerHTML = slider.value;");
            client.println("var modeChecker = document.getElementById(\"modeChecker\");");
            client.println(" var sliderBlock = document.getElementById(\"sliderBlock\");");
            client.println("modeChecker.addEventListener('change', (event) => {"
                            "if (event.currentTarget.checked) {"
                            "  sliderBlock.className += \" disabled\";"
                            "} else {"
                            "  sliderBlock.className = \"\";"
                            "}"
                            "var mode = event.currentTarget.checked ? 1 : 0;"
                            "$.get(\"/?mode=\" + mode + \"&\");"
                            "{Connection: close};"
                            "});");
            client.println("slider.oninput = function() { slider.value = this.value; servoP.innerHTML = this.value; }");
            client.println("$.ajaxSetup({timeout:1000}); function servo(pos) { ");
            client.println("$.get(\"/?value=\" + pos + \"&\"); {Connection: close};}");

            client.println("</script>");
            client.println("</body></html>");     
            
            //GET /?value=180& HTTP/1.1
            if(header.indexOf("GET /?value=")>=0) {
              pos1 = header.indexOf('=');
              pos2 = header.indexOf('&');
              valueString = header.substring(pos1+1, pos2);
              
              // вращаем ось сервомотора:
              myservo.write(valueString.toInt());
              Serial.println(valueString); 
            }
            if (header.indexOf("GET /?mode=")>=0) {
              pos1 = header.indexOf('=');
              pos2 = header.indexOf('&');
              valueString = header.substring(pos1+1, pos2);
              
              workMode = valueString.toInt();
              Serial.println(valueString);       
            }
            // конец HTTP-ответа задается 
            // с помощью дополнительной пустой строки:
            client.println();
            // выходим из цикла while(): 
            break;
          } else { // если получили символ новой строки,
                   // очищаем текущую строку «currentLine»:
            currentLine = "";
          }
        } else if (c != '\r') {  // если получили любые данные,
                                 // кроме символа возврата каретки,
          currentLine += c;      // добавляем эти данные 
                                 // в конец строки «currentLine»
        }
      }
    }
    // очищаем переменную «header»:
    header = "";
    // отключаем соединение:
    client.stop();
    Serial.println("Client disconnected.");
               //  "Клиент отключился."
    Serial.println("");
  }
  delay(20);
  }
  vTaskDelete(NULL);
}

void scheduleWork(void * unused) {
  Serial.print("Schedule running on core ");
  Serial.println(xPortGetCoreID());
  
  for(;;) {
    if (workMode == 0) {
      delay(20);
    } else {
      runBySchedule();
    }
  }
  vTaskDelete(NULL);
}

void runBySchedule() {
      myservo.write(90);
      delay(2000);
      myservo.write(0);
      delay(4000);
}

void loop(){
  vTaskDelete(NULL);
}
